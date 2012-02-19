ENTRY(loader)
OUTPUT_FORMAT("elf32-i386")

SECTIONS {
    . = 0x00120000;
    
    .text : {
        *(.text)
    }
    
    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }
    
    .data ALIGN (0x1000) : {
        *(.data)
    }
    
    .bss : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
}
