#!/bin/bash
# this script will install the toolchain, qtopia binaries and 
# scripts onto your host machine. 
# this is not supported!
#
TOOLCHAINDIR=/usr/local

function install_qtopia()
{
if [ ! -e /opt/Qtopia ]; then
 sudo mkdir -p /opt/Qtopia
fi
	sudo nice -10 sudo tar -xjvf qtopia.tar.bz2 --directory=/opt/Qtopia 
}
											
function install_tools()
{
if [ ! -e $TOOLCHAINDIR ]; then
		sudo mkdir -p $TOOLCHAINDIR
fi
	sudo nice -10 tar -xjvf tools.tar.bz2 --directory=$TOOLCHAINDIR

	if [ ! -e /opt/toolchains ]; then
			mkdir -p /opt/toolchains
	fi
}

function install_extras()
{
	sudo nice -10 tar -xjf extras.tar.bz2 --directory=/
	if [ -e /opt/Qtopia/extras/bin/extras.sh ]; then
		sudo /opt/Qtopia/extras/bin/extras.sh
	fi
}

while getopts ":qteh" Option
do
  case $Option in
    q ) install_qtopia ;;
    t ) install_tools ;;
    e ) install_extras ;;
    h ) show_help ;;
   esac
done
shift $(($OPTIND - 1))

install_extras
install_qtopia
install_tools

echo "Please add /opt/Qtopia/SDK/scripts into your $PATH"
