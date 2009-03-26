# This file should be sourced something like this:
# if [ -f "$(dirname $0)/../src/build/bin/getpaths.sh" ]; then . "$(dirname $0)/../src/build/bin/getpaths.sh"; else echo "Could not find getpaths.sh"; exit 1; fi

###
### WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
###
### This code must be kept in sync with src/build/bin/Qtopia/Paths.pm
###
### WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
###

# Try to figure out where the bulid tree is
QPEDIR=
depotpath=

do_check()
{
    # Locate config.cache so we can be sure that we've actually found QPEDIR
    while [ -n "$check" ]; do
        if [ -f "$check/config.cache" ]; then
            export QPEDIR=$check
            return 0
        fi
        if [ -f "$check/configure" ]; then
            export depotpath=$check
            return 1
        fi
        if [ "$check" = "$(dirname $check)" ]; then
            return 1
        fi
        check="$(dirname $check)"
    done
    return 1
}

for check in "$(dirname $0)", "$PWD", "$(/bin/pwd)"; do
    if [ -n "$check" ] && do_check; then
        break
    fi
done

# The SDK defaults to $QPEDIR (but it can be changed)
SDKROOT="$QPEDIR"
if [ -z "$QPEDIR" ]; then
    echo "ERROR: Could not locate the Qtopia build tree."
    echo "       Did you run configure?"
    if [ -n "$depotpath" ]; then
        altscript="$(echo $0 | perl -pe '$depotpath = "'$depotpath'"; s/\Q$depotpath\E//; s/^\///;')"
        echo "       Please try running $(basename $0) from the build tree."
        echo "       eg. /path/to/build/$altscript"
    fi
    exit 1
fi

# Use the values in config.cache
commands="$(grep '^paths\.' $QPEDIR/config.cache | sed 's/^paths\./export /' | sed 's/=/="/' | sed 's/$/"/')"
eval "$commands"

