#include <gc.h>
#include <vm.h>
#include <value.h>
#include <stdarg.h>
#include "multiboot.h"
#include "mm.h"
#include "console.h"
#include "panic.h"

static VAL console_log(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t i;
    for(i = 0; i < argc; i++) {
       kprintf("%s%s", i ? " " : "", js_value_get_pointer(js_to_string(argv[i]))->string.buff);
    }
    kprintf("\n");
    return js_value_undefined();
}

void kmain_(struct multiboot_info* mbd, uint32_t magic)
{
    int dummy;
    console_clear();
    
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC)         panic("multiboot did not pass correct magic number");
    if(!(mbd->flags & MULTIBOOT_INFO_MEMORY))       panic("multiboot did not pass memory information");
    if(!(mbd->flags & MULTIBOOT_INFO_MEM_MAP))      panic("multiboot did not pass memory map");
    
    kprintf("JSOS\n");
    mm_init((multiboot_memory_map_t*)mbd->mmap_addr, mbd->mmap_length);
    
    js_gc_init(&dummy);
    js_vm_t* vm = js_vm_new();
    
    VAL console = js_value_make_object(js_value_null(), js_value_null());
    js_object_put(console, js_cstring("log"), js_value_make_native_function(vm, NULL, js_cstring("log"), console_log, NULL));
    js_object_put(vm->global_scope->global_object, js_cstring("console"), console);
}

void kmain(struct multiboot_info* mbd, uint32_t magic)
{
    kmain_(mbd, magic);
    __asm__ volatile("cli \n hlt");
}

