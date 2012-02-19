#include "math.h"

int __inline_isfinite_double(double x)
{
    return x == x && __builtin_fabs(x) != __builtin_inf();
}

double floor(double d)
{
    return __builtin_floor(d);
}

double fmod(double a, double b)
{
    return __builtin_fmod(a, b);
}

double fabs(double d)
{
    return __builtin_fabs(d);
}