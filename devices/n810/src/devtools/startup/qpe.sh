#!/bin/sh
# 
#  qtopia start-up script for Nokia n810
# 
# echo '1' >> /proc/sys/kernel/printk
export LOGNAME=user
export HOME=/home/$LOGNAME
export QPEDIR=/opt/Qtopia
export QTDIR=$QPEDIR
export LD_LIBRARY_PATH=$QTDIR/lib
export PATH=$QPEDIR/bin:$PATH

export QTOPIA_PHONE_DUMMY=1
export SXE_DISCOVERY_MODE=1

export QWS_MOUSE_PROTO="tslib:/dev/input/event3"
export QWS_SIZE="800x480"
# when started from charge mode, x and friends get run
# from runlevel 5
TAG=`ps aux | grep Xomap | grep -v grep`
if [ -n "$TAG" ]
 then
        /etc/init.d/x-server stop
        /etc/init.d/osso-systemui stop
		/etc/init.d/esd stop
		/etc/init.d/metalayer-crawler0 stop
fi                                    

# stop the auto-mass-media mounting - done by a specific daemon -
# by running the init.d script with stop argument if script is exec
if [ -x /etc/init.d/ke-recv ]; then
    /etc/init.d/ke-recv stop
fi

BLK=`mount | grep mmcblk0p1`
if [ -z "$BLK" ]; then
    mount /media/mmc1
fi
BLK=`mount | grep mmcblk1p1`
if [ -z "$BLK" ]; then
    mount /media/mmc2
fi

if [ -e /sys/devices/platform/gpio-switch/slide/state ]; then
# n810
                export QWS_KEYBOARD="n810kbdhandler"
else
#n800
                export QWS_KEYBOARD="n800kbdhandler"
fi


KILLPROGS="qpe quicklauncher mediaserver sipagent messageserver telepathyagent"

    /sbin/rmmod /mnt/initfs/lib/modules/2.6.21-omap1/g_file_storage.ko
    /sbin/insmod /mnt/initfs/lib/modules/2.6.21-omap1/g_ether.ko
    /sbin/ifup usb0


killproc() {
        pid=`/bin/pidof $1'`
        [ "$pid" != "" ] && kill $pid
}

# Start up the mini-syslogd in our logread implementation.
# FIXME: Remove this once BusyBox is reconfigured with syslogd.
if [ -e $QPEDIR/bin/logread ]; then
    killall logread 2>/dev/null
    logread -d &
fi

case $1 in
'start')
    if [ -f /etc/mce/ ] ; then  
        /usr/sbin/update-rc.d -f mce remove; 
    fi
	echo "Starting QPE..."
        if [ -f /opt/Qtopia/pics/qgn_indi_nokia_hands_qt.jpg ]; then
            # bend fb-progress to do a splash screen by causing time-based progress bar to be same colour
            # as image bg and completed in the minimum time 2secs - image will remain until fb next written to
            /usr/sbin/fb-progress -c -b ffffff -p ffffff -l /opt/Qtopia/pics/qgn_indi_nokia_hands_qt.jpg 2 &
        fi

	cd $HOME
        dsmetool -t "/opt/Qtopia/bin/qpe 2>&1 | logger -t 'Qtopia'" 

	;;
'stop')
        echo "Killing QPE..."
        dsmetool -k "/opt/Qtopia/bin/qpe 2>&1 | logger -t 'Qtopia'"

    	killall qpe $KILLPROGS 2>/dev/null
        dspctl reset;
        killall oss-media-server;
        killall multimediad
	
        ;;

esac

