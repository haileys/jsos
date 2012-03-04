#include "gdt.h"

static gdt_entry_t* gdt = (gdt_entry_t*)0xf000;
static gdtr_t gdtr;

static int end_of_image;

void gdt_init()
{
    uint32_t code_end = ((uint32_t)&end_of_image + 4095) / 4096;
    
    // 0x08 is a code selector:
    gdt[1].limit_0_15               = code_end & 0xffff;
    gdt[1].base_0_15                = 0x0000;
    gdt[1].base_16_23               = 0x00;
    gdt[1].access                   = 0x02 /* readable */ | 0x08 /* code segment */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[1].limit_16_19_and_flags    = ((code_end >> 16) & 0xf) | 0x40 /* 32 bit */ | 0x80 /* 4 KiB */;
    gdt[1].base_24_31               = 0x00;
    
    // 0x10 is a data selector:
    gdt[2].limit_0_15               = 0xffff;
    gdt[2].base_0_15                = 0x0000;
    gdt[2].base_16_23               = 0x00;
    gdt[2].access                   = 0x02 /* readable */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[2].limit_16_19_and_flags    = 0xf | 0x40 /* 32 bit */ | 0x80 /* 4 KiB */;
    gdt[2].base_24_31               = 0x00;
    
    // 0x18 is a 16 bit code selector
    gdt[3].limit_0_15               = 0xffff;
    gdt[3].base_0_15                = 0;
    gdt[3].base_16_23               = 0;
    gdt[3].access                   = 0x02 /* readable */ | 0x08 /* code segment */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[3].limit_16_19_and_flags    = 0 /* neither 16 bit or 4 KiB */;
    gdt[3].base_24_31               = 0x00;
    
    // 0x20 is a 16 bit data selector
    gdt[4].limit_0_15               = 0xffff;
    gdt[4].base_0_15                = 0;
    gdt[4].base_16_23               = 0;
    gdt[4].access                   = 0x02 /* readable */ | 0x10 /* reserved */ | 0x80 /* present */;
    gdt[4].limit_16_19_and_flags    = 0 /* neither 16 bit or 4 KiB */;
    gdt[4].base_24_31               = 0x00;
    
    // maybe 16 bit selectors in future?
    
    gdtr.size = (sizeof(gdt_entry_t) * 5) - 1;
    gdtr.offset = (uint32_t)gdt;
    __asm__ volatile("lgdt (%0)" :: "m"(gdtr));
    gdt_reload_segment_registers();
}