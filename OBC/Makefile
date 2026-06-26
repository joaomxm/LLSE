# Nome dos arquivos
BOOT_SRC = boot.asm
STAGE2_SRC = stage2.asm

BOOT_BIN = boot.bin
STAGE2_BIN = stage2.bin
IMAGE    = boot.img

TMP_BIN = temp.bin

# Comandos
NASM    = nasm
QEMU    = qemu-system-i386

# Regra principal: compilar e rodar
all: run

# Gera a imagem de disquete (1440k)
$(IMAGE): $(BOOT_BIN) $(STAGE2_BIN)
	cat $(BOOT_BIN) $(STAGE2_BIN) > $(TMP_BIN)
	cp $(TMP_BIN) $(IMAGE)
	truncate -s 1440k $(IMAGE)


# Compila o ASM
$(BOOT_BIN): $(BOOT_SRC)
	$(NASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)

# Compila o ASM
$(STAGE2_BIN): $(STAGE2_SRC)
	$(NASM) -f bin $(STAGE2_SRC) -o $(STAGE2_BIN)

# Executa no QEMU
run: $(IMAGE)
	$(QEMU) -drive file=$(IMAGE),format=raw,index=0,if=ide -display curses -d cpu_reset -D qemu.log



# Limpeza: apaga os arquivos temporários
clean:
	rm -f $(BOOT_BIN) $(STAGE2_BIN) $(TMP_BIN) $(IMAGE)