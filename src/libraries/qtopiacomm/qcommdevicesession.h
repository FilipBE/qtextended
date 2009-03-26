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

#ifndef QCOMMDEVICESESSION_H
#define QCOMMDEVICESESSION_H

#include <qglobal.h>
#include <qobject.h>

#include <qtopiaglobal.h>

class QByteArray;

class QCommDeviceSession_Private;
class QTOPIACOMM_EXPORT QCommDeviceSession : public QObject
{
    Q_OBJECT

    friend class QCommDeviceSession_Private;

public:
    enum WaitType { Block, BlockWithEventLoop };

    explicit QCommDeviceSession(const QByteArray &deviceId, QObject *parent = 0);
    ~QCommDeviceSession();

    void startSession();
    void endSession();

    const QByteArray &deviceId() const;

    static QCommDeviceSession * session(const QByteArray &deviceId,
                                        WaitType type = BlockWithEventLoop,
                                        QObject *parent = 0);

signals:
    void sessionOpen();
    void sessionFailed();
    void sessionClosed();

private:
    QCommDeviceSession_Private *m_data;
};

#endif
