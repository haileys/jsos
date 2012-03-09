global loader
global end_of_image
global kstack_max
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

STACKSIZE equ 1024*1024 ; 1 MiB

loader:
    mov esp, kstack_max+STACKSIZE
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

    resb 64*1024
    ; 64 KiB slack space. we can detect a looming stack overflow by comparing
    ; the address of a local variable against the address of kstack_max
kstack_max:
    resb STACKSIZE

section .end_of_image
end_of_image: