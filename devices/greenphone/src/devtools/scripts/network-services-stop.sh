#!/bin/sh

kill $( pidof inetd )

# notify Qtopia that some network devices may have disappeared 
qcop send "QPE/NetworkState" "updateNetwork()"
