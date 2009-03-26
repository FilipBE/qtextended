#!/bin/bash
ROOT_UID=0
VMPLAYER_DIR=`which vmplayer`
if [ ! -n "$VMPLAYER_DIR" ] ; then
    VMPLAYER_DIR=`which vmware`
fi

SCRIPT="$0"
DIR=${SCRIPT%"install.sh"}
#echo $DIR
if [ "$DIR" != "./" -a -n "$DIR" ]; then
    cd $DIR
fi
clear
cat license.txt
echo ""
echo "Have you agreed? Press Y for Yes"
read -e ANSWER
if [ "$ANSWER" != "Y" ]; then
exit 1;
fi

more release.txt

# try to be tricky here....
VMXFILE=`ls *.vmx`
SDKNAME=`grep displayName $VMXFILE | awk 'BEGIN{FS="\""}{print $2}'`

echo "Qt Extended $SDKNAME SDK Linux Installation"
echo
if [ ! -n "$VMPLAYER_DIR" ] ; then
    echo "Installation of vmplayer"
    echo "If not installed download and install before continuing"
    echo "at http://www.vmware.com"
    echo 
    echo -n "Unable to find vmplayer. Have you installed vmplayer? (y/n) : "
    read option
    if [ "$option" != "y" ] ; then
        echo "Qt Extended $SDKNAME SDK requires vmplayer or equivalent to be installed."
        echo
        exit
    fi
else
    echo "Found VMPlayer installed at: $VMPLAYER_DIR"
fi
echo
echo "NOTE: SDK requires 3.2GB free disk space and 512MB RAM"
echo
echo -n "What directory would you like to install the SDK? [$HOME/QtExtended"$SDKNAME"SDK] : "
read dir
if [ ! -n "$dir" ] ; then
    dir="$HOME/QtExtended"$SDKNAME"SDK"
fi
if [ ! -e "$dir" ] && [ ! -L "$dir" ] ; then
    echo
    echo -n "Directory does not exist. Do you wish to create the directory $dir? (y/n) : "
    read option
    if [ "$option" != "y" ] ; then
        echo "Installation Aborted by user."
        echo
        exit
    fi
    mkdir -p "$dir"
fi

if [ ! -w "$dir" ] ; then
		if [ -L "$dir" ] ; then
				echo
				echo "$dir is a symbolic link. Please choose a directory you have permission to write to."
				echo
				exit 1
		fi 

    echo
    echo "Please choose a directory you have permission to write to."
    echo
    exit 1
fi 
echo
echo "Directory to install to: $dir"
echo 
echo -n "Continue with installation? (y/n) : "
read option
if [ "$option" != "y" ] ; then
    echo "Installation Aborted by user."
    echo
    exit
fi


echo "Installing $SDKNAME Qt Extended SDK....please wait"

if [ -e "$dir/release.html" ]; then
  rm -f "$dir/release.html"
fi
cp release.html "$dir"
chmod 664 "$dir/release.html"

if [ -e "$dir/$VMXFILE" ]; then
  rm -f "$dir/$VMXFILE"
fi

if ! cp $VMXFILE "$dir"; then
		echo "Copying vmx file failed."
		exit 1
fi

chmod 664 "$dir/$VMXFILE"

if [ -e "$dir/license.txt" ]; then
  rm -f "$dir/license.txt"
fi
if ! cp license.txt "$dir"; then
		echo "Copying licence.txt file failed."
		exit 1
fi

chmod 664 "$dir/license.txt"

export currentdir=$PWD
cd "$dir"

ERROR="0"

function unpack()
{
if ! tar -xvjf "$1" 
      then
      ERROR="1"
      echo "FAILED!"
      rm -f "$1"
      exit 1
  fi
}

if [ "$ERROR" -ne "1" ]; then
  echo "Installing qtopia SDK... Part 1/6"
  unpack "$currentdir/qtopia.dat"
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/qtopiasrc.vmdk" ]; then
    echo -n "Do you want to update the devel partition? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
      echo "Installing devel... Part 2/6"
      unpack "$currentdir/qtopiasrc.dat"
    fi
  else
    echo "Installing devel... Part 2/6"
    unpack "$currentdir/qtopiasrc.dat"
  fi
fi

if [ "$ERROR" -ne "1" ]; then
  echo "Installing toolchain.... Part 3/6"
  unpack "$currentdir/toolchain.dat"
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/home.vmdk" ]; then
    echo -n "Do you want to update the home directory and settings? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
        echo "Updating home........... Part 4/6"
        unpack "$currentdir/home.dat"
    fi
  else
     echo "Installing home......... Part 4/6"
     unpack "$currentdir/home.dat"
  fi
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/rootfs.vmdk" ]; then
    echo -n "Do you want to update the root filesystem? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
      echo "Updating rootfs......... Part 5/6"
      unpack "$currentdir/rootfs.dat"
    fi
  else
    echo "Installing rootfs....... Part 5/6"
    unpack "$currentdir/rootfs.dat"
  fi
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/qtextended.iso" ]; then
    echo -n "Do you want to update the Qt Extended update iso? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
			if !	rm -f "$dir/qtextended.iso"
					then
						ERROR="1"
						exit 1
				fi
      echo "Updating qtextended iso..... Part 6/6"
      cp "$currentdir/qtextended.iso" "$dir"
      chmod 664 "$dir/qtextended.iso"
    fi
  else
    echo "Installing qtextended iso....... Part 6/6"
      cp "$currentdir/qtextended.iso" "$dir"
      chmod 664 "$dir/qtextended.iso"
  fi

fi


if [ "$ERROR" -ne "1" ]; then

if [ -e "$dir/home.vmdk" ] ; then
  if [ -e "$dir/qtopiasrc.vmdk" ] ; then
    if [ -e "$dir/qtopia.vmdk" ] ; then
      if [ -e "$dir/toolchain.vmdk" ] ; then
        if [ -e "$dir/rootfs.vmdk" ] ; then
          if [ -e "$dir/$VMXFILE" ] ; then
            echo 
            echo "Installation successful."
            echo "to start vmplayer $dir/$VMXFILE"
            echo
            exit
          fi
        fi
      fi
    fi
  fi
fi

fi

echo "Installation FAILED!"
echo
