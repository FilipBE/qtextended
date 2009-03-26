#!/bin/sh

# We require that arm-linux-strip is in the PATH.
TOOLCHAIN=/opt/toolchains/greenphone/gcc-4.1.1-glibc-2.3.6/arm-linux/bin
PATH=$TOOLCHAIN:$PATH

strip_install()
{
    local stripwhat=$1
    local src=$2
    local dest=$3
    shift 3

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        sudo cp -a $src/$i $dest/$i
        [ -f $dest/$i ] && sudo arm-linux-strip --strip-$stripwhat $dest/$i
    done
}

die()
{
    echo "Error: $@"
    exit 1
}

device_ssh()
{
    ssh $DEVICE_USER@$DEVICE_IP -p $DEVICE_PORT "$@"
}

optimize_prelink_all()
{
    echo "Optimizing dynamic shared objects"

    CHROOT=$( device_ssh "mktemp -d /tmp/prelink.XXXXXX" )

    device_ssh "test -d $CHROOT && mount -t nfs -o nolock,tcp $HOST_IP:$EXPORTED_ROOTFS $CHROOT" || die at line $LINENO
    device_ssh "test -d $CHROOT/opt/Qtopia && mount -t nfs -o nolock,tcp $HOST_IP:$EXPORTED_QTOPIA $CHROOT/opt/Qtopia" || die at line $LINENO

    device_ssh "/usr/sbin/chroot $CHROOT /usr/sbin/prelink -v -a --ld-library-path=/opt/Qtopia/lib" || die at line $LINENO

    device_ssh "umount $CHROOT/opt/Qtopia" || die at line $LINENO
    device_ssh "umount $CHROOT" || die at line $LINENO

    device_ssh "rmdir $CHROOT" || die at line $LINENO
}

optimize_prelink_qtopia()
{
    echo "Optimizing Qtopia dynamic shared objects"

    cat > $EXPORTED_QTOPIA/prelink_helper.sh <<EOF
#!/bin/sh
find /opt/Qtopia/bin /opt/Qtopia/lib -type f \
    -exec /opt/Qtopia/prelink_helper2.sh \\{\\} \\; |
    xargs -0 /usr/sbin/prelink -v -N --ld-library-path=/opt/Qtopia/lib
EOF

    cat > $EXPORTED_QTOPIA/prelink_helper2.sh <<EOF
#!/bin/sh
if [ \"\`hexdump -s 1 -n 3 -e \"%c\" "\$1"\`\" = \"ELF\" ]; then
    echo -en "\$1\\0"
fi
EOF

    chmod a+x $EXPORTED_QTOPIA/prelink_helper.sh $EXPORTED_QTOPIA/prelink_helper2.sh

    CHROOT=$( device_ssh "mktemp -d /tmp/prelink.XXXXXX" )

    device_ssh "test -d $CHROOT && mount -o bind,ro / $CHROOT" || die at line $LINENO
    device_ssh "test -d $CHROOT/opt/Qtopia && mount -t nfs -o nolock,tcp $HOST_IP:$EXPORTED_QTOPIA $CHROOT/opt/Qtopia" || die at line $LINENO

    device_ssh "/usr/sbin/chroot $CHROOT /opt/Qtopia/prelink_helper.sh" || die at line $LINENO

    device_ssh "umount $CHROOT/opt/Qtopia" || die at line $LINENO
    device_ssh "umount $CHROOT" || die at line $LINENO

    device_ssh "rmdir $CHROOT" || die at line $LINENO

    rm -f $EXPORTED_QTOPIA/prelink_helper.sh
    rm -f $EXPORTED_QTOPIA/prelink_helper2.sh
}

optimize_lids_all()
{
    # Generating LIDS rules on the Greenphone take minutes.  The LIDS rules
    # only need to be generated once, at first boot.  Optimize this by
    # pre-generating the LIDS rules during image creation.  The pre-generated
    # LIDS rules need to be updated during first boot with the correct
    # filesystem id and inode information.  This optimization decreases the
    # time taken to generate LIDS rules at first boot from minutes to seconds.

    # Create a chroot environment that mimics the filesystem layout of the real
    # device.  Run /etc/rc.d/sxe_boot in the chroot environment to generate
    # LIDS rules.  During boot on the real device run "lidsconf -U" for all
    # LIDS states (all, POSTBOOT, SHUTDOWN) to update inode information.

    echo "Optimizing LIDS rules"

    CHROOT=$( device_ssh "mktemp -d /tmp/lids.XXXXXX" )

    device_ssh "test -d $CHROOT && mount -t nfs -o nolock,tcp $HOST_IP:$EXPORTED_ROOTFS $CHROOT"
    device_ssh "test -d $CHROOT/opt/Qtopia && mount -t nfs -o nolock,tcp $HOST_IP:$EXPORTED_QTOPIA $CHROOT/opt/Qtopia"

    # We cannot mount proc directly as it will have the same filesystem id and
    # inode values as the real proc, and we will be denied access.  Mount a
    # tmpfs and create fake files and directories for the entries we will be
    # generating rules for.
    device_ssh "test -d $CHROOT/proc && mount -t tmpfs none $CHROOT/proc"
    device_ssh "
        mkdir $CHROOT/proc/lids;
        touch $CHROOT/proc/lids/keys;
        touch $CHROOT/proc/lids/suid;
        mkdir $CHROOT/proc/sys;
        mkdir $CHROOT/proc/sys/lids;
        touch $CHROOT/proc/sys/lids/locks;
        mkdir $CHROOT/proc/sys/kernel;
        touch $CHROOT/proc/sys/kernel/tainted;
        mkdir $CHROOT/proc/pcf;
        touch $CHROOT/proc/pcf/RTCSCA;
        touch $CHROOT/proc/pcf/RTCMNA;
        touch $CHROOT/proc/pcf/RTCHRA;
        touch $CHROOT/proc/pcf/RTCWDA;
        touch $CHROOT/proc/pcf/RTCDTA;
        touch $CHROOT/proc/pcf/RTCMTA;
        touch $CHROOT/proc/pcf/RTCYRA;
        mkdir $CHROOT/proc/bulverde;
        mkdir $CHROOT/proc/bulverde/registers;
        touch $CHROOT/proc/bulverde/registers/PSSR;
    "

    device_ssh "test -d $CHROOT/dev && mount -t tmpfs none $CHROOT/dev"
    device_ssh "tar -C $CHROOT/dev -xzf $CHROOT/dev.tar.gz"
    device_ssh "touch $CHROOT/dev/log"
    device_ssh "test -d $CHROOT/var && mount -t tmpfs none $CHROOT/var"
    device_ssh "tar -C $CHROOT/var -xzf $CHROOT/var.tar.gz"
    device_ssh "touch $CHROOT/var/log/wtmp"
    device_ssh "test -d $CHROOT/home && mount -t tmpfs none $CHROOT/home"

    device_ssh "mkdir $CHROOT/etc/lids/.original; cp -a $CHROOT/etc/lids/* $CHROOT/etc/lids/.original"
    device_ssh "/usr/sbin/chroot $CHROOT /etc/rc.d/sxe_boot"
    device_ssh "mkdir $CHROOT/etc/lids/.pregenerated; mv $CHROOT/etc/lids/* $CHROOT/etc/lids/.pregenerated"
    device_ssh "mv $CHROOT/etc/lids/.original/* $CHROOT/etc/lids; rmdir $CHROOT/etc/lids/.original"

    device_ssh "umount $CHROOT/home"
    device_ssh "umount $CHROOT/var"
    device_ssh "umount $CHROOT/dev"

    device_ssh "umount $CHROOT/proc"

    device_ssh "umount $CHROOT/opt/Qtopia"
    device_ssh "umount $CHROOT"

    device_ssh "rmdir $CHROOT"
}

optimize()
{
    # Export images via NFS to Greenphone
    EXPORTED_ROOTFS=$ROOTFS_IMAGE_PATH
    EXPORTED_QTOPIA=$QTOPIA_IMAGE_PATH
    if [ "x$EXPORTED_ROOTFS" != "x" ]; then
        sudo /usr/sbin/exportfs -o rw,no_root_squash,async,insecure $DEVICE_IP:$EXPORTED_ROOTFS || die at line $LINENO
    fi
    sudo /usr/sbin/exportfs -o rw,no_root_squash,async,insecure $DEVICE_IP:$EXPORTED_QTOPIA || die at line $LINENO

    # Shut down Qtopia; prelink can use a lot of memory so Qtopia's memory must be freed.
    device_ssh "if [ -x /etc/rc.d/rc.qtopia ]; then /etc/rc.d/rc.qtopia stop; fi" || die at line $LINENO

    if [ $OPTION_OPTIMIZE_PRELINK -eq 1 ]; then
        if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
            optimize_prelink_all
        elif [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
            optimize_prelink_qtopia
        fi
    fi

    if [ $OPTION_OPTIMIZE_LIDS -eq 1 ]; then
        if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
            optimize_lids_all
        fi
    fi

    # Unexport images
    if [ "x$EXPORTED_ROOTFS" != "x" ]; then
        sudo /usr/sbin/exportfs -u $DEVICE_IP:$EXPORTED_QTOPIA || die at line $LINENO
    fi
    sudo /usr/sbin/exportfs -u $DEVICE_IP:$EXPORTED_ROOTFS || die at line $LINENO
}

partition_tffsa()
{
    echo "Partitioning disk1 image"

    local FIRST_SECTOR=63
    local ROOTFS_PARTITION_SIZE=$(( $(stat -c %s $DISK1_PART1_FILENAME) / 512 ))
    local ACTUAL_QTOPIA_PARTITION_SIZE=$(( $(stat -c %s $DISK1_PART2_FILENAME) / 512 ))
    local QTOPIA_PARTITION_SIZE=$(( ($DISK1_SIZELIMIT / 512) - ($FIRST_SECTOR + $ROOTFS_PARTITION_SIZE) ))

    echo "Root filesystem size: $(( $ROOTFS_PARTITION_SIZE * 512 )) bytes"
    echo "Space allocated for Qtopia: $(( $QTOPIA_PARTITION_SIZE * 512 )) bytes"
    echo "Qtopia filesystem size: $(( $ACTUAL_QTOPIA_PARTITION_SIZE * 512 )) bytes"

    dd if=/dev/zero of=$DISK1_FILENAME bs=1 count=0 seek=$DISK1_SIZELIMIT 2>/dev/null

    for i in 0 1 2 3 4 5 6 7; do
        LOOP_DEVICE=/dev/loop$i
        sudo /sbin/losetup $LOOP_DEVICE $DISK1_FILENAME 2>/dev/null && break
    done

    # create partitions
    sudo /sbin/sfdisk -uS -f -L -q $LOOP_DEVICE >/dev/null 2>&1 <<EOF
$FIRST_SECTOR,$ROOTFS_PARTITION_SIZE,L
$(($FIRST_SECTOR + $ROOTFS_PARTITION_SIZE)),$QTOPIA_PARTITION_SIZE,L
EOF

    sudo /sbin/losetup -d $LOOP_DEVICE

    dd if=$DISK1_PART1_FILENAME of=$DISK1_FILENAME bs=512 seek=$FIRST_SECTOR conv=notrunc 2>/dev/null
    dd if=$DISK1_PART2_FILENAME of=$DISK1_FILENAME bs=512 seek=$(($FIRST_SECTOR + $ROOTFS_PARTITION_SIZE)) conv=notrunc 2>/dev/null
}

make_flash_image()
{
    echo "Creating USB Flash image"

    local OUTPUT_IMAGE="$1"
    shift

    rm -f flash.conf

    IMAGE_FILES=""

    if [ ! -z $KERNEL_IMAGE ] && [ ! -z $KERNEL_FILENAME ] && [ -f $KERNEL_FILENAME ]; then
        MD5SUM=`md5sum $KERNEL_FILENAME | awk '{print $1}'`
        echo "$KERNEL_NAME:$KERNEL_FILENAME:2:$MD5SUM" >> flash.conf
        IMAGE_FILES="$IMAGE_FILES $KERNEL_FILENAME"
    fi

    if [ ! -z $ROOTFS_IMAGE ] && [ ! -z $DISK1_FILENAME ]; then
        MD5SUM=`md5sum $DISK1_FILENAME | awk '{print $1}'`
        echo "$DISK1_NAME ($ROOTFS_VERSION):$DISK1_FILENAME:3:$MD5SUM" >> flash.conf
        IMAGE_FILES="$IMAGE_FILES $DISK1_FILENAME"
    fi

    while [ $# -gt 0 ]; do
        case $1 in
            --blank-disk2)
                dd if=/dev/zero of=$DISK2_FILENAME bs=$DISK2_SIZELIMIT count=1 2>/dev/null
                MD5SUM=`md5sum $DISK2_FILENAME | awk '{print $1}'`
                echo "$DISK2_NAME:$DISK2_FILENAME:4:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $DISK2_FILENAME"
                ;;
            --blank-disk3)
                dd if=/dev/zero of=$DISK3_FILENAME bs=$DISK3_SIZELIMIT count=1 2>/dev/null
                MD5SUM=`md5sum $DISK3_FILENAME | awk '{print $1}'`
                echo "$DISK3_NAME:$DISK3_FILENAME:5:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $DISK3_FILENAME"
                ;;
            --blank-disk4)
                dd if=/dev/zero of=$DISK4_FILENAME bs=$DISK4_SIZELIMIT count=1 2>/dev/null
                MD5SUM=`md5sum $DISK4_FILENAME | awk '{print $1}'`
                echo "$DISK4_NAME:$DISK4_FILENAME:6:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $DISK4_FILENAME"
                ;;
        esac

        shift
    done

    tar -czhf "$OUTPUT_IMAGE" flash.conf $IMAGE_FILES
    echo "Finished creating USB Flash image : $PWD/$OUTPUT_IMAGE"
}

make_update_image()
{
    echo "Creating update image"

    UPDATE_IMAGE_PATH=$(mktemp -d $PWD/update.XXXXXX)
    [ -d $UPDATE_IMAGE_PATH ] || die "Could not create temporary update directory"
    trap "rm -rf $UPDATE_IMAGE_PATH" 0

    cp $DISK1_PART2_FILENAME $UPDATE_IMAGE_PATH/qtopia.cramfs

    cp $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/flash-files/flash-status-*.gif $UPDATE_IMAGE_PATH
    chmod 660 $UPDATE_IMAGE_PATH/*.gif

    if [ -s $UPDATE_IMAGE_PATH/qtopia.cramfs -a -s $UPDATE_IMAGE_PATH/flash-status-flashing.gif ]; then
        tar -czhf $UPDATE_IMAGE.tar.gz -C $UPDATE_IMAGE_PATH .
        echo "Finished creating update image : $PWD/$UPDATE_IMAGE.tar.gz"
        zip -jq $UPDATE_IMAGE.zip $UPDATE_IMAGE_PATH/*
        echo "Finished creating update image : $PWD/$UPDATE_IMAGE.zip"
    else
        rm -f $UPDATE_IMAGE.tar.gz # clean up after ourselves
        rm -f $UPDATE_IMAGE.zip
        echo "Error: Failed to create update image"
        exit 1
    fi

    rm -rf $UPDATE_IMAGE_PATH
}

usage()
{
    echo -e "Usage: `basename $0` [--clean] [--qtopia] [--flash] [--optimize] [--qtopia-image <path>] [--rootfs <file>]"
    echo -e "If no options are specified defaults to --qtopia --qtopia-image $QTOPIA_BUILD_PATH\n" \
            "   --clean               Clean image files.\n" \
            "   --qtopia              Make Qtopia image.\n" \
            "   --flash               Make flash image.\n" \
            "   --qtopia-image        Location of Qtopia image.\n" \
            "   --rootfs              Location of rootfs image.\n" \
            "   --kernel              Location of Linux kernel image.\n" \
            "   --build-path          Location of rootfs build directory.\n" \
            "   --optimize            Optimize built image with all applicable optimizations.\n" \
            "                         Some optimization options require network access to a\n" \
            "                         Greenphone-compatible device. The IP address of the device\n" \
            "                         can be specified with the DEVICE_IP environment variable.\n" \
            "   --optimize-prelink    Optimize dynamic shared objects. Requires network\n" \
            "                         access to a Greenphone.\n" \
            "   --optimize-lids       Pregenerate LIDS rules. Requires network access to a\n" \
            "                         Greenphone.\n" \
            "   --dont-strip          Do not strip Qtopia binaries and libraries.\n\n" \
            "Using $QTOPIA_SOURCE_PATH as Qtopia source path.  Override with QTOPIA_DEPOT_PATH environment variable if required.\n"
}

DEFAULT_OPTIONS=1
OPTION_CLEAN=0
OPTION_OPTIMIZE_PRELINK=0
OPTION_OPTIMIZE_LIDS=0
OPTION_QTOPIA_IMAGE=1
OPTION_FLASH_IMAGE=0
OPTION_DONT_STRIP=0

OPTION_OPTIMIZE=0

DEFAULT_IMAGES=1

if [ -z $QTOPIA_DEPOT_PATH ]; then
    QTOPIA_SOURCE_PATH=$(dirname $(dirname $(readlink -f $0)))
    export QTOPIA_DEPOT_PATH=$QTOPIA_SOURCE_PATH
else
    QTOPIA_SOURCE_PATH=$QTOPIA_DEPOT_PATH
fi

QTOPIA_BUILD_PATH=$PWD/image

. `dirname $0`/functions
[ "x$QPEVER" = "x" ] && QPEVER=`version`

# Default Flash image data
KERNEL_NAME="Greenphone Kernel (2.4.19)"
KERNEL_FILENAME=greenphone_kernel.bin
KERNEL_SIZELIMIT=1048576
ROOTFS_VERSION="unknown"
DISK1_NAME="Greenphone Root and Qtopia Filesystem"
DISK1_FILENAME=greenphone_rootqtopia.bin
DISK1_SIZELIMIT=44040192
DISK1_PART1_NAME="Greenphone Root Filesystem"
DISK1_PART1_FILENAME=greenphone_root.cramfs
DISK1_PART2_NAME="Greenphone Qtopia Filesystem"
DISK1_PART2_FILENAME=greenphone_qtopia.cramfs
DISK2_NAME="Blank Home"
DISK2_FILENAME=greenphone_home.ext2
DISK2_SIZELIMIT=33554432
DISK3_NAME="Blank System Configuration"
DISK3_FILENAME=greenphone_sysconf.ext2
DISK3_SIZELIMIT=5242880
DISK4_NAME="Blank Documents"
DISK4_FILENAME=greenphone_doc.vfat
DISK4_SIZELIMIT=37355520

# Output filenames
FLASH_IMAGE="qtopia-greenphone-flash"
UPDATE_IMAGE="qtopia-greenphone-update"

while [ $# -ne 0 ]; do
    case $1 in
        --qtopia)
            OPTION_QTOPIA_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --flash)
            OPTION_FLASH_IMAGE=1
            DEFAULT_OPTIONS=1
            ;;
        --clean)
            OPTION_CLEAN=1
            DEFAULT_OPTIONS=0
            ;;
        --qtopia-build|--qtopia-image)
            if [ $# -ge 2 ]; then
                QTOPIA_BUILD_PATH="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --rootfs)
            if [ $# -ge 2 ]; then
                ROOTFS_IMAGE="$2"
                DEFAULT_IMAGES=0
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --kernel)
            if [ $# -ge 2 ]; then
                KERNEL_IMAGE="$2"
                DEFAULT_IMAGES=0
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --optimize)
            OPTION_OPTIMIZE_PRELINK=1
            OPTION_OPTIMIZE_LIDS=1
            OPTION_OPTIMIZE=1
            DEFAULT_OPTIONS=0
            ;;
        --optimize-prelink)
            OPTION_OPTIMIZE_PRELINK=1
            OPTION_OPTIMIZE=1
            DEFAULT_OPTIONS=0
            ;;
        --optimize-lids)
            OPTION_OPTIMIZE_LIDS=1
            OPTION_OPTIMIZE=1
            DEFAULT_OPTIONS=0
            ;;
        --dont-strip)
            OPTION_DONT_STRIP=1
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
    shift
done

# Use default image filenames if not specified on command line
if [ $DEFAULT_IMAGES -eq 1 ] && [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    # Work out which server has the rootfs and kernel images
    ping -c 1 qtopiaweb.trolltech.com.au > /dev/null 2>/dev/null
    if [ $? = 0 ]; then
        IMAGE_HOST=qtopiaweb.trolltech.com.au
    else
        IMAGE_HOST=qtopiaweb-nokia.trolltech.com.au
    fi

    # Default root filesystem image
    if [ -f /opt/Qtopia/extras/images/greenphone-rootfs.tar.gz ]; then
        # Greenphone SDK
        DEFAULT_ROOTFS_IMAGE=/opt/Qtopia/extras/images/greenphone-rootfs.tar.gz
    else
        # Trolltech internal
        wget -q -N http://${IMAGE_HOST}/dist/qtopia/$QPEVER/greenphone-rootfs.tar.gz
        if [ -f $PWD/greenphone-rootfs.tar.gz ]; then
            DEFAULT_ROOTFS_IMAGE=$PWD/greenphone-rootfs.tar.gz
        fi
    fi
    if [ -z $DEFAULT_ROOTFS_IMAGE ]; then
        echo "Default root filesystem image not found, specify --rootfs"
        usage
        exit 1
    fi

    # Default kernel image
    if [ -f /opt/Qtopia/extras/images/greenphone-kernel ]; then
        # Greenphone SDK
        DEFAULT_KERNEL_IMAGE=/opt/Qtopia/extras/images/greenphone-kernel
    else
        # Trolltech internal
        wget -q -N http://${IMAGE_HOST}/dist/qtopia/$QPEVER/greenphone-kernel
        if [ -f $PWD/greenphone-kernel ]; then
            DEFAULT_KERNEL_IMAGE=$PWD/greenphone-kernel
        fi
    fi
    if [ -z $DEFAULT_KERNEL_IMAGE ]; then
        echo "Default kernel image not found, specify --kernel"
        usage
        exit 1
    fi

    ROOTFS_IMAGE=$DEFAULT_ROOTFS_IMAGE
    KERNEL_IMAGE=$DEFAULT_KERNEL_IMAGE
fi

# Make sure all input files and directories exist

# Check --qtopia-image
[ -d "$QTOPIA_BUILD_PATH" ] || die "$QTOPIA_BUILD_PATH does not exist or is not a directory"
if [ ! -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
    echo "$QTOPIA_BUILD_PATH does not contain an image - check \"--qtopia-image\" argument"
    if [ -f "$QTOPIA_BUILD_PATH/config.status" ]; then
        echo "    $QTOPIA_BUILD_PATH looks like the build tree"
        QTOPIA_BUILD_PATH="$QTOPIA_BUILD_PATH/image"
        if [ -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
            echo "    using $QTOPIA_BUILD_PATH as image"
        else
            exit 1;
        fi
    else
        exit 1;
    fi
fi

# Check --qtopia-source
[ -d "$QTOPIA_SOURCE_PATH" ] || die "Qtopia source path does not exist or is not a directory"

# Clean old files
if [ $OPTION_CLEAN -eq 1 ]; then
    rm -rf fimage
    rm -f $KERNEL_FILENAME
    rm -f $DISK1_FILENAME
    rm -f $UPDATE_IMAGE.tar.gz
    rm -f $UPDATE_IMAGE.zip
    rm -f $FLASH_IMAGE
    rm -f $FLASH_IMAGE-full
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    # Check --kernel
    if [ ! -z "$KERNEL_IMAGE" ]; then
        [ -f "$KERNEL_IMAGE" ] || die "Kernel image does not exist"

        KERNEL_SIZE=`stat -c '%s' $KERNEL_IMAGE`

        [ $KERNEL_SIZE -le $KERNEL_SIZELIMIT ] ||
            die "Linux kernel larger than allocated flash size. $KERNEL_SIZE > $KERNEL_SIZELIMIT."

        cp $KERNEL_IMAGE $KERNEL_FILENAME
    fi

    # Check --rootfs
    if [ ! -z "$ROOTFS_IMAGE" ]; then
        [ -f "$ROOTFS_IMAGE" ] || die "Rootfs image $ROOTFS_IMAGE does not exist"

        OPTION_QTOPIA_IMAGE=1
    fi
fi

# Copy Qtopia to temporary location
if [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
    QTOPIA_IMAGE_PATH=$(mktemp -d $PWD/qtopia.XXXXXX)
    [ -d $QTOPIA_IMAGE_PATH ] || die "Could not create temporary qtopia directory"
    trap "rm -rf $QTOPIA_IMAGE_PATH" 0

    cp -a $QTOPIA_BUILD_PATH/* $QTOPIA_IMAGE_PATH

    if [ $OPTION_DONT_STRIP -eq 0 ]; then
        files="$(find $QTOPIA_IMAGE_PATH -type f | xargs file | grep ELF | grep 'not stripped' | awk '{print $1}' | sed 's/:$//')"
        for file in $files; do
            arm-linux-strip --strip-all -R .note -R .comment $file
        done
        files="$(find $QTOPIA_IMAGE_PATH -type f | xargs file | grep ELF | grep 'not stripped' | awk '{print $1}' | sed 's/:$//')"
        for file in $files; do
            arm-linux-strip --strip-unneeded -R .note -R .comment $file
        done
    fi
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
    ROOTFS_IMAGE_PATH=$(mktemp -d $PWD/rootfs.XXXXXX)
    [ -d $ROOTFS_IMAGE_PATH ] || die "Could not create temporary rootfs directory"
    trap "sudo rm -rf $ROOTFS_IMAGE_PATH" 0

    sudo chmod 1777 $ROOTFS_IMAGE_PATH
    sudo chown root.root $ROOTFS_IMAGE_PATH

    sudo tar -C $ROOTFS_IMAGE_PATH -xf $ROOTFS_IMAGE || die "sudo failed at line $LINENO"

    if [ -r $ROOTFS_IMAGE_PATH/fs.ver ]; then
        ROOTFS_VERSION=$(sed -ne '/^troll/ s/^troll v//p' < $ROOTFS_IMAGE_PATH/fs.ver)
    fi

    # Move atd to /usr/sbin
    sudo mv $QTOPIA_IMAGE_PATH/bin/atd $ROOTFS_IMAGE_PATH/usr/sbin

    # Copy zoneinfo into /usr/share
    sudo mkdir -p $ROOTFS_IMAGE_PATH/usr/share
    sudo cp -a $QTOPIA_SOURCE_PATH/etc/zoneinfo $ROOTFS_IMAGE_PATH/usr/share
    sudo chown -R root.root $ROOTFS_IMAGE_PATH/usr/share/zoneinfo
fi

if [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
    # Remove atd from /opt/Qtopia/bin
    rm -f $QTOPIA_IMAGE_PATH/bin/atd
fi

# Optimization of images requires connection to a device of the same
# type as the target.  Use DEVICE_IP environment variable as the address
# of the device to connect to, HOST_IP as the address of the host from
# the device's point of view, otherwise default to 10.10.10.20/hostpc
# Note: for compatibility: read from GREENPHONE_IP first
if [ "x$GREENPHONE_IP" != "x" ]; then DEVICE_IP=$GREENPHONE_IP; fi
DEVICE_IP=${DEVICE_IP-10.10.10.20}
HOST_IP=${HOST_IP-hostpc}
DEVICE_USER=${DEVICE_USER-root}
DEVICE_PORT=${DEVICE_PORT-22}

# Perform optimizations
if [ $OPTION_OPTIMIZE -eq 1 ]; then
    echo "Optimizing image..."

    if device_ssh "/bin/true" 2>/dev/null; then
        optimize
    else
        echo "Not optimizing, cannot connect to device at $DEVICE_IP"
    fi
fi

# Create Qtopia filesystem cramfs
if [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
    echo "Creating Qtopia filesystem image"

    /sbin/mkfs.cramfs -n "$DISK1_PART2_NAME" $QTOPIA_IMAGE_PATH $DISK1_PART2_FILENAME

    # Save valid Qtopia filesystem id for pregenerated LIDS rules
    if [ $OPTION_OPTIMIZE_LIDS -eq 1 ]; then
        hexdump -e '"%08x"' -n 48 $DISK1_PART2_FILENAME | sudo sh -c "cat > $ROOTFS_IMAGE_PATH/etc/lids/qtopia.id"
    fi

    rm -rf $QTOPIA_IMAGE_PATH
fi

# Create root filesystem cramfs
if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z "$ROOTFS_IMAGE" ]; then
    echo "Creating root filesystem image"

    /sbin/mkfs.cramfs -n "$DISK1_PART1_NAME" $ROOTFS_IMAGE_PATH $DISK1_PART1_FILENAME

    sudo rm -rf $ROOTFS_IMAGE_PATH
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
    partition_tffsa
fi

# Create update image
make_update_image $UPDATE_IMAGE

# Create flash images
if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    DISK1_SIZE=$(stat -c '%s' $DISK1_FILENAME)
    [ $DISK1_SIZE -le $DISK1_SIZELIMIT ] || die "$DISK1_NAME is larger than allocated flash size. $DISK1_SIZE > $DISK1_SIZELIMIT."

    make_flash_image $FLASH_IMAGE
    make_flash_image $FLASH_IMAGE-full --blank-disk2 --blank-disk3 --blank-disk4

    # Clean up
    rm -f $KERNEL_FILENAME $DISK1_PART1_FILENAME $DISK1_PART2_FILENAME $DISK1_FILENAME $DISK2_FILENAME $DISK3_FILENAME $DISK4_FILENAME
fi

