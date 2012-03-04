#include <stdlib.h>
#include "mm.h"
#include "panic.h"
#include "console.h"

#define PAGE_SIZE 4096

static uint32_t base;
static uint32_t max_size;
static uint32_t current_increment;

extern int end_of_image;

static uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));

static void paging_init(uint32_t max_addr)
{
    // this maps in page table entries up to 
    uint32_t i;
    uint32_t max_dir_ent = max_addr / (PAGE_SIZE * 1024);
    for(i = 0; i <= max_dir_ent; i++) {
        // calculate page base address:
        uint32_t page_base_addr = i * PAGE_SIZE * 1024;
        // reserve 4 KiB for page table, base is already page aligned...
        uint32_t* page_table = (uint32_t*)base;
        base += PAGE_SIZE;
        // set page directory entry:
        page_directory[i] = (uint32_t)page_table | 1 /* present */ | 2 /* read/write */;
        uint32_t j;
        for(j = 0; j < 1024; j++) {
            if(i == 0 && j == 0) {
                // page out zero page
                page_table[j] = 0;
            }
            // identity map:
            page_table[j] = (page_base_addr + j * PAGE_SIZE) | 1 /* present */;
            if(page_base_addr < 0x00120000 /* start of code */ || page_base_addr >= (uint32_t)&end_of_image /* end of code */) {
                page_table[j] |= 2; // allow writes to non-code pages
            }
        }
    }
    paging_set_directory(&page_directory);
}

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
            // page align base address:
            base = ((start_addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
            max_size = len;
            kprintf("%d MiB available memory (0x%x - 0x%x)\n", len / (1024 * 1024), start_addr, start_addr + len);
            paging_init(start_addr + len);
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
        return NULL;
        panic("out of memory");
    }
    void* ptr = (void*)(base + current_increment);
    current_increment += increment;
    return ptr;
}