nasm -f elf32 boot.asm -o kasm.o
C:\MinGW\bin\gcc -m32 -c kernel.c -o kc.o
C:\MinGW\bin\ld -T link.ld -o k kasm.o kc.o -build-id=none
C:\MinGW\bin\objcopy -O elf32-i386 k kernel-5
pause