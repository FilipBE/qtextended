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

#ifndef SCOMISC_P_H
#define SCOMISC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/*
    This is a private subsystem of the Bluetooth Handsfree / Headset profile
    implementations.  It is meant to allow system integrators to access the
    raw voice data coming in on the SCO socket connection.

    System integrators can provide their own implementation that handles
    the necessary audio routing steps.  Since the most common scenario on
    phones is to have the audio routed over PCM, most devices do not
    actually need to perform any management, and these functions can be empty.

    See the greenphone integration for an example of this.  If software
    routing is required, an example implementation using the btsco kernel
    module is provided.  It is not supported and will not work in many situations.
*/

/*
    Sets the SCO socket file descriptor for handle.  Note that the sco_fd
    can be -1, in this case it is indicated that audio should not be routed
    to the bluetooth headset, otherwise audio should be routed.

    return true if the set operation succeeded, and false otherwise.

    Note that the sco_fd is a bare socket descriptor, you must perform
    all management yourself (e.g. you must notify the audio gateway that
    the socket has been closed for instance)

    See the greenphone scomisc integration for an example.
*/
bool bt_sco_set_fd(void *handle, int sco_fd);

/*
    Tries to find a device with \a idPref.  The Headset service will always
    pass in "Headset" and Handsfree service will always pass in "Handsfree".

    Return a null QByteArray if no devices that are capable of routing
    the audio exist on the system.  Otherwise return a non-empty QByteArray.

    The contents will be used to call bt_sco_open.
*/
QByteArray find_btsco_device(const QByteArray &idPref = QByteArray());

/*
    Closes the bt_sco handle given by \a handle.
*/
void bt_sco_close(void *handle);

/*
    Opens the device given by \a dev.  The handle is returned in \a handle.
    Note that the handle should store a non-NULL pointer, otherwise
    it is assumed that the device failed to open.

    Returns true if the open succeeded, and false otherwise.
*/
bool bt_sco_open(void **handle, const char *dev);

#endif
