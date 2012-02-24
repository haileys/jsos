#ifndef LIBC_LIMITS_H
#define LIBC_LIMITS_H

// this libc is only intended to run on 32 bit
#define __WORDSIZE 32

#define CHAR_BIT    8
#define SCHAR_MIN   (-128)
#define SCHAR_MAX   127
#define UCHAR_MAX   255
#define CHAR_MIN    SCHAR_MIN
#define CHAR_MAX    SCHAR_MAX
#define SHRT_MIN    (-32768)
#define SHRT_MAX    32767
#define USHRT_MAX   65535
#define INT_MIN     (-2147483648)
#define INT_MAX     2147483647
#define UINT_MAX    4294967295U
#define LONG_MIN    INT_MIN
#define LONG_MAX    INT_MAX
#define ULONG_MAX   UINT_MAX

#endif