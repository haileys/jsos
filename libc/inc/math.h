#ifndef LIBC_MATH_H
#define LIBC_MATH_H

#define NAN (0.0/0.0)
#define INFINITY (1.0/0.0)

int __inline_isfinite_double(double x);

#define isfinite(x) __inline_isfinite_double(x)

double floor(double x);
double fmod(double x, double y);
double fabs(double x);

#endif