#!/bin/sh
echo 1 > /sys/bus/platform/devices/neo1973-pm-bt.0/power_on

if [ -e /sys/devices/platform/s3c2440-i2c ]; then
    echo 3300 > /sys/devices/platform/s3c2440-i2c/i2c-adapter/i2c-0/0-0073/voltage_ldo4
    echo 1 > /sys/bus/platform/drivers/neo1973-pm-bt/neo1973-pm-bt.0/reset
    echo 0 > /sys/bus/platform/drivers/neo1973-pm-bt/neo1973-pm-bt.0/reset
fi

