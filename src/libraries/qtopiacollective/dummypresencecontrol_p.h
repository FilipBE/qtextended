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

#ifndef DUMMYPRESENCECONTROL_P_H
#define DUMMYPRESENCECONTROL_P_H

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

#include <qtopiaglobal.h>
#include <QAbstractIpcInterface>
#include <qcollectivenamespace.h>
#include "qcollectivepresence.h"

class QCollectivePresenceInfo;
template <class T> class QList;

class QTOPIACOLLECTIVE_EXPORT DummyPresenceControl : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    DummyPresenceControl(const QString &service = QString(),
                         QObject *parent = 0, QAbstractIpcInterface::Mode = Client);
    ~DummyPresenceControl();

public slots:
    virtual void updatePresences(const QList<QCollectivePresenceInfo> &presences);
    virtual void simulateIncomingPublishRequest(const QString &uri);
    virtual void simulateSubscribeAccept(const QString &uri);
    virtual void simulateSubscribeReject(const QString &uri);

private:
    Q_DISABLE_COPY(DummyPresenceControl)
};

#endif
