#!/bin/sh

while true; do
    arm_ioctl /dev/omega_bcm2121 0x5401 1
    arm_ioctl /dev/omega_bcm2121 0x5400 1
    sleep 10
done
