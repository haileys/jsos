#ifndef MM_H
#define MM_H

#include <stdbool.h>
#include <stdint.h>
#include "multiboot.h"

#define MAX_PAGE (1024*1024) // 4GiB address space, 4KiB pages
#define PAGE_SIZE 4096
#define ADDRESS_SPACE_SIZE 4294967296ULL

void mm_init(multiboot_memory_map_t* mmap, uint32_t length, uint32_t highest_module);
void* sbrk(int32_t increment);

void paging_set_directory(void* page_directory);

#endif