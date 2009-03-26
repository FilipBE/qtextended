#!/bin/bash

# This script is invoked by the Dialup plugin.
# see $QTOPIA_DEPOT_PATH/src/plugins/network/dialing/ppp-network.template for more details.


#############################################
# WARNING: This script is SuSE 9.3 specific #
# see ppp-network.template for more details #
#############################################


DEBUG=1
LOG=0
LOG_FILE="/tmp/qtopia-network.log"
PPPD="/usr/sbin/pppd";


print_debug()
{
    if [ $DEBUG -eq 1 ]; then
        echo "PPP: $1";
    fi
    if [ $LOG -eq 1 ]; then
        echo "PPP: $1" >> $LOG_FILE;
    fi
}

print_options()
{
    echo ;
    echo "Qtopia network interface";
    echo "Usage: ppp-network (install|cleanup|start|stop|route)";
    echo "";
    echo "install   peer <new peer file> -> install the given peer file as new peer file";
    echo "          dns <dns1> <dns2> -> installs the given IPs as dns server";
    echo "start     <additional pppd command line parameter> -> starts pppd using the given parameter";
    echo "stop      <ppp interface name> -> stops pppd on given interface";
    echo "cleanup   peer <peer filename> -> deletes peer file";
    echo "          dns -> delete dns details";
    echo "route     <ppp interface name> -> the given interface becomes the default gateway for IP traffic";
        
    exit 1;
}


# install peer file
install()
{
    # Replace following code if necessary for your system
    
    #peer and dns
    NEXT=$1;
    case $NEXT in 
        peer)
            print_debug "installing $2 to /etc/ppp/peers"
            sudo cp -f $2 /etc/ppp/peers
            ;;
        dns)
            print_debug "writing dns server to /etc/ppp/resolv.conf";
            shift;
            sudo rm -f /tmp/resolv.conf.tmp;
            if [ $# -gt 0 ]; then
                while [ $# -gt 0 ]; do
                    echo "nameserver $1" >> /tmp/resolv.conf.tmp
                    shift;
                done
                sudo mv /tmp/resolv.conf.tmp /etc/ppp/resolv.conf
            fi
            sudo rm -rf /etc/resolv.conf
            sudo cp /etc/ppp/resolv.conf /etc/resolv.conf
            ;;    
        *)
            print_debug "Unknown option: $ACTION";
            print_options;
            exit 1;
            ;;
    esac
    
}

start()
{
    # Replace following code if necessary for your system
    
    print_debug "starting pppd using parameters: $*"
    $PPPD $* &
}

stop()
{
    # Replace following code if necessary for your system
    
    print_debug "stopping pppd for interface $1";
    if [ -f "/var/run/$1.pid" ]; then
        print_debug "pppd pid: `cat /var/run/$1.pid`"
        sudo kill -SIGTERM `cat /var/run/$1.pid`
    fi
}

cleanup()
{
    # Replace following code if necessary for your system
    
    NEXT=$1;
    case $NEXT in
        peer)
            shift;
            print_debug "deleting peer file /etc/ppp/peers/$1";
            sudo rm -f "/etc/ppp/peers/$1";
            ;;
        dns)
            print_debug "deinstalling dns server";
            ;;
        *) 
            print_debug "Unknown option: $ACTION";
            print_options;
            exit 1;
            ;;
    esac
}

route()
{
    gateway=`sudo /sbin/route -n|grep -i "$1" | cut -d' ' -f1`;
    gateway=`echo $gateway | cut -d' ' -f1`;
    print_debug "changing default route"
    if [ -n "$gateway" ]; then
        print_debug "removing old default route";
        sudo route del default;
        print_debug "adding new default route via $1";
        sudo /sbin/route add default gw $gateway;
    fi
}

#######################################
#Parse command line

print_debug "Starting config script for dialup plugin"

ACTION=$1;
shift;
case $ACTION in
    start)
        start $*
        ;;
    stop)
        stop $1
        ;;
    install)
        install $*;
        ;;
    cleanup)
        cleanup $*;
        ;;
    route)
        route $*;
        ;;
    *)
        print_options;
        exit 1;
        ;;
esac
