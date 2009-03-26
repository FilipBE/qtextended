#
# Definitions (make the rules easier to read)
#

# a bit convoluted... depend on the dependenies of our steps
build_mkconfargsdeps+=\
    $(SOURCE)/src/build/mkconf/depot.mk\
    $(forcecleanbuilddeps)
# steps required to fulfil the dependencies above (executed in series)
build_mkconfargssteps+=\
    $(forcecleanbuild)

# This is a big hammer that forces everyone to do a clean build
forcecleanbuild=src/build/mkconf/force_clean_build
forcecleanbuilddeps=\
    $(SOURCE)/force_clean_build

#
# Rules
#

# Force a clean build if requested to do so.
$(forcecleanbuild): $(forcecleanbuilddeps)
	echo "'-mkconf' '-no-skip-qt'" >> $(mkconfargs)
	echo "'-mkconf' '-clean'" >> $(mkconfargs)
	touch $@

# Don't let VIM expand tabs (it makes it too easy to introduce syntax errors)
# vim:noexpandtab
