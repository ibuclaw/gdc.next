/* d-port.cc -- D frontend for GCC.
   Copyright (C) 2011-2015 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* Implementation of Port wrapper around compiler/back-end specific things.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "dfrontend/port.h"

#include "tm.h"
#include "hash-set.h"
#include "machmode.h"
#include "vec.h"
#include "double-int.h"
#include "input.h"
#include "alias.h"
#include "symtab.h"
#include "inchash.h"
#include "tree.h"
#include "stor-layout.h"
#include "fold-const.h"
#include "real.h"


longdouble Port::ldbl_nan;
longdouble Port::snan;
longdouble Port::ldbl_infinity;
longdouble Port::ldbl_max;

/* Returns TRUE if longdouble value R is NaN.  */

int
Port::isNan(longdouble r)
{
  return __builtin_isnanl(r);
}

/* Same as isNan, but also check if is signalling.  */

int
Port::isSignallingNan(longdouble r)
{
  return __builtin_isnanl(r);
}

/* Returns TRUE if longdouble value is +Inf.  */

int
Port::isInfinity(longdouble r)
{
  return __builtin_isinfl(r);
}

longdouble
Port::fmodl(longdouble x, longdouble y)
{
  return __builtin_fmodl(x, y);
}

/* Returns TRUE if longdouble value X is identical to Y.  */

int
Port::fequal(longdouble x, longdouble y)
{
  return x == y;
}

char *
Port::strupr(char *s)
{
  char *t = s;

  while (*s)
    {
      *s = TOUPPER(*s);
      s++;
    }

  return t;
}

int
Port::memicmp(const char *s1, const char *s2, int n)
{
  int result = 0;

  for (int i = 0; i < n; i++)
    {
      char c1 = s1[i];
      char c2 = s2[i];

      result = c1 - c2;
      if (result)
	{
	  result = TOUPPER(c1) - TOUPPER(c2);
	  if (result)
	    break;
	}
    }

  return result;
}

int
Port::stricmp(const char *s1, const char *s2)
{
  int result = 0;

  for (;;)
    {
      char c1 = *s1;
      char c2 = *s2;

      result = c1 - c2;
      if (result)
	{
	  result = TOUPPER(c1) - TOUPPER(c2);
	  if (result)
	    break;
	}
      if (!c1)
	break;
      s1++;
      s2++;
    }

  return result;
}

/* Return a longdouble value from string BUFFER rounded to float mode.  */

longdouble
Port::strtof(const char *buffer, char **endp)
{
  return ldouble(::strtof(buffer, endp));
}

/* Return a longdouble value from string BUFFER rounded to double mode.  */

longdouble
Port::strtod(const char *buffer, char **endp)
{
  return ldouble(::strtod(buffer, endp));
}

/* Return a longdouble value from string BUFFER rounded to long double mode.  */

longdouble
Port::strtold(const char *buffer, char **endp)
{
  return ldouble(::strtold(buffer, endp));
}

