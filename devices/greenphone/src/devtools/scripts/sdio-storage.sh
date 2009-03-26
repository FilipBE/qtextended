#!/bin/sh

# check if the SD storage modules are already loaded
grep '^mmc_omega' /proc/modules >/dev/null && exit

# remove WiFi module
rmmod p300wlan 2>/dev/null

# load SD storage modules
insmod mmc_base
insmod mmc_omega

