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

#include <qtopialog.h>
#include "dummypresencecontrol_p.h"

#include <QCollectivePresenceInfo>

DummyPresenceControl::DummyPresenceControl(const QString &service, QObject *parent,
                                           QAbstractIpcInterface::Mode mode)
                : QAbstractIpcInterface( "/Communications", "DummyPresenceControl",
                                 service, parent, mode )
{
    proxyAll(staticMetaObject);
}

DummyPresenceControl::~DummyPresenceControl()
{

}

void DummyPresenceControl::updatePresences(const QList<QCollectivePresenceInfo> &presences)
{
    invoke(SLOT(updatePresences(QList<QCollectivePresenceInfo>)), QVariant::fromValue(presences));
}

void DummyPresenceControl::simulateIncomingPublishRequest(const QString &uri)
{
    invoke(SLOT(simulateIncomingPublishRequest(QString)), QVariant::fromValue(uri));
}

void DummyPresenceControl::simulateSubscribeAccept(const QString &uri)
{
    invoke(SLOT(simulateSubscribeAccept(QString)), QVariant::fromValue(uri));
}

void DummyPresenceControl::simulateSubscribeReject(const QString &uri)
{
    invoke(SLOT(simulateSubscribeReject(QString)), QVariant::fromValue(uri));
}
