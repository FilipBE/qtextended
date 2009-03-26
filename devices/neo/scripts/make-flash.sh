#!/bin/sh
# this is a flash creating script for Neo 1973
# It requires a working root filesystem which can either be downloaded
# or built using openembedded.

DEVICE=neo
# 
if [ -z $QPEVER ]; then
	QPEVER=`cat $QTOPIA_DEPOT_PATH/src/libraries/qtopiabase/version.h | grep QPE_VERSION |awk '{print $3}'|sed 's/"//g'`

	#	exit 1
fi

BUILDDIR=`pwd`

make_flash()
{
    #make jffs flash image system
    FILES="rootfs-gta01.tgz rootfs-gta02.tgz"
    for file in $FILES; do
    echo $file

		if ! wget http://qtopiaweb.trolltech.com.au/dist/input/qtopia/neo/$file; then
				echo "wget failed to get rootfs"
				exit 1
		fi

		if [ -e rootfs ]; then 
				rm -rf rootfs
		fi

				mkdir rootfs 
				echo `pwd`
				echo "Unpack rootfs"
				sudo tar -C rootfs -xpzf $file
        rm $file
   
		if [ -e rootfs/opt/Trolltech/Qtopia ]; then
				rm -rf rootfs/opt/Trolltech/Qtopia
		fi

    mkdir -p rootfs/opt/Trolltech/Qtopia

		if [ ! -e /opt/Qtopia/SDK/${QPEVER}/$DEVICE/image ]; then
        cp -a /opt/Qtopia/SDK/${QPEVER}/$DEVICE/image/* rootfs/opt/Trolltech/Qtopia
     else
        cp -a image/* rootfs/opt/Trolltech/Qtopia
     fi

    if [ "$file" == "rootfs-gta01.tgz" ]; then
       create_flash
    else
        create_freerunner_flash
    fi
done  
    if !  sudo rm -rf rootfs; then
        echo "removing rootfs failed"
        exit 1
    fi

echo " make_flash DONE"

}
create_freerunner_flash()
{
    IMAGE_NAME=qtextended-gta02-rootfs
    sudo mkfs.jffs2 -v --eraseblock=0x20000 --pagesize=0x800 --no-cleanmarkers --little-endian --pad -o $IMAGE_NAME.tmp -drootfs
    #To add the summaries, something like this:
    sudo sumtool --eraseblock=0x20000 --no-cleanmarkers --littleendian --pad -i $IMAGE_NAME.tmp -o $IMAGE_NAME
}

create_flash()
{
    IMAGE_NAME=qtextended-gta01-rootfs
    echo "Creating jffs2 file"
    LASTDATETIME=`/bin/date -u +%m%d%H%M`
    sudo mkfs.jffs2 --pad=0x700000 -o $IMAGE_NAME.jffs2 -e 0x4000 -n -drootfs
}

create_update()
{
	if [ ! -e   /opt/Qtopia/extras/images ]; then
		mkdir -p  /opt/Qtopia/extras/images
	fi
    if ! tar -cpzf  /opt/Qtopia/extras/images/qtextended-$DEVICE-update.tar.gz *; then
        echo "creating update failed"
	cd $BUILDDIR
	exit 1
    fi
}

make_update()
{
    echo "Creating update file "$QPEVER
    if [ -e /opt/Qtopia/SDK/${QPEVER}/$DEVICE/image ]; then
		cd  /opt/Qtopia/SDK/${QPEVER}/$DEVICE/image
		create_update
    else
		if [ -e image ]; then
			cd image
			create_update
        else
			echo "Cannot find image directory"
		fi
    fi
    cd $BUILDDIR
}


#make_flash
make_update

echo "make flash DONE!"

