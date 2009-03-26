#!/bin/bash

# This script is invoked by the (W)LAN plugin.
# see $QTOPIA_DEPOT_PATH/src/plugins/network/lan/lan-network.template for more details.


#############################################
# WARNING: This script is SuSE 9.3 specific #
# see lan-network.template for more details #
#############################################

DEBUG=1
LOG=0
LOG_FILE="/tmp/qtopia-network.log"
TMP_FILE="/tmp/lan-intern-network.temp";

ACTION="";


print_debug()
{
    if [ $DEBUG -eq 1 ]; then
        echo "LAN: $1";
    fi
    if [ $LOG -eq 1 ]; then
        echo "LAN: $1" >> $LOG_FILE;
    fi
}

print_options()
{
    echo ;
    echo "Qtopia network interface";
    echo "Usage: lan-network (install|cleanup|start|stop|route)";
    exit 1;
}

clean_config()
{
    #filter all options which might be overriden
    #keep other/unknown entries
    rm -rf "$TMP_FILE";
    cat $1 | while read line; do
        line=${line/BOOTPROTO=*/};
        line=${line/STARTMODE=*/};
        line=${line/NETMASK=*/};
        line=${line/BROADCAST=*/};
        line=${line/DHCLIENT_MODIFY_RESOLV_CONF=*/};

        save=${line/REMOTE_IPADDR=*/};
        if [ -n "$save" ]; then
            line=${line/IPADDR=*/};
        fi
        if [ -n "$line" ]; then 
           echo "$line" >> $TMP_FILE; 
        fi
    done
}

clean_wireless_config()
{
    #filter wireless options
    rm -rf "$TMP_FILE";
    awk '!/WIRELESS_/ {print $0}' $1 >> $TMP_FILE;
}

# install peer file
install()
{
    # Replace following code if necessary for your system
    
    #peer and dns
    IFACE=$1;
    shift;
    MODE=$1;
    shift

    if [ "$MODE" = "dns" ]; then
        print_debug "Installing dns server";
        rm -rf "$TMP_FILE";
 
        if [ $# -eq 2 ]; then # if no ip passed assume dhcp
            echo "nameserver $1" >> $TMP_FILE;
            echo "nameserver $2" >> $TMP_FILE;
        else
            echo "DHCP will set DNS details";
            local a=`sudo dhcpcd-test $IFACE|grep DNS|cut -d= -f2`; 
            local dns1 dns2="";
            dns1=`echo $a | cut -d, -f1`
            dns2=`echo $a | cut -d, -f2`
            print_debug "DNS1: $dns1 DNS2: $dns2";
            if [ -n "$dns1" ]; then
                echo "nameserver $dns1" >> $TMP_FILE;
            fi
            if [ -n "$dns2" ]; then
                echo "nameserver $dns2" >> $TMP_FILE;
            fi
        fi 
        
        sudo mv -f $TMP_FILE /etc/resolv.conf;
        return 0;
    fi
    
    #work out which config file to edit
    local FILE="";
    local MAC="";
    local next last="";
    local devtype=${IFACE/[0-9]*/};

    #get mac address
    for next in $( ip link show $IFACE 2>/dev/null ); do
        if [ "$last" == "link/ether" ]; then
            MAC=$next;
            break;
        fi
        last=$next;
    done
   
    #find config file 
    local cfgname="";  
    if [ -f /etc/sysconfig/network/ifcfg-eth-id-$MAC ]; then
        FILE="/etc/sysconfig/network/ifcfg-$devtype-id-$MAC";
        cfgname="$devtype-id-$MAC";
        print_debug "Writing to $FILE"; 
    elif [ -f /etc/sysconfig/network/ifcfg-wlan-$IFACE ]; then
        FILE="/etc/sysconfig/network/ifcfg-wlan-$IFACE";
        cfgname="$IFACE";
        print_debug "Writing to $FILE";
    elif [ -f /etc/sysconfig/network/ifcfg-$IFACE ]; then
        FILE="/etc/sysconfig/network/ifcfg-$IFACE";
        cfgname="$IFACE";
        print_debug "Writing to $FILE";
    else
        #other names are possible too but we ignore them for now
        print_debug "No network config found";
        return 1;
    fi

    case $MODE in 
        dhcp)
            clean_config $FILE;
            echo "BOOTPROTO='dhcp'" >> $TMP_FILE;
            echo "STARTMODE='manual'" >> $TMP_FILE;
            echo "DHCLIENT_MODIFY_RESOLV_CONF='no'" >> $TMP_FILE
            
            #replace config
            sudo mv -f $TMP_FILE $FILE;

            #add default route
            local gateway=`sudo dhcpcd-test $IFACE|grep GATEWAY|cut -d= -f2`; 
            echo "default $gateway - -" > $TMP_FILE;
            sudo mv -f $TMP_FILE /etc/sysconfig/network/ifroute-$cfgname; 
            ;;
        static)
            clean_config $FILE;
            echo "BOOTPROTO='static'" >> $TMP_FILE;  
            echo "STARTMODE='manual'" >> $TMP_FILE;  
            echo "IPADDR='$1'" >> $TMP_FILE;  
            echo "NETMASK='$2'" >> $TMP_FILE;  
            echo "BROADCAST='$3'" >> $TMP_FILE;  
            
            #replace config
            sudo mv -f $TMP_FILE $FILE;

            #add default route
            print_debug "Writing default route to /etc/sysconfig/network/ifroute-$cfgname";
            echo "default $4 - -" > $TMP_FILE;
            sudo mv -f $TMP_FILE /etc/sysconfig/network/ifroute-$cfgname; 
            ;;
        wireless)
            clean_wireless_config $FILE;
            while [ $# -gt 0 ]; do
                case "$1" in 
                    -mode)
                        shift;
                        echo "WIRELESS_MODE='$1'" >> $TMP_FILE;
                        ;;
                    -essid)
                        shift;
                        echo "WIRELESS_ESSID=$1" >> $TMP_FILE;
                        ;;
                    -ap)
                        shift;
                        echo "WIRELESS_AP=$1" >> $TMP_FILE;
                        ;;
                    -bitrate)
                        shift;
                        if [ "$1" = "0" ]; then
                            echo "WIRELESS_BITRATE='auto'" >> $TMP_FILE;
                        else
                            echo "WIRELESS_BITRATE='$1'" >> $TMP_FILE;
                        fi  
                        ;;
                    -nick)
                        shift;
                        echo "WIRELESS_NICK=$1" >>$TMP_FILE;
                        ;;
                    -channel)
                        shift;
                        if [ "$1" = "0" ]; then
                            echo "WIRELESS_CHANNEL=''" >> $TMP_FILE;
                        else
                            echo "WIRELESS_CHANNEL='$1'" >> $TMP_FILE;
                        fi
                        ;;
                    -keylength)
                        shift;
                        echo "WIRELESS_KEY_LENGTH='$1'" >> $TMP_FILE;
                        ;;
                    -authmode)
                        shift;
                        local mode="";
                        case "$1" in 
                            open|shared|none)
                                if [ "$1" = "open" ]; then
                                    echo "WIRELESS_AUTH_MODE='open'" >> $TMP_FILE
                                elif [ "$1" = "none" ]; then #SuSE uses open & empty password to disable encryption
                                    echo "WIRELESS_AUTH_MODE='open'" >> $TMP_FILE
                                else
                                    echo "WIRELESS_AUTH_MODE='sharedkey'" >> $TMP_FILE
                                fi
                                shift;
                                echo "WIRELESS_KEY=''" >> $TMP_FILE;
                                local ka kb kc kd="";
                                if [ "$1" = "-phrase" ]; then
                                    echo "WIRELESS_DEFAULT_KEY='0'">> $TMP_FILE;
                                    shift;
                                    echo "WIRELESS_KEY_0=h:$1" >> $TMP_FILE;
                                elif [ "$1" = "-nokey" ]; then
                                    echo "WIRELESS_DEFAULT_KEY='0'">> $TMP_FILE;
                                    print_debug "No password/keys given";
                                elif [ "$1" = "-multikey" ]; then
                                    shift;
                                    echo "WIRELESS_DEFAULT_KEY='$1'">> $TMP_FILE;
                                    shift;
                                    ka=$1;shift;kb=$1;shift;kc=$1;shift;kd=$1;
                                fi
                                echo "WIRELESS_KEY_0=$ka">> $TMP_FILE;
                                echo "WIRELESS_KEY_1=$kb">> $TMP_FILE;
                                echo "WIRELESS_KEY_2=$kc">> $TMP_FILE;
                                echo "WIRELESS_KEY_3=$kd">> $TMP_FILE;

                                #for completeness addedd
                                echo "WIRELESS_WPA_PSK=''" >> $TMP_FILE;
                                ;;
                            WPA-PSK)
                                echo "WIRELESS_AUTH_MODE='psk'" >> $TMP_FILE
                                shift;
                                if [ "$1" = "none" ]; then
                                    echo "WIRELESS_WPA_PSK=''";
                                fi
                                echo "WIRELESS_WPA_PSK=$1" >> $TMP_FILE;
                                shift; #we ignore AES,TKIP parameter, SuSE doesn't need it
                                
                                #for completeness addedd
                                echo "WIRELESS_DEFAULT_KEY='0'">> $TMP_FILE;
                                echo "WIRELESS_KEY=''" >> $TMP_FILE;
                                echo "WIRELESS_KEY_0=''">> $TMP_FILE;
                                echo "WIRELESS_KEY_1=''">> $TMP_FILE;
                                echo "WIRELESS_KEY_2=''">> $TMP_FILE;
                                echo "WIRELESS_KEY_3=''">> $TMP_FILE;
                                ;;
                            WPA-EAP)
                                shift;
                                shift;  # PEAP,TLS,TTLS
                                shift;  # identity/client cert
                                shift;  # password/server_cert
                                # not supported by suse 9.2
                                # add later 
                                ;;
                            *)
                                print_debug "Unknown option: $1" 
                                shift;
                                ;;
                        esac
                        ;;
                    *)  
                        print_debug "Unknown option $1";
                        shift
                        ;;
                   
                esac;
                shift;
            done

            #these options are not configurable but for completeness we add them
            echo "WIRELESS_FREQUENCY=''" >> $TMP_FILE;
            echo "WIRELESS_NWID=''" >> $TMP_FILE;
            echo "WIRELESS_POWER='yes'" >> $TMP_FILE;
            #replace config

            sudo mv -f $TMP_FILE $FILE;
#           cat $TMP_FILE;
#           rm $TMP_FILE;
            ;;
        *)
            print_debug "Unknown option: $MODE";
            print_options;
            exit 1;
            ;;
    esac
}

start()
{
    # Replace following code if necessary for your system
    
    print_debug "starting interface $1";
    sudo /sbin/ifup $1;
}

stop()
{
    # Replace following code if necessary for your system
    
    print_debug "stopping interface $1";
    sudo /sbin/ifdown $1
}

cleanup()
{
    # Replace following code if necessary for your system
    print_debug "cleaning interface configuration";
    # anything to do?
}

route()
{
    # Replace following code if necessary for your system
    
    print_debug "changing default route"
    
    if [ $# -gt 2 ]; then
        # gateway address was passed to us
        gateway=$3
    else
        # Read dhcp information and search for Gateway
        gateway=`sudo dhcpcd-test $1|grep GATEWAY|cut -d= -f2`; 
    fi

    if [ -n "$gateway" ]; then
        print_debug "removing old default route";
        sudo route del default;
        print_debug "adding new default route via $1 -> $gateway";
        sudo /sbin/route add default gw $gateway;
    fi
}

#######################################
#Parse command line

print_debug "############Starting config script for lan plugin"
print_debug "$*";
ACTION=$1;
shift;
case $ACTION in
    start)
        start "$@"
        ;;
    stop)
        stop "$@" 
        ;;
    install)
        install "$@";
        ;;
    cleanup)
        cleanup "$@";
        ;;
    route)
        route "$@";
        ;;
    *)
        print_debug "Unknown option: $ACTION";
        print_options;
        exit 1;
        ;;
esac
