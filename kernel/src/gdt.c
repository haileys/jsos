#include "gdt.h"

static gdt_entry_t gdt[3];

static gdtr_t gdtr;

void gdt_init()
{    
    // 0x08 is a code selector:
    gdt[1].limit_0_15             = 0xffff;
    gdt[1].base_0_15              = 0x0000;
    gdt[1].base_16_23             = 0x00;
    gdt[1].access                 = 0x02 /* readable */ | 0x08 /* code segment */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[1].limit_16_19_and_flags  = 0xf | 0x40 /* 32 bit */ | 0x80 /* 4 KiB */;
    gdt[1].base_24_31             = 0x00;
    
    // 0x10 is a data selector:
    gdt[2].limit_0_15             = 0xffff;
    gdt[2].base_0_15              = 0x0000;
    gdt[2].base_16_23             = 0x00;
    gdt[2].access                 = 0x02 /* readable */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[2].limit_16_19_and_flags  = 0xf | 0x40 /* 32 bit */ | 0x80 /* 4 KiB */;
    gdt[2].base_24_31             = 0x00;
    
    // maybe 16 bit selectors in future?
    
    gdtr.size = sizeof(gdt) - 1;
    gdtr.offset = (uint32_t)&gdt;
    __asm__ volatile("lgdt (%0)" :: "m"(gdtr));
    gdt_reload_segment_registers();
}