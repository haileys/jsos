#include <stdlib.h>
#include <vm.h>
#include <gc.h>
#include "interrupt.h"
#include "io.h"
#include "console.h"

static idtr_t idtr;
static idt_entry_t idt[256];

static VAL js_isr_table;

void idt_set_gate(uint8_t gate, idt_entry_t entry)
{
    idt[gate] = entry;
}

static void remap_irqs()
{	
	// remap IRQ table
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);
}

void asm_isr_init();

void idt_init(js_vm_t* vm)
{
    asm_isr_init();
    remap_irqs();
    idtr.size = sizeof(idt) - 1;
    idtr.offset = (uint32_t)&idt;
    __asm__ volatile("lidt (%0)" :: "m"(idtr));
    
    js_isr_table = js_make_object(vm);
    js_gc_register_global((void**)&js_isr_table);
    js_object_put(js_object_get(vm->global_scope->global_object, js_cstring("Kernel")), js_cstring("isrs"), js_isr_table);
}

void idt_register_handler(uint8_t gate, uint32_t isr)
{
    idt[gate].offset_lo = isr & 0xffff;
    idt[gate].offset_hi = (isr >> 16) & 0xffff;
    idt[gate].cs = 0x08;
    idt[gate].zero = 0;
    idt[gate].type = 0x80 /* present */ | 0xf /* 32 bit trap gate */;
}

uint32_t isr_dispatch(uint32_t interrupt, uint32_t error)
{
    if(interrupt == 39) {
        return interrupt;
    }
    char buff[8];
    itoa(interrupt, buff, 10);
    VAL isr = js_object_get(js_isr_table, js_cstring(buff));
    VAL errcode = js_value_make_double(error);
    if(js_value_get_type(isr) == JS_T_FUNCTION) {
        js_call(isr, js_isr_table, 1, &errcode);
    }
    return interrupt;
}