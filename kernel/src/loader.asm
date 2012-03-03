global loader
global end_of_image
extern kmain

section .text
align 4
MultiBootHeader:
    FLAGS       equ	3
    MAGIC       equ	0x1BADB002
    CHECKSUM    equ	-(MAGIC + FLAGS)
    
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

STACKSIZE equ 0x10000 ; 64KiB

loader:
    mov esp, kstack+STACKSIZE
    push eax ; multiboot magic number
    push ebx ; pointer to multiboot struct
    
    fninit
    mov eax, cr0
    or eax, 1 << 5  ; FPU NE bit
    mov cr0, eax

    push 0
    jmp kmain
 
section .bss
align 4
kstack:
    resb STACKSIZE

section .end_of_image
end_of_image: