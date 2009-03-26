#!/bin/sh

. /opt/Qtopia/SDK/scripts/devel-x86.sh

if [ "$1" = "-arm" ] ; then
    kdbg --config arm-linux-kdbgrc -r `phoneip`:22222
else
    kdbg
fi
