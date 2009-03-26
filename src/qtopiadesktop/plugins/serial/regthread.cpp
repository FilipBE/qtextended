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
#include "regthread.h"

#include <trace.h>

using namespace QDWIN32;

// =====================================================================

RegThread::RegThread()
    : QThread()
{
    handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    quit = CreateEvent(NULL, FALSE, FALSE, NULL);
    hKey = openRegKey( HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM" );
    getPorts();
}

RegThread::~RegThread()
{
    CloseHandle( handle );
    CloseHandle( quit );
    if ( hKey )
        RegCloseKey( hKey );
}

void RegThread::run()
{
#define HANDLES 2
    HANDLE handles[HANDLES] = { handle, quit };
    for ( ;; ) {
        RegNotifyChangeKeyValue( hKey, false, REG_NOTIFY_CHANGE_LAST_SET, handle, true );
        DWORD ret = WaitForMultipleObjects( HANDLES, handles, false, INFINITE );
        if ( ret == WAIT_OBJECT_0 ) {
            getPorts();
            emit comPortsChanged();
        } else if ( ret == WAIT_OBJECT_0+1 ) {
            break;
        }
    }
}

void RegThread::getPorts()
{
    TRACE(QDLink) << "RegThread::getPorts";
    portMutex.lock();
    mPorts = QStringList();
    if ( hKey )
        mPorts = readRegKeys( hKey );
    portMutex.unlock();
}

QStringList RegThread::ports()
{
    portMutex.lock();
    QStringList ret = mPorts;
    portMutex.unlock();
    return ret;
}

