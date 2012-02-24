global gdt_reload_segment_registers

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