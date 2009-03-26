#!/bin/sh

TOP=$PWD

# Current versions used
KERNEL_VERSION=2.6.14
LINUX=linux-$KERNEL_VERSION
LIDS=lids-2.2.2-$KERNEL_VERSION
LIDSTOOLS=lidstools-2.2.7

# Download URL's for the above packages (see build-inc.sh for local mirror setup)
LIDS_DL=http://www.lids.org/download/v2.6/$KERNEL_VERSION

# Uncomment for more verbose output
VERBOSE=1

test -z $SXE && SXE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1 ;
}

source $SXE/build-inc.sh || die "Could not load build-inc.sh - is \$SXE set correctly?" 

function get_source()
{
    test -f $LIDS.tar.gz || download $LIDS_DL/$LIDS.tar.gz
    msg "	Untarring lids"
    tar zvxf $LIDS.tar.gz | $TOP/progress $LOG   || die "Untarring $LIDS.tar.gz" ;
}

LOG="$TOP/SXE_Build.log"

# Build a little progress indicator
test -x $TOP/progress || gcc -o progress $SXE/progress.c

# Setup all the source trees
msg "Building the LIDs MAC userspace tools"
test -d $LIDS || get_source


ln -sf $LIDS lids
cd $TOP/$LIDS                 || die "cd $TOP/$LIDS"

# Later packages have the tools in a seperate download
if [ ! -d $LIDSTOOLS ]; then
    test -f $TOP/$LIDSTOOLS.tar.gz || download $LIDS_DL/$LIDSTOOLS.tar.gz
    tar zxf $LIDSTOOLS.tar.gz
fi

ln -sf $LIDSTOOLS lidstools   || die "ln -s $LIDSTOOLS lidstools"
cd $TOP/$LIDS/$LIDSTOOLS/src  || die "cd $LIDS/$LIDSTOOLS"
if [ ! -f .patched ]; then
    msg "	Applying lidstools patches"
    patch < $SXE/stdin_pw.patch                    || die "applying patch $SXE/stdin_pw.patch"
#    patch -p2 < $SXE/lidstools_troll.diff          || die "applying patch $SXE/lidstools_troll.diff"
    patch -p3 < $SXE/lids-2.2.1-2.6.13.patch       || die "applying patch lids-2.2.1-2.6.13.patch"
    touch .patched
fi
cd ..
msg "   configuring"
# ./configure KERNEL_DIR=$TOP/linux LDFLAGS=-static | $TOP/progress $LOG
echo "$PWD ./configure KERNEL_DIR=$TOP/linux | $TOP/progress $LOG"
test -f Makefile && make distclean
./configure KERNEL_DIR=$TOP/linux | $TOP/progress $LOG
msg "   compiling"
make | $TOP/progress $LOG

cat <<FINISHED

The lids tools are now ready to install.  Try:

    sudo \$SXE/install-lidstools.sh 
FINISHED
