#!/bin/sh
#
# Build SDK Qt Extended 4
#
. $QTOPIA_DEPOT_PATH/scripts/functions

BUILDDIR=`pwd`

if [ -z $QTOPIA_DEPOT_PATH ]; then
	echo "You need to set QTOPIA_DEPOT_PATH to the Qt Extended sources directory"
	exit 1
fi

if [ -z $QT_DEPOT ]; then
	echo "You need to set QT_DEPOT to the Qt sources directory"
	exit 1
fi

export TEAMBUILDER=1

output_error()
{
	grep -B 10 -ns " Error" $LOGFILE
	echo "For more information on this error look at $LOGFILE for details"
}

output_install_error()
{
	grep -B 10 -ns " Error" $LOGFILE
	echo "For more information on this error look at $LOGFILE for details"
}

skip_host_tools()
{
	if [ -e $QPEDIR/qtopiacore/host/bin/qmake ] ; then
		SKIP="-skip-dqt"
	else
		SKIP=""
	fi 
}


make_x86_sdk()
{
	echo "make_x86_sdk()"
	if [ "$CLEAN" == "1" ] ;then  
		echo "cleaning "$QPEDIR 
		rm -Rf $QPEDIR/x86-sdk
		rm -f .built-x86
		rm -f .installed-x86
		rm -f .sdk-x86
		rm -f .mkdocs
		rm -rf /opt/Qtopia/SDK/$QPEVER/x86
	fi
	if [ ! -e $QPEDIR/x86-sdk ]; then
		mkdir -p $QPEDIR/x86-sdk
	fi
	export TEAMBUILDER=1

	cd $QPEDIR/x86-sdk
	LOGFILE=`pwd`/build.log
	echo 0>$LOGFILE

	if [ "$QUIET" == "1" ] ; then
		xterm -e "tail -f $LOGFILE" &
	fi
#	skip_host_tools

	EXTRA_OPTIONS=""
	test "$DRM" = "1" && EXTRA_OPTIONS="-drm"
	test "$SXE" = "1" && EXTRA_OPTIONS=$EXTRA_OPTIONS"-sxe"

	if [ $CLEAN == "1" ]; then	   
		echo "TIME:`date "+%H:%M:%S"` Configuring Qt Extended for x86"

		if $QTOPIA_DEPOT_PATH/configure -device $DEVICE -qvfb -languages en_US -verbose -sdk /opt/Qtopia/SDK/$QPEVER/x86 -image /opt/Qtopia/SDK/$QPEVER/x86/image -debug -confirm-license -extra-qt-config="-release" -extra-qt-config="-qt-libpng" -force-build-qt $EXTRA_OPTIONS >${OUTPUT} 2>&1; then
			echo "configure DONE"
		else
			echo "$0 FAILED in configure for x86 SDK"
			if [ "$QUIET" = "1" ] ; then
				echo "Try running $0 without the --quiet option to get more details on why configure failed" 
			fi
			exit 1
		fi
	else
		if [ ! -f $PWD/config.status ]; then
			echo "$0 FAILED in configure for x86 SDK, \"-no-clean\" option selected but no build was found in $PWD"
			exit 1
		fi
	fi

	if [ ! -e .built-x86 ]; then 
		echo "TIME:`date "+%H:%M:%S"` Building x86..."
		if bin/qbuild >>${OUTPUT} 2>&1; then
			echo "qbuild DONE"
			touch .built-x86
		else
			output_error
			echo "$0 FAILED in qbuild for x86 SDK"
			exit 1
		fi
	fi


#### make some documents now
########### FIXME! add arguments - -opensource -no-jira -platform -phone -home 
	if [ ! -e .mkdocs ]; then
		echo "TIME:`date "+%H:%M:%S"` Making docs x86..."
		$QTOPIA_DEPOT_PATH/scripts/mkdocs
		touch .mkdocs
	fi
##########################################

	if [ ! -e .installed-x86 ]; then
		echo "TIME:`date "+%H:%M:%S"` Installing x86..."
		if bin/qbuild image >>${OUTPUT} 2>&1; then
			echo "qbuild image DONE"
			touch .installed-x86
		else
			output_install_error
			echo "$0 FAILED in qbuild image for x86 SDK"
			exit 1
		fi
	fi


	if [ ! -e .sdk-x86 ]; then
		echo "TIME:`date "+%H:%M:%S"` Creating a standalone SDK x86..."
		if bin/qbuild sdk >>${OUTPUT} 2>&1; then
			echo "qbuild sdk DONE"
			touch .sdk-x86
		else
			output_install_error
			echo "$0 FAILED in qbuild sdk for x86 SDK"
			exit 1
		fi
	fi


	# Remove some helix file formats
	if [ "$ALL_HELIX" == "0" -a -d /opt/Qtopia/SDK/$QPEVER/x86/image/lib/helix ]; then
		echo "Removing non distributable helix binaries"
		cd /opt/Qtopia/SDK/$QPEVER/x86/image/lib/helix
		rm -f amrff.so
		rm -f amrn.so
		rm -f amrw.so
		rm -f d263.so
		rm -f h263render.so
		rm -f mp3fformat.so
		rm -f mp3render.so
		rm -f mp4arender.so
		rm -f mp4fformat.so
	fi

	
  # Update the x86 version of the sxe_qtopia script to point to the x86 path
	if [ "$SXE" == "1" ] ; then
		cd /opt/Qtopia/SDK/$QPEVER/x86/image/etc/sxe_qtopia
		chmod +w sxe_qtopia
		sed -e "/^#!/a test -z \$QTOPIA_DIR && export QTOPIA_DIR=/opt/Qtopia/SDK/$QPEVER/x86" sxe_qtopia >sxe_qtopia.tmp
		cat sxe_qtopia.tmp > sxe_qtopia
		rm sxe_qtopia.tmp
		chmod -w sxe_qtopia
	fi

	cd $QPEDIR
}

make_device_sdk()
{
	echo "make_device_sdk()"

	echo  $QT_DEPOT $CLEAN
	export TEAMBUILDER=1


	if [ "$CLEAN" == "1" ] ;then  
		echo "clean"
		rm -Rf $DEVICE-sdk
		mkdir -p $DEVICE-sdk/qtopiacore
		rm -rf /opt/Qtopia/SDK/$QPEVER/$DEVICE
		rm -f .built-$DEVICE
		rm -f .installed-$DEVICE
		rm -f .sdk-$DEVICE
		rm -f .mkdocs

	fi

	if [ ! -e $QPEDIR/$DEVICE-sdk ]; then
		mkdir -p $QPEDIR/$DEVICE-sdk
	fi

	cd $DEVICE-sdk
	LOGFILE=`pwd`/build.log
	echo 0 > $LOGFILE
	skip_host_tools

	if [ "$QUIET" == "1" ] ; then
		xterm -e "tail -f $LOGFILE" &
	fi
	EXTRA_OPTIONS=""
	echo $DRM " "$SXE
	test "$DRM" = "1" && EXTRA_OPTIONS=" -drm"
	test "$SXE" = "1" && EXTRA_OPTIONS=$EXTRA_OPTIONS" -sxe"


	if [ "$CLEAN" == "1" ]; then
		echo "TIME:`date "+%H:%M:%S"` Configuring Qt Extended for $DEVICE"
		if $QTOPIA_DEPOT_PATH/configure -verbose -device $DEVICE -languages en_US -platform linux-g++ -sdk /opt/Qtopia/SDK/$QPEVER/$DEVICE -image /opt/Qtopia/SDK/$QPEVER/$DEVICE/image -confirm-license $SKIP $EXTRA_OPTIONS >${OUTPUT} 2>&1; then
			echo "configure DONE"
		else
			echo "$0 FAILED in configure for $DEVICE SDK"
			if [ "$QUIET" = "1" ] ; then
				echo "Try running $0 without the --quiet option to get more details on why configure failed" 
			fi
			exit 1
		fi
	else
		if [ ! -f $PWD/config.status ]; then
			echo "$0 FAILED in configure for $DEVICE SDK, \"-no-clean\" option selected but no build was found in $PWD"
			exit 1
		fi
	fi


	if [ ! -e .built-$DEVICE ]; then
		echo "TIME:`date "+%H:%M:%S"` Building $DEVICE..."
		if bin/qbuild >>${OUTPUT} 2>&1; then
			echo "qbuild DONE"
			touch .built-$DEVICE
		else
			output_install_error
		#		echo "$0 FAILED in qbuild for $DEVICE SDK"
			exit 1
		fi
	fi


	if [ ! -e .installed-$DEVICE ]; then
		echo "TIME:`date "+%H:%M:%S"` Installing $DEVICE..."
		if bin/qbuild image >>${OUTPUT} 2>&1; then
			echo "qbuild image DONE"
			touch .installed-$DEVICE
		else
			output_install_error
		#		echo "$0 FAILED in qbuild image for $DEVICE SDK"
			exit 1
		fi
	fi


	if [ ! -e .sdk-$DEVICE ]; then
		echo "TIME:`date "+%H:%M:%S"` Creating a standalone SDK $DEVICE..."
		if bin/qbuild sdk >>${OUTPUT} 2>&1; then
			echo "qbuild sdk DONE"
			touch  .sdk-$DEVICE 
		else
			output_install_error
		#		echo "$0 FAILED in qbuild sdk for $DEVICE SDK"
			exit 1
		fi
	fi

	# Remove some helix file formats
	if [ "$ALL_HELIX" == "0" -a -e /opt/Qtopia/SDK/$QPEVER/$DEVICE/image/lib/helix ]; then
		echo "Removing non distributable helix binaries"
		cd /opt/Qtopia/SDK/$QPEVER/$DEVICE/image/lib/helix
		rm -f amrff.so
		rm -f amrn.so
		rm -f amrw.so
		rm -f d263.so
		rm -f h263render.so
		rm -f mp3fformat.so
		rm -f mp3render.so
		rm -f mp4arender.so
		rm -f mp4fformat.so
	fi


}


make_device_img()
{
	echo "make_device_img()"
	rm -Rf $QPEDIR/$IMAGE_PATH
	cp -dpR /opt/Qtopia/SDK/$QPEVER/$DEVICE/image $QPEDIR/$IMAGE_PATH
}

update_extras()
{
echo "update_extras()"
	mkdir -p /opt/Qtopia/extras/bin
#	cd /opt/Qtopia/extras/bin
#	rm -f /opt/Qtopia/extras/bin/*
#	wget -nv http://qtopiaweb.trolltech.com.au/dist/input/qtopia/sdk/extras/uic3
#	chmod 555 uic3
#	wget -nv http://qtopiaweb.trolltech.com.au/dist/input/qtopia/sdk/extras/qt3to4
#	chmod 555 qt3to4


#addional device specific extra add-ons here 
# to be run at update time
	if [ -e $QTOPIA_DEPOT_PATH/devices/$DEVICE/scripts/extras.sh ]; then
		cp -a $QTOPIA_DEPOT_PATH/devices/$DEVICE/scripts/extras.sh /opt/Qtopia/extras/bin
		chmod +x /opt/Qtopia/extras/bin/extras.sh
	fi

# copy one or all flashing gif's
	cp -f $QTOPIA_DEPOT_PATH/devices/$DEVICE/src/devtools/flash-files/*.gif /opt/Qtopia/extras

}

make_flash()
{
	cd $BUILDDIR
	echo "make_flash "$BUILDDIR

	if [ ! -e /opt/Qtopia/extras/images ]; then
		mkdir -p /opt/Qtopia/extras/images
	fi

	# make flash image and update tarball
	$QTOPIA_DEPOT_PATH/devices/$DEVICE/scripts/make-flash.sh

	# remove devices installed iamge directory now we dont need it
	 rm -rf /opt/Qtopia/SDK/$QPEVER/$DEVICE/image

	# FILE=`ls *.jffs2`
	# if [ -e $FILE ]; then
	#	mv $FILE /opt/Qtopia/extras/images
	# fi
}

optimize_space()
{
	echo "Optimizing space for sdk iso"


  # Remove the docs and symlink to the $DEVICE's
  # optimize for space
	cd /opt/Qtopia/SDK/$QPEVER/$DEVICE
	if [ -e /opt/Qtopia/SDK/$QPEVER/$DEVICE/doc ]; then
		rm -rf /opt/Qtopia/SDK/$QPEVER/$DEVICE/doc
		ln -s /opt/Qtopia/SDK/$QPEVER/x86/doc
	fi

#	rm -rf /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/qt/doc
#	ln -s /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/doc /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/qt/doc
#	ln -s /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/tools /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/qt/tools


  # Optimise the space requirements for the docs.
  # Most of the Qt docs exist in the Qtopia doc directory so we save about 55MB.
#	cd /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/doc/html
#	rm -r images
#	ln -s ../../../../doc/html/images .
#	for file in *.html; do
#		if [ "$file" != "*.html" ]; then
#					#	if diff -q $file ../../../../doc/html/$file >/dev/null 2>&1; then
#			rm $file
#			ln -s ../../../../doc/html/$file .
#					#	fi
#		fi
#	done

# symlink redundant libs

DIRS="/opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/target/lib"
for f in $DIRS; do
	cd $f
	rm -f libQtCore.so.${QTVER}
	rm -f libQtDBus.so.${QTVER}
	rm -f libQtGui.so.${QTVER}
	rm -f libQtNetwork.so.${QTVER}
	rm -f libQtScript.so.${QTVER}
	rm -f libQtSql.so.${QTVER}
	rm -f libQtSvg.so.${QTVER}
	rm -f libQtTest.so.${QTVER}
	rm -f libQtWebKit.so.${QTVER}
	rm -f libQtXml.so.${QTVER}
	ln -s ../../../image/lib/libQtCore.so.4 libQtCore.so.${QTVER}
	ln -s ../../../imdge/lib/libQtDBus.so.4 libQtDBus.so.${QTVER}
	ln -s ../../../image/lib/libQtGui.so.4 libQtGui.so.${QTVER}
	ln -s ../../../image/lib/libQtNetwork.so.4 libQtNetwork.so.${QTVER}
	ln -s ../../../image/lib/libQtScript.so.4 libQtScript.so.${QTVER}
	ln -s ../../../image/lib/libQtSql.so.4 libQtSql.so.${QTVER}
	ln -s ../../../image/lib/libQtSvg.so.4 libQtSvg.so.${QTVER}
	ln -s ../../../image/lib/libQtTest.so.4 libQtTest.so.${QTVER}
	ln -s ../../../image/lib/libQtWebKit.so.4 libQtWebKit.so.${QTVER}
	ln -s ../../../image/lib/libQtXml.so.4 libQtXml.so.${QTVER}
done


# optimize redundant headers
    cd /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host/include
    FILES="Qt QtCore QtGui QtNetwork QtScript QtSql QtSvg QtTest QtXml"
    for f in $FILES; do
        rm -rf $f
        ln -s ../../target/include/$f $f
	done

# mkspecs as well

	cd /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/target
	rm -rf mkspecs
	ln -s ../../../x86/qtopiacore/target/mkspecs mkspecs

    cd /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host
    rm -rf mkspecs
    ln -s ../target/mkspecs mkspecs

# now optimize the extras
#	if [ -e /opt/Qtopia/extras/bin/uic3 ] ; then
#		if [ -e /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/host/bin ] ; then
#			rm -f /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/host/bin/uic3
#			rm -f /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/host/bin/qt3to4

#			ln -s /opt/Qtopia/extras/bin/uic3 /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/host/bin/uic3
#			ln -s /opt/Qtopia/extras/bin/qt3to4 /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/host/bin/qt3to4
#		fi
#	fi


# translations
	cd /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/target
	rm -rf translations
	ln -s /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/target/translations translations

# plugins too
	cd /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host
	rm -rf plugins
	ln -s /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/target/plugins plugins

# and a few misc things
# breaks
#	cd /opt/Qtopia/SDK/$QPEVER/$DEVICE
#	rm -rf modules
#	ln -s ../x86/modules modules

	rm -rf src
	ln -s ../x86/src src

        #remove unneeded qvfb skins
    rm -rf /opt/Qtopia/SDK/$QPEVER/x86/src/tools

# weeeeeee! we saved heaps of space!

	cd $BUILDDIR
}

make_iso()
{
	cd $BUILDDIR
	echo "make_iso "
	$QTOPIA_DEPOT_PATH/scripts/make-iso.sh	-d $DEVICE
}


HELP=yes
OUTPUT="/dev/stdout"
SDKDEVICE=0
SDKX86=0
SDKIMG=0
FLASH=0
EXTRAS=1
VMWARE=0
ROOTFS=1
QUIET=0
SXE=0
DRM=0
IMAGE_PATH=image
DEVICE=""
BUILD_PATH=$PWD/rootfs
CLEAN=1
ALL_HELIX=0
ISO=0

while [ -n "$1" ] ; do
	case $1 in
		-h | -help | --help)
		HELP=yes
		;;
		--device)
		DEVICE="$2"
		shift 1
		;;
		-q | -quiet | --quiet)
		OUTPUT="build.log"
		HELP=no
		QUIET=1
		;;
		-sdk | --sdk)
		SDKDEVICE=1
		SDKX86=1
		EXTRAS=1
		SDKIMG=1
		HELP=no
		;;
		-sdk-device | --sdk-device)
		SDKDEVICE=1
		HELP=no
		;;
		-sdk-x86 | --sdk-x86)
		SDKX86=1
		HELP=no
		;;
		-sdk-img | --sdk-img)
		SDKIMG=1
		HELP=no
		;;
		-fl | --flash)
		FLASH=1
		HELP=no
		;;
		-ne | --no-extras)
		EXTRAS=0
		ROOTFS=0
		HELP=no
		;;
		-sx | --sxe)
		SXE=1
		;;
		-dr | --drm)
		DRM=1
		;;
		-i | --iso)
		ISO=1
		HELP=no
		;;	 
		-ip | --image-path)
		IMAGE_PATH="$2"
		shift 1
		;;
		-bp | --build-path)
		BUILD_PATH="$2"
		shift 1
		;;
		-hx | --helix)
		ALL_HELIX=1
		;;
		-nc | --no-clean)
		CLEAN=0
		;;
		*)
		echo $1: unknown argument
		;;
	esac
	shift
done

if [ "$HELP" = "yes" ] ; then
	echo
	echo "FORMAT: $0 <options>"
	echo
	echo "Options:"
	echo 
	echo " --help			   Display command help"
	echo " --quiet			   Quiet mode"
	echo " --device <device>   Set \$DEVICE to <device> SDK"
	echo " --sdk			   Build x86, $DEVICE and image for SDK"
	echo " --sdk-device		   Build $DEVICE SDK"
	echo " --sdk-x86		   Build x86 SDK"
	echo " --sdk-img		   Create Qt Extended dir from $DEVICE SDK binaries."
	echo " --flash			   Create \"$DEVICE\"_flash file"
	echo " --no-extras		   Do not update qt3to4, uic3 etc"
	echo " --sxe			   Builds the specified SDK with SXE"
	echo " --drm			   Builds the specified SDK with DRM"
	echo " --image-path		   Specify the path where the Qt Extended filesystem is."
	echo " --build-path		   Specify the path where the rootfs is."
	echo " --no-clean		   Clean, reconfigure and build"
	echo " --external-toolchain Use downloadable toolchain"
	echo " --iso			  Create installable iso"
	echo
	exit
fi

if [ "`hostname`" == $DEVICE"sdk" ] ; then
  #We force if detect building inside vmware since don't have access to extras
	EXTRAS=0
	ROOTFS=0
	echo "Extras option has been disabled"
fi

export QPEVER=`version`
export QTVER=`qt_version`

export TEAMBUILDER=1

export QPEDIR=`pwd`
#export PATH=${QPEDIR}/bin:${QPEDIR}/scripts:${PATH}

if [ "$SDKX86" == "1" ]; then
	if [ "$CLEAN" == "1" ]; then
		rm -Rf /opt/Qtopia/SDK/$QPEVER/x86
	fi
	make_x86_sdk
fi

if [ "$SDKDEVICE" == "1" ]; then
	if [ "$SDKX86" == "1" ]; then
		# Use the x86 SDK's Qt as the system Qt
		export PATH=/opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host/bin:$PATH
	fi
	if [ "$CLEAN" == "1" ]; then
		rm -Rf /opt/Qtopia/SDK/$QPEVER/$DEVICE
	fi
	make_device_sdk
fi

if [ "$SDKIMG" == "1" ]; then
	make_device_img
fi

if [ "$EXTRAS" == "1" ]; then
	rm -Rf /opt/Qtopia/extras
	update_extras
fi

if [ "$FLASH" == "1" ]; then
	make_flash
fi


if [ -z $DEVICE ]; then
	echo "DEVICE is not set "$DEVICE
	exit 1
fi
if [ "$SDKDEVICE" == "1" ] ; then
	if [ ! -e /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/tools/qvfb/$DEVICE.skin ] ; then
		mkdir -p /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/tools/qvfb 
		cp -dpR $QTOPIA_DEPOT_PATH/devices/$DEVICE/*.skin /opt/Qtopia/SDK/$QPEVER/$DEVICE/qtopiacore/qt/tools/qvfb 
	fi

	rm -rf /opt/Qtopia/SDK/scripts
	mkdir -p /opt/Qtopia/SDK/scripts
	cp $QTOPIA_DEPOT_PATH/bin/mkPackages /opt/Qtopia/SDK/scripts

#copy any device specific scripts
	echo "copy SDK files"
	if [ -e $QTOPIA_DEPOT_PATH/devices/$DEVICE/sdk/scripts ]; then
	for FILE in `ls $QTOPIA_DEPOT_PATH/devices/$DEVICE/sdk/scripts`
	  do
	  cp -v $QTOPIA_DEPOT_PATH/devices/$DEVICE/sdk/scripts/$FILE /opt/Qtopia/SDK/scripts
	  chmod 775 /opt/Qtopia/SDK/scripts/$FILE
	done
	fi

	cp -a $QTOPIA_DEPOT_PATH/scripts/sdk/scripts/* /opt/Qtopia/SDK/scripts

	for FILE in `ls /opt/Qtopia/SDK/scripts`; do
		if [ -x /opt/Qtopia/SDK/scripts/$FILE ]; then
			chmod +w /opt/Qtopia/SDK/scripts/$FILE
		fi
	done
#rename this one to something device specific
	mv /opt/Qtopia/SDK/scripts/devel- /opt/Qtopia/SDK/scripts/devel-$DEVICE.sh


	echo -en "\043\041/bin/bash\nversion()\n{\necho \"$QPEVER\"\n}\n\nqt_version()\n{\necho \"$QTVER\"\n}\n">/opt/Qtopia/SDK/scripts/functions

	
# add some device specific functions
# take TOOLCHAIN from device environment
	cat $QTOPIA_DEPOT_PATH/devices/$DEVICE/environment >> /opt/Qtopia/SDK/scripts/functions

# add usb vendor and product info
	cat $QTOPIA_DEPOT_PATH/devices/$DEVICE/scripts/functions >> /opt/Qtopia/SDK/scripts/functions

# add standard sdk functions
	cat $QTOPIA_DEPOT_PATH/scripts/sdk/scripts/functions >> /opt/Qtopia/SDK/scripts/functions

# get the dpi from defaultbuttons
	MMWIDTH=`grep QWS_DISPLAY $QTOPIA_DEPOT_PATH/devices/$DEVICE/src/libraries/qtopiabase/etc/defaultbuttons.conf  | awk 'BEGIN{FS=":"}{print $2}'`
	MMHEIGHT=`grep QWS_DISPLAY $QTOPIA_DEPOT_PATH/devices/$DEVICE/src/libraries/qtopiabase/etc/defaultbuttons.conf   | awk 'BEGIN{FS=":"}{print $3}'`
	echo "export QWS_DISPLAY=QVFb:$MMWIDTH:$MMHEIGHT:0" >> /opt/Qtopia/SDK/scripts/functions


	chmod 755 /opt/Qtopia/SDK/scripts/functions
# version information
	echo "Built: `date`" > /opt/Qtopia/SDK/versioninfo
	echo "By:	 `whoami`" >> /opt/Qtopia/SDK/versioninfo
	echo "Host:	 `hostname`" >> /opt/Qtopia/SDK/versioninfo
	echo "Device: $DEVICE" >> /opt/Qtopia/SDK/versioninfo

fi

# stop doing this for now, it seems to be breaking things
optimize_space

echo $DEVICE
if [ "$ISO" == "1" ] ; then
	make_iso
fi
