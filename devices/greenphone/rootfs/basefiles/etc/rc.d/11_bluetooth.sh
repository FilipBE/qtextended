#!/bin/sh

case $1 in
start)
    # Device address is not actually burned into the device ROM, must be set in software
    # The address is stored in the TAT table
    # If the address is not set accordingly, run the fixbdaddr tool in devtools
    BDADDR=$(/usr/bin/tat bdaddr)

    hciattach /dev/ttyS1 stlc2500 921600 noflow $BDADDR

    # Make sure hcid and sdpd are started afterwards
    hcid -s
    ;;
stop)
    kill $( pidof hcid hciattach ) 2>/dev/null
    ;;
*)
    echo "usage: $0 { start | stop }"
    exit 2
    ;;
esac

exit 0

