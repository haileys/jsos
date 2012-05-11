use32

global asm_isr_init
extern isr_dispatch
extern idt_register_handler
extern kprintf
extern panic

lol: db "interrupt %d", 10, 0

%define WITH_ERROR push eax

%macro isr 1-2 push dword 0
    
    jmp isr_%1.fin
    isr_%1:
        cli
        pusha
        %2
        push dword %1
        call isr_dispatch
        add esp, 8
        mov al, 0x20
        out 0xa0, al
        out 0x20, al
        popa
        sti
        iret
    .fin:
    push dword isr_%1
    push dword %1
    call idt_register_handler
    add esp, 8

%endmacro

asm_isr_init:

	isr 0
	isr 1
	isr 2
	isr 3
	isr 4
	isr 5
	isr 6
	isr 7
	isr 8, WITH_ERROR
	isr 9

	isr 10, WITH_ERROR
	isr 11, WITH_ERROR
	isr 12, WITH_ERROR
	isr 13, WITH_ERROR
	
;	isr 14, WITH_ERROR
jmp isr_14.fin
isr_14:
    pop eax
    pop ecx
    pop edx
    pop ebx
    push ecx
    push .msg
    call panic
    cli
    hlt
    .msg db "Page Fault", 0
.fin:
push isr_14
push 14
call idt_register_handler
add esp, 8

	isr 15
	isr 16
	isr 17, WITH_ERROR
	isr 18
	isr 19

	isr 20
	isr 21
	isr 22
	isr 23
	isr 24
	isr 25
	isr 26
	isr 27
	isr 28
	isr 29

	isr 30
	isr 31
	isr 32
	isr 33
	isr 34
	isr 35
	isr 36
	isr 37
	isr 38
	isr 39

	isr 40
	isr 41
	isr 42
	isr 43
	isr 44
	isr 45
	isr 46
	isr 47
	isr 48
	isr 49

	ret