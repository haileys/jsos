#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct gdtr {
	uint16_t size;
	uint32_t offset;
} __attribute__((__packed__)) gdtr_t;

typedef struct gdt_entry {
    uint16_t limit_0_15;
    uint16_t base_0_15;
    uint8_t base_16_23;
    uint8_t access;
    uint8_t limit_16_19_and_flags;
    uint8_t base_24_31;
} __attribute__((__packed__)) gdt_entry_t;

void gdt_reload_segment_registers();
void gdt_init();

#endif