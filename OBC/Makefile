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

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -c

OBJ = stage2.o \
      kernel.o \
      util.o \
      drivers/video.o \
      drivers/keyboard.o \
      memory/pmm.o \
      memory/paging.o \
      hardware/idt.o \
      hardware/timer.o

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
	$(CC) $(CFLAGS) $(KERNEL_SRC) -o kernel.o

util.o: util.c util.h
	$(CC) $(CFLAGS) util.c -o util.o

# Pasta Drivers
drivers/video.o: drivers/video.c drivers/video.h
	$(CC) $(CFLAGS) drivers/video.c -o drivers/video.o

drivers/keyboard.o: drivers/keyboard.c drivers/keyboard.h hardware/io.h
	$(CC) $(CFLAGS) drivers/keyboard.c -o drivers/keyboard.o

# Pasta Memory
memory/pmm.o: memory/pmm.c memory/pmm.h
	$(CC) $(CFLAGS) memory/pmm.c -o memory/pmm.o

memory/paging.o: memory/paging.c memory/paging.h
	$(CC) $(CFLAGS) memory/paging.c -o memory/paging.o

# Pasta Hardware
hardware/idt.o: hardware/idt.c hardware/idt.h hardware/io.h
	$(CC) $(CFLAGS) hardware/idt.c -o hardware/idt.o

hardware/timer.o: hardware/timer.c hardware/timer.h hardware/io.h
	$(CC) $(CFLAGS) hardware/timer.c -o hardware/timer.o

# Compila o ASM
$(STAGE2_BIN): $(OBJ)
	$(LD) -m elf_i386 -T linker.ld $(OBJ) -o $(STAGE2_BIN) --oformat binary

# Executa no QEMU
run: $(IMAGE)
	$(QEMU) -drive file=$(IMAGE),format=raw,index=0,if=ide -display curses -d cpu_reset -D qemu.log



# Limpeza: apaga os arquivos temporários
clean:
	rm -f $(BOOT_BIN) $(STAGE2_BIN) $(IMAGE) *.o drivers/*.o memory/*.o hardware/*.o qemu.log