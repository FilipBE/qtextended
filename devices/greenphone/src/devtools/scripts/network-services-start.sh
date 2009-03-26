#!/bin/sh

if [ -z "$( pidof inetd )" ]; then
    /usr/sbin/inetd
fi

# notify Qtopia that some new network devices may have appeared 
qcop send "QPE/NetworkState" "updateNetwork()"

