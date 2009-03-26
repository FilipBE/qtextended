#!/bin/sh
BUILD_DIR="$QPEDIR"
DEPOT_DIR="$QTOPIA_DEPOT_PATH"
DEVICE="192.168.0.202"
QTOPIA_IMAGE="qtopia-update.tgz"

for opt in $* ; do
    case $opt in
        --build=*) BUILD_DIR=`echo "$opt" | sed 's/^--build=//g'` ;;
        --depot=*) DEPOT_DIR=`echo "$opt" | sed 's/^--depot=//g'` ;;
        --device=*) DEVICE=`echo "$opt" | sed 's/^--device=//g'` ;;
	--image=*) QTOPIA_IMAGE=`echo "$opt" | sed 's/^--image=//g'` ;;
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
if ! ping -W 2 -c 1 "$DEVICE" >/dev/null ; then
    echo "$0: Cannot contact device with IP address $DEVICE"
    echo "$0: Probably the USB network needs to be brought up."
    exit 1
fi

if [ ! -e "$QTOPIA_IMAGE" ]; then
	cp -p "$DEPOT_DIR/devices/neo/src/devtools/startup/qpe.sh" "$BUILD_DIR/image/qpe.sh"
	cp -p "$DEPOT_DIR/devices/neo/src/devtools/startup/qpe.env" "$BUILD_DIR/image/qpe.env"
	cd "$BUILD_DIR/image"
	tar cf - * | gzip | ssh "root@$DEVICE" '(set -x;rm -f /tmp/restart-qtopia;killall qpe;sleep 2; rm -rf /opt/Qtopia;rm -rf /opt/Trolltech/Qtopia/*; mkdir -p /opt/Trolltech/Qtopia;cd /opt/Trolltech/Qtopia;gunzip |tar xvf -;/etc/init.d/qpe start &)'

else

	cat $QTOPIA_IMAGE | ssh "root@DEVICE" '(set -x;rm -f /tmp/restart-qtopia;killall qpe; rm -rf /opt/Qtopia;rm -rf /opt/Trolltech/Qtopia/*; mkdir -p /opt/Trolltech/Qtopia; cd /opt/Trolltech/Qtopia;gunzip |tar xvf -;/etc/init.d/qpe start &)'

fi

exit 0
