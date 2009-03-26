#!/bin/sh

die()
{
    echo "Error: $@"
    exit 1
}

tool_check()
{
    local TOOL_CHECK="$1"
    shift

    TOOL_CHECK_PATH=$( which $TOOL_CHECK 2> /dev/null )
    if [ -z $TOOL_CHECK_PATH ] && [ ! -x $PWD/$TOOL_CHECK ]; then
        wget -q -N $TOOLS_URL/$TOOL_CHECK
        if [ $? -eq 0 ]; then 
            TOOL_CHECK_PATH=$PWD/$TOOL_CHECK
            chmod 755 $TOOL_CHECK_PATH
        else
            return 1
        fi
    elif [ -x $PWD/$TOOL_CHECK ]; then
        TOOL_CHECK_PATH=$PWD/$TOOL_CHECK
    fi
    echo $TOOL_CHECK_PATH
}

make_flash_image()
{
    echo "Creating USB Flash image"

    local OUTPUT_IMAGE="$1"
    shift

    IMAGE_FILES=""

    sudo $MKFS_JFFS2_PATH -v -r $ROOTFS_IMAGE_PATH -o $OUTPUT_IMAGE.temp -e 128 -l -n >/dev/null 2> /dev/null

    sudo $SUMTOOL_PATH -i $OUTPUT_IMAGE.temp -o $OUTPUT_IMAGE -e 128KiB -l -n

    sudo rm $OUTPUT_IMAGE.temp
    echo "Finished creating USB Flash image : $PWD/$OUTPUT_IMAGE"
}

usage()
{
    echo -e "Usage: `basename $0` [--clean] [--qtopia] [--flash] [--optimize] [--qtopia-image <path>] [--rootfs <file>|<URL>] [--device <dev>] [--flash-name <name>]"
    echo -e "If no options are specified defaults to --qtopia --qtopia-image $QTOPIA_BUILD_PATH\n" \
            "   --clean               Clean image files.\n" \
            "   --qtopia              Make Qtopia image.\n" \
            "   --flash               Make flash image.\n" \
            "   --qtopia-image <path> Location of Qtopia image.\n" \
            "   --rootfs <file>|<URL> Path and Name of rootfs image | URL ( beginning with http:// ) of image to download to, and use from, current directory.\n" \
            "   --device <dev>        Device image is being built for (default: n810).\n" \
            "   --flash-name <name>   Name for the finished flash image (default: <dev>-qtopia-nokia-flash.jffs2).\n"\
            "Using $QTOPIA_SOURCE_PATH as Qtopia source path.  Override with QTOPIA_DEPOT_PATH environment variable if required."
}

# flash image tools maybe in one of the 'sbin' dirs.
PATH=$PATH:/sbin:/usr/sbin:/usr/local/sbin

DEFAULT_OPTIONS=1
OPTION_CLEAN=0
#OPTION_QTOPIA_IMAGE=1
OPTION_FLASH_IMAGE=0
DEVICE="n810"
ROOTFS_IMAGE=""
TOOLS_URL=""

DEFAULT_IMAGES=1

if [ -z $QTOPIA_DEPOT_PATH ]; then
    QTOPIA_SOURCE_PATH=$(dirname $(dirname $(readlink -f $0)))
    export QTOPIA_DEPOT_PATH=$QTOPIA_SOURCE_PATH
else
    QTOPIA_SOURCE_PATH=$QTOPIA_DEPOT_PATH
fi

QTOPIA_BUILD_PATH=$PWD/image

if [ -f $( dirname $0 )/make-flash.conf ]; then
    . $( dirname $0 )/make-flash.conf
fi

# Default Flash image data

while [ $# -ne 0 ]; do
    case $1 in
        --qtopia)
#            OPTION_QTOPIA_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --flash)
            OPTION_FLASH_IMAGE=1
            DEFAULT_OPTIONS=1
            ;;
        --clean)
            OPTION_CLEAN=1
            DEFAULT_OPTIONS=0
            ;;
        --qtopia-build|--qtopia-image)
            if [ $# -ge 2 ]; then
                QTOPIA_BUILD_PATH="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --rootfs)
            if [ $# -ge 2 ]; then
                ROOTFS_IMAGE="$2"
                DEFAULT_IMAGES=0
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --device)
            if [ $# -ge 2 ]; then
                DEVICE="$2"
                shift 1
                # remove this - between here ( if script is setup for use with more than one device )
                if [ $DEVICE != "n810" ]; then
                    echo "At time of script creation only device supported was n810"
                    echo "Please disable or remove this warning if the script"
                    echo "has been checked or being checked for other devices."
                    exit 1
                fi
                # and here
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --flash-name)
            if [ $# -ge 2 ]; then
                FLASH_IMAGE="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
       --help|-?)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
    shift
done

# If we don't have anything for ROOTFS_IMAGE complain to the user and exit
if [ "${#ROOTFS_IMAGE}" -eq 0 ]; then
    echo ""
    echo "ERROR: If no 'make-flash.conf' file, with a minimum of ROOTFS_IMAGE defined, is present in the same"
    echo "directory as this script, then you need to specify a path, or URL, for the argument --rootfs"
    echo "See Usage below."
    echo ""
    echo "make-flash.conf:"
    echo "Contains organisation dependent variables, such as URLs or file paths and names"
    echo "Example make-flash.conf - leave out those not used OR supply only an open and close i.e \"\" quote"
    echo "ROOTFS_IMAGE=\"<File Path>|<URL>\"    # specify as: local file path OR full URL to file"
    echo "FLASH_IMAGE=\"<name>\"                # The name for the finished flash image"
    echo "TOOLS_URL=\"<URL>\"                   # URL to download pre-compilied binaries of required tools"
    echo "                                      WARNING: You should build your own binaries." 
    echo ""
    echo "Usage:"
    usage
    exit 1
fi

if [ ${#FLASH_IMAGE} -eq 0 ];
then
    FLASH_IMAGE=$DEVICE"-flash-image.jffs2"
fi

SUMTOOL_PATH=$( tool_check sumtool )
if [ -z $SUMTOOL_PATH ]; then
    echo "sumtool required but not found"
    exit 1
fi
MKFS_JFFS2_PATH=$( tool_check mkfs.jffs2 )
if [ -z $MKFS_JFFS2_PATH ]; then
    echo "mkfs.jffs2 required but not found"
    exit 1
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    # Check if the first seven chars are http:// if so we are dealing with a URL
    if [ "${ROOTFS_IMAGE:0:7}" == "http://" ]; then
        # make wget to be silent and check if the server copy is newer than existing (if present) local copy
        wget -q -N $ROOTFS_IMAGE
        # wget will return 0 if the server file is no newer or if it downloads a newer copy - in all other cases give user warning
        if [ $? -ne 0 ]; then
            echo "WARNING: Could not check if server has newer copy of file than copy (if present) in current directory."
            echo "WARNING: If a copy exists in the current directory it will be used BUT maybe OUT OF DATE."
        fi
        # strip the URL to get the file name and check the file exists - if so set our path to the file
        if [ -f "$PWD/${ROOTFS_IMAGE##*\/}" ]; then
            ROOTFS_IMAGE="$PWD/${ROOTFS_IMAGE##*\/}"
        fi
    # otherwise check if there is a file at the supplied path AND if not check for a file prefixed with the device name
    elif [ ! -f "$ROOTFS_IMAGE" ] && [ -f "$PWD/$DEVICE-rootfs.tgz" ]; then
        echo "WARNING: Path to Root filesystem image does not appear to contain a file"
        echo "WARNING: A file named $DEVICE-rootfs.tgz exists in the current directory - it will be used BUT maybe OUT OF DATE."
        ROOTFS_IMAGE="$PWD/$DEVICE-rootfs.tgz"
    fi
    # check if we still do not have a path with an actual file - if we don't we need to terminate
    if [ ! -f "$ROOTFS_IMAGE" ]; then
        echo "Root filesystem image not found, you may need to use --rootfs argument"
        echo "Or check the path or URL supplied in make-flash.conf"
        usage
        exit 1
    fi
fi

# Make sure all input files and directories exist

# Check --qtopia-image
[ -d "$QTOPIA_BUILD_PATH" ] || die "$QTOPIA_BUILD_PATH does not exist or is not a directory"
if [ ! -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
    echo "$QTOPIA_BUILD_PATH does not contain an image - check \"--qtopia-image\" argument"
    if [ -f "$QTOPIA_BUILD_PATH/config.status" ]; then
        echo "    $QTOPIA_BUILD_PATH looks like the build tree"
        QTOPIA_BUILD_PATH="$QTOPIA_BUILD_PATH/image"
        if [ -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
            echo "    using $QTOPIA_BUILD_PATH as image"
        else
            exit 1;
        fi
    else
        exit 1;
    fi
fi

# Check --qtopia-source
[ -d "$QTOPIA_SOURCE_PATH" ] || die "Qtopia source path does not exist or is not a directory"

# Clean old files
if [ $OPTION_CLEAN -eq 1 ]; then
#    rm -rf fimage
    rm -f $FLASH_IMAGE.*
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    # Check --rootfs
    if [ ! -z "$ROOTFS_IMAGE" ]; then
        [ -f "$ROOTFS_IMAGE" ] || die "Rootfs image $ROOTFS_IMAGE does not exist"

#        OPTION_QTOPIA_IMAGE=1
    fi
fi

# Unpack rootfs & Copy Qtopia to temporary location
if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
    ROOTFS_IMAGE_PATH=$(mktemp -d $PWD/rootfs.XXXXXX)
    [ -d $ROOTFS_IMAGE_PATH ] || die "Could not create temporary rootfs directory"
    trap "sudo rm -rf $ROOTFS_IMAGE_PATH" 0

    sudo chmod 1777 $ROOTFS_IMAGE_PATH
    sudo chown root.root $ROOTFS_IMAGE_PATH

    sudo tar -C $ROOTFS_IMAGE_PATH -xf $ROOTFS_IMAGE || die "sudo failed at line $LINENO"

    sudo cp -a $QTOPIA_BUILD_PATH/* $ROOTFS_IMAGE_PATH/opt/Qtopia/ > /dev/null 2>&1
    # the rootfs archive may be slightly different in dir structure so take account of it...
    if [ $? -ne 0 ]; then
        echo "Appears the rootfs may need the first part of the path of each file striped on extraction."
        echo "Attempting this..."
        sudo rm -r $ROOTFS_IMAGE_PATH/*
        sudo tar --strip-components 1 -C $ROOTFS_IMAGE_PATH -xf $ROOTFS_IMAGE || die "sudo failed at line $LINENO"
        sudo cp -a $QTOPIA_BUILD_PATH/* $ROOTFS_IMAGE_PATH/opt/Qtopia/
    fi
    # check we were able to copy the 'image' into the correct place
    [ -f "$ROOTFS_IMAGE_PATH/opt/Qtopia/qpe.sh" ] || die "appears we didn't copy into a rootfs image - aborting"
fi

# Create flash images
if [ $OPTION_FLASH_IMAGE -eq 1 ]; then

    make_flash_image $FLASH_IMAGE

    # Clean up
fi

