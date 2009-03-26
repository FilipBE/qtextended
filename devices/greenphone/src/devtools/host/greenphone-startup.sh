#!/bin/sh
DEVNAME=eth1
/sbin/ifconfig $DEVNAME 10.10.10.21 netmask 255.255.255.0 up

# The commands below set up IP forwarding between the greenphone and the local network
# Enabling these will place the Greenphone on local network as 10.10.10.20.
# Because all Greenphones have the same IP address these commands are disabled.
#/sbin/rcSuSEfirewall2 stop
#/usr/sbin/iptables -t nat -A POSTROUTING -o eth0 -s 10.10.0.0/16 -j MASQUERADE
#/sbin/sysctl -w net.ipv4.ip_forward=1
#/sbin/sysctl -w net.ipv4.conf.eth0.proxy_arp=1
#/sbin/sysctl -w net.ipv4.conf.$DEVNAME.proxy_arp=1
