#!/bin/sh

TOP=$PWD

# The file system image
test -z $FILESYS && FILESYS=$TOP/fs

test -z $VTE && VTE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1;
}

# The operations need root privileges
test `id -u` == 0 || die "Run this script as root"

test -d $FILESYS || die "Expected to find \"$FILESYS\" directory containing file-system"
test -d $TOP/lids || die "Expected to find \"$TOP/lids\" directory containing lids user-space tools"
test -f $TOP/lids/lidstools/src/lidsadm || die "$TOP/lids/lidstools/src/lidsadm does not exist, run \$VTE/build-lidstools.sh ..?"

echo "Installing the LIDs MAC userspace tools"
mkdir -p $FILESYS/sbin                                    || die "mkdir $FILESYS/sbin"
cp -uf $TOP/lids/lidstools/src/lidsadm $TOP/lids/lidstools/src/lidsconf $FILESYS/sbin   || die "copying lids tools"

cat <<SUCCESS

lids tools successfully installed

Now run sudo \$VTE/build.sh to create the new VM
SUCCESS
