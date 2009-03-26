/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef CUSTOM_LINUX_GENERIC_GPP_H
#define CUSTOM_LINUX_GENERIC_GPP_H

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

// The serial device for AT command access to the phone
// hardware:
//
#define QTOPIA_PHONE_DEVICE "/dev/ttyS0"

// Displays the homescreen background in the phone launcher.
#if !defined(Q_WS_X11)
#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#endif

// Displays the homescreen background in all applications.
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Builds the calibration settings app into the server.
//#define QPE_NEED_CALIBRATION

// Define the phone vendor.  This indicates to load "libfoovendor.so"
// as the phone vendor plugin, and "libfoomultiplex.so" as the multiplexer.
//#define QTOPIA_PHONE_VENDOR "foo"

// Define this if wireless LAN support should be removed from the lan plugin
// Removing Wireless LAN support reduces the size of the lan plugin by about 1 MB.
// Extended Wireless LAN support (scanning and active reconnection) requires Wireless extension v14+ and will only be enabled if the
// device supports WE v14+
//#define NO_WIRELESS_LAN

// Defines devices whose packages are compatible with this device,
// by convention the first device listed is this device.
#define QTOPIA_COMPATIBLE_DEVICES "Desktop"

#ifndef V4L_VIDEO_DEVICE
#define V4L_VIDEO_DEVICE            "/dev/video"
#endif

#endif
