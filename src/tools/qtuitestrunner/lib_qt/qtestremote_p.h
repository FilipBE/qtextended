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

#ifndef QTESTREMOTE_P_H
#define QTESTREMOTE_P_H

#include "qtestprotocol_p.h"
#include <qtuitestglobal.h>

class QString;

class QTUITEST_EXPORT QTestRemote : public QTestProtocol
{
    Q_OBJECT

public:
    QTestRemote();
    virtual ~QTestRemote();

    void openRemote( const QString &ip, int port );
    virtual void processMessage( QTestMessage *msg );

    bool must_stop_event_recording;
    bool event_recording_aborted;

signals:
    void abort();
};

#endif

