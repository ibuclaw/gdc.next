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

