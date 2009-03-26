#!/bin/sh

STARTUPFLAGS_CONF="$HOME/Settings/Trolltech/StartupFlags.conf"

parseConf()
{
    awk 'BEGIN { group = "" } \
/^\[.*\]$/ { group = toupper(gensub("[\][]", "", "g")) "_FLAG" } \
/^State=/ && group != "" { printf "%s=%s\n", group, gensub("^State=", "", "") }' $1
}

modifyConf()
{
    awk 'BEGIN { group = "" } \
/^\[.*\]$/ { group = toupper(gensub("[\][]", "", "g")) "_FLAG" } \
/^State=/ && group == "'$2'" { printf "State=%d\n", '$3' ; continue } \
{ print $0 }' $1
}

if [ $# -eq 0 ]; then
    # Read all startup flags
    [ -r $STARTUPFLAGS_CONF ] && parseConf $STARTUPFLAGS_CONF
elif [ $# -eq 2 ]; then
    # Modify the value of a startup flag
    
    TEMPFILE=`mktemp /tmp/startupflags.XXXXXX`
    
    if [ -r $STARTUPFLAGS_CONF ]; then
        if parseConf $STARTUPFLAGS_CONF | grep $1 >/dev/null; then
            modifyConf $STARTUPFLAGS_CONF $1 $2 > $TEMPFILE
            cp $TEMPFILE $STARTUPFLAGS_CONF
            rm -f $TEMPFILE
        fi
    fi
else
    echo "Usage: $0 [Flag] [Value]"
fi

