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
#include "outlookthread.h"
#include "qmapi.h"
#include "outlooksync.h"

#include <qdwin32.h>
using namespace QDWIN32;

#include <qdplugin.h>
#include <trace.h>
QD_LOG_OPTION(OutlookThread)

#include <qcopenvelope_qd.h>
#include <desktopsettings.h>

#include <QBuffer>
#include <QApplication>
#include <QXmlStreamReader>
#include <QTimer>
#include <QProcess>
#include <QPointer>

#define _WIN32_DCOM
#include <objbase.h>

// =====================================================================

OutlookThread::OutlookThread( QObject *parent )
    : QDThread( parent ), o( 0 )
{
    TRACE(OutlookThread) << "OutlookThread::OutlookThread";
}

OutlookThread::~OutlookThread()
{
    TRACE(OutlookThread) << "OutlookThread::~OutlookThread";
    quit();
    wait();
}

void OutlookThread::t_init()
{
    TRACE(OutlookThread) << "OutlookThread::t_init";

    LOG() << "CoInitialize(0)";
    HRESULT hr = CoInitialize(0);
    if ( hr != S_OK )
        return;

    o = new OutlookThreadObject;
}

void OutlookThread::t_quit()
{
    TRACE(OutlookThread) << "OutlookThread::t_quit";
    if ( !o )
        return;

    delete o;
    o = 0;

    LOG() << "CoUninitialize";
    CoUninitialize();
}

OutlookThread *OutlookThread::getInstance( QObject *syncObject )
{
    TRACE(OutlookThread) << "OutlookThread::getInstance";
    static QPointer<OutlookThread> sInstance;

    if ( sInstance.isNull() ) {
        LOG() << "Creating singleton instance";
        sInstance = new OutlookThread( syncObject );
        sInstance->start();
    }

    return sInstance;
}

// =====================================================================

OutlookThreadObject::OutlookThreadObject()
    : QObject()
{
    TRACE(OutlookThread) << "OutlookThreadObject::OutlookThreadObject";
}

OutlookThreadObject::~OutlookThreadObject()
{
    TRACE(OutlookThread) << "OutlookThreadObject::~OutlookThreadObject";
    if ( ns ) {
        LOG() << "ns->Logoff()";
        ns->Logoff();
        LOG() << "ns.Detach()->Release()";
        ns.Detach()->Release();
    }
    if ( ap ) {
        LOG() << "ap.Detach()->Release()";
        ap.Detach()->Release();
    }
}

bool OutlookThreadObject::logon()
{
    TRACE(OutlookThread) << "OutlookThreadObject::logon";

    if ( ap ) {
        // This is not the first call to logon()
        return true;
    }

    ap.CreateInstance("Outlook.Application");

    if ( ap ) {
        LOG() << "ns = ap->GetNamespace(\"MAPI\")";
        ns = ap->GetNamespace("MAPI");
        LOG() << "ns->Logon()";
        ns->Logon();
        return true;
    }

    WARNING() << "Can't connect to Outlook";
    return false;
}

#include "outlookthread.moc"
