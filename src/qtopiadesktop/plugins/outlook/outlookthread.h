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
#ifndef OUTLOOKTHREAD_H
#define OUTLOOKTHREAD_H

#include "qoutlook.h"

#include <QDThread>
#include <QStringList>

class OutlookSyncPlugin;
namespace QMAPI {
    class Session;
};
class QDateTime;
class OutlookThreadObject;

// =====================================================================

class OutlookThread : public QDThread
{
    Q_OBJECT
private:
    OutlookThread( QObject *parent = 0 );
    ~OutlookThread();

    void t_init();
    void t_quit();

public:
    static OutlookThread *getInstance( QObject *syncObject );
    OutlookThreadObject *o;
};

// =====================================================================

// This object lives on the Outlook thread
class OutlookThreadObject : public QObject
{
    Q_OBJECT
public:
    OutlookThreadObject();
    ~OutlookThreadObject();

    bool logon();

public:
    Outlook::_ApplicationPtr ap;
    Outlook::_NameSpacePtr ns;
};

#endif
