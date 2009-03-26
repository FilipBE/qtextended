# This file should be run like this:
# make -C "<build tree>" -f "<source tree>/src/build/mkconf/configure.mk" "SOURCE=<source tree>" "DEPOT=<1 or 0, extra checks enabled when 1>"

#
# Definitions (make the rules easier to read)
#

# This handles the dependencies behind running configure (somewhat bogus right now!)
configure=src/build/mkconf/configure
configuredeps=\
    $(SOURCE)/configure\
    $(SOURCE)/src/build/mkconf/configure.mk\
    $(SOURCE)/src/build/bin/configure\
    $(userargs)\
    $(mkconfargs)

# The user's arguments go here
userargs=src/build/mkconf/userargs

# mkconf can override the user's arguments
mkconfargs=src/build/mkconf/mkconfargs
force_build_mkconfargs=src/build/mkconf/force_build_mkconfargs
build_mkconfargs=src/build/mkconf/build_mkconfargs
# a bit convoluted... depend on the dependenies of our steps
build_mkconfargsdeps=\
    $(userargs)\
    $(SOURCE)/src/build/mkconf/configure.mk\
    $(force_build_mkconfargs)
# steps required to fulfil the dependencies above (executed in series)
build_mkconfargssteps=

#
# Establish the primary rule before we start pulling in overrides...
#

first: $(configure)
.PHONY: first

# Pull in stuff specific to depot builds (if we're building from the depot)
ifeq ($(DEPOT), 1)
include $(SOURCE)/src/build/mkconf/depot.mk
endif

#
# Rules
#

# run configure.
# once we've successfully configured, we want to force mkconfargs to get rebuilt.
$(configure): $(configuredeps)
	+eval "$(SOURCE)/src/build/bin/configure" $$(cat $(userargs)) $$(cat $(mkconfargs))
	touch $(force_build_mkconfargs)
	touch $@
# FIXME don't do this unconditionally (need to sort out more of the mkconf system before I can do that though)
.PHONY: $(configure)

# If there's no userargs file, create one.
$(userargs):
	touch $@

$(force_build_mkconfargs):
	touch $@

$(mkconfargs): $(build_mkconfargs)
	touch $@

# mkconfargs are cleared and then the dependencies cheks are performed in serial
# (to avoid problems with concurrent access).
$(build_mkconfargs): $(build_mkconfargsdeps)
	:> $(mkconfargs)
	for target in $(build_mkconfargssteps); do\
	    $(MAKE) -f "$(SOURCE)/src/build/mkconf/configure.mk" $$target "SOURCE=$(SOURCE)" "DEPOT=$(DEPOT)";\
	done
	echo "'-mkconf' '-make $(MAKE)'" >> $(mkconfargs)
	touch $@

# Don't let VIM expand tabs (it makes it too easy to introduce syntax errors)
# vim:noexpandtab
