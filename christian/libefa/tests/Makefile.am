AM_CPPFLAGS = -DDATADIR='"$(datadir)"' -I$(top_builddir)/include -I$(top_builddir)/src
AM_CFLAGS = @LIBEFA_CFLAGS@ @WARN_CFLAGS@ $(LIBXML_CFLAGS)

#ME = tests/Makefile.am

check_PROGRAMS = efatests

TESTS = $(check_PROGRAMS)
TEST_EXTENSIONS = 

efatests_SOURCES = CuTest.c CuTest.h efatests.c efatooltests.h efatooltests.c
efatests_LDADD = $(top_builddir)/src/libefa.la $(LIBXML_LIBS)

# FAILMALLOC_START ?= 1
# FAILMALLOC_REP   ?= 20
# FAILMALLOC_PROG ?= ./fatest

include $(top_srcdir)/Makefile.inc

.PHONY: valgrind
