#!/bin/sh
# this scripts gets run on qtopia sdk update/install

. /opt/Qtopia/SDK/scripts/functions

# symlink the toolchain to the proper place
if [ ! -e /opt/toolchains ]; then
	 mkdir /opt/toolchains
fi
if [ ! -e  /opt/toolchains/arm920t-eabi ]; then
	sudo ln -s /usr/local/arm920t-eabi /opt/toolchains/arm920t-eabi
fi

if [ ! -e /usr/local/arm-linux ]; then
	sudo ln -s /usr/local/arm920t-eabi /usr/local/arm-linux
fi

QTOPIA_VERSION=`version`
sudo mkdir -p /home/user/Settings/Trolltech
sudo cp /opt/Qtopia/SDK/$QTOPIA_VERSION/$DEVICE/devices/$DEVICE/etc/default/Trolltech/* /home/user/Settings/Trolltech
#quickly fix up trampled permissions
sudo chown -R root.root /etc
sudo chown root.root /home
sudo chown -R user.user /home/user
sudo chown -R user.user /opt/Qtopia

sudo sh -c 'echo "192.168.0.202 neo" >> /etc/hosts'
