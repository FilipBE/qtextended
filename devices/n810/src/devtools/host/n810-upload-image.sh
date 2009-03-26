#!/bin/sh
BUILD_DIR="$QPEDIR"
DEPOT_DIR="$QTOPIA_DEPOT_PATH"
DEVICE="192.168.2.15"
for opt in $* ; do
    case $opt in
        --build=*) BUILD_DIR=`echo "$opt" | sed 's/^--build=//g'` ;;
        --depot=*) DEPOT_DIR=`echo "$opt" | sed 's/^--depot=//g'` ;;
        --device=*) DEVICE=`echo "$opt" | sed 's/^--device=//g'` ;;
        *) echo "$0: unknown option $opt"
           exit 1 ;;
    esac
done
if test "x$BUILD_DIR" = "x" ; then
    echo "$0: Must specifiy build directory with --build=DIR"
    exit 1
fi
if test "x$DEPOT_DIR" = "x" ; then
    echo "$0: Must specifiy depot directory with --depot=DIR"
    exit 1
fi

cd "$BUILD_DIR/image"
tar cf - * | gzip | ssh "root@$DEVICE" '(set -x;/etc/init.d/qpe stop;sleep 2;mkdir -p /opt/Qtopia;rm -rf /opt/Qtopia/*; cd /opt/Qtopia;gunzip |tar xvf -)'
exit 0
