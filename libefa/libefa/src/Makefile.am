AM_CPPFLAGS = -DDATADIR='"$(datadir)"' -I $(top_builddir)/include
AM_CFLAGS = @LIBEFA_CFLAGS@ @WARN_CFLAGS@ $(LIBXML_CFLAGS)

EXTRA_DIST = efa_sym.version

SUBDIRS =

if BUECHI
SUBDIRS += buechi
endif

if FAMOD
SUBDIRS += famod
endif

bin_PROGRAMS = efatool

lib_LTLIBRARIES = libefa.la

if USE_VERSION_SCRIPT
  LIBEFA_VERSION_SCRIPT = $(VERSION_SCRIPT_FLAGS)$(srcdir)/efa_sym.version
else
  LIBEFA_VERSION_SCRIPT = 
endif

libefa_la_LDFLAGS = $(LIBEFA_VERSION_SCRIPT) -version-info $(LIBEFA_VERSION_INFO)
libefa_la_LIBADD = 
libefa_la_SOURCES = efa.c $(top_builddir)/include/libefa/efa.h \
		    $(top_builddir)/include/libefa/errno.h \
		    error.c $(top_builddir)/include/libefa/error.h \
                    $(top_builddir)/include/list.h \
                    memory.c $(top_builddir)/include/libefa/memory.h

if BUECHI
libefa_la_SOURCES += $(srcdir)/buechi/buechi.c \
                     $(top_builddir)/include/libefa/buechi.h
endif

if FADOT
libefa_la_SOURCES += fadot.c \
                     $(top_builddir)/include/libefa/fadot.h
endif

if FAMOD
libefa_la_SOURCES += $(srcdir)/famod/famod.c \
                     $(top_builddir)/include/libefa/famod.h
endif

if FAXML
libefa_la_LIBADD += $(LIBXML_LIBS)
libefa_la_SOURCES += faxml.c $(top_builddir)/include/libefa/faxml.h
endif


if HASH
libefa_la_SOURCES += $(top_builddir)/include/libefa/hash.h hash.c \
		     $(srcdir)/hash/additive.h $(srcdir)/hash/additive.c \
		     $(srcdir)/hash/bernstein.h $(srcdir)/hash/bernstein.c \
		     $(srcdir)/hash/cessu.h $(srcdir)/hash/cessu.c \
		     $(srcdir)/hash/crc.h $(srcdir)/hash/crc.c \
		     $(srcdir)/hash/gencrc.h $(srcdir)/hash/gencrc.c \
		     $(srcdir)/hash/MurmurHash1.h $(srcdir)/hash/MurmurHash1.c \
		     $(srcdir)/hash/MurmurHash2.h $(srcdir)/hash/MurmurHash2.c \
		     $(srcdir)/hash/MurmurHash3.h $(srcdir)/hash/MurmurHash3.c \
		     $(srcdir)/hash/one-at-a-time.h $(srcdir)/hash/one-at-a-time.c \
		     $(srcdir)/hash/paul_hsieh.h $(srcdir)/hash/paul_hsieh.c \
		     $(srcdir)/hash/rotating.h $(srcdir)/hash/rotating.c
AM_CFLAGS += -msse2
endif

efatool_SOURCES = efatool.c
efatool_LDADD = libefa.la

include $(top_srcdir)/Makefile.inc
