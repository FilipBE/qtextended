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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qtestremote_p.h"

#include <stdlib.h> // used for exit

QTestRemote::QTestRemote()
{
    must_stop_event_recording = false;
    event_recording_aborted = false;
}

QTestRemote::~QTestRemote()
{
}

void QTestRemote::processMessage( QTestMessage *msg )
{
    if (msg->event() == "stopEventRecording") {
        must_stop_event_recording = true;
    } else if (msg->event() == "eventRecordingAborted") {
        must_stop_event_recording = true;
        event_recording_aborted = true;
    } else if (msg->event() == "abort") {
        emit abort();
    }
}

/*!
    \internal
    Opens a socket connection to a remote test tool and uses the connection to
    communicate with the connected tool.
*/
void QTestRemote::openRemote( const QString &ip, int port )
{
    connect( ip, port );
    if (!waitForConnected(5000)) {
        // it really doesn't make sense to continue testing if we cannot communicate with the remote tool so we abort immediately.
        exit( REMOTE_CONNECT_ERROR );
    }
}

