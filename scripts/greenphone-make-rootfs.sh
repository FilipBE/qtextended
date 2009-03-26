#!/bin/sh

install()
{
    local src=$1
    local dest=$2
    shift 2

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        cp -a $src/$i $dest/$i
    done
}

strip_install()
{
    local stripwhat=$1
    local src=$2
    local dest=$3
    shift 3

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        cp -a $src/$i $dest/$i
        [ -f $dest/$i ] && chmod u+w $dest/$i
        [ -f $dest/$i ] && $CROSS_PREFIX-strip --strip-$stripwhat $dest/$i
    done
}

die()
{
    if [ $OPTION_INTERACTIVE -eq 1 ]; then
        echo
        echo "===== $LOGFILE ====="
        tail -10 $LOGFILE
        echo "===== $LOGFILE ====="
        echo
        echo "Starting interactive shell"
        echo "Type 'exit 0' to continue build or 'exit 1' to abort"
        echo "Error: $@"
        bash || exit 1
    else
        echo
        echo "===== $LOGFILE ====="
        tail -10 $LOGFILE
        echo "===== $LOGFILE ====="
        echo
        echo "Error: $@"
    
        exit 1
    fi
}

source_changed()
{
    local SOURCE_STAMP=1
    local BUILD_STAMP=0

    if [ $# -ge 2 ]; then
        TMP=`find $1 -type f -printf '%T@\n' 2>/dev/null | sort -n | tail -1`
        if [ $TMP ]; then
            SOURCE_STAMP=$TMP
        fi
        shift
    fi

    if [ -e $1 ]; then
        BUILD_STAMP=`stat -c %Y $1 2>/dev/null`
    fi

    if [ $BUILD_STAMP -lt $SOURCE_STAMP ]; then
        # source changed require rebuild
        return 0
    else
        # source not changed don't need to rebuild
        return 1
    fi
}

clean_package()
{
    echo "Cleaning $1"
    
    case $1 in
    toolchain)
        rm -rf $CROSSTOOL
        rm -f $STAMPDIR/$CROSSTOOL.buildstamp
        ;;
    busybox)
        rm -rf $BUSYBOX
        rm -f $STAMPDIR/$BUSYBOX.buildstamp
        ;;
    libungif)
        rm -rf $LIBUNGIF
        rm -f $STAMPDIR/$LIBUNGIF.buildstamp
        ;;
    ilib)
        rm -rf $ILIB
        rm -f $STAMPDIR/$ILIB.buildstamp
        ;;
    dosfstools)
        rm -rf $DOSFSTOOLS
        rm -f $STAMPDIR/$DOSFSTOOLS.buildstamp
        ;;
    strace)
        rm -rf $STRACE
        rm -f $STAMPDIR/$STRACE.buildstamp
        ;;
    ppp)
        rm -rf $PPP
        rm -f $STAMPDIR/$PPP.buildstamp
        ;;
    samba)
        rm -rf $SAMBA
        rm -rf host/$SAMBA
        rm -f $STAMPDIR/$SAMBA.buildstamp
        ;;
    sxetools)
        rm -rf sxe_tools
        rm -f $STAMPDIR/sxetools.buildstamp
        ;;
    armioctl)
        rm -rf armioctl
        rm -f $STAMPDIR/armioctl.buildstamp
        ;;
    tat)
        rm -rf tat
        rm -f $STAMPDIR/tat.buildstamp
        ;;
    getkeycode)
        rm -rf getkeycode
        rm -f $STAMPDIR/getkeycode.buildstamp
        ;;
    bootcharger)
        rm -rf bootcharger
        rm -f $STAMPDIR/bootcharger.buildstamp
        ;;
    prelink)
        rm -rf $LIBELF
        rm -rf $PRELINK
        rm -f $STAMPDIR/$LIBELF.buildstamp
        rm -f $STAMPDIR/$PRELINK.buildstamp
        ;;
    dropbear)
        rm -rf $DROPBEAR
        rm -f $STAMPDIR/$DROPBEAR.buildstamp
        ;;
    expat)
        rm -rf $EXPAT
        rm -f $STAMPDIR/$EXPAT.buildstamp
        ;;
    dbus)
        rm -rf $DBUS
        rm -f $STAMPDIR/$DBUS.buildstamp
        ;;
    bluez-libs)
        rm -rf $BLUEZLIBS
        rm -f $STAMPDIR/$BLUEZLIBS.buildstamp
        ;;
    bluez-utils)
        rm -rf $BLUEZUTILS
        rm -f $STAMPDIR/$BLUEZUTILS.buildstamp
        ;;
    bluez-hcidump)
        rm -rf $BLUEZHCIDUMP
        rm -f $STAMPDIR/$BLUEZHCIDUMP.buildstamp
        ;;
    bluez-firmware)
        rm -rf $BLUEZFIRMWARE
        rm -f $STAMPDIR/$BLUEZFIRMWARE.buildstamp
        ;;
    wireless-tools)
        rm -rf $WIRELESSTOOLS
        rm -f $STAMPDIR/$WIRELESSTOOLS.buildstamp
        ;;
    wpa_supplicant)
        rm -rf $WPASUPPLICANT
        rm -f $STAMPDIR/$WPASUPPLICANT.buildstamp
        ;;
    initrd)
        rm -rf initrd
        rm -f $STAMPDIR/initrd.buildstamp
        ;;
    linux)
        rm -rf $GPH_LINUX
        rm -f $STAMPDIR/$GPH_LINUX.buildstamp
        ;;
    lids)
        rm -rf $LIDS
        rm -f $STAMPDIR/$LIDS.buildstamp
        ;;
    popt)
        rm -rf $POPT
        rm -f $STAMPDIR/$POPT.buildstamp
        ;;
    raid)
        rm -rf $RAID
        rm -f $STAMPDIR/$RAID.buildstamp
        ;;
    openssl)
        rm -rf $OPENSSL
        rm -f $STAMPDIR/$OPENSSL.buildstamp
        ;;
    bootchart)
        rm -rf $BOOTCHART
        rm -f $STAMPDIR/$BOOTCHART.buildstamp
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
}

download_package()
{
    echo "Downloading $1"
    CUR_DIR=$(pwd)

    cd $DOWNLOAD_DIR
    case $1 in
    toolchain)
        if [ "$OWNTOOLCHAIN" != "1" ]; then
            [ -e $CROSSTOOL.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$CROSSTOOL.tar.gz || die "downloading toolchain/$CROSSTOOL"
            for file in $BINUTILS $GCCSTRAP $GCC $GDB $GLIBC $LINUXTHREADS $LINUX; do
                [ -e $file.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$file.tar.bz2 || die "downloading toolchain/$file"
            done
		fi
        ;;
    busybox)
        [ -e $BUSYBOX.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BUSYBOX.tar.bz2 || die "downloading $BUSYBOX"
        ;;
    libungif)
        [ -e $LIBUNGIF.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$LIBUNGIF.tar.bz2 || die "downloading $LIBUNGIF"
        ;;
    ilib)
        [ -e $ILIB-min.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$ILIB-min.tar.gz || die "downloading $ILIB"
        ;;
    dosfstools)
        [ -e $DOSFSTOOLS.src.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$DOSFSTOOLS.src.tar.gz || die "downloading $DOSFSTOOLS"
        ;;
    strace)
        [ -e $STRACE.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$STRACE.tar.bz2 || die "downloading $STRACE"
        ;;
    ppp)
        [ -e $PPP.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$PPP.tar.gz || die "downloading $PPP"
        ;;
    samba)
        [ -e $SAMBA.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$SAMBA.tar.gz || die "downloading $SAMBA"
        ;;
    sxetools)
        ;;
    armioctl)
        ;;
    tat)
        ;;
    getkeycode)
        ;;
    bootcharger)
        ;;
    prelink)
        [ -e $LIBELF.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$LIBELF.tar.gz || die "downloading prelink/$LIBELF"
        [ -e $PRELINK.orig.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$PRELINK.orig.tar.gz || die "downloading $PRELINK"
        ;;
    dropbear)
        [ -e $DROPBEAR.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$DROPBEAR.tar.gz || die "downloading $DROPBEAR"
        ;;
    expat)
        [ -e $EXPAT.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$EXPAT.tar.gz || die "downloading $EXPAT"
        ;;
    dbus)
        [ -e $DBUS.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$DBUS.tar.gz || die "downloading $DBUS"
        ;;
    bluez-libs)
        [ -e $BLUEZLIBS.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BLUEZLIBS.tar.gz || die "downloading $BLUEZLIBS"
        ;;
    bluez-utils)
        [ -e $BLUEZUTILS.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BLUEZUTILS.tar.gz || die "downloading $BLUEZUTILS"
        ;;
    bluez-hcidump)
        [ -e $BLUEZHCIDUMP.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BLUEZHCIDUMP.tar.gz || die "downloading $BLUEZHCIDUMP"
        ;;
    bluez-firmware)
        [ -e $BLUEZFIRMWARE.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BLUEZFIRMWARE.tar.gz || die "downloading $BLUEZFIRMWARE"
        ;;
    wireless-tools)
        [ -e $WIRELESSTOOLS.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$WIRELESSTOOLS.tar.gz || die "downloading $WIRELESSTOOLS"
        ;;
    wpa_supplicant)
        [ -e $WPASUPPLICANT.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$WPASUPPLICANT.tar.gz || die "downloading $WPASUPPLICANT"
        ;;
    initrd)
        [ -e $BUSYBOX.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BUSYBOX.tar.bz2 || die "downloading $BUSYBOX"
        ;;
    linux)
        [ -n "$KERNEL_SOURCE_PATH" ] || die "kernel source path not defined."
        [ -d "$KERNEL_SOURCE_PATH" ] || die "kernel source path ($KERNEL_SOURCE_PATH) does not exist or is not a directory."
        ;;
    popt)
        [ -e $POPT.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$POPT.tar.gz || die "downloading $POPT"
        ;;
    raid)
        [ -e $RAID.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$RAID.tar.gz || die "downloading $LIDS"
        ;;
    lids)
        [ -e $LIDS.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$LIDS.tar.gz || die "downloading $LIDS"
        ;;
    openssl)
        [ -e $OPENSSL.tar.gz ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$OPENSSL.tar.gz || die "downloading $OPENSSL"
        ;;
    bootchart)
        [ -e $BOOTCHART.tar.bz2 ] || wget -nc $VERBOSITY $PACKAGE_LOCATION/$BOOTCHART.tar.bz2 || die "downloading $PACKAGE_LOCATION/$BOOTCHART.tar.bz2"
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
    cd $CUR_DIR
}

build_package()
{
    # download 3rd party source packages if required
    download_package $1

    test -x progress || gcc -o progress $QTOPIA_SOURCE_PATH/src/tools/sxe_tools/progress.c 2>&1 >/dev/null

    echo "Building $1"
    cd $ROOTFS_BUILD_PATH
    case $1 in
    toolchain)

		if [ "$OWNTOOLCHAIN" = "1" ]; then
		  return 0
		fi

        source_changed $ROOTFS_SOURCE_PATH/toolchain $STAMPDIR/$CROSSTOOL.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$CROSSTOOL

        tar -xzf $DOWNLOAD_DIR/$CROSSTOOL.tar.gz || die "toolchain extract"
        cp -a $ROOTFS_SOURCE_PATH/toolchain/* $ROOTFS_BUILD_PATH/$CROSSTOOL ||
            die "toolchain patch"
        cd $ROOTFS_BUILD_PATH/$CROSSTOOL
        LD_LIBRARY_PATH_SAVED="$LD_LIBRARY_PATH"
        unset LD_LIBRARY_PATH
        TEAMBUILDER=0 ./trolltech-greenphone.sh 2>&1 > /dev/null || die "toolchain build"
        LD_LIBRARY_PATH="$LD_LIBRARY_PATH_SAVED"
        unset LD_LIBRARY_PATH_SAVED
        cd ..
        touch $STAMPDIR/$CROSSTOOL.buildstamp
        ;;
    busybox)
        source_changed $ROOTFS_SOURCE_PATH/busybox $STAMPDIR/$BUSYBOX.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BUSYBOX

        tar -xjf $DOWNLOAD_DIR/$BUSYBOX.tar.bz2 || die "busybox extract"

        cd $ROOTFS_BUILD_PATH/$BUSYBOX
        for i in $ROOTFS_SOURCE_PATH/busybox/*.patch; do
            patch -p 1 < $i || die "busybox patch"
        done
        cp $ROOTFS_SOURCE_PATH/busybox/config-greenphone .config ||
            die "busybox configure"
        make >$LOGFILE 2>&1 || die "busybox build"
        cd ..

        touch $STAMPDIR/$BUSYBOX.buildstamp
        ;;
    libungif)
        source_changed $STAMPDIR/$LIBUNGIF.buildstamp || return 0

        tar -xjf $DOWNLOAD_DIR/$LIBUNGIF.tar.bz2 || die "libungif extract"

        cd $ROOTFS_BUILD_PATH/$LIBUNGIF
        ./configure --prefix=/usr --host=$CROSS_PREFIX >$LOGFILE 2>&1 ||
            die "libungif configure"
        make >$LOGFILE 2>&1 || die "libungif build"
        cd ..

        touch $STAMPDIR/$LIBUNGIF.buildstamp
        ;;
    ilib)
        source_changed $ROOTFS_SOURCE_PATH/ilib $STAMPDIR/$ILIB.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$ILIB

        tar -xzf $DOWNLOAD_DIR/$ILIB-min.tar.gz || die "ilib extract"

        cd $ROOTFS_BUILD_PATH/$ILIB
        for i in $ROOTFS_SOURCE_PATH/ilib/*.patch; do
            patch -p 1 < $i || die "ilib patch"
        done

        make makefiles >$LOGFILE 2>&1 || die "ilib configure"
        cd lib
        make >$LOGFILE 2>&1 || die "ilib build"
        cd ../clients/ifont2h
        make >$LOGFILE 2>&1 || die "ilib/ifont2h build"
        cd ../../addtext
        make >$LOGFILE 2>&1 || die "ilib/addtext build"
        cd ../..

        touch $STAMPDIR/$ILIB.buildstamp
        ;;
    dosfstools)
        source_changed $STAMPDIR/$DOSFSTOOLS.buildstamp || return 0

        tar -xzf $DOWNLOAD_DIR/$DOSFSTOOLS.src.tar.gz || die "dosfstools extract"

        cd $ROOTFS_BUILD_PATH/$DOSFSTOOLS
        make CC=$CROSS_PREFIX-gcc >$LOGFILE 2>&1 || die "dosfstools build"
        cd ..

        touch $STAMPDIR/$DOSFSTOOLS.buildstamp
        ;;
    strace)
        source_changed $ROOTFS_SOURCE_PATH/strace $STAMPDIR/$STRACE.buildstamp || return 0

        tar -xjf $DOWNLOAD_DIR/$STRACE.tar.bz2 || die "strace extract"

        cd $ROOTFS_BUILD_PATH/$STRACE
        for i in $ROOTFS_SOURCE_PATH/strace/*.patch; do
            patch -p 1 < $i || die "strace patch"
        done
        ./configure --prefix=/usr --host=$CROSS_PREFIX ||
            die "strace configure"
        make >$LOGFILE 2>&1 || die "strace build"
        cd ..

        touch $STAMPDIR/$STRACE.buildstamp
        ;;
    ppp)
        source_changed $ROOTFS_SOURCE_PATH/ppp $STAMPDIR/$PPP.buildstamp || return 0

        tar -xzf $DOWNLOAD_DIR/$PPP.tar.gz || die "ppp extract"

        cd $ROOTFS_BUILD_PATH/$PPP
        for i in $ROOTFS_SOURCE_PATH/ppp/*.patch; do
            patch -p 1 < $i || die "ppp patch"
        done
        ./configure --prefix=/usr || die "ppp configure"
        make CC=$CROSS_PREFIX-gcc >$LOGFILE 2>&1 || die "ppp build"
        cd ..

        touch $STAMPDIR/$PPP.buildstamp
        ;;
    samba)
        source_changed $ROOTFS_SOURCE_PATH/samba $STAMPDIR/$SAMBA.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$SAMBA

        tar -xzf $DOWNLOAD_DIR/$SAMBA.tar.gz || die "samba extract"

        cd $ROOTFS_BUILD_PATH/$SAMBA/source
        CC=$CROSS_PREFIX-gcc ./configure --host=$CROSS_PREFIX --disable-cups >$LOGFILE 2>&1 ||
            die "samba configure"
        echo '#define HAVE_GETTIMEOFDAY_TZ 1' >> include/config.h
        echo '#define uint32 unsigned int' >> include/config.h
        echo '#define int32 int' >> include/config.h
        make >$LOGFILE 2>&1 || die "samba build"
        cd ../..

        # host tools
        rm -rf host/$SAMBA

        mkdir -p host
        tar -xzf $DOWNLOAD_DIR/$SAMBA.tar.gz -C host || die "samba/make_unicodemap host extract"

        cd host/$SAMBA/source
        ./configure >$LOGFILE 2>&1 || die "samba/make_unicodemap configure"
        make bin/make_unicodemap >$LOGFILE 2>&1 || die "samba/make_unicodemap build"
        cd ../../..

        touch $STAMPDIR/$SAMBA.buildstamp
        ;;
    sxetools)
        source_changed $QTOPIA_SOURCE_PATH/src/tools/sxe_tools $STAMPDIR/sxetools.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/sxetools

        mkdir $ROOTFS_BUILD_PATH/sxetools
        for f in $QTOPIA_SOURCE_PATH/src/tools/sxe_tools/test/*; do
            cp $f sxetools/. || die "sxe tools extract"
        done

        cd $ROOTFS_BUILD_PATH/sxetools
        CC=$CROSS_PREFIX-gcc make all >$LOGFILE 2>&1 || die "sxetools build"
        cd ..

        touch $STAMPDIR/sxetools.buildstamp
        ;;
    armioctl)
        source_changed $ROOTFS_SOURCE_PATH/armioctl $STAMPDIR/armioctl.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/armioctl

        mkdir $ROOTFS_BUILD_PATH/armioctl
        cp $ROOTFS_SOURCE_PATH/armioctl/* armioctl/ || die "armioctl extract"

        cd $ROOTFS_BUILD_PATH/armioctl
        CC=$CROSS_PREFIX-gcc make >$LOGFILE 2>&1 || die "armioctl build"
        cd ..

        touch $STAMPDIR/armioctl.buildstamp
        ;;
    tat)
        source_changed $ROOTFS_SOURCE_PATH/tat $STAMPDIR/tat.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/tat

        mkdir $ROOTFS_BUILD_PATH/tat
        cp $ROOTFS_SOURCE_PATH/tat/* tat/ || die "tat extract"

        cd $ROOTFS_BUILD_PATH/tat
        CFLAGS=-I$QTOPIA_SOURCE_PATH/devices/greenphone/include CC=$CROSS_PREFIX-gcc make -e >$LOGFILE 2>&1 || die "tat
build"
        cd ..

        touch $STAMPDIR/tat.buildstamp
        ;;
    getkeycode)
        source_changed $ROOTFS_SOURCE_PATH/getkeycode $STAMPDIR/getkeycode.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/getkeycode

        mkdir $ROOTFS_BUILD_PATH/getkeycode
        cp $ROOTFS_SOURCE_PATH/getkeycode/* getkeycode/ || die "getkeycode extract"

        cd $ROOTFS_BUILD_PATH/getkeycode
        CC=$CROSS_PREFIX-gcc make -e >$LOGFILE 2>&1 || die "getkeycode build"
        cd ..

        touch $STAMPDIR/getkeycode.buildstamp
        ;;
    bootcharger)
        source_changed $ROOTFS_SOURCE_PATH/bootcharger $STAMPDIR/bootcharger.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/bootcharger

        mkdir $ROOTFS_BUILD_PATH/bootcharger
        cp $ROOTFS_SOURCE_PATH/bootcharger/* bootcharger/ || die "bootcharger extract"

        cd $ROOTFS_BUILD_PATH/bootcharger
        CC=$CROSS_PREFIX-gcc CFLAGS=-I$ROOTFS_SOURCE_PATH/../include make -e >$LOGFILE 2>&1 || die "bootcharger build"
        cd ..

        touch $STAMPDIR/bootcharger.buildstamp
        ;;
    prelink)
        if source_changed $STAMPDIR/$LIBELF.buildstamp; then
            rm -rf $ROOTFS_BUILD_PATH/$LIBELF

            tar -xzf $DOWNLOAD_DIR/$LIBELF.tar.gz || die "prelink/libelf extract"

            cd $ROOTFS_BUILD_PATH/$LIBELF
            CC=$CROSS_PREFIX-gcc ./configure --host=$CROSS_PREFIX --prefix=/usr >/dev/null 2>/dev/null ||
                die "prelink/libelf configure"
            make >$LOGFILE 2>&1 || die "prelink/libelf build"
            cd ..

            touch $STAMPDIR/$LIBELF.buildstamp
        fi

        source_changed $ROOTFS_SOURCE_PATH/prelink $STAMPDIR/$PRELINK.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$PRELINK

        tar -xzf $DOWNLOAD_DIR/$PRELINK.orig.tar.gz || die "prelink extract"

        cd $ROOTFS_BUILD_PATH/$PRELINK
        for i in $ROOTFS_SOURCE_PATH/prelink/*.patch; do
            patch -p 1 < $i || die "prelink patch"
        done

        mkdir -p libelf/include/libelf libelf/lib
        for file in elf_repl.h gelf.h libelf.h nlist.h sys_elf.h; do
            cp ../$LIBELF/lib/$file libelf/include/libelf/ || die "prelink libelf include"
        done
        cp ../$LIBELF/lib/libelf.a libelf/lib/ || die "prelink libelf install"

        export CFLAGS="-I`pwd`/libelf/include -I`pwd`/libelf/include/libelf"
        export CPPFLAGS=$CFLAGS
        export LDFLAGS="-L`pwd`/libelf/lib"
        ./configure --host=$CROSS_PREFIX --prefix=/usr >/dev/null 2>/dev/null ||
            die "prelink configure"

        # src/execstack fails to build for arm
        # force make to continue anyway and test
        # if src/prelink has been built.
        make -k >$LOGFILE 2>&1
        [ -e src/prelink ] || die "prelink build"

        unset LDFLAGS
        unset CPPFLAGS
        unset CFLAGS
        cd ..
        
        touch $STAMPDIR/$PRELINK.buildstamp
        ;;
    dropbear)
        source_changed $ROOTFS_SOURCE_PATH/dropbear $STAMPDIR/$DROPBEAR.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$DROPBEAR

        tar -xzf $DOWNLOAD_DIR/$DROPBEAR.tar.gz || die "dropbear extract"

        cd $ROOTFS_BUILD_PATH/$DROPBEAR
        for file in $ROOTFS_SOURCE_PATH/dropbear/*.patch; do
            if [ "$file" != '$ROOTFS_SOURCE_PATH/dropbear/*.patch' ]; then
                patch < $file
            fi
        done

        ./configure --host=$CROSS_PREFIX --prefix=/usr --disable-zlib --disable-lastlog >$LOGFILE 2>&1 || die "dropbear configure"
        if [ -f $ROOTFS_SOURCE_PATH/dropbear/options.h ]; then
            cp $ROOTFS_SOURCE_PATH/dropbear/options.h .
        fi
        make PROGRAMS="dropbear dropbearkey dbclient scp" MULTI=1 SCPPROGRESS=1 >$LOGFILE 2>&1 || die "dropbear build"
        cd ..

        touch $STAMPDIR/$DROPBEAR.buildstamp
        ;;
    expat)
        source_changed $STAMPDIR/$EXPAT.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$EXPAT

        tar -xzf $DOWNLOAD_DIR/$EXPAT.tar.gz || die "expat extract"

        cd $ROOTFS_BUILD_PATH/$EXPAT
        ./configure --host=$CROSS_PREFIX --prefix=/usr >$LOGFILE 2>&1 || die "expat configure"
        make >$LOGFILE 2>&1 || die "expat build"
        cd ..

        touch $STAMPDIR/$EXPAT.buildstamp
        ;;
    dbus)
        source_changed $STAMPDIR/$DBUS.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$DBUS

        tar -xzf $DOWNLOAD_DIR/$DBUS.tar.gz || die "dbus extract"

        cd $ROOTFS_BUILD_PATH/$DBUS
        for i in $ROOTFS_SOURCE_PATH/dbus/*.patch; do
            patch -p 1 < $i || die "dbus patch"
        done
        autoreconf

        export CFLAGS="-I$ROOTFS_BUILD_PATH/$EXPAT/lib"
        export LDFLAGS="-L$ROOTFS_BUILD_PATH/$EXPAT/.libs"
        export ac_cv_have_abstract_sockets=yes

        ./configure --host=$CROSS_PREFIX --prefix=/usr --sysconfdir=/etc \
                    --localstatedir=/var --cache-file=config.cache \
                    --disable-qt --disable-qt3 --disable-gtk --disable-glib \
                    --disable-gcj --disable-mono --disable-python \
                    --disable-selinux \
                    --with-xml=expat --without-x >/dev/null 2>/dev/null ||
            die "dbus configure"

        make >$LOGFILE 2>&1 || die "dbus build"

        unset CFLAGS LDFLAGS ac_cv_have_abstract_sockets

        cd ..

        touch $STAMPDIR/$DBUS.buildstamp
        ;;
    bluez-libs)
        source_changed $STAMPDIR/$BLUEZLIBS.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BLUEZLIBS

        tar -xzf $DOWNLOAD_DIR/$BLUEZLIBS.tar.gz || die "bluez-libs extract"

        cd $ROOTFS_BUILD_PATH/$BLUEZLIBS
        ./configure --host=$CROSS_PREFIX --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-libs configure"
        make >$LOGFILE 2>&1 || die "bluez-libs build"
        cd ..

        touch $STAMPDIR/$BLUEZLIBS.buildstamp
        ;;
    bluez-utils)
        source_changed $ROOTFS_SOURCE_PATH/bluez $STAMPDIR/$BLUEZUTILS.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BLUEZUTILS

        tar -xzf $DOWNLOAD_DIR/$BLUEZUTILS.tar.gz || die "bluez-utils extract"

        cd $ROOTFS_BUILD_PATH/$BLUEZUTILS
        export BLUEZ_LIBS="-L$ROOTFS_BUILD_PATH/$BLUEZLIBS/src/.libs -lbluetooth"
        export BLUEZ_CFLAGS="-I$ROOTFS_BUILD_PATH/$BLUEZLIBS/include"
        export DBUS_CFLAGS="-I$ROOTFS_BUILD_PATH/$DBUS"
        export DBUS_LIBS="-L$ROOTFS_BUILD_PATH/$DBUS/dbus/.libs -ldbus-1"
        export CFLAGS="-I$ROOTFS_BUILD_PATH/$EXPAT/lib"
        export LDFLAGS="-L$ROOTFS_BUILD_PATH/$EXPAT/.libs"

        # Apply devdown fix for the greenphone
        cd hcid
        patch < $QTOPIA_SOURCE_PATH/devices/greenphone/rootfs/bluez/find_adapter.patch
        cd ..

        ./configure --host=$CROSS_PREFIX --prefix=/usr --sysconfdir=/etc \
                    --localstatedir=/etc/bluetooth \
                    --enable-test --enable-expat \
                    --disable-glib --disable-obex --disable-alsa \
                    --disable-hal --disable-usb --disable-gstreamer \
                    --disable-serial --disable-network --disable-sync \
                    --disable-echo --disable-sdpd --disable-hidd \
                    --disable-bccmd --disable-avctrl --disable-hid2hci \
                    --disable-dfutool --without-cups >/dev/null 2>/dev/null ||
            die "bluez-utils configure"

        make >$LOGFILE 2>&1 || die "bluez-utils build"

        unset BLUEZ_LIBS BLUEZ_CFLAGS DBUS_LIBS DBUS_CFLAGS CFLAGS LDFLAGS

        cd ..

        touch $STAMPDIR/$BLUEZUTILS.buildstamp
        ;;
    bluez-hcidump)
        source_changed $STAMPDIR/$BLUEZHCIDUMP.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BLUEZHCIDUMP

        tar -xzf $DOWNLOAD_DIR/$BLUEZHCIDUMP.tar.gz || die "bluez-hcidump extract"

        cd $ROOTFS_BUILD_PATH/$BLUEZHCIDUMP
        export BLUEZ_LIBS="-L$ROOTFS_BUILD_PATH/$BLUEZLIBS/src/.libs -lbluetooth"
        export BLUEZ_CFLAGS="-I$ROOTFS_BUILD_PATH/$BLUEZLIBS/include"

        ./configure --host=$CROSS_PREFIX --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-hcidump configure"
        make >$LOGFILE 2>&1 || die "bluez-hcidump build"

        unset BLUEZ_LIBS BLUEZ_CFLAGS

        cd ..

        touch $STAMPDIR/$BLUEZHCIDUMP.buildstamp
        ;;
    bluez-firmware)
        source_changed $STAMPDIR/$BLUEZFIRMWARE.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BLUEZFIRMWARE

        tar -xzf $DOWNLOAD_DIR/$BLUEZFIRMWARE.tar.gz || die "bluez-firmware extract"

        cd $ROOTFS_BUILD_PATH/$BLUEZFIRMWARE
        ./configure --host=$CROSS_PREFIX --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-firmware configure"
        make >$LOGFILE 2>&1 || die "bluez-firmware build"
        cd ..

        touch $STAMPDIR/$BLUEZFIRMWARE.buildstamp
        ;;
    wireless-tools)
        source_changed $STAMPDIR/$WIRELESSTOOLS.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$WIRELESSTOOLS

        tar -xzf $DOWNLOAD_DIR/$WIRELESSTOOLS.tar.gz || die "wireless-tools extract"

        cd $ROOTFS_BUILD_PATH/$WIRELESSTOOLS
        make CC=$CROSS_PREFIX-gcc iwmulticall
        cd ..

        touch $STAMPDIR/$WIRELESSTOOLS.buildstamp
        ;;
    wpa_supplicant)
        source_changed $ROOTFS_SOURCE_PATH/wpa_supplicant $STAMPDIR/$WPASUPPLICANT.buildstamp || return 0

        rm -rf $STAMPDIR/$WPASUPPLICANT.buildstamp

        tar -xzf $DOWNLOAD_DIR/$WPASUPPLICANT.tar.gz || die "wpa_supplicant extract"

        cd $ROOTFS_BUILD_PATH/$WPASUPPLICANT
        cp -f $ROOTFS_SOURCE_PATH/wpa_supplicant/config .config ||
            die "wpa_supplicant configure"
        make >$LOGFILE 2>&1 || die "wpa_supplicant build"
        cd ..

        touch $STAMPDIR/$WPASUPPLICANT.buildstamp
        ;;
    initrd)
        source_changed $ROOTFS_SOURCE_PATH/initrd $STAMPDIR/initrd.buildstamp || return 0

        rm -rf initrd

        mkdir initrd

        # insmod for initrd
        tar -C initrd -xjf $DOWNLOAD_DIR/$BUSYBOX.tar.bz2 || die "initrd busybox extract"

        cd initrd/$BUSYBOX
        for i in $ROOTFS_SOURCE_PATH/busybox/*.patch; do
            patch -p 1 < $i || die "initrd busybox patch"
        done
        cp $ROOTFS_SOURCE_PATH/initrd/busybox-config .config ||
            die "initrd busybox configure"
        make oldconfig >$LOGFILE 2>&1 || die "initrd busybox configure"
        make >$LOGFILE 2>&1 || die "initrd busybox build"
        cd ../../

        # linuxrc for initrd
        cd initrd
        cp $ROOTFS_SOURCE_PATH/initrd/linuxrc.c $ROOTFS_SOURCE_PATH/initrd/Makefile .
        make CROSS=$CROSS_PREFIX linuxrc >$LOGFILE 2>&1 || die "build initrd linuxrc"
        cd ..

        # make initrd image
        cd initrd
        dd if=/dev/zero of=$ROOTFS_BUILD_PATH/$INITRD_FILENAME bs=1024 seek=1024 count=0
        /sbin/mkfs.ext2 -F $ROOTFS_BUILD_PATH/$INITRD_FILENAME
        MNTPNT=`mktemp -d initrd.XXXXXX`
        sudo mount -o loop $ROOTFS_BUILD_PATH/$INITRD_FILENAME $MNTPNT || die "sudo failed at line $LINENO"
        
        sudo mkdir -p $MNTPNT/dev || die "sudo failed at line $LINENO"
        sudo mknod $MNTPNT/dev/console c 5 1 || die "sudo failed at line $LINENO"
        sudo cp -a linuxrc $MNTPNT/ || die "sudo failed at line $LINENO"
        sudo cp -a $ROOTFS_SOURCE_PATH/initrd/tffs.o $MNTPNT/ || die "sudo failed at line $LINENO"
        sudo umount $MNTPNT || die "sudo failed at line $LINENO"
        rmdir $MNTPNT
        cd ..

        touch $STAMPDIR/initrd.buildstamp
        ;;
    linux)
        CONFIG_PATH=arch/arm/def-configs/greenphone

        if [ $OPTION_DEVEL -eq 0 ]; then
            source_changed "$ROOTFS_SOURCE_PATH/linux" $STAMPDIR/$GPH_LINUX.buildstamp ||
            source_changed "$KERNEL_SOURCE_PATH" $STAMPDIR/$GPH_LINUX.buildstamp ||
            { [ $CONFIG_BOOTCHART -eq 1 ] && ! grep -q bootchart $ROOTFS_BUILD_PATH/$GPH_LINUX/$CONFIG_PATH; } ||
            { [ $CONFIG_BOOTCHART -eq 0 ] && grep -q bootchart $ROOTFS_BUILD_PATH/$GPH_LINUX/$CONFIG_PATH; } ||
            return 0
        fi

        if [ $OPTION_DEVEL -eq 0 ]; then
            # Grab everything from the depot
            rm -rf $GPH_LINUX

            if [ -x progress ]; then
                echo -n "   fetching kernel source"
                cp -av $KERNEL_SOURCE_PATH $GPH_LINUX | ./progress
            else
                cp -a $KERNEL_SOURCE_PATH $GPH_LINUX
            fi
        else
            # Grab only changed/added files from the depot
            if [ -x progress ]; then
                echo -n "   freshening kernel dirs"
                find $KERNEL_SOURCE_PATH -type d -printf '%P\0' |
                    xargs -0 -n 1 -i sh -c "echo x;test -d $GPH_LINUX/'{}' || mkdir -p $GPH_LINUX/'{}'" | ./progress
                echo -n "   freshening kernel files"
                find $KERNEL_SOURCE_PATH -type f -printf '%P\0' |
                    xargs -0 -n 1 -i sh -c "echo x;test -f $GPH_LINUX/'{}' || cp -l $KERNEL_SOURCE_PATH/'{}' $GPH_LINUX/'{}'" | ./progress /dev/null 11460
            else
                find $KERNEL_SOURCE_PATH -type d -printf '%P\0' |
                    xargs -0 -n 1 -i sh -c "test -d $GPH_LINUX/'{}' || mkdir -p $GPH_LINUX/'{}'"
                find $KERNEL_SOURCE_PATH -type f -printf '%P\0' |
                    xargs -0 -n 1 -i sh -c "test -f $GPH_LINUX/'{}' || cp -l $KERNEL_SOURCE_PATH/'{}' $GPH_LINUX/'{}'"
            fi
        fi

        cd $ROOTFS_BUILD_PATH/$GPH_LINUX

        if [ $CONFIG_BOOTCHART -eq 1 ] && ! grep -q bootchart $CONFIG_PATH; then
            sed -r -e 's|^(CONFIG_CMDLINE="[^"]+)"|\1 init=/sbin/bootchartd"|' -i $CONFIG_PATH || die "set init=bootchartd"
        fi
        if [ $CONFIG_BOOTCHART -eq 0 ] && grep -q bootchart $CONFIG_PATH; then
            sed -r -e '/^CONFIG_CMDLINE/ s| init=/sbin/bootchartd||' -i $CONFIG_PATH || die "remove init=bootchartd"
        fi


        if [ $OPTION_DEVEL -eq 0 ]; then
            # Only apply patches if not in DEVEL mode
            chmod -R u+w .
            for i in $ROOTFS_SOURCE_PATH/linux/*.patch; do
                if echo $i | grep -q -v -i 'lids' || [ $CONFIG_LIDS -eq 1 ]; then
                    patch -l -p 1 < $i || die "linux patch"
                fi
            done
        fi

        if [ -e $ROOTFS_BUILD_PATH/$INITRD_FILENAME ]; then
            chmod u+w init/initrd.bin
            cp $ROOTFS_BUILD_PATH/$INITRD_FILENAME init/initrd.bin ||
                die "linux initrd image"
        fi

        make greenphone_config >$LOGFILE 2>&1 || die "linux greenphone_config"

        if [ $OPTION_CONFIG_LINUX -eq 1 ]; then
            make menuconfig
        else
            if [ -x ../progress ]; then
                echo -n "   configuring"
                make oldconfig 2>&1 | ../progress $LOGFILE || die "linux config"
            else
                make oldconfig >$LOGFILE 2>&1 || die "linux config"
            fi
        fi

        if [ -x ../progress ]; then
            echo -n "   building deps"
            make dep 2>&1 | ../progress $LOGFILE || die "linux build dep"
            echo -n "   building image"
            make zImage 2>&1 | ../progress $LOGFILE || die "linux build" 
            echo -n "   building modules"
            make modules 2>&1 | ../progress $LOGFILE || die "linux modules build"
        else
            make dep >$LOGFILE 2>&1 || die "linux build dep"
            make zImage >$LOGFILE 2>&1 || die "linux build"
            make modules >$LOGFILE 2>&1 || die "linux modules build"
        fi

        cd ..

        touch $STAMPDIR/$GPH_LINUX.buildstamp
        ;;

    lids)
				if [ $CONFIG_LIDS -eq 1 ]; then
        # only build lids if kernel has lids support
        [ -r $ROOTFS_BUILD_PATH/$GPH_LINUX/kernel/lids.c ] || return 0

        source_changed "$ROOTFS_SOURCE_PATH/lids" $STAMPDIR/$LIDS.buildstamp ||
        source_changed "$ROOTFS_BUILD_PATH/$GPH_LINUX" $STAMPDIR/$LIDS.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$LIDS

        tar -xzf $DOWNLOAD_DIR/$LIDS.tar.gz || die "lids extract"

        cd $ROOTFS_BUILD_PATH/$LIDS
        for i in $ROOTFS_SOURCE_PATH/lids/*.patch; do
            patch -p 1 < $i || die "lids patch"
        done

        cd lidstools-0.5.6
        ./configure KERNEL_DIR=$ROOTFS_BUILD_PATH/$GPH_LINUX --prefix=/ --host=$CROSS_PREFIX >$LOGFILE 2>&1 || die "lids configure"
        cd src
        make >$LOGFILE 2>&1 || die "lids build"
        cd ..
        cd ..
        cd ..

        # build host version
        rm -rf host/$LIDS

        mkdir -p host
        tar -xzf $DOWNLOAD_DIR/$LIDS.tar.gz -C host || die "lids host extract"

        cd host/$LIDS
        for i in $ROOTFS_SOURCE_PATH/lids/*.patch; do
            patch -p 1 < $i || die "lids host patch"
        done

        cd lidstools-0.5.6
        ./configure KERNEL_DIR=$ROOTFS_BUILD_PATH/$GPH_LINUX LDFLAGS=-static --prefix=/ >$LOGFILE 2>&1 || die "lids host configure"
        cd src
        make >$LOGFILE 2>&1 || die "lids host build"
        cd ..
        cd ..
        cd ../..

        touch $STAMPDIR/$LIDS.buildstamp
fi
        ;;
   popt)
        source_changed $STAMPDIR/$POPT.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$POPT

        tar -xzf $DOWNLOAD_DIR/$POPT.tar.gz || die "popt extract"

        cd $ROOTFS_BUILD_PATH/$POPT
        ./configure --host=$CROSS_PREFIX --enable-static --prefix=/usr || die "popt configure"
        make  || die" popt build"
        touch $STAMPDIR/$POPT.buildstamp
				;;
		raid)
        source_changed $STAMPDIR/$RAID.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$RAID

        tar -xzf $DOWNLOAD_DIR/$RAID.tar.gz || die "raid extract"

        cd $ROOTFS_BUILD_PATH/$RAID

        for i in $ROOTFS_SOURCE_PATH/raid/*.diff; do
             patch -l -p 1 < $i || die "linux patch"
         done
        
 CPPFLAGS="-I$ROOTFS_BUILD_PATH/$POPT" ./configure --host=$CROSS_PREFIX --prefix=/usr  LDFLAGS="-L$ROOTFS_BUILD_PATH/$POPT/.libs" || die "raid configure"
# sleep 4
        make >$LOGFILE 2>&1 || die "raid build"
        touch $STAMPDIR/$RAID.buildstamp

						;;		
    openssl)
        source_changed $STAMPDIR/$OPENSSL.buildstamp || return 0

        # build target version
        rm -rf $ROOTFS_BUILD_PATH/$OPENSSL

        tar -xzf $DOWNLOAD_DIR/$OPENSSL.tar.gz || die "openssl extract"

        cd $ROOTFS_BUILD_PATH/$OPENSSL
        ./Configure --prefix=/usr -DL_ENDIAN shared linux-generic32
        make CC=arm-linux-gcc || die "openssl build"
        cd ..

        # build host version
        rm -rf host/$OPENSSL

        mkdir -p host
        tar -xzf $DOWNLOAD_DIR/$OPENSSL.tar.gz -C host || die "openssl host extract"

        cd host/$OPENSSL
        ./config --prefix=/usr || die "openssl host configure"
        make || die "openssl host build"
        cd ../..

        touch $STAMPDIR/$OPENSSL.buildstamp
        ;;
    bootchart)
        source_changed $ROOTFS_SOURCE_PATH/bootchart $STAMPDIR/$BOOTCHART.buildstamp || return 0

        rm -rf $ROOTFS_BUILD_PATH/$BOOTCHART

        tar -xjf $DOWNLOAD_DIR/$BOOTCHART.tar.bz2 || die "bootchart extract"

        cd $ROOTFS_BUILD_PATH/$BOOTCHART

        for i in $ROOTFS_SOURCE_PATH/bootchart/*.patch; do
            patch -p 1 < $i || die "bootchart patch"
        done

        touch $STAMPDIR/$BOOTCHART.buildstamp
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
}

install_package()
{
    echo "Installing $1"

    case $1 in
    toolchain)
    	if [ "$OWNTOOLCHAIN" = "1" ]; then
        TOOLCHAIN_ROOT=$TOOLCHAIN_PATH/$CROSS_PREFIX
			else
        TOOLCHAIN_ROOT=$RESULT_TOP/gcc-4.1.1-glibc-2.3.6/$CROSS_PREFIX/$CROSS_PREFIX

			fi
        LIB_LINKS=`find $TOOLCHAIN_ROOT/lib -type l -printf "%P\n"`
        LIBS=`echo $LIB_LINKS | tr ' ' '\n' | xargs -iXFILEX ls -l $TOOLCHAIN_ROOT/lib/XFILEX | awk '{print $NF}'`
        strip_install unneeded $TOOLCHAIN_ROOT/lib $ROOTFS_IMAGE_DIR/lib $LIBS $LIB_LINKS
        strip_install unneeded $TOOLCHAIN_ROOT/lib $ROOTFS_IMAGE_DIR/lib libSegFault.so libmemusage.so libpcprofile.so

    	if [ "$OWNTOOLCHAIN" = "1" ]; then
#        install all $TOOLCHAIN_ROOT/../bin $ROOTFS_IMAGE_DIR/usr/bin gdbserver 
        install $TOOLCHAIN_ROOT/$CROSS_PREFIX/sbin/ldd $ROOTFS_IMAGE_DIR/sbin 
        install $TOOLCHAIN_ROOT/$CROSS_PREFIX/sbin/ldconfig $ROOTFS_IMAGE_DIR/sbin 
#        install $ROOTFS_BUILD_PATH/$CROSSTOOL/build/$CROSS_PREFIX/gcc-4.1.1-glibc-2.3.6/build-glibc/elf $ROOTFS_IMAGE_DIR/sbin ldd
			else
        strip_install all $TOOLCHAIN_ROOT/../bin $ROOTFS_IMAGE_DIR/usr/bin gdbserver 
        strip_install all $ROOTFS_BUILD_PATH/$CROSSTOOL/build/$CROSS_PREFIX/gcc-4.1.1-glibc-2.3.6/build-glibc/elf $ROOTFS_IMAGE_DIR/sbin ldconfig
        install $ROOTFS_BUILD_PATH/$CROSSTOOL/build/$CROSS_PREFIX/gcc-4.1.1-glibc-2.3.6/build-glibc/elf $ROOTFS_IMAGE_DIR/sbin ldd
			fi
        install $TOOLCHAIN_ROOT/etc $ROOTFS_IMAGE_DIR/etc rpc
        ;;
    busybox)
        cd $ROOTFS_BUILD_PATH/$BUSYBOX
        make install linkopts="-s" INSTALL_OPTS=--noclobber PREFIX=$ROOTFS_IMAGE_DIR >$LOGFILE 2>&1
        cd ..
        ;;
    libungif)
        cd $ROOTFS_BUILD_PATH/$LIBUNGIF
        strip_install unneeded lib/.libs $ROOTFS_IMAGE_DIR/usr/lib libungif.so.4 libungif.so.4.1.4
        cd ..
        ;;
    ilib)
        cd $ROOTFS_BUILD_PATH/$ILIB
        strip_install all addtext $ROOTFS_IMAGE_DIR/bin addtext
        cd ..
        ;;
    dosfstools)
        cd $ROOTFS_BUILD_PATH/$DOSFSTOOLS
        strip_install all dosfsck $ROOTFS_IMAGE_DIR/sbin dosfsck
        ln -s dosfsck $ROOTFS_IMAGE_DIR/sbin/fsck.vfat
        strip_install all mkdosfs $ROOTFS_IMAGE_DIR/sbin mkdosfs
        ln -s mkdosfs $ROOTFS_IMAGE_DIR/sbin/mkfs.vfat
        cd ..
        ;;
    strace)
        cd $ROOTFS_BUILD_PATH/$STRACE
        strip_install all . $ROOTFS_IMAGE_DIR/usr/bin strace
        cd ..
        ;;
    ppp)
        cd $ROOTFS_BUILD_PATH/$PPP
        strip_install all pppdump $ROOTFS_IMAGE_DIR/usr/bin pppdump
        strip_install all pppstats $ROOTFS_IMAGE_DIR/usr/bin pppstats
        strip_install all chat $ROOTFS_IMAGE_DIR/usr/sbin chat
        strip_install all pppd $ROOTFS_IMAGE_DIR/usr/sbin pppd
        install etc.ppp $ROOTFS_IMAGE_DIR/etc/ppp chap-secrets options pap-secrets
        mkdir -p $ROOTFS_IMAGE_DIR/etc/ppp/peers
        mkdir -p $ROOTFS_IMAGE_DIR/etc
        cd ..
        ;;
    samba)
        cd $ROOTFS_BUILD_PATH/$SAMBA/source
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/bin
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/lib
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/lib/codepages
        mkdir -p $ROOTFS_IMAGE_DIR/var/run/samba
        ln -sf /var/run/samba $ROOTFS_IMAGE_DIR/usr/local/samba/var

        strip_install all bin $ROOTFS_IMAGE_DIR/usr/local/samba/bin smbd nmbd

        mkdir -p $ROOTFS_IMAGE_DIR/etc/samba
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/private
        ln -sf /etc/samba/secrets.tdb $ROOTFS_IMAGE_DIR/usr/local/samba/private/secrets.tdb
        cd ../..

        cd host/$SAMBA/source
        bin/make_unicodemap 932 codepages/CP932.TXT $ROOTFS_IMAGE_DIR/usr/local/samba/lib/codepages/unicode_map.932
        cd ../../..
        ;;
    sxetools)
        strip_install all sxetools $ROOTFS_IMAGE_DIR/usr/bin proc_keys sandboxed suid
        # these two binaries are identical (the same file) but will get different lids rules
        cp $ROOTFS_IMAGE_DIR/usr/bin/suid $ROOTFS_IMAGE_DIR/usr/bin/suid2
        # create some arbitrary text in a test file
        mkdir -p $ROOTFS_IMAGE_DIR/etc/sxetools
        cal > $ROOTFS_IMAGE_DIR/etc/sxetools/test_text
        ;;
    armioctl)
        strip_install all armioctl $ROOTFS_IMAGE_DIR/usr/bin arm_ioctl
        ;;
    tat)
        strip_install all tat $ROOTFS_IMAGE_DIR/usr/bin tat
        ;;
    getkeycode)
        strip_install all getkeycode $ROOTFS_IMAGE_DIR/bin getkeycode
        ;;
    bootcharger)
        strip_install all bootcharger $ROOTFS_IMAGE_DIR/usr/bin bootcharger
        install bootcharger $ROOTFS_IMAGE_DIR/usr/share/bootcharger charging.gif fully_charged.gif powering_off.gif
        ;;
    prelink)
        strip_install all $PRELINK/src $ROOTFS_IMAGE_DIR/usr/sbin prelink
        ;;
    dropbear)
        strip_install all $DROPBEAR $ROOTFS_IMAGE_DIR/usr/sbin dropbearmulti

        ln -sf dropbearmulti $ROOTFS_IMAGE_DIR/usr/sbin/dropbear
        ln -sf dropbearmulti $ROOTFS_IMAGE_DIR/usr/sbin/dropbearkey

        mkdir -p $ROOTFS_IMAGE_DIR/usr/bin
        ln -sf ../sbin/dropbearmulti $ROOTFS_IMAGE_DIR/usr/bin/dbclient
        ln -sf ../sbin/dropbearmulti $ROOTFS_IMAGE_DIR/usr/bin/scp

        mkdir -p $ROOTFS_IMAGE_DIR/etc/dropbear
        ;;
    expat)
        cd $ROOTFS_BUILD_PATH/$EXPAT
        strip_install unneeded .libs $ROOTFS_IMAGE_DIR/usr/lib libexpat.so.1 libexpat.so.1.5.0
        cd ..
        ;;
    dbus)
        cd $ROOTFS_BUILD_PATH/$DBUS
        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1

        mkdir -p $ROOTFS_IMAGE_DIR/var/run/dbus

        install .install/etc/dbus-1 $ROOTFS_IMAGE_DIR/etc/dbus-1 session.conf system.conf
        strip_install all .install/usr/bin $ROOTFS_IMAGE_DIR/usr/bin dbus-cleanup-sockets dbus-launch dbus-send dbus-uuidgen
        strip_install all bus/.libs $ROOTFS_IMAGE_DIR/usr/bin dbus-daemon
        strip_install unneeded .install/usr/lib $ROOTFS_IMAGE_DIR/usr/lib libdbus-1.so.3 libdbus-1.so.3.2.0
        cd ..
        ;;
    bluez-libs)
        cd $ROOTFS_BUILD_PATH/$BLUEZLIBS
        strip_install unneeded src/.libs $ROOTFS_IMAGE_DIR/usr/lib libbluetooth.so.2 libbluetooth.so.2.8.5
        cd ..
        ;;
    bluez-utils)
        cd $ROOTFS_BUILD_PATH/$BLUEZUTILS
        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1

        mkdir -p $ROOTFS_IMAGE_DIR/etc/bluetooth/lib

        install .install/etc/dbus-1/system.d $ROOTFS_IMAGE_DIR/etc/dbus-1/system.d bluetooth.conf
        strip_install all .install/usr/bin $ROOTFS_IMAGE_DIR/usr/bin ciptool dund hcitool l2ping pand rfcomm sdptool
        strip_install all .install/usr/sbin $ROOTFS_IMAGE_DIR/usr/sbin hciattach hciconfig hcid
        strip_install all test/.libs $ROOTFS_IMAGE_DIR/usr/sbin bdaddr
        cd ..

        cp -a $QTOPIA_SOURCE_PATH/devices/greenphone/rootfs/bluez/hcid.conf $ROOTFS_IMAGE_DIR/etc/bluetooth/hcid.conf || die "Couldn't copy greenphone hcid.conf"
        ;;
    bluez-hcidump)
        cd $ROOTFS_BUILD_PATH/$BLUEZHCIDUMP
        strip_install all src $ROOTFS_IMAGE_DIR/usr/sbin hcidump
        cd ..
        ;;
    bluez-firmware)
        cd $ROOTFS_BUILD_PATH/$BLUEZFIRMWARE
        install st $ROOTFS_IMAGE_DIR/usr/lib/firmware STLC2500_R4_00_03.ptc STLC2500_R4_00_06.ssf STLC2500_R4_02_02_WLAN.ssf STLC2500_R4_02_04.ptc
        mkdir -p $ROOTFS_IMAGE_DIR/lib
        ln -sf /usr/lib/firmware $ROOTFS_IMAGE_DIR/lib/firmware
        cd ..
        ;;
    wireless-tools)
        cd $ROOTFS_BUILD_PATH/$WIRELESSTOOLS
        make PREFIX=`pwd`/.install install-iwmulticall >/dev/null 2>/dev/null
        strip_install all .install/sbin $ROOTFS_IMAGE_DIR/sbin iwconfig iwgetid iwlist iwpriv iwspy
        cd ..
        ;;
    wpa_supplicant)
        cd $ROOTFS_BUILD_PATH/$WPASUPPLICANT
        strip_install all . $ROOTFS_IMAGE_DIR/sbin wpa_supplicant wpa_cli wpa_passphrase
        cd ..
        ;;
    initrd)
        ;;
    linux)
        cd $ROOTFS_BUILD_PATH/$GPH_LINUX
        make MODLIB=$ROOTFS_IMAGE_DIR/lib/modules DEPMOD=true modules_install >$LOGFILE 2>&1
        rm -f $ROOTFS_IMAGE_DIR/lib/modules/build

        cp arch/arm/boot/zImage $ROOTFS_BUILD_PATH/$KERNEL_FILENAME

        # Check if kernel will fit in allocated flash space
        if [ ! -e $ROOTFS_BUILD_PATH/$KERNEL_FILENAME ]; then
            die "linux install kernel doesn't exist."
        fi

        KERNEL_SIZE=`stat -c '%s' $ROOTFS_BUILD_PATH/$KERNEL_FILENAME`

        [ $KERNEL_SIZE -le $KERNEL_SIZELIMIT ] ||
            die "linux install kernel larger than allocated flash size. $KERNEL_SIZE > $KERNEL_SIZELIMIT."

        cd ..
        ;;
    lids)
			 if [ $CONFIG_LIDS -eq 1 ]; then
        [ -r $STAMPDIR/$LIDS.buildstamp ] || return 0

        cd $ROOTFS_BUILD_PATH/$LIDS/lidstools-0.5.6/src

        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1

        strip_install all .install/sbin $ROOTFS_IMAGE_DIR/sbin lidsadm lidsconf

        cd ../../..

        cd host/$LIDS/lidstools-0.5.6/src

        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1
        mkdir -p .install/etc/lids

        # The configuration files for lids are stored in two places
        # tffsa1:/etc/lids contains the BOOT rules. These are RO
        # tffsc:/etc/lids contains the POSTBOOT rules. These are RW. tffsc:/etc/lids
        # is bind mounted over tffsa1:/etc/lids before switching to POSTBOOT.

        # Generate lids password
        LIDS_PASSWORD=greenphone
        echo -n -e "$LIDS_PASSWORD\n$LIDS_PASSWORD\n" | sudo chroot `pwd`/.install /sbin/lidsconf -P >/dev/null 2>&1 || die "sudo failed at line $LINENO"
        mkdir -p $ROOTFS_IMAGE_DIR/etc/lids
        echo "$LIDS_PASSWORD" > $ROOTFS_IMAGE_DIR/etc/lids/lids.secret

        mkdir -p $ROOTFS_IMAGE_DIR/etc/lids
        # all capabilities enabled at BOOT
        sed -e 's/^-/+/g' -e 's/^+30/-30/g' < ../example/lids.boot.cap > $ROOTFS_IMAGE_DIR/etc/lids/lids.boot.cap

        install ../example $ROOTFS_IMAGE_DIR/etc/lids lids.ini lids.cap lids.postboot.cap lids.shutdown.cap
        install .install/etc/lids $ROOTFS_IMAGE_DIR/etc/lids lids.pw
        # No ACLs defined in ro root partition
        touch $ROOTFS_IMAGE_DIR/etc/lids/lids.conf
        touch $ROOTFS_IMAGE_DIR/etc/lids/lids.boot.conf
        touch $ROOTFS_IMAGE_DIR/etc/lids/lids.postboot.conf
        touch $ROOTFS_IMAGE_DIR/etc/lids/lids.shutdown.conf

        cd ../../..
fi
        ;;
		popt)
        cd $ROOTFS_BUILD_PATH/$POPT
        strip_install unneeded .libs $ROOTFS_IMAGE_DIR/usr/lib libpopt.so libpopt.so.0 libpopt.so.0.0.0
        cd ..
				;;
    openssl)
        cd $ROOTFS_BUILD_PATH/$OPENSSL
        make INSTALL_PREFIX=$(pwd)/.install install >/dev/null 2>/dev/null
        strip_install unneeded .install/usr/lib $ROOTFS_IMAGE_DIR/usr/lib libcrypto.so libcrypto.so.0.9.8
        strip_install unneeded .install/usr/lib $ROOTFS_IMAGE_DIR/usr/lib libssl.so libssl.so.0.9.8
        for file in $(find .install/usr/lib/engines -printf '%P\n'); do
            strip_install unneeded .install/usr/lib/engines $ROOTFS_IMAGE_DIR/usr/lib/engines $file
        done
        install .install/usr/ssl $ROOTFS_IMAGE_DIR/usr/ssl openssl.cnf
        cd ..
        
        mkdir -p $ROOTFS_IMAGE_DIR/usr/ssl/certs
        cp $ROOTFS_BUILD_PATH/host/$OPENSSL/certs/*.0 $ROOTFS_IMAGE_DIR/usr/ssl/certs

        ;;
    bootchart)
        cd $ROOTFS_BUILD_PATH/$BOOTCHART

        install script $ROOTFS_IMAGE_DIR/sbin bootchartd
        install script $ROOTFS_IMAGE_DIR/etc  bootchartd.conf

        install /dev/null $ROOTFS_IMAGE_DIR/var/tmp/bootchart

        ;;
    basefiles)
        install_basefiles || die "Basefiles install"
        ;;
		raid)
        cd $ROOTFS_BUILD_PATH/$RAID
        strip_install all $ROOTFS_BUILD_PATH/$RAID  $ROOTFS_IMAGE_DIR/bin lsraid
        strip_install all $ROOTFS_BUILD_PATH/$RAID  $ROOTFS_IMAGE_DIR/bin mkraid
        strip_install all $ROOTFS_BUILD_PATH/$RAID  $ROOTFS_IMAGE_DIR/bin raidstart
        strip_install all $ROOTFS_BUILD_PATH/$RAID  $ROOTFS_IMAGE_DIR/bin raidreconf
				sudo cp  $ROOTFS_SOURCE_PATH/raid/raidtab $ROOTFS_IMAGE_DIR/etc
				sudo cp -v $ROOTFS_SOURCE_PATH/raid/fstab $ROOTFS_IMAGE_DIR/etc
				sudo cp -v $ROOTFS_SOURCE_PATH/raid/rc.filesystems $ROOTFS_IMAGE_DIR/etc/rc.d
        cd ..
				 ;;
    *)
        die "Unknown package $package"
        ;;
    esac
}

install_basefiles()
{
    cd $ROOTFS_IMAGE_DIR

    # Create FHS standard directories
    mkdir -p bin dev etc lib media mnt opt proc sbin usr var home

    # SD card mount point
    mkdir -p media/sdcard

    # Mount points for user data
    mkdir -p home
    mkdir -p mnt/documents

    # Qtopia
    mkdir -p opt/Qtopia

    # Writable /etc
    mkdir etc/resolvconf

    # Kernel modules
    mkdir -p lib/modules

    # Various temporary mount points for debugging
    mkdir -p mnt/nfs mnt/loop mnt/other

    # Populate /var and /tmp
    mkdir -p var var/log var/lock/subsys var/mnt var/run var/spool/at var/tmp
    ln -s var/tmp tmp

    # Directory for Beep Science DRM database
    mkdir -p etc/bscidrm2

    # Create minimal /dev
    # The real /dev will be a tmpfs and populated with the contents of
    # /dev.tar.gz on boot.  This is required as CRAMFS shares inode #1
    # between all empty directories, char, block, pipe, and sock inodes.
    # This is not compatible with LIDS which uses block device and inode
    # information in ACLs.
    cd $ROOTFS_IMAGE_DIR/dev

    sudo mknod initctl p || die "sudo failed at line $LINENO"

    sudo mknod mem     c 1 1 || die "sudo failed at line $LINENO"
    sudo mknod kmem    c 1 2 || die "sudo failed at line $LINENO"
    sudo mknod null    c 1 3 || die "sudo failed at line $LINENO"
    sudo mknod zero    c 1 5 || die "sudo failed at line $LINENO"
    sudo mknod random  c 1 8 || die "sudo failed at line $LINENO"
    sudo mknod urandom c 1 9 || die "sudo failed at line $LINENO"

    sudo mknod ttyS0 c 4 64 || die "sudo failed at line $LINENO"
    sudo mknod ttyS1 c 4 65 || die "sudo failed at line $LINENO"
    sudo mknod ttyS2 c 4 66 || die "sudo failed at line $LINENO"

    sudo mknod tty     c 5 0 || die "sudo failed at line $LINENO"
    sudo mknod console c 5 1 || die "sudo failed at line $LINENO"

    sudo mknod fb0 c 29 0 || die "sudo failed at line $LINENO"
    sudo ln -s fb0 fb || die "sudo failed at line $LINENO"

    sudo mknod tffsa  b 100  0 || die "sudo failed at line $LINENO"
    sudo mknod tffsa1 b 100  1 || die "sudo failed at line $LINENO"
    sudo mknod tffsa2 b 100  2 || die "sudo failed at line $LINENO"
    sudo mknod tffsb  b 100 32 || die "sudo failed at line $LINENO"
    sudo mknod tffsc  b 100 64 || die "sudo failed at line $LINENO"
    sudo mknod tffsd  b 100 96 || die "sudo failed at line $LINENO"
    sudo ln -s tffsa1 root || die "sudo failed at line $LINENO"

    sudo mknod mmca  b 241 0 || die "sudo failed at line $LINENO"
    sudo mknod mmca1 b 241 1 || die "sudo failed at line $LINENO"
    sudo mknod mmca2 b 241 2 || die "sudo failed at line $LINENO"

    # Create complete /dev
    cd $DEVFS_IMAGE_DIR

    mkdir pts

    sudo mknod initctl p || die "sudo failed at line $LINENO"

    sudo mknod mem c 1 1 || die "sudo failed at line $LINENO"
    sudo mknod kmem c 1 2 || die "sudo failed at line $LINENO"
    sudo mknod null c 1 3 || die "sudo failed at line $LINENO"
    sudo mknod zero c 1 5 || die "sudo failed at line $LINENO"
    sudo mknod random c 1 8 || die "sudo failed at line $LINENO"
    sudo mknod urandom c 1 9 || die "sudo failed at line $LINENO"

    sudo mknod ptyp0 c 2 0 || die "sudo failed at line $LINENO"

    sudo mknod ttyp0 c 3 0 || die "sudo failed at line $LINENO"

    sudo mknod tty0 c 4 0 || die "sudo failed at line $LINENO"
    sudo mknod tty1 c 4 1 || die "sudo failed at line $LINENO"
    sudo mknod tty2 c 4 2 || die "sudo failed at line $LINENO"
    sudo mknod tty3 c 4 3 || die "sudo failed at line $LINENO"
    sudo mknod tty4 c 4 4 || die "sudo failed at line $LINENO"
    sudo mknod tty5 c 4 5 || die "sudo failed at line $LINENO"
    sudo mknod tty6 c 4 6 || die "sudo failed at line $LINENO"
    sudo mknod tty7 c 4 7 || die "sudo failed at line $LINENO"
    sudo mknod tty8 c 4 8 || die "sudo failed at line $LINENO"

    sudo mknod ttyS0 c 4 64 || die "sudo failed at line $LINENO"
    sudo mknod ttyS1 c 4 65 || die "sudo failed at line $LINENO"
    sudo mknod ttyS2 c 4 66 || die "sudo failed at line $LINENO"

    sudo mknod tty c 5 0 || die "sudo failed at line $LINENO"
    sudo mknod console c 5 1 || die "sudo failed at line $LINENO"
    sudo mknod ptmx c 5 2 || die "sudo failed at line $LINENO"

    sudo mknod ts c 10 17 || die "sudo failed at line $LINENO"
    sudo mknod imm c 10 63 || die "sudo failed at line $LINENO"
    sudo mknod dpmc c 10 90 || die "sudo failed at line $LINENO"
    sudo mknod ipmc c 10 90 || die "sudo failed at line $LINENO"
    sudo mknod watchdog c 10 130 || die "sudo failed at line $LINENO"

    sudo mknod mixer0 c 14 0 || die "sudo failed at line $LINENO"
    sudo mknod sequencer c 14 1 || die "sudo failed at line $LINENO"
    sudo mknod midi00 c 14 2 || die "sudo failed at line $LINENO"
    sudo mknod dsp0 c 14 3 || die "sudo failed at line $LINENO"
    sudo mknod audio0 c 14 4 || die "sudo failed at line $LINENO"
    sudo mknod dspW0 c 14 5 || die "sudo failed at line $LINENO"
    sudo mknod sndstat c 14 6 || die "sudo failed at line $LINENO"
    sudo mknod dmfm0 c 14 7 || die "sudo failed at line $LINENO"
    sudo mknod mixer1 c 14 16 || die "sudo failed at line $LINENO"
    sudo mknod midi01 c 14 18 || die "sudo failed at line $LINENO"
    sudo mknod dsp1 c 14 19 || die "sudo failed at line $LINENO"
    sudo mknod audio1 c 14 20 || die "sudo failed at line $LINENO"
    sudo mknod dspW1 c 14 21 || die "sudo failed at line $LINENO"
    sudo mknod mixer2 c 14 32 || die "sudo failed at line $LINENO"
    sudo mknod midi02 c 14 34 || die "sudo failed at line $LINENO"
    sudo mknod dsp2 c 14 35 || die "sudo failed at line $LINENO"
    sudo mknod audio2 c 14 36 || die "sudo failed at line $LINENO"
    sudo mknod dspW2 c 14 37 || die "sudo failed at line $LINENO"
    sudo mknod mixer3 c 14 48 || die "sudo failed at line $LINENO"
    sudo mknod midi03 c 14 50 || die "sudo failed at line $LINENO"
    sudo mknod dsp3 c 14 51 || die "sudo failed at line $LINENO"
    sudo mknod audio3 c 14 52 || die "sudo failed at line $LINENO"
    sudo mknod dspW3 c 14 53 || die "sudo failed at line $LINENO"
    sudo ln -s audio0 audio || die "sudo failed at line $LINENO"
    sudo ln -s mixer0 audioctl || die "sudo failed at line $LINENO"
    sudo ln -s dmfm0 dmfm || die "sudo failed at line $LINENO"
    sudo ln -s dsp0 dsp || die "sudo failed at line $LINENO"
    sudo ln -s dspW0 dspW || die "sudo failed at line $LINENO"
    sudo ln -s dsp0 dspdefault || die "sudo failed at line $LINENO"
    sudo ln -s midi00 midi || die "sudo failed at line $LINENO"
    sudo ln -s mixer0 mixer || die "sudo failed at line $LINENO"

    sudo mknod fb0 c 29 0 || die "sudo failed at line $LINENO"
    sudo mknod fb1 c 29 32 || die "sudo failed at line $LINENO"
    sudo mknod fb2 c 29 64 || die "sudo failed at line $LINENO"
    sudo mknod fb3 c 29 96 || die "sudo failed at line $LINENO"
    sudo mknod fb4 c 29 128 || die "sudo failed at line $LINENO"
    sudo mknod fb5 c 29 160 || die "sudo failed at line $LINENO"
    sudo mknod fb6 c 29 192 || die "sudo failed at line $LINENO"
    sudo mknod fb7 c 29 224 || die "sudo failed at line $LINENO"
    sudo ln -s fb0 fb || die "sudo failed at line $LINENO"

    sudo mknod ttyP0 c 57 0 || die "sudo failed at line $LINENO"
    sudo mknod ttyP1 c 57 1 || die "sudo failed at line $LINENO"
    sudo mknod ttyP2 c 57 2 || die "sudo failed at line $LINENO"
    sudo mknod ttyP3 c 57 3 || die "sudo failed at line $LINENO"

    sudo mknod ixs_oscr c 59 0 || die "sudo failed at line $LINENO"

    sudo mknod video0 c 81 0 || die "sudo failed at line $LINENO"
    sudo ln -s video0 video || die "sudo failed at line $LINENO"

    sudo mknod ppp c 108 0 || die "sudo failed at line $LINENO"

    sudo mknod ttyUSB0 c 188 0 || die "sudo failed at line $LINENO"

    i=0
    while [ $i -le 31 ]; do
        sudo mknod rfcomm$i c 216 $i || die "sudo failed at line $LINENO"
        i=$(($i + 1))
    done

    sudo mknod omega_bt c 250 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_vibrator c 57 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_detect c 102 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_alarm c 120 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_rtcalarm c 121 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_kpbl c 127 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_bcm2121 c 200 0 || die "sudo failed at line $LINENO"
    sudo mknod IPMC_FORCE_SLEEP c 201 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_chgled c 251 0 || die "sudo failed at line $LINENO"
    sudo mknod sdcard_pm c 252 0 || die "sudo failed at line $LINENO"
    sudo mknod camera c 253 0 || die "sudo failed at line $LINENO"
    sudo mknod lcdctrl c 254 0 || die "sudo failed at line $LINENO"
    sudo mknod omega_lcdctrl c 254 0 || die "sudo failed at line $LINENO"

    sudo mknod loop0 b 7 0 || die "sudo failed at line $LINENO"
    sudo mknod loop1 b 7 1 || die "sudo failed at line $LINENO"
    sudo mknod loop2 b 7 2 || die "sudo failed at line $LINENO"
    sudo mknod loop3 b 7 3 || die "sudo failed at line $LINENO"
    sudo mknod loop4 b 7 4 || die "sudo failed at line $LINENO"
    sudo mknod loop5 b 7 5 || die "sudo failed at line $LINENO"
    sudo mknod loop6 b 7 6 || die "sudo failed at line $LINENO"
    sudo mknod loop7 b 7 7 || die "sudo failed at line $LINENO"

    sudo mknod docparatable b 58 1 || die "sudo failed at line $LINENO"
    
    sudo mknod flh0 b 60 0 || die "sudo failed at line $LINENO"
    sudo mknod flh1 b 60 1 || die "sudo failed at line $LINENO"
    sudo mknod flh2 b 60 2 || die "sudo failed at line $LINENO"
    sudo mknod flh3 b 60 3 || die "sudo failed at line $LINENO"
    
    sudo mknod mmca  b 241 0 || die "sudo failed at line $LINENO"
    sudo mknod mmca1 b 241 1 || die "sudo failed at line $LINENO"
    sudo mknod mmca2 b 241 2 || die "sudo failed at line $LINENO"

    sudo mknod ram0 b 1 0 || die "sudo failed at line $LINENO"
    sudo mknod ram1 b 1 1 || die "sudo failed at line $LINENO"
    sudo mknod ram2 b 1 2 || die "sudo failed at line $LINENO"
    sudo mknod ram3 b 1 3 || die "sudo failed at line $LINENO"

    sudo mknod tffsa  b 100 0 || die "sudo failed at line $LINENO"
    sudo mknod tffsa1 b 100 1 || die "sudo failed at line $LINENO"
    sudo mknod tffsa2 b 100 2 || die "sudo failed at line $LINENO"
    sudo mknod tffsb  b 100 32 || die "sudo failed at line $LINENO"
    sudo mknod tffsc  b 100 64 || die "sudo failed at line $LINENO"
    sudo mknod tffsd  b 100 96 || die "sudo failed at line $LINENO"
    sudo ln -s tffsa1 root || die "sudo failed at line $LINENO"

    if [ $CONFIG_RAID -eq 1 ]; then
        sudo mknod md0 b 9 0  || die "sudo failed at line $LINENO"
    fi

    # Install oui.txt required for Bluetooth
    cd $ROOTFS_IMAGE_DIR
    wget -P usr/share/misc -nc $VERBOSITY http://standards.ieee.org/regauth/oui/oui.txt

    # Greenphone base files
    files=`find $ROOTFS_SOURCE_PATH/basefiles -mindepth 1 -maxdepth 1 2>/dev/null`
    for i in $files; do
        sudo cp -a $i $ROOTFS_IMAGE_DIR || die "sudo failed at line $LINENO"
    done
    ln -s . $ROOTFS_IMAGE_DIR/lib/modules/2.4.19-rmk7-pxa2-greenphone

    # Setup symlinks in /etc
    sudo ln -s /proc/mounts $ROOTFS_IMAGE_DIR/etc/mtab || die "sudo failed at line $LINENO"

    cd ..
}

create_default_tgzs()
{
    echo "Creating default tarballs"

    cd $DEVFS_IMAGE_DIR || die "cd $DEVFS_IMAGE_DIR"
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/dev.tar.gz * || die "sudo failed at line $LINENO"

    cd $ROOTFS_IMAGE_DIR/var || die "cd $ROOTFS_IMAGE_DIR/var"
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/var.tar.gz * || die "sudo failed at line $LINENO"

    cd $ROOTFS_BUILD_PATH
}

make_rootfs_image()
{
    echo "Creating intermediate root filesystem image"

    rm -f $ROOTFS_BUILD_PATH/$ROOTFS_FILENAME

    cd $ROOTFS_IMAGE_DIR
    sudo tar --owner=0 --group=0 -czf $ROOTFS_BUILD_PATH/$ROOTFS_FILENAME * || die "sudo failed at line $LINENO"
}


usage()
{
    echo -e "Usage: `basename $0` [--clean] [--build] [--install] [--image] [--packages=<package list>] [--source-url <url>]"
    echo -e "If no options are specified defaults to --build --install --images\n" \
            "   --clean          Clean build directory.\n" \
            "   --build          Build packages, only if not previously built.\n" \
            "   --install        Install built packages.\n" \
            "   --image          Make rootfs image.\n" \
            "   --packages=      Select packages for build/install. <package list> is a comma seperated list of packages.\n" \
            "   --interactive    Start an interactive shell on error.\n" \
            "   --develmode      Enable devel mode. When enabled some things are done differently. This mode is NOT suppoted.\n" \
            "   --config-linux   Run \"make menuconfig\" when building Linux.\n" \
            "   --kernel-source  Location of Linux kernel source tree.\n" \
            "   --source-url     Location to download source packages from.\n" \
            "   --stampdir       path to build stamps directory.\n" \
            "   --own-toolchain  Don't build toolchain.\n" \
            "   --cross-prefix   Cross toolchain prefix.\n" \
            "   --toolchain-path Path to pre built toolchain.\n" \
            "   --verbose        Use verbose download.\n" \
            "   --download-dir   Location to download source packages to.\n" \
            "   --no-lids        Do not use lids.\n" \
            "   --raid           Use software raid.\n" \
            "   --bootchart      Use bootchartd as init process (requires kernel recompile).\n\n" \
            "Available packages:\n" \
            "$ALL_PACKAGES\n\n" \
            "Running without any options will build a rootfs image.\n\n" \
            "Using $QTOPIA_SOURCE_PATH as Qtopia source path.  Override with QTOPIA_DEPOT_PATH environment variable if required.\n"
}

. `dirname $0`/functions

DEFAULT_OPTIONS=1
OPTION_CLEAN=0
OPTION_BUILD=0
OPTION_INSTALL=0
OPTION_IMAGE=0
OPTION_INTERACTIVE=0
OPTION_DEVEL=0
OPTION_CONFIG_LINUX=0
OWNTOOLCHAIN=0
DOWNLOAD_DIR=$PWD
STAMPDIR=$PWD
BUILDDIR=
CONFIG_LIDS=1
CONFIG_RAID=0
CONFIG_BOOTCHART=0

TOOLCHAIN_PATH="/opt/toolchains/greenphone/gcc-4.1.1-glibc-2.3.6/arm-linux"
CROSS_PREFIX="arm-linux"

if [ -z $QTOPIA_DEPOT_PATH ]; then
    QTOPIA_SOURCE_PATH=$(dirname $(dirname $(readlink -f $0)))
else
    QTOPIA_SOURCE_PATH=$QTOPIA_DEPOT_PATH
fi

# The order that some of these packages are built in is important.
ALL_PACKAGES="toolchain busybox"
ALL_PACKAGES="$ALL_PACKAGES libungif ilib"
ALL_PACKAGES="$ALL_PACKAGES dosfstools"
ALL_PACKAGES="$ALL_PACKAGES ppp"
ALL_PACKAGES="$ALL_PACKAGES sxetools armioctl tat getkeycode bootcharger"
ALL_PACKAGES="$ALL_PACKAGES openssl"
ALL_PACKAGES="$ALL_PACKAGES dropbear"
#ALL_PACKAGES="$ALL_PACKAGES samba"
ALL_PACKAGES="$ALL_PACKAGES strace prelink bootchart"
ALL_PACKAGES="$ALL_PACKAGES expat dbus"
ALL_PACKAGES="$ALL_PACKAGES bluez-libs bluez-utils bluez-hcidump bluez-firmware"
ALL_PACKAGES="$ALL_PACKAGES wireless-tools wpa_supplicant"
ALL_PACKAGES="$ALL_PACKAGES initrd linux"
PACKAGES=$ALL_PACKAGES

while [ $# -ne 0 ]; do
    case $1 in
        --clean)
            OPTION_CLEAN=1
            DEFAULT_OPTIONS=0
            ;;
        --build)
            OPTION_BUILD=1
            DEFAULT_OPTIONS=0
            ;;
        --install)
            OPTION_INSTALL=1
            DEFAULT_OPTIONS=0
            ;;
        --image)
            OPTION_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --packages=*)
            PACKAGES_ORDERED=""
            PACKAGES=`echo ${1#--packages=} | tr ',' ' '`
            for p in $ALL_PACKAGES; do
                if echo "$PACKAGES" | grep $p 2>&1 >/dev/null; then
                    PACKAGES_ORDERED="$PACKAGES_ORDERED $p"
                fi
            done
            PACKAGES=$PACKAGES_ORDERED
            ;;
        --develmode)
            OPTION_DEVEL=1
            DEFAULT_OPTIONS=0
            ;;
        --interactive)
            OPTION_INTERACTIVE=1
            DEFAULT_OPTIONS=0
            ;;
        --config-linux)
            OPTION_CONFIG_LINUX=1
            DEFAULT_OPTIONS=0
            ;;
        --kernel-source)
            if [ $# -ge 2 ]; then
                KERNEL_SOURCE_PATH="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --source-url)
            if [ $# -ge 2 ]; then
                PACKAGE_LOCATION="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
				--own-toolchain)
						OWNTOOLCHAIN=1
						;;
				--stampdir)
						STAMPDIR="$2"
            shift 1
						;;
				--download-dir)
						DOWNLOAD_DIR="$2"
						shift 1
						;;
        --cross-prefix)
						CROSS_PREFIX="$2"
						shift 1
						;;
        --toolchain-path)
						TOOLCHAIN_PATH="$2"
						shift 1
						;;
        --build-dir)
             BUILDDIR="$2"
             shift 1
             ;; 
        --verbose)
						VERBOSITY=1
						;;  
        --bootchart)
						CONFIG_BOOTCHART=1
						;;
        --no-lids)
						CONFIG_LIDS=0
						;;
        --raid)
						CONFIG_RAID=1
# kernel is too big with lids and raid
						CONFIG_LIDS=0
            OPTION_CONFIG_LINUX=1
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

if [ $CONFIG_LIDS -eq 1 ]; then
    echo "build with lids"
    PACKAGES="$PACKAGES lids"
fi
PACKAGES="$PACKAGES basefiles"
if [ $CONFIG_RAID -eq 1 ]; then
    echo "build with raid"
    PACKAGES="$PACKAGES popt raid"
fi

# Default options if non specified
if [ $DEFAULT_OPTIONS -eq 1 ]; then
    OPTION_BUILD=1
    OPTION_INSTALL=1
    OPTION_IMAGE=1
fi

if [ "$VERBOSITY" != "1" ]; then
  VERBOSITY="-q"
  else
  VERBOSITY=""
fi
if [ ! -e $STAMPDIR ]; then
   mkdir -p $STAMPDIR
fi

if [ ! -e $DOWNLOAD_DIR ]; then
  mkdir -p $DOWNLOAD_DIR
fi

export PATH=/opt/teambuilder/bin:$TOOLCHAIN_PATH/bin:$PATH
export TEAMBUILDER_CC_VERSION=4.1.1
export TEAMBUILDER=0

# Package location
[ -z $PACKAGE_LOCATION ] && PACKAGE_LOCATION=http://www.qtextended.org/downloads/greenphone/rootfs

# Source and destination
ROOTFS_SOURCE_PATH=$QTOPIA_SOURCE_PATH/devices/greenphone/rootfs
if [ $QTOPIA_SOURCE_PATH = $PWD ]; then
    ROOTFS_BUILD_PATH=$QTOPIA_SOURCE_PATH/rootfs
    mkdir -p $ROOTFS_BUILD_PATH
    cd $ROOTFS_BUILD_PATH
else
    ROOTFS_BUILD_PATH=$PWD/$BUILDDIR
    mkdir -p $ROOTFS_BUILD_PATH
fi

LOGFILE=$PWD/build.log
echo >$LOGFILE

# Define locations where images will be built
ROOTFS_IMAGE_DIR=$ROOTFS_BUILD_PATH/rootfs
DEVFS_IMAGE_DIR=$ROOTFS_BUILD_PATH/devfs

# Outputs of this script
ROOTFS_FILENAME=greenphone-rootfs.tar.gz
KERNEL_FILENAME=greenphone-kernel
KERNEL_SIZELIMIT=1048576
INITRD_FILENAME=greenphone-initrd

# toolchain
CROSSTOOL=crosstool-0.42
export TARBALLS_DIR=$DOWNLOAD_DIR
export RESULT_TOP=$ROOTFS_BUILD_PATH/$CROSSTOOL/build/opt/toolchains/greenphone
BINUTILS=binutils-2.17
GCCSTRAP=gcc-3.4.5
GCC=gcc-4.1.1
GDB=gdb-6.4
GLIBC=glibc-2.3.6
LINUXTHREADS=glibc-linuxthreads-2.3.6
LINUX=linux-2.4.19

# package versions
BUSYBOX=busybox-1.2.1
LIBUNGIF=libungif-4.1.4
ILIB=Ilib-1.1.9
DOSFSTOOLS=dosfstools-2.11
STRACE=strace-4.5.14
PPP=ppp-2.4.3
SAMBA=samba-2.2.9
LIBELF=libelf-0.8.6
PRELINK=prelink-0.0.20060712
EXPAT=expat-2.0.0
DBUS=dbus-1.0.2
BLUEZLIBS=bluez-libs-3.19
BLUEZUTILS=bluez-utils-3.19
BLUEZHCIDUMP=bluez-hcidump-1.40
BLUEZFIRMWARE=bluez-firmware-1.2
WIRELESSTOOLS=wireless_tools.28
WPASUPPLICANT=wpa_supplicant-0.5.7
LIDS=lids-1.2.2-2.4.28
DROPBEAR=dropbear-0.48.1
POPT=popt-1.10.4
RAID=raidtools-1.00.3
OPENSSL=openssl-0.9.8e
BOOTCHART=bootchart-0.9

# package built from the depot
GPH_LINUX=linux

if [ $OPTION_CLEAN -eq 1 ]; then
    for package in $PACKAGES; do
        clean_package $package
    done

    sudo rm -rf $ROOTFS_IMAGE_DIR || die "sudo failed at line $LINENO"
    sudo rm -rf $DEVFS_IMAGE_DIR || die "sudo failed at line $LINENO"
    [ -d host ] && rmdir --ignore-fail-on-non-empty host

    rm -f $ROOTFS_FILENAME
fi
if [ $OPTION_BUILD -eq 1 ]; then
    for package in $PACKAGES; do
        build_package $package
    done
fi
if [ $OPTION_INSTALL -eq 1 ]; then
    sudo rm -rf $ROOTFS_IMAGE_DIR $DEVFS_IMAGE_DIR || die "sudo failed at line $LINENO"
    mkdir -p $ROOTFS_IMAGE_DIR $DEVFS_IMAGE_DIR
    rm -rf $ROOTFS_BUILD_PATH/junk

    for package in $PACKAGES; do
        install_package $package
    done
fi

if [ $OPTION_IMAGE -eq 1 ]; then
    if [ -d $ROOTFS_IMAGE_DIR -a -d $DEVFS_IMAGE_DIR ]; then
        create_default_tgzs

        make_rootfs_image
    else
        echo "No installation to create image from - specify --install"
        exit 1
    fi
fi

