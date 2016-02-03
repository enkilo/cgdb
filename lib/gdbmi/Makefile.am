<<<<<<< HEAD
noinst_LIBRARIES=libgdbmi.a

libgdbmi_a_SOURCES= \
    gdbmi_grammar.y \
    gdbmi_lexer.l \
    gdbmi_oc.c \
    gdbmi_parser.c \
    gdbmi_pt.c

EXTRA_DIST = \
    gdbmi_grammar.h \
    gdbmi_oc.h \
    gdbmi_parser.h \
    gdbmi_pt.h

# gdbmi driver program
noinst_PROGRAMS = gdbmi_driver

gdbmi_driver_LDADD = libgdbmi.a
gdbmi_driver_SOURCES = gdbmi_driver.c
=======
SUBDIRS=src
DIST_SUBDIRS=src

dist-hook:
	rm -rf `find $(distdir) -name .svn`
>>>>>>> 7af60661c707f873fc31f1755b892c7656c68f2e
