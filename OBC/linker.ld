ENTRY(start)
OUTPUT_FORMAT("binary")

SECTIONS
{
    /* O código começará exatamente em 0x7E00 */
    . = 0x7E00;

    /* Primeiro, coloca o código de boot do Stage 2 */
    .text : {
        *(.text)
    }

    /* Em seguida, as tabelas e dados */
    .data : {
        *(.data)
        *(.rodata)
    }

    /* Por fim, variáveis globais não inicializadas */
    .bss : {
        *(.bss)
    }
}