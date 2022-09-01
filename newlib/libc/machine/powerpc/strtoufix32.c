/*
Copyright (c) 1984,2000 S.L. Moshier

Permission to use, copy, modify, and distribute this software for any
purpose without fee is hereby granted, provided that this entire notice
is included in all copies of any software which is or includes a copy
or modification of this software and in all copies of the supporting
documentation for such software.

THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
WARRANTY.  IN PARTICULAR,  THE AUTHOR MAKES NO REPRESENTATION
OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY OF THIS
SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */
#ifdef __SPE__ 

#include <_ansi.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <reent.h>
#include "vfieeefp.h"

/*
 * Convert a string to a fixed-point 32-bit value.
 *
 * Ignores `locale' stuff.
 */
__uint32_t
_strtoufix32 (
	const char *nptr,
	char **endptr)
{
  union double_union dbl;
  int exp, negexp;
  __uint32_t tmp, tmp2, result = 0;

  dbl.d = strtod (nptr, endptr);

  /* treat NAN as domain error, +/- infinity as saturation */
  if (!finite(dbl.d))
    {
      if (isnan (dbl.d))
	{
	  _REENT_ERRNO(rptr) = EDOM;
	  return 0;
	}
      _REENT_ERRNO(rptr) = ERANGE;
      if (word0(dbl) & Sign_bit)
	return 0;
      return ULONG_MAX;
    }

  /* check for normal saturation */
  if (dbl.d >= 1.0)
    {
      _REENT_ERRNO(rptr) = ERANGE;
      return ULONG_MAX;
    }
  else if (dbl.d < 0)
    {
      _REENT_ERRNO(rptr) = ERANGE;
      return 0;
    }

  /* otherwise we have normal positive number in range */

  /* strip off exponent */
  exp = ((word0(dbl) & Exp_mask) >> Exp_shift) - Bias;
  negexp = -exp;
  if (negexp > 32)
    return 0;
  word0(dbl) &= ~(Exp_mask | Sign_bit);
  /* add in implicit normalized bit */
  word0(dbl) |= Exp_msk1;
  /* shift so result is contained left-justified in word */
  tmp = word0(dbl) << Ebits;
  tmp |= ((unsigned long)word1(dbl) >> (32 - Ebits));
  /* perform rounding */
  if (negexp > 1)
    {
      tmp2 = tmp + (1 << (negexp - 2));
      result = (tmp2 >> (negexp - 1));
      /* if rounding causes carry, add carry bit in */
      if (tmp2 < tmp)
	result += 1 << (32 - negexp);
    }
  else
    {
      result = tmp + ((word1(dbl) & (1 << (32 - Ebits - 1))) != 0);
      /* if rounding causes carry, then saturation has occurred */
      if (result < tmp)
	{
	  _REENT_ERRNO(rptr) = ERANGE;
	  return ULONG_MAX;
	}
    }

  return result;
}

#endif /* __SPE__ */
