# The difference between SUBDIRS and DIST_SUBDIRS is that the
# readline directory simply needs to be copied as is, where as
# the rest of the directories have a make dist rule run on them
<<<<<<< HEAD
SUBDIRS=lib cgdb doc test
DIST_SUBDIRS=lib cgdb doc test
=======
SUBDIRS=lib various tgdb cgdb doc test
DIST_SUBDIRS=lib various tgdb cgdb doc test
>>>>>>> 7af60661c707f873fc31f1755b892c7656c68f2e

# The order is important here.
# various is the lowest level, so it must be done first.
# TGDB uses various, so it is done next.
# CGDB uses both various and TGDB, so it is done last.
doxygen:
<<<<<<< HEAD
	cd lib; doxygen Doxyfile
	cd lib/tgdb; doxygen Doxyfile
	cd cgdb; doxygen Doxyfile
=======
	cd various; doxygen Doxyfile
	cd tgdb; doxygen Doxyfile
	cd cgdb; doxygen Doxyfile

dist-hook:
	rm -rf `find $(distdir) -name .svn`
>>>>>>> 7af60661c707f873fc31f1755b892c7656c68f2e
