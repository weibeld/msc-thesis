ACLOCAL_AMFLAGS = -I m4

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	 $(SHELL) ./config.status libtool

SUBDIRS=include src tests doc man etc

EXTRA_DIST=Makefile.am HACKING

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libefa.pc

dicclean-local:
	 -find $(top_builddir)/build/* -maxdepth 0 -not | xargs rm -rf

ChangeLog:
	 if test -d $(top_srcdir)/.git; then
		$(top_srcdir)/gitlog-to-changelog > $@;
	 fi

dist: ChangeLog

.PHONY: ChangeLog
