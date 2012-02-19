#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "image.h"
#include "vm.h"
#include "gc.h"

char* read_until_eof(FILE* f, uint32_t* len)
{
    size_t cap = 4096;
    size_t idx = 0;
    char* buff = js_alloc(cap);
    while(!feof(stdin)) {
        idx += fread(buff + idx, 1, 4096, f);
        if(idx >= cap) {
            cap *= 2;
            buff = js_realloc(buff, cap);
        }
    }
    *len = idx;
    return buff;
}

int main()
{
    uint32_t dummy;
    uint32_t len;
    char* buff;
    js_image_t* image;
    uint32_t i, j, op;
    double number;
    js_gc_init(&dummy);
    buff = read_until_eof(stdin, &len);
    image = js_image_parse(buff, len);
    printf("read %d sections\n", image->section_count);
    for(i = 0; i < image->section_count; i++) {
        printf("\nsection %d:\n", i);
        for(j = 0; j < image->sections[i].instruction_count; j++) {
            op = image->sections[i].instructions[j];
            printf("    %04d  %-12s", j, js_instruction(op)->name);
            switch(js_instruction(op)->operand) {
                case OPERAND_NONE:
                    printf("\n");
                    break;
                case OPERAND_NUMBER:
                    number = *(double*)&image->sections[i].instructions[++j];
                    printf("%lf\n", number);
                    j++;
                    break;
                case OPERAND_UINT32:
                    op = image->sections[i].instructions[++j];
                    printf("%u\n", op);
                    break;
                case OPERAND_UINT32_UINT32:
                    op = image->sections[i].instructions[++j];
                    printf("%u, ", op);
                    op = image->sections[i].instructions[++j];
                    printf("%u\n", op);
                    break;
                case OPERAND_STRING:
                    op = image->sections[i].instructions[++j];
                    printf("\"%s\" (%d)\n", image->strings[op].buff, op);
                    break;
            }
        }
    }
    printf("\nstrings:\n");
    for(i = 0; i < image->string_count; i++) {
        printf("    %04d  \"%s\"\n", i, image->strings[i].buff);
    }
    
    printf("\n\nmemory in use before gc: %lu KiB\n", js_gc_memory_usage() / 1024);
    js_gc_run();
    printf("memory in use after gc: %lu KiB\n", js_gc_memory_usage() / 1024);
    
    return 0;
}