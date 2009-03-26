#!/bin/sh
KERNEL_VER=`uname -r`
case $1 in
    start)
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/ifup usb0
        ;;
    stop)
        /sbin/ifdown usb0
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        ;;
    restart)
        /sbin/ifdown usb0
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        wait 1
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/ifup usb0       
        ;;
    *)
        echo "$0 {start|stop|restart}"
        exit 1;
        ;;
esac
