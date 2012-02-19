#include <stdlib.h>
#include <stdio.h>
#include "gc.h"

void realmain()
{    
    uint32_t* x;
    x = js_alloc(4);
    printf("&x is %p\n", &x);
    printf("x is %p\n", x);
    *x = 123456789;
    printf("x = %d\n", *x);
    printf("running gc\n");
    js_gc_run();
    printf("x = %d\n", *x);
}

int main()
{
    int x;
    js_gc_init(&x);
    realmain();
    return 0;
}