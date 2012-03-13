#include <string.h>
#include "vm.h"
#include "jit.h"
#include "gc.h"

#define RESIZE_BUFF(new_size) do {\
        if(capacity <= new_size) { \
            capacity *= 2; \
            buff = js_realloc(buff, new_size); \
        } \
    } while(0)

#define APPEND(code, len) do { \
        RESIZE_BUFF(idx + len); \
        memcpy(buff + idx, code, len); \
        idx += len; \
    } while(0)

#define APPEND_U32(u32) do { \
        RESIZE_BUFF(idx + 4); \
        *(uint32_t*)(buff + idx) = (u32); \
        idx += 4; \
    } while(0)

js_native_callback_t js_jit_section(js_section_t* section, uint32_t* length)
{
    uint32_t idx = 0;
    uint32_t capacity = 32;
    uint8_t* buff = js_alloc(32);
    
    // 55                push ebp
    // 89EC              mov ebp,esp
    APPEND("\x55\x89\xE5", 3);
    
    uint32_t ins_i = 0;
    while(ins_i < section->instruction_count) {
        switch(section->instructions[ins_i++]) {
            
            case JS_OP_UNDEFINED: {
                // 680000FAFF        push dword 0xfffa0000
                // 6A01              push byte +0x1
                APPEND("\x68\x00\x00\xFA\xFF\x6A\x01", 7);
                break;
            }
            
            case JS_OP_RET: {
                // 8B4508            mov eax,[ebp+0x8]
                // 59                pop ecx
                // 8908              mov [eax],ecx
                // 59                pop ecx
                // 894804            mov [eax+0x4],ecx
                // C9                leave
                // C20400            ret
                APPEND("\x8B\x45\x08\x59\x89\x08\x59\x89\x48\x04\xC9\xC2\x04\x00", 14);
                break;
            }
            
            case JS_OP_PUSHNUM: {
                // 68xxxxxxxx        push dword HIGH_DWORD
                // 68xxxxxxxx        push dword LOW_DWORD
                uint32_t lo = section->instructions[ins_i++];
                uint32_t hi = section->instructions[ins_i++];
                APPEND("\x68", 1);
                APPEND_U32(hi);
                APPEND("\x68", 1);
                APPEND_U32(lo);
                break;
            }
            
            default:
                // could not compile this instruction
                if(length) {
                    *length = 0;
                }
                return NULL;
        }
    }
    
    if(length) {
        *length = idx;
    }
    return (js_native_callback_t)(void*)buff;
}