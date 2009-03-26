#!/bin/sh

TOP=$PWD

NEW_KERNEL=$1

# This is produced by "make bzImage" in the build-kernel.sh script
test -z $KERNEL_IMAGE && KERNEL_IMAGE=arch/i386/boot/bzImage

# The file system image
test -z $FILESYS && FILESYS=$TOP/fs

test -z $SXE && SXE=`dirname $0`

# Like perl's die function
function die()
{
    echo "Fatal error: $1"
    exit 1;
}

# The operations need root privileges
test `id -u` == 0 || die "Run this script as root"

test -d $FILESYS || die "Expected to find \"$FILESYS\" directory containing file-system"

test -d $TOP/linux || die "Expected to find \"$TOP/linux\" directory containing kernel build"
test -f $TOP/linux/$KERNEL_IMAGE || die "$TOP/linux/$KERNEL_IMAGE does not exist - run build-kernel.sh ..?"

test -d $TOP/lids || die "Expected to find \"$TOP/lids\" directory containing lids user-space tools"
test -f $TOP/lids/lidstools/src/lidsadm || die "$TOP/lids/lidstools/src/lidsadm does not exist, run build-kernel.sh ..?"

source linux/.config
KERNEL_VERSION=`readlink linux`
KERNEL_VERSION=${KERNEL_VERSION##linux-}
VERSION=$KERNEL_VERSION$CONFIG_LOCALVERSION

echo "Installing kernel version $VERSION in $FILESYS"

test -z $NEW_KERNEL && NEW_KERNEL=vmlinuz-$VERSION

mkdir -p $FILESYS/boot || die "mkdir -p $FILESYS/boot"
cp -uf linux/$KERNEL_IMAGE $FILESYS/boot/$NEW_KERNEL || die "Could not install kernel as $FILESYS/boot/$NEW_KERNEL"
cp -uf linux/System.map $FILESYS/boot/System.map-$VERSION || die "Could not install System.map as $FILESYS/boot/System.map-$VERSION"

TEMPDIR=$TOP/.temp_module_install

test -d $TEMPDIR && rm -Rf $TEMPDIR
mkdir -p $TEMPDIR  || die "mkdir -p $TEMPDIR"

cd linux || die "cd linux kernel directory"
make modules_install INSTALL_MOD_PATH=$TEMPDIR 2>&1 >/dev/null

mkdir -p $FILESYS/lib/modules/$VERSION
cp $TEMPDIR/lib/modules/$VERSION/modules.dep $FILESYS/lib/modules/$VERSION
cat /dev/null >$FILESYS/etc/modules
for MODULE in `cat $SXE/modules.list`; do
    MODTARGET=$MODULE
    MODTARGET=${MODTARGET##kernel/}
    MODTARGET=${MODTARGET##drivers/}
    DIR=`dirname $FILESYS/lib/modules/$VERSION/$MODTARGET`
    mkdir -p $DIR
    cp $TEMPDIR/lib/modules/$VERSION/$MODULE $FILESYS/lib/modules/$VERSION/$MODTARGET
    echo $MODTARGET >>$FILESYS/etc/modules
done

cd $TOP
$SXE/install-lidstools.sh

if [ ! $NEW_KERNEL = "bzImage" ]; then
    echo "You probably need to edit $FILESYS/boot/grub/menu.lst so that /boot/$NEW_KERNEL boots"
fi

cat <<SUCCESS

Kernel successfully installed

Note - you may need to edit $FILESYS/boot/grub/menu.lst

Now run \$SXE/build.sh to create the new VM
SUCCESS
