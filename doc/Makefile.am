info_TEXINFOS = cgdb.texinfo
<<<<<<< HEAD
cgdb_TEXINFOS = gpl.texi

dist_man_MANS = cgdb.1
EXTRA_DIST = cgdb.txt
=======
cgdb_TEXINFOS = gpl.texi fdl.texi

dist_man_MANS = cgdb.1
EXTRA_DIST = cgdb.txt htdocs
>>>>>>> 7af60661c707f873fc31f1755b892c7656c68f2e

dist_pkgdata_DATA = cgdb.txt

# generate the man page using help2man.
cgdb.1:
<<<<<<< HEAD
	help2man --output=$(top_srcdir)/doc/cgdb.1 $(top_builddir)/cgdb/cgdb$(EXEEXT)
=======
	help2man --output=$(top_srcdir)/doc/cgdb.1 $(top_builddir)/cgdb/src/cgdb$(EXEEXT)
>>>>>>> 7af60661c707f873fc31f1755b892c7656c68f2e

cgdb.txt: cgdb.texinfo $(srcdir)/version.texi $(cgdb_TEXINFOS)
TEXTS = cgdb.txt
text-am: $(TEXTS)
MAKEINFOTXT = $(MAKEINFO) --plaintext
AM_MAKEINFOTXTFLAGS = $(AM_MAKEINFOFLAGS)

.texinfo.txt:
	rm -rf $(@:.txt=.htp)
	if $(MAKEINFOTXT) $(AM_MAKEINFOTXTFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) \
	-o $(@:.txt=.htp) $<; \
        then \
          rm -rf $@; \
          if test ! -d $(@:.txt=.htp) && test -d $(@:.txt=); then \
            mv $(@:.txt=) $@; else mv $(@:.txt=.htp) $@; fi; \
        else \
          if test ! -d $(@:.txt=.htp) && test -d $(@:.txt=); then \
            rm -rf $(@:.txt=); else rm -Rf $(@:.txt=.htp) $@; fi; \
          exit 1; \
        fi
