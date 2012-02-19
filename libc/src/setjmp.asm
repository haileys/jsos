global _setjmp
global _longjmp

section .data
; this is used to easily dump the registers before having to load up arguments into registers
setjmp_temp:
    .eax dd 0
    .ecx dd 0
    .edx dd 0
    .ebx dd 0
    
    .esp dd 0
    .ebp dd 0
    .esi dd 0
    .edi dd 0
    
    .eip dd 0    
    .eflags dd 0

section .text
_setjmp:
    ; save registers before doing anything else
    mov [setjmp_temp.eax], eax
    mov [setjmp_temp.ecx], ecx
    mov [setjmp_temp.edx], edx
    mov [setjmp_temp.ebx], ebx
    mov [setjmp_temp.esp], esp
    mov [setjmp_temp.ebp], ebp
    mov [setjmp_temp.esi], esi
    mov [setjmp_temp.edi], edi
    
    mov eax, [esp] ; return address
    mov [setjmp_temp.eip], eax
    
    pushfd
    pop eax
    mov [setjmp_temp.eflags], eax
    
    mov esi, setjmp_temp
    mov edi, [esp+4] ; pointer to struct __jmp_buf_tag
    mov ecx, 10
    rep movsd ; copy setjmp_temp into user provided pointer
    
    mov eax, 0 ; return 0 from direct invocation
    ret

_longjmp:    
    mov esi, [esp+4] ; pointer to struct __jmp_buf_tag
    mov edi, setjmp_temp
    mov ecx, 10
    rep movsd ; copy user provided pointer into setjmp_temp
    
    mov eax, [setjmp_temp.eflags]
    push eax
    popfd
    
    ; setup return address on new stack pointer
    mov eax, [setjmp_temp.eip]
    mov ebx, [setjmp_temp.esp]
    mov [ebx], eax
    
    ; setup return value properly
    mov eax, [esp+8] ; return value
    cmp eax, 0
    jne .dont_set_eax_to_one
    mov eax, 1 ; if return value was 0, set it to 1
    .dont_set_eax_to_one:
    
    ; restore registers
    mov edi, [setjmp_temp.edi]
    mov esi, [setjmp_temp.esi]
    mov ebp, [setjmp_temp.ebp]
    mov esp, [setjmp_temp.esp]
    mov ebx, [setjmp_temp.ebx]
    mov ecx, [setjmp_temp.ecx]
    mov edx, [setjmp_temp.edx]
    ; don't copy eax back because it's our return value
    
    ; i think we're done
    ret