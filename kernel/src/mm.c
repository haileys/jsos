#include <stdlib.h>
#include "mm.h"
#include "panic.h"
#include "console.h"

static uint32_t base;
static uint32_t max_size;
static uint32_t current_increment;

extern int end_of_image;

void mm_init(multiboot_memory_map_t* mmap, uint32_t length, uint32_t highest_module)
{
    uint32_t i, len;
    if((uint32_t)&end_of_image > highest_module) {
        highest_module = (uint32_t)&end_of_image;
    }
    uint32_t start_addr = ((highest_module + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    uint64_t end;
    for(i = 0; i < length / sizeof(multiboot_memory_map_t); i++) {
        if(mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            if(mmap[i].addr >= ADDRESS_SPACE_SIZE) {
                continue;
            }
            end = mmap[i].addr + mmap[i].len;
            if(end > ADDRESS_SPACE_SIZE) {
                end = ADDRESS_SPACE_SIZE - 1;
            }
            len = end - mmap[i].addr;
            if(mmap[i].addr + len <= start_addr) {
                continue;
            }
            if(mmap[i].addr > start_addr) {
                len -= (uint32_t)mmap[i].addr - start_addr;
                start_addr = (uint32_t)mmap[i].addr;
            }
            base = start_addr;
            max_size = len;
            kprintf("%d MiB available memory (0x%x - 0x%x)\n", len / (1024 * 1024), start_addr, start_addr + len);
            return;
        }
    }
    panic("did not find suitable memory region");
}

void* sbrk(int32_t increment)
{
    if(increment < 0 && ((uint32_t)-increment) > current_increment) {
        // trying to decrement the brk to below 0
        panic("trying to sbrk to negative current_increment");
    }
    if(current_increment + increment >= max_size) {
        panic("out of memory");
    }
    void* ptr = (void*)(base + current_increment);
    current_increment += increment;
    return ptr;
}