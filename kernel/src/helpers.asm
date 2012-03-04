global gdt_reload_segment_registers
global paging_set_directory

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

paging_set_directory:
    ; page directory address:
    mov eax, [esp+4]
    mov cr3, eax
    ; enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    ret