#!/bin/sh

. /opt/Qtopia/SDK/scripts/functions

QPEVER=`version`
export BUILD_ARCH=x86

export QPEDIR=/opt/Qtopia/SDK/$QPEVER/x86
export PATH=/opt/Qtopia/SDK/scripts:$QPEDIR/bin:$QPEDIR/scripts:$PATH
