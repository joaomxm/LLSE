def calculate_crc16_ccitt_false(data: bytes) -> int:
    """Calcula o CRC16/CCITT-FALSE (Poly 0x1021, Init 0xFFFF)

    Abrange Sender ID, Receiver ID, Length e Payload.
    """
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def build_ipc_frame(sender_id: int, receiver_id: int, payload: bytes) -> bytes:
    """Monta o quadro de comunicação IPC.

    Estrutura: [SYNC1][SYNC2][SENDER][RECEIVER][LEN][PAYLOAD...][CRC_MSB][CRC_LSB]
    """
    sync_bytes = bytes([0xAA, 0x55])
    header_data = bytes([sender_id, receiver_id, len(payload)])

    data_to_crc = header_data + payload
    crc_val = calculate_crc16_ccitt_false(data_to_crc)

    # Extrai o MSB e LSB (Big-Endian para CCITT-FALSE)
    crc_msb = (crc_val >> 8) & 0xFF
    crc_lsb = crc_val & 0xFF

    frame = sync_bytes + data_to_crc + bytes([crc_msb, crc_lsb])
    return frame


def send_socket(
    host: str, port: int, sender: int, receiver: int, payload: bytes
):
    """Envia o pacote para uma porta TCP/Serial (como a porta 4555 do QEMU)."""
    import socket

    frame = build_ipc_frame(sender, receiver, payload)

    print("\n--- Quadro Gerado (QEMU Socket) ---")
    print(f"Payload: {payload}")
    print(f"Bytes enviados: {frame.hex(' ').upper()}")

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((host, port))
            s.sendall(frame)
            print(f"[+] Enviado com sucesso para {host}:{port}!")
    except Exception as e:
        print(f"[-] Erro de conexão socket {host}:{port}: {e}")


if __name__ == "__main__":
    # --- PARÂMETROS DO PACOTE DE TESTE ---
    SENDER_ID = 0x01
    RECEIVER_ID = 0x00
    PAYLOAD = b"help"

    send_socket("127.0.0.1", 4555, SENDER_ID, RECEIVER_ID, PAYLOAD)