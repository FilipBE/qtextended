#!/bin/sh

getvars()
{
    while [ $# -gt 0 ]; do
        if [ "${1##DEVICE_CONFIG_PATH=}" != "$1" ]; then
            eval "$1"
        fi
        if [ "${1##DEFAULT_DEVICE_PATH=}" != "$1" ]; then
            eval "$1"
        fi
        if [ "${1##DEVICE_BIN=}" != "$1" ]; then
            eval "$1"
        fi
        if [ "${1##DEVICE_BUILDING_FOR_DESKTOP=}" != "$1" ]; then
            eval "$1"
        fi
        shift
    done
}

getvars "$@"

# NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
#
# The environment file handling logic is duplicated in configure
#
# NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE

# Only pull in these files if we're actually building for a device
if [ -n "$DEVICE_CONFIG_PATH" -a "$DEVICE_BUILDING_FOR_DESKTOP" = 0 ]; then
    . "$DEFAULT_DEVICE_PATH/environment"
    [ -f "$DEVICE_CONFIG_PATH/environment" ] && . "$DEVICE_CONFIG_PATH/environment"
    setup_path
fi

exec env "$@"

