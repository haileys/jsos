#include <string.h>
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "gc.h"
#include "value.h"

js_string_t* js_string_concat(js_string_t* a, js_string_t* b)
{
    js_string_t* str = js_alloc(sizeof(js_string_t));
    str->length = a->length + b->length;
    str->buff = js_alloc(str->length + 1);
    memcpy(str->buff, a->buff, a->length);
    memcpy(str->buff + a->length, b->buff, b->length);
    str->buff[str->length] = 0;
    return str;
}

bool js_string_eq(js_string_t* a, js_string_t* b)
{
    if(a->length != b->length) {
        return false;
    }
    return memcmp(a->buff, b->buff, a->length) == 0;
}

#ifndef JSOS
    static char* itoa(int value, char* buff, int base)
    {
        char* charset = "0123456789abcdefghijklmnopqrstuvwxyz";
        char* ret = buff;
        char scratch[64];
        int idx = 0;
        if(value < 0) {
            *buff++ = '-';
            value = -value;
        }
        if(value == 0) {
            *buff++ = '0';
            *buff = 0;
            return ret;
        }
        while(value > 0) {
            scratch[idx++] = charset[value % base];
            value /= base;
        }
        while(idx > 0) {
            *buff++ = scratch[--idx];
        }
        *buff = 0;
        return ret;
    }
#endif

static void ftoa(float Value, char* Buffer)
 {
     union
     {
         float f;
     
         struct
         {
             unsigned int    mantissa_lo : 16;
             unsigned int    mantissa_hi : 7;    
             unsigned int     exponent : 8;
             unsigned int     sign : 1;
         };
     } helper;
     
     unsigned long mantissa;
     signed char exponent;
     unsigned int int_part;
     char frac_part[3];
     int i, count = 0;
     
     helper.f = Value;
     //mantissa is LS 23 bits
     mantissa = helper.mantissa_lo;
     mantissa += ((unsigned long) helper.mantissa_hi << 16);
     //add the 24th bit to get 1.mmmm^eeee format
     mantissa += 0x00800000;
     //exponent is biased by 127
     exponent = (signed char) helper.exponent - 127;
     
     //too big to shove into 8 chars
     if (exponent > 18)
     {
         Buffer[0] = 'I';
         Buffer[1] = 'n';
         Buffer[2] = 'f';
         Buffer[3] = '\0';
         return;
     }
     
     //too small to resolve (resolution of 1/8)
     if (exponent < -3)
     {
         Buffer[0] = '0';
         Buffer[1] = '\0';
         return;
     }
     
     count = 0;
     
     //add negative sign (if applicable)
     if (helper.sign)
     {
         Buffer[0] = '-';
         count++;
     }
     
     //get the integer part
     int_part = mantissa >> (23 - exponent);    
     //convert to string
     itoa(int_part, &Buffer[count], 10);
     
     //find the end of the integer
     for (i = 0; i < 8; i++)
         if (Buffer[i] == '\0')
         {
             count = i;
             break;
         }        
 
     //not enough room in the buffer for the frac part    
     if (count > 5)
         return;
     
     //add the decimal point    
     Buffer[count++] = '.';
     
     //use switch to resolve the fractional part
     switch (0x7 & (mantissa  >> (20 - exponent)))
     {
         case 0:
             frac_part[0] = '0';
             frac_part[1] = '0';
             frac_part[2] = '0';
             break;
         case 1:
             frac_part[0] = '1';
             frac_part[1] = '2';
             frac_part[2] = '5';            
             break;
         case 2:
             frac_part[0] = '2';
             frac_part[1] = '5';
             frac_part[2] = '0';            
             break;
         case 3:
             frac_part[0] = '3';
             frac_part[1] = '7';
             frac_part[2] = '5';            
             break;
         case 4:
             frac_part[0] = '5';
             frac_part[1] = '0';
             frac_part[2] = '0';            
             break;
         case 5:
             frac_part[0] = '6';
             frac_part[1] = '2';
             frac_part[2] = '5';            
             break;
         case 6:
             frac_part[0] = '7';
             frac_part[1] = '5';
             frac_part[2] = '0';            
             break;
         case 7:
             frac_part[0] = '8';
             frac_part[1] = '7';
             frac_part[2] = '5';                    
             break;
     }
     
     //add the fractional part to the output string
     for (i = 0; i < 3; i++)
         if (count < 7)
             Buffer[count++] = frac_part[i];
     
     //make sure the output is terminated
     Buffer[count] = '\0';
 }

js_string_t* js_string_from_double(double number)
{
    char buff[200];
    int len, i;
    ftoa((float)number, buff);
    len = strlen(buff);
    if(memchr(buff, '.', len)) {
        for(i = 1; i <= len; i++) {
            if(buff[len - i] == '0') {
                buff[len - i] = 0;
            } else if(buff[len - i] == '.') {
                buff[len - i] = 0;
                break;
            }
        }
    }
    return js_cstring(buff);
}

js_string_t* js_string_format(char* fmt, ...)
{
    js_string_t* retn;
    va_list va;
    va_start(va, fmt);
    retn = js_string_vformat(fmt, va);
    va_end(va);
    return retn;
}

js_string_t* js_string_vformat(char* fmt, va_list args)
{
    js_string_t* str = js_alloc(sizeof(js_string_t));
    str->buff = js_alloc(1024);
    str->length = vsnprintf(str->buff, 1023, fmt, args);
    str->buff[1023] = 0;
    return str;
}