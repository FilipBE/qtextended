#!/bin/ash

restore_default()
{
    echo "Restoring system default"

    # Destroy all data in /home and /mnt/documents

    startupflags.sh RESTOREDEFAULTS_FLAG 0

    # Restart this script. It is possible that it was changed.
    exec $0
}

setup_updates()
{
    if [ ! -f $UPDATE_DIR/etc/defaultbuttons.conf ]; then
        mkdir -p $UPDATE_DIR/etc
        cp $QTOPIA_DIR/etc/defaultbuttons.conf $UPDATE_DIR/etc
    fi
}

# setup $UPDATE_DIR
setup_updates

# load startup flags from conf file
eval $(startupflags.sh)

# Restore default settings.
if [ "$RESTOREDEFAULTS_FLAG" = 1 ]; then
    restore_default
fi

if [ "$PHONEDUMMY_FLAG" = 1 ]; then
    export QTOPIA_PHONE_DUMMY=1
else
    export QTOPIA_PHONE_DUMMY=0
fi

if [ "$PHONEDEVICE_FLAG" != "" ]; then
    export QTOPIA_PHONE_DEVICE=$PHONEDEVICE_FLAG
fi

if [ "$PHONEBOUNCE_FLAG" = 1 ] ; then
    # Turn on the modem
    apm --modem-poweron
    apm --wakeup

    phonebounce $QTOPIA_PHONE_DEVICE 12345 &
    modem_keep_alive.sh &

    export QTOPIA_PHONE_DUMMY=1
fi
    
if [ "$PERFTEST_FLAG" = 1 ] && [ "$QTOPIA_PERFTEST" != 1 ]; then
    # Use the perftest screen driver
    export OLD_QWS_DISPLAY=$QWS_DISPLAY
    if [ "x$QWS_DISPLAY" = "x" ]; then eval "export `egrep '^QWS_DISPLAY' /opt/Qtopia/etc/defaultbuttons.conf`"; fi
    export QWS_DISPLAY="perftestlinuxfb:$QWS_DISPLAY"
    export QTOPIA_PERFTEST=1
    echo "Enabling performance testing" | logger -p local5.notice -t 'Qtopia'
    if [ "$PERFTESTSLEEP_FLAG" = 1 ]; then
        echo "Performance: sleeping for 2 minutes" | logger -p local5.notice -t 'Qtopia'
        sleep 120
        echo "Performance: finished sleeping" | logger -p local5.notice -t 'Qtopia'
    fi

elif [ "$PERFTEST_FLAG" != 1 ] && [ "$QTOPIA_PERFTEST" = 1 ]; then
    # Revert to original screen driver
    export QWS_DISPLAY=$OLD_QWS_DISPLAY
    if [ "x$QWS_DISPLAY" = "x" ]; then unset QWS_DISPLAY; fi
    unset QTOPIA_PERFTEST
    echo "Disabling performance testing" | logger -p local5.notice -t 'Qtopia'
fi

if [ "$TEST_FLAG" = 1 ]; then
    export QTOPIA_TEST=1
else
    unset QTOPIA_TEST
fi

if [ "$SPYGRIND_FLAG" = 1 ]; then
    SPYGRIND=$UPDATE_DIR/lib/libspygrind.so.1
    [ -f $SPYGRIND ] || SPYGRIND=$QTOPIA_DIR/lib/libspygrind.so.1
    if ! [ -f $SPYGRIND ]; then
        echo "Spygrind was requested, but it is not installed (missing $SPYGRIND)" | logger -p local5.notice -t 'Qtopia'
        unset SPYGRIND
    else
        export SPYGRIND_OUT_PATH=$HOME
    fi
fi

# clean up shared memory and semaphores
# but not for resources created by syslogd
clearipc $(pidof syslogd)

#chvol SYSTEM 100
chvol CALL 60

if [ "$BOOTCHART_FLAG" = 1 ]; then
    { sleep 120; /sbin/bootchartd stop; } &
    /sbin/bootchartd start qpe 2>&1 | logger -p local5.notice -t Qtopia
else
    # For accurate perftest results, this MUST be the last nontrivial line before invoking qpe
    export QTOPIA_UPTIME_AT_LAUNCH=`cat /proc/uptime`
    [ "x$SPYGRIND" != "x" ] && export LD_PRELOAD=$SPYGRIND
    qpe 2>&1 | logger -p local5.notice -t 'Qtopia'
    unset LD_PRELOAD
fi

# Make sure that all Qtopia applications quit
KILLPROGS="qpe quicklauncher mediaserver mediaplayer sipagent phonebounce modem_keep_alive.sh"
kill $( pidof $KILLPROGS ) 2>/dev/null

if [ ! -f /tmp/restart-qtopia ] && [ ! -f /tmp/updating-qtopia ]; then
    splash "Qtopia terminated" "Press Hangup" "to restart"

    while [ $(getkeycode) != "16" ]; do
        :
    done

    echo 0 > /tmp/progress
    touch /tmp/restart-qtopia
fi

