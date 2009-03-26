#!/bin/sh

TOP=$PWD

if [ -f SXE_CONFIG ]; then
    source SXE_CONFIG
fi

# Current versions used
test -z $KERNEL_VERSION && KERNEL_VERSION=2.6.14             && echo "KERNEL_VERSION=$KERNEL_VERSION"         >SXE_CONFIG
test -z $LINUX          && LINUX=linux-$KERNEL_VERSION       && echo "LINUX=linux-\$KERNEL_VERSION"           >>SXE_CONFIG
test -z $LIDS_VERSION   && LIDS_VERSION=2.2.2                && echo "LIDS_VERSION=$LIDS_VERSION"             >>SXE_CONFIG
test -z $LIDS           && LIDS=lids-2.2.2-$KERNEL_VERSION   && echo "LIDS=lids-\$LIDS_VERSION-\$KERNEL_VERSION"   >>SXE_CONFIG
test -z $LIDSTOOLS      && LIDSTOOLS=lidstools-2.2.7         && echo "LIDSTOOLS=$LIDSTOOLS"                   >>SXE_CONFIG

if [ "$1xx" = "--configxx" ]; then
    echo "SXE_CONFIG file created.  Please edit for your kernel"
    exit 0
fi

# Download URL's for the above packages (see build-inc.sh for local mirror setup)
LIDS_DL=http://www.lids.org/download/v2.6/$KERNEL_VERSION
LINUX_DL=http://public.planetmirror.com/pub/linux/kernel/v2.6

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

LOG="$TOP/SXE_Build.log"

if [ -d $LINUX -a -f $LINUX/.config ]; then
    cat <<PROMPT
This script will overwrite the current Linux build tree (found
in $LINUX directory) with a new build.  If that is not wanted,
press <ctrl-c> now to abort.

Try \$SXE/make-kernel.sh to just build the existing kernel tree.
PROMPT
    read -p "Continue? "
fi

# Build a little progress indicator
gcc -o progress $SXE/progress.c

test -f $LINUX.tar.bz2 || download $LINUX_DL/$LINUX.tar.bz2
test -f $LIDS.tar.gz || download $LIDS_DL/$LIDS.tar.gz

# Setup all the source trees
msg "Building the MAC SXE linux kernel"
test -d $LINUX && ( rm -Rf $LINUX || die "Removing stale $LINUX directory" )
test -d $LIDS && ( rm -Rf $LIDS   || die "Removing stale $LIDS directory" )

msg "	Untarring lids"
tar zvxf $LIDS.tar.gz | ./progress $LOG   || die "Untarring $LIDS.tar.gz"

msg "	Untarring linux"
tar xvjf $LINUX.tar.bz2 | ./progress $LOG 19500 || die "Untarring $LINUX.tar.bz2"

ln -sf $LINUX linux
ln -sf $LIDS lids

cd $TOP/$LINUX                                 || die "cd $LINUX"

#apply apprpriate set of kernel patches
patch -p1 < $TOP/$LIDS/$LIDS.patch             || die "applying patch $TOP/$LIDS/$LIDS.patch"

cp $SXE/configs/lids-vm-$LINUX.config .config  || die "copy $SXE/configs/lids-vm-$LINUX.config"

if [ "$1xx" = "--installxx" ]; then
    echo
    echo "SXE kernel sources setup.  Please configure for your architecture"
    echo "Then run \"\$SXE/make-kernel.sh\""
    echo
    exit 0
fi

msg "configuring kernel"
make oldconfig 2>&1 | ../progress $LOG
msg "compiling kernel"
make bzImage 2>&1 | ../progress $LOG

msg "compiling modules"
make modules 2>&1 | ../progress $LOG

cd $TOP

$SXE/build-lidstools.sh

cat <<FINISHED

The kernel is now ready to install.  Try:

    sudo \$SXE/install-kernel.sh 
FINISHED
