global loader
global end_of_image
extern kmain

; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ	1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ	1<<1                   ; provide memory map
FLAGS       equ	MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ	0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ	-(MAGIC + FLAGS)        ; checksum required

section .text
align 4
MultiBootHeader:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
 
; reserve initial kernel stack space
STACKSIZE equ 0x10000                  ; that's 64k.
 
loader:
    mov esp, kstack+STACKSIZE           ; set up the stack
    push eax                           ; pass Multiboot magic number
    push ebx                           ; pass Multiboot info structure

    push 0
    jmp kmain                       ; call kernel proper
 
section .bss
align 4
kstack:
    resb STACKSIZE                     ; reserve 16k stack on a doubleword boundary

section .end_of_image
end_of_image: