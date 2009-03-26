#!/bin/sh

#
# Generate a SXE virtual machine disk file (in VMDK - vmware format)
#

TOP=$PWD
IMAGE=$TOP/vte.img
VM=$TOP/vte.vmdk
LOOP=/dev/loop0
ALOOP=/dev/loop1
GRUBLIB=/lib/grub/i386-pc
GRUB=/usr/sbin/grub
FILESYS=$1
TARGET=$2

test -z $SXE && SXE=`dirname $0`
test -z $TARGET && TARGET=$TOP/target
test -z $FILESYS && FILESYS=$TOP/fs


# The operations need root privileges
test `id -u` == 0 || { echo "Run this script as root"; exit 1; }

function die()
{
    echo "$1"
    umount $TARGET
    /sbin/losetup -d $LOOP
    /sbin/losetup -d $ALOOP
    exit 1
}

echo "Checking for dependencies"
test -e $GRUBLIB/stage1 || GRUBLIB=/usr/lib/grub
test -e $GRUBLIB/stage1 || GRUBLIB=/usr/lib/grub/i386-pc
test -e $GRUBLIB/stage1 || die "Could not find grub stage1 file"
test -x $GRUB || GRUB=/sbin/grub
test -x $GRUB || die "Could not find \"grub\" binary"
which qemu-img >/dev/null || die "Could not run \"qemu-img\" binary"


# Reset after any failed attempts
/sbin/losetup -a | grep $LOOP && /sbin/losetup -d $LOOP
/sbin/losetup -a | grep $ALOOP && /sbin/losetup -d $ALOOP
mount | grep $TARGET && umount $TARGET

# Save/move aside old image

test -e $IMAGE && mv -v $IMAGE $IMAGE.bak

# Create a new disk image.  For background on this technique see:
# http://www.mega-tokyo.com/osfaq/Disk%20Images%20Under%20Linux

# With 16 heads, 63 sectors/track, 512 bytes/sector, 
# each cylinder = 516096 bytes (16*63*512), or ~0.5Mb

# Set $CYL = 500Mb / 0.5Mb = 1000 (adjust $CYL for bigger disk)
CYL=1000

echo "Creating disk image $IMAGE..."
dd if=/dev/zero of=$IMAGE bs=516096 count=$CYL

# Set up the loop device.
echo "Binding $LOOP to $IMAGE"
/sbin/losetup -v $LOOP $IMAGE || die "losetup $LOOP failed - run \"losetup -d $LOOP\" to free device"
echo "Done"

# Use values for sectors and heads fixed above
# The ,,L,* tells sfdisk to default the first 2 parameters
# partition start == 0, partition end = max, make it Linux (type 83) and bootable (*)
# Using the --DOS parameter tells it to skip the first sector and use
# it for the DOS Master Boot Record - this is where grub writes stage1
# --Linux suppresses warnings about thing linux doesnt care about
cat <<PART

    
    Partitioning file-system on loop device (ignore errors here)

--------8<----------- START IGNORE ERRORS ------------------------------
PART
echo ',,L,*' | /sbin/sfdisk --DOS -C$CYL -H16 -S63 $LOOP
echo "Done"

cat <<ENDPART

--------8<------------ END IGNORE ERRORS --------------------------------

ENDPART

SECTORS=`/sbin/fdisk -l -u $LOOP| grep ${LOOP}p1 | awk '{print $3}'`
BLOCKS=`/sbin/fdisk -l -u $LOOP | grep ${LOOP}p1 | awk '{print $5}'`
BLOCKS=${BLOCKS%%+}
PARTNAME=`/sbin/fdisk -l -u $LOOP | grep ${LOOP}p1 | awk '{print $1}'`
echo "Done. $IMAGE partition $PARTNAME - starts at $SECTORS sectors - consists of $BLOCKS blocks"

# Detach image and then re-attach, offset past the MBR
/sbin/losetup -d $LOOP

# -o32256 	Move the start 32256 bytes into the file, past MBR. This is 512 * 63
# Use following non-hardcoded version if sector size ever changes:
# let OFFSET=512*$SECTORS
# /sbin/losetup -o$OFFSET $ALOOP $IMAGE
/sbin/losetup -o32256 $ALOOP $IMAGE || die "losetup $LOOP failed - run \"losetup -d $ALOOP\" to free device?"

mkdir -p $TARGET  || die "mkdir $TARGET"

# Format and mount it
# Can use non-hardcoded version, mke2fs can figure these out itself, but for other fs
# may need to pass in explicitly
# /sbin/mke2fs -b1024 /dev/loop0 $BLOCKS
/sbin/mke2fs $ALOOP || die "formatting ext2 on $ALOOP failed"
mount -t ext2 -v $ALOOP $TARGET || die "mount $ALOOP on $TARGET failed"

echo "Building file system..."
# If the filesystem directory is present then assume that it contains a complete
# filesystem based on the i686-vm-fs.tar.gz or whatever.
#
# If not then use the standard file-system.
if [ -d $FILESYS ]; then
    #  classic tar-pipe copy
    ( cd $FILESYS && tar cf - .)|( cd $TARGET && tar xfp -)
else
    cd $TARGET
    test -e $TOP/i686-vm-fs.tar.gz || die "Cannot find file-system structure i686-vm-fs.tar.gz"
    tar zxf $TOP/i686-vm-fs.tar.gz
fi

cd $TARGET/boot/grub
cp -vu $GRUBLIB/stage1 .
cp -vu $GRUBLIB/stage2 .
cp -vu $GRUBLIB/e2fs_stage1_5 .

cd $TOP
umount $TARGET
/sbin/losetup -d $ALOOP
sync;sync

/sbin/losetup $LOOP $IMAGE || die "losetup $LOOP failed - run \"losetup -d $LOOP\" to free device?"
/sbin/losetup -o32256 $ALOOP $IMAGE || die "losetup $LOOP failed - run \"losetup -d $ALOOP\" to free device?"

# Work around for grub partition bug
# https://savannah.gnu.org/bugs/index.php?func=detailitem&item_id=11771
ln -svf $ALOOP $PARTNAME

echo "(hd0) $LOOP" > $TOP/dev.map
test -f $TOP/dev.map || die "Map file not created - aborting"

# This command could be dangerous if it goes wrong.  In the worst case
# where the device mapping breaks down, it should just simply reinstall
# the current desktop machines existing MBR.
$GRUB --batch --device-map=$TOP/dev.map <<END
root (hd0,0)
setup (hd0)
quit
END

/sbin/losetup -d $LOOP
/sbin/losetup -d $ALOOP
sync;sync

test -e $VM && mv -f $VM $VM.bak
qemu-img convert -f raw $IMAGE -O vmdk $VM  || die "Could not convert image from raw format"
chmod 0666 $VM

cat <<SUCCESS

SXE image successfully created.

Try:

   vmplayer vte.vmx

SUCCESS
