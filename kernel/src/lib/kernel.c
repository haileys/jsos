#include <value.h>
#include <vm.h>
#include <gc.h>
#include <exception.h>
#include <string.h>
#include <jit.h>
#include "panic.h"
#include "console.h"
#include "lib.h"

static VAL Kernel;

static VAL Kernel_load_image(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_STRING) {
        js_throw_message(vm, "Kernel.loadImage() expects string");
    }
    js_string_t* str = js_to_js_string_t(argv[0]);
    js_image_t* image = js_image_parse(str->buff, str->length);
    if(!image) {
        js_throw_message(vm, "Couldn't parse image");
    }
    js_vm_exec(vm, image, 0, vm->global_scope, vm->global_scope->global_object, 0, NULL);
    return js_value_true();
}

static VAL Kernel_memory_usage(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    return js_value_make_double(js_gc_memory_usage());
}

static VAL Kernel_run_gc(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_gc_run();
    return js_value_undefined();
}

extern int _binary_src_realmode_bin_start;

static VAL Kernel_real_exec(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_STRING) {
        js_throw_message(vm, "Kernel.realExec() expects string");
    }
    js_string_t* str = js_to_js_string_t(argv[0]);
    
    // create a backup of the IDT and GDT
    char idt_backup[256*8];
    memcpy(idt_backup, (void*)0x1600, sizeof(idt_backup));
    
    // copy 16 bit code to 0x9000
    memcpy((void*)0x9000, str->buff, str->length);
    
    // call 32<->16 bit handler which in turn calls into 0x9000
    memcpy((void*)0x8000, &_binary_src_realmode_bin_start, 0x1000);
    ((void(*)())0x8000)();
    
    // restore IDT backup
    memcpy((void*)0x1600, idt_backup, sizeof(idt_backup));
    __asm__ volatile("sti");
    
    return js_value_undefined();
}

static VAL Kernel_panic(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    js_string_t* str = js_cstring("");
    for(uint32_t i = 0; i < argc; i++) {
        str = js_string_concat(str, js_to_js_string_t(argv[i]));
    }
    panic(str->buff);
}

static VAL Kernel_memcpy(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t dest, src, len;
    js_scan_args(vm, argc, argv, "III", &dest, &src, &len);
    memcpy((void*)dest, (void*)src, len);
    return js_value_undefined();
}

static VAL Kernel_memset(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t ptr, val, count;
    js_scan_args(vm, argc, argv, "III", &ptr, &val, &count);
    memset((void*)ptr, (uint8_t)val, count);
    return js_value_undefined();
}

static VAL Kernel_read_memory(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr, size;
    js_scan_args(vm, argc, argv, "II", &addr, &size);
    return js_value_make_string((void*)addr, size);
}

static VAL Kernel_write_memory(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    VAL buff;
    js_string_t* str;
    js_scan_args(vm, argc, argv, "IS", &addr, &buff);
    str = &js_value_get_pointer(buff)->string;
    memcpy((void*)addr, str->buff, str->length);
    return js_value_undefined();
}

static VAL Kernel_peek8(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint8_t*)addr);
}

static VAL Kernel_peek16(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint16_t*)addr);
}

static VAL Kernel_peek32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr;
    js_scan_args(vm, argc, argv, "I", &addr);
    return js_value_make_double(*(uint32_t*)addr);
}

static VAL Kernel_poke8(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr, val;
    js_scan_args(vm, argc, argv, "II", &addr, &val);
    *(uint8_t*)addr = (uint8_t)val;
    return js_value_undefined();
}

static VAL Kernel_poke16(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr, val;
    js_scan_args(vm, argc, argv, "II", &addr, &val);
    *(uint16_t*)addr = (uint16_t)val;
    return js_value_undefined();
}

static VAL Kernel_poke32(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t addr, val;
    js_scan_args(vm, argc, argv, "II", &addr, &val);
    *(uint32_t*)addr = (uint32_t)val;
    return js_value_undefined();
}

static VAL Kernel_malloc(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t sz;
    js_scan_args(vm, argc, argv, "I", &sz);
    return js_value_make_double((uint32_t)malloc(sz));
}

static VAL Kernel_free(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    uint32_t ptr;
    js_scan_args(vm, argc, argv, "I", &ptr);
    free((void*)ptr);
    return js_value_undefined();
}

static VAL Kernel_hlt(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    __asm__ volatile("hlt");
    return js_value_undefined();
}

static VAL Kernel_cli(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    __asm__ volatile("cli");
    return js_value_undefined();
}

static VAL Kernel_sti(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    __asm__ volatile("sti");
    return js_value_undefined();
}

static VAL Kernel_jit(js_vm_t* vm, void* state, VAL this, uint32_t argc, VAL* argv)
{
    if(argc == 0 || js_value_get_type(argv[0]) != JS_T_FUNCTION) {
        js_throw_error(vm->lib.TypeError, "expected function as first parameter");
    }
    js_function_t* fn = (js_function_t*)js_value_get_pointer(argv[0]);
    if(fn->is_native) {
        js_throw_error(vm->lib.TypeError, "can only JIT js functions");
    }
    js_native_callback_t jit = js_jit_section(&fn->js.image->sections[fn->js.section], NULL);
    if(jit) {
        return js_value_make_native_function(vm, NULL, NULL, jit, NULL);
    } else {
        js_throw_error(vm->lib.Error, "failed to JIT");
    }
}

void lib_kernel_init(js_vm_t* vm)
{
    Kernel = js_make_object(vm);
    js_gc_register_global(&Kernel, sizeof(Kernel));
    js_object_put(vm->global_scope->global_object, js_cstring("Kernel"), Kernel);
    
    js_object_put(Kernel, js_cstring("loadImage"), js_value_make_native_function(vm, NULL, js_cstring("loadImage"), Kernel_load_image, NULL));
    js_object_put(Kernel, js_cstring("memoryUsage"), js_value_make_native_function(vm, NULL, js_cstring("memoryUsage"), Kernel_memory_usage, NULL));
    js_object_put(Kernel, js_cstring("runGC"), js_value_make_native_function(vm, NULL, js_cstring("runGC"), Kernel_run_gc, NULL));
    js_object_put(Kernel, js_cstring("realExec"), js_value_make_native_function(vm, NULL, js_cstring("realExec"), Kernel_real_exec, NULL));
    js_object_put(Kernel, js_cstring("panic"), js_value_make_native_function(vm, NULL, js_cstring("panic"), Kernel_panic, NULL));
    js_object_put(Kernel, js_cstring("memcpy"), js_value_make_native_function(vm, NULL, js_cstring("memcpy"), Kernel_memcpy, NULL));
    js_object_put(Kernel, js_cstring("memset"), js_value_make_native_function(vm, NULL, js_cstring("memset"), Kernel_memset, NULL));
    js_object_put(Kernel, js_cstring("readMemory"), js_value_make_native_function(vm, NULL, js_cstring("readMemory"), Kernel_read_memory, NULL));
    js_object_put(Kernel, js_cstring("writeMemory"), js_value_make_native_function(vm, NULL, js_cstring("writeMemory"), Kernel_write_memory, NULL));
    js_object_put(Kernel, js_cstring("peek8"), js_value_make_native_function(vm, NULL, js_cstring("peek8"), Kernel_peek8, NULL));
    js_object_put(Kernel, js_cstring("peek16"), js_value_make_native_function(vm, NULL, js_cstring("peek16"), Kernel_peek16, NULL));
    js_object_put(Kernel, js_cstring("peek32"), js_value_make_native_function(vm, NULL, js_cstring("peek32"), Kernel_peek32, NULL));
    js_object_put(Kernel, js_cstring("poke8"), js_value_make_native_function(vm, NULL, js_cstring("poke8"), Kernel_poke8, NULL));
    js_object_put(Kernel, js_cstring("poke16"), js_value_make_native_function(vm, NULL, js_cstring("poke16"), Kernel_poke16, NULL));
    js_object_put(Kernel, js_cstring("poke32"), js_value_make_native_function(vm, NULL, js_cstring("poke32"), Kernel_poke32, NULL));
    js_object_put(Kernel, js_cstring("malloc"), js_value_make_native_function(vm, NULL, js_cstring("malloc"), Kernel_malloc, NULL));
    js_object_put(Kernel, js_cstring("free"), js_value_make_native_function(vm, NULL, js_cstring("free"), Kernel_free, NULL));
    js_object_put(Kernel, js_cstring("cli"), js_value_make_native_function(vm, NULL, js_cstring("cli"), Kernel_cli, NULL));
    js_object_put(Kernel, js_cstring("sti"), js_value_make_native_function(vm, NULL, js_cstring("sti"), Kernel_sti, NULL));
    js_object_put(Kernel, js_cstring("hlt"), js_value_make_native_function(vm, NULL, js_cstring("hlt"), Kernel_hlt, NULL));
    js_object_put(Kernel, js_cstring("jit"), js_value_make_native_function(vm, NULL, js_cstring("jit"), Kernel_jit, NULL));
}