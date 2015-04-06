/* d-frontend.h -- D frontend for GCC.
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

#ifndef GCC_D_FRONTEND_H
#define GCC_D_FRONTEND_H

/* dfrontend/traits.c  */
void initTraitsStringTable (void);

/* d-frontend.cc  */
void d_frontend_init_options (const char *, const char *);
void d_frontend_parse_file (const char **, size_t);

/* d-incpath.c  */
void d_frontend_add_paths (const char *, const char *, bool);

#endif	// GCC_D_FRONTEND_H

