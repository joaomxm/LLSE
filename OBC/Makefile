# Nome dos arquivos
BOOT_SRC = boot.asm
STAGE2_SRC = stage2.asm
KERNEL_SRC = kernel.c

BOOT_BIN = boot.bin
STAGE2_BIN = stage2.bin
IMAGE    = boot.img

TMP_BIN = temp.bin

# Comandos
NASM    = nasm
QEMU    = qemu-system-i386
CC 		= gcc
LD		= ld

# Regra principal: compilar e rodar
all: run

# Gera a imagem de disquete (1440k)
$(IMAGE): $(BOOT_BIN) $(STAGE2_BIN)
	dd if=/dev/zero of=$(IMAGE) bs=1024 count=1440
	dd if=$(BOOT_BIN) of=$(IMAGE) bs=512 count=1 conv=notrunc
	dd if=$(STAGE2_BIN) of=$(IMAGE) bs=512 seek=1 conv=notrunc


# Compila o ASM
$(BOOT_BIN): $(BOOT_SRC)
	$(NASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)

stage2.o: $(STAGE2_SRC)
	$(NASM) -f elf32 $(STAGE2_SRC) -o stage2.o

kernel.o: $(KERNEL_SRC)
	$(CC) -m32 -ffreestanding -fno-pie -fno-stack-protector -c $(KERNEL_SRC) -o kernel.o

# Compila o ASM
$(STAGE2_BIN): stage2.o kernel.o
	$(LD) -m elf_i386 -T linker.ld stage2.o kernel.o -o $(STAGE2_BIN)

# Executa no QEMU
run: $(IMAGE)
	$(QEMU) -drive file=$(IMAGE),format=raw,index=0,if=ide -display curses -d cpu_reset -D qemu.log



# Limpeza: apaga os arquivos temporários
clean:
	rm -f $(BOOT_BIN) $(STAGE2_BIN) $(IMAGE) *.o