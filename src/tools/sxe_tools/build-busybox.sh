#!/bin/sh

# Current versions used
BUSYBOX=busybox-1.2.1

# Download URL's for the above packages
BUSYBOX_DL=http://www.busybox.net/downloads

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1
}

function usage()
{
    echo "$1"
    echo "Usage:"
    echo "   export SXE=/path/to/sxe_tools"
    echo "   \$SXE/build-busybox.sh"
    exit 1;
}

test -z $SXE && SXE=`dirname $0`

source $SXE/build-inc.sh || usage "Could not load build-inc.sh"

TOP=$PWD

LOG="$TOP/SXE_Build.log"

VERBOSE=1

# Build a little progress indicator
test -x progress || gcc -o progress $SXE/progress.c

if [ -z $CROSS ]; then
    # default to in-house i686 qemu toolchain
    TOOLBIN=/opt/toolchains/i686/gcc-3.4.3-glibc-2.3.4/i686-linux/bin
    if [ -x $TOOLBIN/i686-linux-gcc ]; then
        export PATH=$TOOLBIN:$PATH
        CROSS=$TOOLBIN/i686-linux-
    fi
fi
if [ -z $CROSS ]; then
    echo "Using no cross-compiler.  If needed do, for example:"
    echo "    export PATH=/path/to/cross/bin:\$PATH"
    echo "    export CROSS=/path/to/cross/bin/arm-linux-"
fi

msg "Building the Busybox user-space utilities"
test -f $BUSYBOX.tar.gz || download $BUSYBOX_DL/$BUSYBOX.tar.gz
test -d $BUSYBOX && rm -Rf $BUSYBOX

TEMPDIR=$TOP/temp-busybox
mkdir $TEMPDIR                           || die "mkdir $TEMPDIR"

# 2 versions of busybox are needed: one with privileges for system functions
# and one non-privileged for user functions.  The privileged version is symlinked
# for all the applets in the /sbin/ and /usr/sbin/ directories, eg mount, syslogd;
# and the non-privileged version is everything else.
for CONF in sbin no-sbin; do
    if [ ! -d $BUSYBOX-$CONF ]; then
        tar zxf $BUSYBOX.tar.gz              || die "Untarring $BUSYBOX.tar.gz"
        mv $BUSYBOX $BUSYBOX-$CONF
    fi
    cd $BUSYBOX-$CONF                    || die "cd $BUSYBOX"
    CONFFILE=$SXE/configs/busybox-$CONF.config
    cp -v $CONFFILE .config              || die "copying $CONFFILE -> .config"
    make CROSS=$CROSS oldconfig 2>&1 >/dev/null
    msg "compiling busybox"
    make CROSS=$CROSS 2>&1 | ../progress $LOG
    mkdir $TEMPDIR-$CONF                 || die "mkdir $TEMPDIR-$CONF"
    make PREFIX=$TEMPDIR-$CONF install
    cd $TEMPDIR-$CONF
    mv bin/busybox bin/busybox-$CONF
    find . -type l -exec env CF=$CONF LL=\{} sh -c \
        'tgt=`readlink $LL`; dir=`dirname $LL`; nm=`basename $LL`; rm -f $LL; (cd $dir && ln -s $tgt-$CF $nm)' \;
    cd $TOP
    (cd $TEMPDIR-$CONF && tar cf - .)|(cd $TEMPDIR && tar xvfp -)
    rm -Rf $TEMPDIR-$CONF
done

echo <<SUCCESS

Busybox has been successfully built for SXE.  Now install to eg "fs" by typing:

    (cd $TEMPDIR && tar cf - .)|(cd fs && sudo tar xvfp -)

SUCCESS
