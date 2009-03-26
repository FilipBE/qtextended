#!/bin/sh
set -ex
TARBALLS_DIR=${TARBALLS_DIR-$HOME/downloads}
RESULT_TOP=${RESULT_TOP-/opt/toolchains/greenphone}
export TARBALLS_DIR RESULT_TOP
GCC_LANGUAGES="c,c++"
export GCC_LANGUAGES

# Really, you should do the mkdir before running this,
# and chown /opt/crosstool to yourself so you don't need to run as root.
mkdir -p $RESULT_TOP

# Build the toolchain.  Takes a couple hours and a couple gigabytes.

 eval `cat greenphone.dat gcc-4.1.1-glibc-2.3.6.dat`  sh all.sh --gdb --notest

echo Done.
