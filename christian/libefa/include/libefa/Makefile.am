AM_CFLAGS = @LIBEFA_CFLAGS@ @WARN_CFLAGS@ $(LIBXML_CFLAGS)

includedir = @includedir@/libefa

include_HEADERS = efa.h \
		  errno.h \
		  error.h \
                  list.h \
                  memory.h

if BUECHI
include_HEADERS += buechi.h
endif

if FADOT
include_HEADERS += fadot.h
endif

if FAMOD
include_HEADERS += famod.h
endif

if FAXML
include_HEADERS += faxml.h
endif

if HASH
include_HEADERS += hash.h
endif
