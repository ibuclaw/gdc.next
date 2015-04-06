/* d-target.cc -- D frontend for GCC.
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

/* Implementation of Target data structure that describes a back-end target.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "dfrontend/target.h"
#include "dfrontend/aggregate.h"
#include "dfrontend/mtype.h"

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

int Target::ptrsize;
int Target::realsize;
int Target::realpad;
int Target::realalignsize;
bool Target::reverseCppOverloads;
int Target::c_longsize;
int Target::c_long_doublesize;


void
Target::init()
{
  /* Map D frontend type and sizes to GCC backend types.  */
  Target::ptrsize = (POINTER_SIZE / BITS_PER_UNIT);
  Target::realsize = int_size_in_bytes(long_double_type_node);
  Target::realpad = TYPE_PRECISION(long_double_type_node) / BITS_PER_UNIT;
  Target::realalignsize = TYPE_ALIGN_UNIT(long_double_type_node);
  Target::reverseCppOverloads = false;
  Target::c_longsize = int_size_in_bytes(long_integer_type_node);
  Target::c_long_doublesize = Target::realsize;

  /* Define what type to use for size_t, ptrdiff_t.  */
  size_t wordsize = int_size_in_bytes(size_type_node);
  if (wordsize == 2)
    Tsize_t = Tuns16;
  else if (wordsize == 4)
    Tsize_t = Tuns32;
  else if (wordsize == 8)
    Tsize_t = Tuns64;
  else
    gcc_unreachable();

  if (POINTER_SIZE == 16)
    Tptrdiff_t = Tint16;
  else if (POINTER_SIZE == 32)
    Tptrdiff_t = Tint32;
  else if (POINTER_SIZE == 64)
    Tptrdiff_t = Tint64;
  else
    gcc_unreachable();
}

/* Return GCC memory alignment size for type TYPE.  */

unsigned
Target::alignsize(Type* type)
{
  return (unsigned)type->size(Loc());
}

/* Return GCC field alignment size for type TYPE.  */

unsigned
Target::fieldalign(Type* type)
{
  return type->alignsize();
}

/* Return size of OS critical section.
   Can't use the sizeof() calls directly since cross compiling is supporte
   and would end up using the host sizes rather than the target sizes.  */

unsigned
Target::critsecsize()
{
  return 0;
}

/* Returns a Type for the va_list type of the target.  */

Type *
Target::va_listType()
{
  return NULL;
}

/* Perform a reinterpret cast of EXPR to type TYPE for use in CTFE.
   The front end should have already ensured that EXPR is a constant,
   so we just lower the value to GCC and return the converted CST.  */

Expression *
Target::paintAsType(Expression *e, Type *type)
{
  return NULL;
}

/* Return 0 if the given type is supported for this target.
   Other possible returns are:
   1: Not supported
   2: Wrong size
   3: Wrong base type  */

int
Target::checkVectorType(int sz, Type *type)
{
  return 0;
}

/* For the given module, perform any post parsing analysis.  */

void
Target::loadModule(Module *m)
{
}

