/* d-lang.c -- D frontend for GCC.
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

/* Implementation of back-end callbacks and data structures.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"

#include "dfrontend/globals.h"
#include "dfrontend/filename.h"
#include "dfrontend/lexer.h"
#include "dfrontend/target.h"
#include "dfrontend/expression.h"
#include "dfrontend/module.h"
#include "dfrontend/mtype.h"
#include "dfrontend/cond.h"

#include "id.h"
#include "d-frontend.h"

#include "opts.h"
#include "signop.h"
#include "hash-set.h"
#include "fixed-value.h"
#include "alias.h"
#include "flags.h"
#include "symtab.h"
#include "tree-core.h"
#include "stringpool.h"
#include "stor-layout.h"
#include "inchash.h"
#include "tree.h"
#include "debug.h"
#include "diagnostic.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "hash-map.h"
#include "is-a.h"
#include "plugin-api.h"
#include "vec.h"
#include "hashtab.h"
#include "machmode.h"
#include "tm.h"
#include "hard-reg-set.h"
#include "function.h"
#include "ipa-ref.h"
#include "dumpfile.h"
#include "cgraph.h"

/* Search prefix and multilib directories, passed by the driver.  */
static const char *iprefix_dir = NULL;
static const char *imultilib_dir = NULL;

/* Zero disables all standard directories for modules.  */
static bool std_inc = true;


/* Language-dependent contents of a type.  */

struct GTY(()) lang_type
{
  char dummy;
};

/* Language-dependent contents of a decl.  */

struct GTY((variable_size)) lang_decl
{
  char dummy;
};

/* Language-dependent contents of an identifier.
   This must include a tree_identifier.  */

struct GTY(()) lang_identifier
{
  struct tree_identifier common;
};

/* The resulting tree type.  */

union GTY((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE"),
	   chain_next ("CODE_CONTAINS_STRUCT (TREE_CODE (&%h.generic), TS_COMMON)"
		       " ? ((union lang_tree_node *) TREE_CHAIN (&%h.generic)) : NULL")))
lang_tree_node
{
  union tree_node GTY((tag ("0"),
		       desc ("tree_node_structure (&%h)"))) generic;
  struct lang_identifier GTY((tag ("1"))) identifier;
};

/* We don't use language_function.  */

struct GTY(()) language_function
{
  int dummy;
};

/* Language hooks.  */

static bool
d_langhook_init (void)
{
  /* This allows the code in d-builtins.c to not have to worry about
     converting (C signed char *) to (D char *) for string arguments of
     built-in functions.
     Parameters are (signed_char = false, short_double = false).  */
  build_common_tree_nodes (false, false);

  /* I don't know why this has to be done explicitly.  */
  void_list_node = build_tree_list (NULL_TREE, void_type_node);

  build_common_builtin_nodes ();

  /* This is the C main, not the D main.  */
  main_identifier_node = get_identifier ("main");

  /* Initialise the frontend.  */
  Lexer::initLexer ();
  Type::init ();
  Id::initialize ();
  Module::init ();
  Target::init ();
  Expression::init ();
  initPrecedence ();
  initTraitsStringTable ();

  /* Add all platform agnostic version conditions.  */
  VersionCondition::addPredefinedGlobalIdent ("GNU");
  VersionCondition::addPredefinedGlobalIdent ("D_Version2");

  if (BYTES_BIG_ENDIAN)
    VersionCondition::addPredefinedGlobalIdent ("BigEndian");
  else
    VersionCondition::addPredefinedGlobalIdent ("LittleEndian");

  /* LP64 only means 64bit pointers in D. */
  if (global.params.isLP64)
    VersionCondition::addPredefinedGlobalIdent ("D_LP64");

  /* Setting global.params.cov forces module info generation which is
     not needed for thee GCC coverage implementation.  Instead, just
     test flag_test_coverage while leaving global.params.cov unset. */
  if (flag_test_coverage)
    VersionCondition::addPredefinedGlobalIdent ("D_Coverage");
  if (flag_pic)
    VersionCondition::addPredefinedGlobalIdent ("D_PIC");
  if (global.params.doDocComments)
    VersionCondition::addPredefinedGlobalIdent ("D_Ddoc");
  if (global.params.useUnitTests)
    VersionCondition::addPredefinedGlobalIdent ("unittest");
  if (global.params.useAssert)
    VersionCondition::addPredefinedGlobalIdent("assert");
  if (!global.params.useArrayBounds)
    VersionCondition::addPredefinedGlobalIdent("D_NoBoundsChecks");

  VersionCondition::addPredefinedGlobalIdent ("all");

  /* Add all platform dependent version conditions.  */
#ifndef TARGET_CPU_D_BUILTINS
# define TARGET_CPU_D_BUILTINS()
#endif

#ifndef TARGET_OS_D_BUILTINS
# define TARGET_OS_D_BUILTINS()
#endif

# define builtin_define(TXT) \
  VersionCondition::addPredefinedGlobalIdent (TXT)

  TARGET_CPU_D_BUILTINS ();
  TARGET_OS_D_BUILTINS ();

  /* Insert all library-configured and user-defined import paths.  */
  d_frontend_add_paths (iprefix_dir, imultilib_dir, std_inc);

  return true;
}

/* Return language mask for option parsing.  */

static unsigned int
d_langhook_option_lang_mask (void)
{
  return CL_D;
}

/* Common initialization before calling option handlers.  */

static void
d_langhook_init_options (unsigned int, cl_decoded_option *decoded_options)
{
  d_frontend_init_options (lang_hooks.name, xstrdup (decoded_options[0].arg));
}

/* Handle D specific options.  Return false if the option is invalid.  */

static bool
d_langhook_handle_option (size_t scode, const char *arg, int value,
			  int kind ATTRIBUTE_UNUSED,
			  location_t loc ATTRIBUTE_UNUSED,
			  const cl_option_handlers *handlers ATTRIBUTE_UNUSED)
{
  opt_code code = (opt_code) scode;
  bool result = true;

  switch (code)
    {
    case OPT_imultilib:
      imultilib_dir = arg;
      break;

    case OPT_iprefix:
      iprefix_dir = arg;
      break;

    case OPT_I:
      global.params.imppath->push (arg);
      break;

    case OPT_J:
      global.params.fileImppath->push (arg);
      break;

    case OPT_nostdinc:
      std_inc = false;
      break;

    case OPT_Wall:
      if (value)
	global.params.warnings = 2;
      break;

    case OPT_Wdeprecated:
      global.params.useDeprecated = value ? 2 : 1;
      break;

    case OPT_Werror:
      if (value)
	global.params.warnings = 1;
      break;

    default:
      /* All other options not handled.  */
      break;
    }

  return result;
}

static void
d_langhook_parse_file (void)
{
  if (!main_input_filename || !main_input_filename[0])
    {
      error ("input file name required; cannot use stdin");
      return;
    }

  /* Start the main input file, if the debug writer wants it.  */
  if (debug_hooks->start_end_main_source_file)
    (*debug_hooks->start_source_file) (0, main_input_filename);

  d_frontend_parse_file (in_fnames, num_in_fnames);

  /* And end the main input file, if the debug writer wants it.  */
  if (debug_hooks->start_end_main_source_file)
    (*debug_hooks->end_source_file) (0);
}

static tree
d_langhook_type_for_mode (enum machine_mode mode, int unsignedp)
{
  if (mode == TYPE_MODE (float_type_node))
    return float_type_node;

  if (mode == TYPE_MODE (double_type_node))
    return double_type_node;

  if (mode == TYPE_MODE (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;

  if (mode == TYPE_MODE (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;

  if (COMPLEX_MODE_P (mode))
    {
      if (mode == TYPE_MODE (complex_float_type_node))
	return complex_float_type_node;
      if (mode == TYPE_MODE (complex_double_type_node))
	return complex_double_type_node;
      if (mode == TYPE_MODE (complex_long_double_type_node))
	return complex_long_double_type_node;
      if (mode == TYPE_MODE (complex_integer_type_node) && !unsignedp)
	return complex_integer_type_node;
    }

  /* gcc_unreachable */
  return NULL;
}

static tree
d_langhook_type_for_size (unsigned int bits ATTRIBUTE_UNUSED,
			  int unsignedp ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
  return NULL;
}

/* Record a builtin function.  We just ignore builtin functions.  */

static tree
d_langhook_builtin_function (tree decl)
{
  return decl;
}

static bool
d_langhook_global_bindings_p (void)
{
  gcc_unreachable ();
  return true;
}

static tree
d_langhook_pushdecl (tree decl ATTRIBUTE_UNUSED)
{
  gcc_unreachable ();
}

static tree
d_langhook_getdecls (void)
{
  return NULL;
}

static void
d_langhook_write_globals (void)
{
  /* This is the hook that runs the middle and backends: */
  symtab->finalize_compilation_unit ();
}

#undef LANG_HOOKS_NAME
#define LANG_HOOKS_NAME		"GNU D"

#undef LANG_HOOKS_INIT
#define LANG_HOOKS_INIT		d_langhook_init

#undef LANG_HOOKS_OPTION_LANG_MASK
#define LANG_HOOKS_OPTION_LANG_MASK	d_langhook_option_lang_mask

#undef LANG_HOOKS_INIT_OPTIONS
#define LANG_HOOKS_INIT_OPTIONS		d_langhook_init_options

#undef LANG_HOOKS_HANDLE_OPTION
#define LANG_HOOKS_HANDLE_OPTION	d_langhook_handle_option

#undef LANG_HOOKS_PARSE_FILE
#define LANG_HOOKS_PARSE_FILE		d_langhook_parse_file

#undef LANG_HOOKS_TYPE_FOR_MODE
#define LANG_HOOKS_TYPE_FOR_MODE	d_langhook_type_for_mode

#undef LANG_HOOKS_TYPE_FOR_SIZE
#define LANG_HOOKS_TYPE_FOR_SIZE	d_langhook_type_for_size

#undef LANG_HOOKS_BUILTIN_FUNCTION
#define LANG_HOOKS_BUILTIN_FUNCTION	d_langhook_builtin_function

#undef LANG_HOOKS_GLOBAL_BINDINGS_P
#define LANG_HOOKS_GLOBAL_BINDINGS_P	d_langhook_global_bindings_p

#undef LANG_HOOKS_PUSHDECL
#define LANG_HOOKS_PUSHDECL		d_langhook_pushdecl

#undef LANG_HOOKS_GETDECLS
#define LANG_HOOKS_GETDECLS		d_langhook_getdecls

#undef LANG_HOOKS_WRITE_GLOBALS
#define LANG_HOOKS_WRITE_GLOBALS	d_langhook_write_globals

struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

#include "gt-d-d-lang.h"
#include "gtype-d.h"
