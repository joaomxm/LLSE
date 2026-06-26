[bits 16]
global start                
extern kernel_main

start:
    ; Configurar os segmentos  e a pilha 
    xor ax, ax
    
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov sp, 0x7E00 ; Pilha cresce para baixo do Stage 2
    
    
    mov si, msg
    call print_msg

    call check_a20

    ; Inicio da Transicao para 32 BITS

    ; 1. Desabilitar interrupcoes mascaraveis (A BIOS nao vai mais funcionar)
    cli

    ; 2. Informar a CPU onde esta a estrutura GDT
    lgdt [gdt_descriptor]

    ; 3. Ativar o modo protegido no registrador de controle CR0 (setar o bit 0)
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; 4. Far jump (Salto Longo)
    ; Isso limpa a fila de execucao de 16 bits do processador e carrega o CS com 32 bits
    jmp CODE_SEG:init_32bit



check_a20:
    mov ax, 0x2401  ; Funcao para habilitar  A20
    int 0x15        ; Interrupcao
    jc .error_nao_suporta       ;Se o Carry Flag dor setado a BIOS nao suporta
    cmp ah, 0x0
    jne .error_ah     ;Se AH nao for 0, houve Erro


    call check_a20_status
    cmp ax, 1
    je .a20_ok

.a20_deu_ruim:
    mov si, msgA20fail
    call print_msg
    jmp $ ; Se falhou, não podemos continuar o boot

.a20_ok:
    mov si, msgA20Pass
    call print_msg
    ret ; Volta para o 'start'

.error_nao_suporta:
    mov si, msgErroNaoSuporta
    call print_msg
    jmp $

.error_ah:
    mov si, msgErroAh
    call print_msg
    jmp $



check_a20_status:
    pushf
    push ds
    push es
    push di
    push si

    cli ; Desabilita interrupções para o teste ser preciso

    ; Vamos comparar o endereço 0000:0500 com FFFF:0510
    ; (Fisicamente, 0xFFFF0 + 0x0510 = 0x100000)

    xor ax, ax
    mov es, ax
    mov di, 0x0500

    not ax ; AX vira 0xFFFF
    mov ds, ax
    mov si, 0x0510

    ; Salva os valores originais para não corromper a memória
    mov al, [es:di]
    push ax
    mov al, [ds:si]
    push ax

    ; Tenta escrever valores diferentes
    mov byte [es:di], 0x00
    mov byte [ds:si], 0xFF

    ; Se a A20 estiver desativada, [es:di] agora será 0xFF
    cmp byte [es:di], 0xFF

    pop ax
    mov [ds:si], al
    pop ax
    mov [es:di], al

    mov ax, 0
    je .done      ; Se for igual, A20 está desativada (retorna 0)
    mov ax, 1     ; Se for diferente, A20 está ativa (retorna 1)

.done:
    sti           ; Reabilita interrupções
    pop si
    pop di
    pop es
    pop ds
    popf
    ret


print_msg:
    mov ah, 0eh
    .loop:
        lodsb
        cmp al, 0
        je .done
        int 10h
        jmp .loop
    .done:
        ret


[bits 32]
init_32bit:
    ; Agora estamos em Modo Protegido!
    ; Precisamos atualizar todos os outros registradores de segmento com o DATA_SEG
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Atualizar a pilha para um local seguro de 32 bits
    mov ebp, 0x90000
    mov esp, ebp

    ; Escrever algo diretamente na tela para testar!
    ; Em 32 bits não temos 'int 10h', escrevemos direto na memória de vídeo (0xB8000)
    mov dword [0xb8000], 0x0a4b0a4f

    call kernel_main

    .freeze:
        hlt          ; Coloca a CPU em estado de "dormir" (Halt) até receber um sinal externo
        jmp .freeze  ; Se por algum motivo ela acordar, faz dormir de novo

msg                 db "Stage 2 carregado...", 13, 10, 0
msgErroNaoSuporta   db "Erro: BIOS nao suporta A20", 13, 10, 0
msgErroAh           db "Erro: BIOS AH status incorreto", 13, 10, 0
msgA20Pass          db "A20 habilitada e testada!", 13, 10, 0
msgA20fail          db "Erro Critico: Falha no teste de wrap-around A20", 13, 10, 0

align 4

; 1. Descritor Nulo: Obrigatorio pelo hardware (8 bytes para zero)
gdt_start:
    dd 0
    dd 0


; 2. Descritor de Código (Segmento de Codigo do Kernel)
gdt_code:
    dw 0xFFFF    ; Limite (bits 0-15) -> 64KB inicial
    dw 0x0000       ; Base (bits 0-15) -> Comeca em 0
    db 0x0       ; Base (bits 16-23)
    db 0x9A ; Bytes de acesso: Presente, Ring 0 (Kernel), Codigo, Executavel/Leitura
    db 0xCF ; Flags (Granularidade de 4KB, Modo 32-bits) + Limite (bits 16-19)
    db 0x00       ; Base (bits 24-31)

; 3. Descritor de dados (Segmento de Dados/Pilha do Kernel)

gdt_data:
    dw 0xFFFF    ; Limite (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00       ; Base (bits 16-23)
    db 0x92 ; Bytes de acesso: Presente, Ring 0 (kernel), Dados, Leitura/Escrita
    db 0xCF ; Flags + Limite (bits 16-19)
    db 0x00       ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Tamanho da tabela (16 bits)
    dd gdt_start               ; Endereco onde a tabela comeca (32 bits) 


;equ -> Ele funciona apenas como um "Localizar e Substituir", pedir ao NASM toda vez que você encontrar a palavra CODE_SEG no meu código, substitua-a pelo número 8 antes de gerar o arquivo final"
CODE_SEG equ gdt_code - gdt_start ; Geralmente resulta em 0x08
DATA_SEG equ gdt_data - gdt_start ; Geralmente resulta em 0x10
