#include <stdlib.h>
#include <vm.h>
#include <gc.h>
#include "interrupt.h"
#include "io.h"
#include "console.h"
#include "panic.h"

static idtr_t idtr;
static idt_entry_t* idt = (idt_entry_t*)0x1600;

static VAL js_isr_table;

#define MAX_QUEUED_INTERRUPTS 32

static struct {
    uint32_t interrupt;
    uint32_t error;
} queued_interrupts[MAX_QUEUED_INTERRUPTS];

static uint32_t queue_front;
static uint32_t queue_back;

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

void idt_register_handler(uint8_t gate, uint32_t isr)
{
    idt[gate].offset_lo = isr & 0xffff;
    idt[gate].offset_hi = (isr >> 16) & 0xffff;
    idt[gate].cs = 0x08;
    idt[gate].zero = 0;
    idt[gate].type = 0x80 /* present */ | 0xf /* 32 bit trap gate */;
}

static char* exceptions[] = {
    [6]     = "Invalid Opcode",
    [8]     = "Double Fault",
    [10]    = "Invalid TSS",
    [11]    = "Segment Not Present",
    [13]    = "General Protection Fault",
    [14]    = "Page Fault"
};

uint32_t isr_dispatch(uint32_t interrupt, uint32_t error)
{
    if(interrupt == 39 /* spurious */) {
        return 0;
    }
    
    if(interrupt == 14 /* page fault */) {
        uint32_t fault_address;
        __asm__ volatile ("movl %%cr2, %0" : "=r"(fault_address));
        panicf("Page Fault at 0x%x - failed on %s\n", fault_address, (error & 2) ? "write" : "read");
    }
    
    if(interrupt < sizeof(exceptions) / sizeof(char*) && exceptions[interrupt]) {
        panicf("%s", exceptions[interrupt]);
    }
    
    // isr_dispatch is already in a cli/hlt so there's no need to do any locking
    if((queue_back + 1) % MAX_QUEUED_INTERRUPTS == queue_front) {
        // ring buffer is full, this interrupt needs to be dropped
        return 1;
    }
    
    // add to ring buffer
    queued_interrupts[queue_back].interrupt = interrupt;
    queued_interrupts[queue_back].error = error;
    queue_back = (queue_back + 1) % MAX_QUEUED_INTERRUPTS;
    
    return 0;
}

void interrupt_dispatch_events()
{
    while(true) {
        // lock this region of code so we're not racing potential interrupts
        cli();
        if(queue_front == queue_back) {
            sti();
            return;
        }
        uint32_t interrupt = queued_interrupts[queue_front].interrupt;
        uint32_t error = queued_interrupts[queue_front].error;
        queue_front = (queue_front + 1) % MAX_QUEUED_INTERRUPTS;
        sti();
        
        char buff[8];
        itoa(interrupt, buff, 10);
        VAL isr = js_object_get(js_isr_table, js_cstring(buff));
        VAL errcode = js_value_make_double(error);
        if(js_value_get_type(isr) == JS_T_FUNCTION) {
            js_call(isr, js_isr_table, 1, &errcode);
        }
    }
}

static VAL Kernel_dispatch_interrupts(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    interrupt_dispatch_events();
    return js_value_undefined();
}

void idt_init(js_vm_t* vm)
{    
    cli();
    asm_isr_init();
    remap_irqs();
    idtr.size = (256*8) - 1;
    idtr.offset = (uint32_t)idt;
    __asm__ volatile("lidt (%0)" :: "m"(idtr));
    
    js_isr_table = js_make_object(vm);
    js_gc_register_global(&js_isr_table, sizeof(js_isr_table));
    VAL Kernel = js_object_get(vm->global_scope->global_object, js_cstring("Kernel"));
    js_object_put(Kernel, js_cstring("isrs"), js_isr_table);
    js_object_put(Kernel, js_cstring("dispatchInterrupts"), js_value_make_native_function(vm, NULL, js_cstring("dispatchInterrupts"), Kernel_dispatch_interrupts, NULL));
    sti();
}