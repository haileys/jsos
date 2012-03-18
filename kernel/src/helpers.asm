global gdt_reload_segment_registers
global paging_set_directory
global x86_64_support
global rdtsc

gdt_reload_segment_registers:
    jmp 0x08:.flush_cs
    .flush_cs:
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

rdtsc:
    rdtsc
    ret

paging_set_directory:
    ; page directory address:
    mov eax, [esp+4]
    mov cr3, eax
    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    ret

x86_64_support:
    push ebx
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    ; ok so extended mode is available, let's see if long mode is available:
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 9
    jz .no_long_mode
    mov eax, 1
    jmp .endif
    .no_long_mode:
    mov eax, 0
    .endif:
    pop ebx
    ret