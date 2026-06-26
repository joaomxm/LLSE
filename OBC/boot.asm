BITS 16
ORG 0x7c00 ; Endereço na memoria onde esta sendo executado esse codigo


start:
    xor ax, ax
    mov ds, ax ; limpa o registrardor ds, essecial para o lodsb encontre a string msg
    mov ds, ax
    mov es, ax
    mov ss, ax
    
    mov si, msg
    call print_msg
    
    call go_stage_two


print_msg:
    mov ah, 0x0E
    .loop:
        lodsb
        cmp al, 0
        je .done
        int 10h
        jmp .loop
    .done:
        ret

go_stage_two:
    xor ax, ax
    mov es, ax
    mov bx, 0x7E00 ; Endereco de destino

    mov ah, 02h ; Na funcao de Leitura de setores do drive
    ;Parametros da Funcao 02h
    mov dl, 80h ; Selecione o primeiro Disco Rigido configurado
    mov al, 4 ; numero de Setores para ler
    mov ch, 0 ; Cilindro
    mov cl, 2 ; Numero do Setor -  A MBR é o setor 1
    mov dh, 0 ; Cabeca de leitura;
   

    int 13h ; Chamada da Interrupcao

    jc disk_error

    jmp 0x0000:0x7E00
    

disk_error:
    mov si, msg_error
    call print_msg
    hlt



msg db "Boot Boot!", 13, 10, 0

msg_error db "Disk Error", 13, 10, 0

TIMES 510-($-$$) db 0
DW 0XAA55
