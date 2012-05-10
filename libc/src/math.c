/*
 * Some code in this file has been extracted from newlib:
 *
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include <stdint.h>
#include "math.h"
#include "float.h"

static int errno;
#define EDOM 1
#define ERANGE 2

int __inline_isfinite_double(double x)
{
    return x == x && __builtin_fabs(x) != __builtin_inf();
}

#define DOUBLE_EXP_OFFS 1023


static const double BIGX = 7.09782712893383973096e+02;
static const double SMALLX = -7.45133219101941108420e+02;
static const double z_rooteps = 7.4505859692e-9;

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)					\
do {								\
  ieee_double_shape_type gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

typedef union 
{
  double value;
  struct 
  {
    uint32_t lsw;
    uint32_t msw;
  } parts;
} ieee_double_shape_type;

static const double huge = 1.0e300;
double floor(double x)
{
    /*
     * ====================================================
     * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
     *
     * Developed at SunPro, a Sun Microsystems, Inc. business.
     * Permission to use, copy, modify, and distribute this
     * software is freely granted, provided that this notice 
     * is preserved.
     * ====================================================
     */
	int32_t i0,i1,j0;
	uint32_t i,j;
	EXTRACT_WORDS(i0,i1,x);
	j0 = ((i0>>20)&0x7ff)-0x3ff;
	if(j0<20) {
	    if(j0<0) { 	/* raise inexact if x != 0 */
		if(huge+x>0.0) {/* return 0*sign(x) if |x|<1 */
		    if(i0>=0) {i0=i1=0;} 
		    else if(((i0&0x7fffffff)|i1)!=0)
			{ i0=0xbff00000;i1=0;}
		}
	    } else {
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) return x; /* x is integral */
		if(huge+x>0.0) {	/* raise inexact flag */
		    if(i0<0) i0 += (0x00100000)>>j0;
		    i0 &= (~i); i1=0;
		}
	    }
	} else if (j0>51) {
	    if(j0==0x400) return x+x;	/* inf or NaN */
	    else return x;		/* x is integral */
	} else {
	    i = ((uint32_t)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) return x;	/* x is integral */
	    if(huge+x>0.0) { 		/* raise inexact flag */
		if(i0<0) {
		    if(j0==20) i0+=1; 
		    else {
			j = i1+(1<<(52-j0));
			if((int32_t)j < i1) i0 +=1 ; 	/* got a carry */
			i1=j;
		    }
		}
		i1 &= (~i);
	    }
	}
	INSERT_WORDS(x,i0,i1);
	return x;
}



double ldexp(double d, int e)
{
    /*
    * ====================================================
    * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
    *
    * Developed at SunPro, a Sun Microsystems, Inc. business.
    * Permission to use, copy, modify, and distribute this
    * software is freely granted, provided that this notice 
    * is preserved.
    * ====================================================
    */

    int exp;
    uint32_t hd;

    GET_HIGH_WORD (hd, d);

    /* Check for special values and then scale d by e. */
    if(d != d /* NAN test */) {
        errno = EDOM;
    } else if(!isfinite(d)) {
        errno = ERANGE;
    } else if(d != 0) {
        exp = (hd & 0x7ff00000) >> 20;
        exp += e;

        if (exp > DBL_MAX_EXP + DOUBLE_EXP_OFFS)
        {
            errno = ERANGE;
            d = INFINITY;
        }
        else if (exp < DBL_MIN_EXP + DOUBLE_EXP_OFFS)
        {
            errno = ERANGE;
            d = -INFINITY;
        }
        else
        {
            hd &= 0x800fffff;
            hd |= exp << 20;
            SET_HIGH_WORD (d, hd);
        }
    }
    return (d);
}

static const double one = 1.0, Zero[] = {0.0, -0.0,};
double fmod(double x, double y)
{
    /*
     * ====================================================
     * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
     *
     * Developed at SunPro, a Sun Microsystems, Inc. business.
     * Permission to use, copy, modify, and distribute this
     * software is freely granted, provided that this notice 
     * is preserved.
     * ====================================================
     */
	int32_t n,hx,hy,hz,ix,iy,sx,i;
	uint32_t lx,ly,lz;

	EXTRACT_WORDS(hx,lx,x);
	EXTRACT_WORDS(hy,ly,y);
	sx = hx&0x80000000;		/* sign of x */
	hx ^=sx;		/* |x| */
	hy &= 0x7fffffff;	/* |y| */

    /* purge off exception values */
	if((hy|ly)==0||(hx>=0x7ff00000)||	/* y=0,or x not finite */
	  ((hy|((ly|-ly)>>31))>0x7ff00000))	/* or y is NaN */
	    return (x*y)/(x*y);
	if(hx<=hy) {
	    if((hx<hy)||(lx<ly)) return x;	/* |x|<|y| return x */
	    if(lx==ly) 
		return Zero[(uint32_t)sx>>31];	/* |x|=|y| return x*0*/
	}

    /* determine ix = ilogb(x) */
	if(hx<0x00100000) {	/* subnormal x */
	    if(hx==0) {
		for (ix = -1043, i=lx; i>0; i<<=1) ix -=1;
	    } else {
		for (ix = -1022,i=(hx<<11); i>0; i<<=1) ix -=1;
	    }
	} else ix = (hx>>20)-1023;

    /* determine iy = ilogb(y) */
	if(hy<0x00100000) {	/* subnormal y */
	    if(hy==0) {
		for (iy = -1043, i=ly; i>0; i<<=1) iy -=1;
	    } else {
		for (iy = -1022,i=(hy<<11); i>0; i<<=1) iy -=1;
	    }
	} else iy = (hy>>20)-1023;

    /* set up {hx,lx}, {hy,ly} and align y to x */
	if(ix >= -1022) 
	    hx = 0x00100000|(0x000fffff&hx);
	else {		/* subnormal x, shift x to normal */
	    n = -1022-ix;
	    if(n<=31) {
	        hx = (hx<<n)|(lx>>(32-n));
	        lx <<= n;
	    } else {
		hx = lx<<(n-32);
		lx = 0;
	    }
	}
	if(iy >= -1022) 
	    hy = 0x00100000|(0x000fffff&hy);
	else {		/* subnormal y, shift y to normal */
	    n = -1022-iy;
	    if(n<=31) {
	        hy = (hy<<n)|(ly>>(32-n));
	        ly <<= n;
	    } else {
		hy = ly<<(n-32);
		ly = 0;
	    }
	}

    /* fix point fmod */
	n = ix - iy;
	while(n--) {
	    hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	    if(hz<0){hx = hx+hx+(lx>>31); lx = lx+lx;}
	    else {
	    	if((hz|lz)==0) 		/* return sign(x)*0 */
		    return Zero[(uint32_t)sx>>31];
	    	hx = hz+hz+(lz>>31); lx = lz+lz;
	    }
	}
	hz=hx-hy;lz=lx-ly; if(lx<ly) hz -= 1;
	if(hz>=0) {hx=hz;lx=lz;}

    /* convert back to floating value and restore the sign */
	if((hx|lx)==0) 			/* return sign(x)*0 */
	    return Zero[(uint32_t)sx>>31];	
	while(hx<0x00100000) {		/* normalize x */
	    hx = hx+hx+(lx>>31); lx = lx+lx;
	    iy -= 1;
	}
	if(iy>= -1022) {	/* normalize output */
	    hx = ((hx-0x00100000)|((iy+1023)<<20));
	    INSERT_WORDS(x,hx|sx,lx);
	} else {		/* subnormal output */
	    n = -1022 - iy;
	    if(n<=20) {
		lx = (lx>>n)|((uint32_t)hx<<(32-n));
		hx >>= n;
	    } else if (n<=31) {
		lx = (hx<<(32-n))|(lx>>n); hx = sx;
	    } else {
		lx = hx>>(n-32); hx = sx;
	    }
	    INSERT_WORDS(x,hx|sx,lx);
	    x *= one;		/* create necessary signal */
	}
	return x;		/* exact output */
}

double frexp (double d, int *exp)
{
  double f;
  uint32_t hd, ld, hf, lf;

  /* Check for special values. */
  if(d != d /* NAN test*/ || !isfinite(d)) {
      errno = EDOM;
      *exp = 0;
      return d;
  } else if(d == 0) {
      *exp = 0;
      return d;
  }

  EXTRACT_WORDS (hd, ld, d);

  /* Get the exponent. */
  *exp = ((hd & 0x7ff00000) >> 20) - 1022;

  /* Get the mantissa. */ 
  lf = ld;
  hf = hd & 0x800fffff;  
  hf |= 0x3fe00000;

  INSERT_WORDS (f, hf, lf);

  return (f);
}

static const double INV_LN2 = 1.4426950408889634074;
static const double LN2 = 0.6931471805599453094172321;
static const double p[] = { 0.25, 0.75753180159422776666e-2,
                     0.31555192765684646356e-4 };
static const double q[] = { 0.5, 0.56817302698551221787e-1,
                     0.63121894374398504557e-3,
                     0.75104028399870046114e-6 };

double exp(double x)
{
  int N;
  double g, z, R, P, Q;

  if(x != x /* NAN */) {
      errno = EDOM;
      return x;
  } else if(!isfinite(x)) {
      errno = ERANGE;
      if(x > 0) {
          return INFINITY;
      } else {
          return 0.0;
      }
  } else if(x == 0) {
      return 1.0;
  }

  /* Check for out of bounds. */
  if (x > BIGX || x < SMALLX)
    {
      errno = ERANGE;
      return (x);
    }

  /* Check for a value too small to calculate. */
  if (-z_rooteps < x && x < z_rooteps)
    {
      return (1.0);
    }

  /* Calculate the exponent. */
  if (x < 0.0)
    N = (int) (x * INV_LN2 - 0.5);
  else
    N = (int) (x * INV_LN2 + 0.5);

  /* Construct the mantissa. */
  g = x - N * LN2;
  z = g * g;
  P = g * ((p[2] * z + p[1]) * z + p[0]);
  Q = ((q[3] * z + q[2]) * z + q[1]) * z + q[0];
  R = 0.5 + P / (Q - P);

  /* Return the floating point value. */
  N++;
  return (ldexp (R, N));
}

static const double a[] = { -0.64124943423745581147e+02,
                             0.16383943563021534222e+02,
                            -0.78956112887481257267 };
static const double b[] = { -0.76949932108494879777e+03,
                             0.31203222091924532844e+03,
                            -0.35667977739034646171e+02 };
static const double C1 =  22713.0 / 32768.0;
static const double C2 =  1.428606820309417232e-06;
static const double C3 =  0.43429448190325182765;

#define __SQRT_HALF 0.70710678118654752440

double logarithm(double x, int ten)
{
  int N;
  double f, w, z;

  /* Check for range and domain errors here. */
  if (x == 0.0)
    {
      errno = ERANGE;
      return (-INFINITY);
    }
  else if (x < 0.0)
    {
      errno = EDOM;
      return (NAN);
    }
  else if (!isfinite(x))
    {
      if (x != x)
        return (NAN);
      else
        return (INFINITY);
    }

  /* Get the exponent and mantissa where x = f * 2^N. */
  f = frexp (x, &N);

  z = f - 0.5;

  if (f > __SQRT_HALF)
    z = (z - 0.5) / (f * 0.5 + 0.5);
  else
    {
      N--;
      z /= (z * 0.5 + 0.5);
    }
  w = z * z;

  /* Use Newton's method with 4 terms. */
  z += z * w * ((a[2] * w + a[1]) * w + a[0]) / (((w + b[2]) * w + b[1]) * w + b[0]);

  if (N != 0)
    z = (N * C2 + z) + N * C1;

  if (ten)
    z *= C3;

  return (z);
}

double log(double x)
{
    return logarithm(x, 0);
}

double modf(double x, double *iptr)
{
    /*
     * ====================================================
     * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
     *
     * Developed at SunPro, a Sun Microsystems, Inc. business.
     * Permission to use, copy, modify, and distribute this
     * software is freely granted, provided that this notice 
     * is preserved.
     * ====================================================
     */
    
	int32_t i0,i1,j0;
	uint32_t i;
	EXTRACT_WORDS(i0,i1,x);
	j0 = ((i0>>20)&0x7ff)-0x3ff;	/* exponent of x */
	if(j0<20) {			/* integer part in high x */
	    if(j0<0) {			/* |x|<1 */
	        INSERT_WORDS(*iptr,i0&0x80000000,0);	/* *iptr = +-0 */
		return x;
	    } else {
		i = (0x000fffff)>>j0;
		if(((i0&i)|i1)==0) {		/* x is integral */
		    uint32_t high;
		    *iptr = x;
		    GET_HIGH_WORD(high,x);
		    INSERT_WORDS(x,high&0x80000000,0);	/* return +-0 */
		    return x;
		} else {
		    INSERT_WORDS(*iptr,i0&(~i),0);
		    return x - *iptr;
		}
	    }
	} else if (j0>51) {		/* no fraction part */
	    uint32_t high;
	    *iptr = x*one;
	    GET_HIGH_WORD(high,x);
	    INSERT_WORDS(x,high&0x80000000,0);	/* return +-0 */
	    return x;
	} else {			/* fraction part in low x */
	    i = ((uint32_t)(0xffffffff))>>(j0-20);
	    if((i1&i)==0) { 		/* x is integral */
	        uint32_t high;
		*iptr = x;
		GET_HIGH_WORD(high,x);
		INSERT_WORDS(x,high&0x80000000,0);	/* return +-0 */
		return x;
	    } else {
	        INSERT_WORDS(*iptr,i0,i1&(~i));
		return x - *iptr;
	    }
	}
}

double pow (double x, double y)
{
    /*
     * ====================================================
     * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
     *
     * Developed at SunPro, a Sun Microsystems, Inc. business.
     * Permission to use, copy, modify, and distribute this
     * software is freely granted, provided that this notice 
     * is preserved.
     * ====================================================
     */
  double d, k, t, r = 1.0;
  int n, sign, exponent_is_even_int = 0;
  uint32_t px;

  GET_HIGH_WORD (px, x);

  k = modf (y, &d);

  if (k == 0.0)
    {
      /* Exponent y is an integer. */
      if (modf (ldexp (y, -1), &t))
        {
          /* y is odd. */
          exponent_is_even_int = 0;
        }
      else
        {
          /* y is even. */
          exponent_is_even_int = 1;
        }
    }

  if (x == 0.0)
    {
      if (y <= 0.0)
        errno = EDOM;
    }
  else if ((t = y * log (fabs (x))) >= BIGX) 
    {
      errno = ERANGE;
      if (px & 0x80000000) 
        {
          /* x is negative. */
          if (k) 
            {
              /* y is not an integer. */
              errno = EDOM;
              x = 0.0;
            }
          else if (exponent_is_even_int)
            x = INFINITY;
          else
            x = -INFINITY;
        }
      else
        {
          x = INFINITY;
        }
    }
  else if (t < SMALLX)
    {
      errno = ERANGE;
      x = 0.0;
    }
  else 
    {
      if ( !k && fabs(d) <= 32767 ) 
        {
          n = (int) d;

          if ((sign = (n < 0)))
            n = -n;

          while ( n > 0 ) 
            {
              if ((unsigned int) n % 2) 
                r *= x;
              x *= x;
              n = (unsigned int) n / 2;
            }

          if (sign)
            r = 1.0 / r;

          return r;
        }
      else 
        {
          if ( px & 0x80000000 ) 
            {
              /* x is negative. */
              if ( k ) 
                {
                  /* y is not an integer. */
                  errno = EDOM;
                  return 0.0;
                }
            }

          x = exp (t);

          if (!exponent_is_even_int)
            {
              if (px & 0x80000000)
                {
                  /* y is an odd integer, and x is negative,
                     so the result is negative. */
                  GET_HIGH_WORD (px, x);
                  px |= 0x80000000;
                  SET_HIGH_WORD (x, px);
                }
            }
        }
    }

  return x;
}

double fabs(double x)
{
    return __builtin_fabs(x);
}