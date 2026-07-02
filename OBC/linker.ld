ENTRY(start)
OUTPUT_FORMAT("binary")

SECTIONS
{
    /* O código começará exatamente em 0x7E00 */
    . = 0x7E00;

    /* Primeiro, coloca o código de boot do Stage 2 */
    .text : ALIGN(16){
        *(.text)
    }
    
    .rodata : ALIGN(16){
        *(.rodata)
    }

    /* Em seguida, as tabelas e dados */
    .data : ALIGN(16){
        *(.data)

    }

    /* Por fim, variáveis globais não inicializadas */
    .bss : ALIGN(16){
        *(COMMON)
        *(.bss)
    }
}