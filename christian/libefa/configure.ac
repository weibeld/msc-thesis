# Initialize Autoconf
AC_PREREQ([2.69])
AC_INIT([Extended Finite Automaton Library], [0.3], 
[christian.goettel@unifr.ch], [libefa], [])

# Enable extensions to C or Posix
AC_USE_SYSTEM_EXTENSIONS

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/efa.c])
AM_INIT_AUTOMAKE([color-tests parallel-tests])
AM_SILENT_RULES([yes]) # make --enable-silent-rules the default.

# Support for module buechi
AC_ARG_ENABLE([buechi],
  [AS_HELP_STRING([--enable-buechi],
	 [enable support for Büchi automata @<:@default=yes@:>@])],
  [],
  [enable_buechi=yes])

AM_CONDITIONAL([BUECHI], [test "x$enable_buechi" != "xno"])

AS_IF([test "x$enable_buechi" != "xno"],
   [AC_DEFINE([HAVE_MODULE_BUECHI], [1], [Define if you have module buechi])])

# Support for module fadot
AC_ARG_ENABLE([fadot],
  [AS_HELP_STRING([--enable-fadot],
	 [enable support for DOT files @<:@default=yes@:>@])],
  [],
  [enable_fadot=yes])

AM_CONDITIONAL([FADOT], [test "x$enable_fadot" != "xno"])

AS_IF([test "x$enable_fadot" != "xno"],
  [AC_DEFINE([HAVE_MODULE_FADOT], [1], [Define if you have module fadot])])

# Support for module famod
AC_ARG_ENABLE([famod],
  [AS_HELP_STRING([--enable-famod],
	 [enable support for normal finite automata @<:@default=yes@:>@])],
  [],
  [enable_famod=yes])

AM_CONDITIONAL([FAMOD], [test "x$enable_famod" != "xno"])

AS_IF([test "x$enable_famod" != "xno"],
  [AC_DEFINE([HAVE_MODULE_FAMOD], [1], [Define if you have module famod])])

# Support for module faxml
AC_ARG_ENABLE([faxml],
  [AS_HELP_STRING([--enable-faxml],
	 [enable support for FAXML files @<:@default=yes@:>@])],
  [],
  [enable_faxml=yes])

AM_CONDITIONAL([FAXML], [test "x$enable_faxml" != "xno"])

AS_IF([test "x$enable_faxml" != "xno"],
   [AC_DEFINE([HAVE_MODULE_FAXML], [1], [Define if you have module faxml])])

# Support for module hash
AC_ARG_ENABLE([hash],
  [AS_HELP_STRING([--enable-hash],
	 [enable support for HAHS data type @<:@default=yes@:>@])],
  [],
  [enable_hash=yes])

AM_CONDITIONAL([HASH], [test "x$enable_hash" != "xno"])

AS_IF([test "x$enable_hash" != "xno"],
   [AC_DEFINE([HAVE_MODULE_HASH], [1], [Define if you have module hash])])

# Support for module red-black-tree
AC_ARG_ENABLE([rbtree],
  [AS_HELP_STRING([--enable-rbtree],
	 [enable support for red-black-tree data type @<:@default=yes@:>@])],
  [],
  [enable_rbtree=yes])

AM_CONDITIONAL([RBTREE], [test "x$enable_rbtree" != "xno"])

AS_IF([test "x$enable_rbtree" != "xno"],
   [AC_DEFINE([HAVE_MODULE_RBTREE], [1], [Define if you have module rbtree])])

# Check for pdflatex
AC_ARG_WITH([pdflatex],
  [AS_HELP_STRING([--with-pdflatex],
    [whether to use pdflatex to build PDF documentation @<:@default=check@:>@])],
  [],
  [with_pdflatex=check])

PDFLATEX=
AS_IF([test "x$whith_pdflatex" != "xno"],
  [AC_PATH_PROG([PDFLATEX], [pdflatex], [no])])
if test "x$PDFLATEX" = xno; then
  AC_MSG_FAILURE(
	 [--with-pdflatex was given, but pdflatex could not be found])
fi
AC_SUBST([PDFLATEX])

# Support for memory test with failmalloc
AC_ARG_WITH([failmalloc],
  [AS_HELP_STRING([--with-failmalloc=FAILMALLOC],
	 [enable failmalloc test targets and use the failmalloc library FAILMALLOC @<:@default=no@:>@])],
  [AC_SUBST([LIBFAILMALLOC], ["$with_failmalloc"])],
  [with_failmalloc=no])

AM_CONDITIONAL([WITH_FAILMALLOC], [test "x$with_failmalloc" != "xno"])

# Check if at least one data type is enabled
AS_IF([test "x$enable_hash" = "xno"],
  [AS_IF([test "x$enable_rbtree" = "xno"],
	 [AC_MSG_ERROR([At least one data type (hash, rbtree) must be defined.])]
  )]
)

# Version info in libtool's versioning system notation
AC_SUBST([LIBEFA_VERSION_INFO], [0:0:0])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_AR

# autoconf wants m4_foreach_w to be used
m4_ifndef([m4_foreach_w],
  [m4_define([m4_foreach_w],
	 [m4_foreach([$1], m4_split(m4_normalize([$2]), [ ]), [$3])])])

LT_INIT([disable-fast-install shared disable-static])
AC_SUBST([LIBTOOL_DEPS])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([assert.h ctype.h errno.h getopt.h limits.h locale.h math.h \
                  memory.h setjmp.h stddef.h stdint.h stdio.h stdlib.h \ 
                  string.h strings.h time.h unistd.h])

AC_MSG_CHECKING(for xmmintrin.h)
AC_LINK_IFELSE([
  AC_LANG_PROGRAM([[
#include <xmmintrin.h>
__m128 testfunc(float *a, float *b) {
  return _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b));
}
  ]])],
  [has_sse=yes],
  [has_sse=no])
AC_MSG_RESULT($has_sse)

if test "$has_sse" = yes; then
  AC_DEFINE([HAVE_XMMINTRIN_H], [1], [Enable SSE support])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_LONG_LONG_INT
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINTMAX_T
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([fprintf memmove printf strchr strcmp strdup strncmp strndup \
strrchr strtol strtoul])

AC_SYS_LARGEFILE

LIBEFA_COMPILE_WARNINGS([maximum])

# Compiler flags to be used everywhere
LIBEFA_CFLAGS=-std=gnu99
AC_SUBST([LIBEFA_CFLAGS])

AC_MSG_CHECKING([how to pass version script to the linker ($LD)])
VERSION_SCRIPT_FLAGS=none
if $LD --help 2>&1 | grep "version-script" >/dev/null 2>/dev/null; then
    VERSION_SCRIPT_FLAGS=-Wl,--version-script=
elif $LD --help 2>&1 | grep "M mapfile" >/dev/null 2>/dev/null; then
    VERSION_SCRIPT_FLAGS="-Wl,-M -Wl,"
fi
AC_MSG_RESULT([$VERSION_SCRIPT_FLAGS])
AC_SUBST([VERSION_SCRIPT_FLAGS])
AM_CONDITIONAL([USE_VERSION_SCRIPT], [test "$VERSION_SCRIPT_FLAGS" != none])

AS_IF([test "x$enable_faxml" != "xno"],
		[PKG_PROG_PKG_CONFIG
		 PKG_CHECK_MODULES([LIBXML], [libxml-2.0])])

AC_CONFIG_FILES([Makefile
                include/Makefile
                include/libefa/Makefile
                src/Makefile
                src/famod/Makefile
                src/buechi/Makefile
                man/Makefile
                tests/Makefile
                etc/Makefile
                doc/Makefile
                libefa.pc])

AC_OUTPUT
