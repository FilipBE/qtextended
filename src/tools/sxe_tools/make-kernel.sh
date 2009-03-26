#!/bin/sh

TOP=$PWD

# Current versions used
LIDS=lids
LIDSTOOLS=lidstools
LINUX=linux

# Uncomment for more verbose output
VERBOSE=1

test -z $SXE && SXE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1
}

source $SXE/build-inc.sh || die "Could not load build-inc.sh - is \$SXE set correctly?" 

LOG="SXE_Build.log"

# Build a little progress indicator
test -f progress || gcc -o progress $SXE/progress.c

# Setup all the source trees
msg "Making the MAC SXE linux kernel (assumes sources downloaded and patched)"
test -d $LINUX   || die "$LINUX directory not found for make"
test -d $LIDS    || die "$LIDS directory not found for make"

LINUX=`readlink $LINUX`
LIDS=`readlink $LIDS`

cd $TOP/$LINUX                            || die "cd $LINUX"
test -f .config      || die "config file $LINUX/.config not found - do \"make menuconfig\" or similar"

msg "configuring kernel"
make oldconfig 2>&1 | ../progress $LOG
msg "compiling kernel"
make bzImage 2>&1 | ../progress $LOG

msg "compiling modules"
make modules 2>&1 | ../progress $LOG

msg "Building the LIDs MAC userspace tools"
cd $TOP/$LIDS                 || die "cd $TOP/$LIDS"
ln -s $LIDSTOOLS lidstools    || die "ln -s $LIDSTOOLS lidstools"
cd $TOP/$LIDS/$LIDSTOOLS      || die "cd $LIDS/$LIDSTOOLS"
msg "   configuring"
./configure KERNEL_DIR=../../$LINUX LDFLAGS=-static | ../../progress $LOG
msg "   compiling"
make | ../../progress $LOG

cat <<FINISHED

The kernel is now ready to install.  Try:

    sudo \$SXE/install-kernel.sh 
FINISHED
