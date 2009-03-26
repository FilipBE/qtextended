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

#ifndef SESSIONEDOBEXCLIENT_H
#define SESSIONEDOBEXCLIENT_H

#include <QObject>

class QByteArray;
class QIODevice;
class QObexHeader;
class SessionedObexClientPrivate;

class SessionedObexClient : public QObject
{
    Q_OBJECT
public:
    SessionedObexClient(const QByteArray &commDeviceId,
                        QObject *parent = 0);
    virtual void begin(const QObexHeader &header, QIODevice *dataToSend);
    virtual void abort();

signals:
    void requestStarted();
    void dataSendProgress(qint64 done, qint64 total);
    void requestFinished(bool error, bool aborted);
    void done();

protected:
    void sendRequest(QIODevice *socket);

    virtual void openedDeviceSession(bool error) = 0;
    virtual void closedDeviceSession();
    virtual void forceAbort() = 0;

private:
    SessionedObexClientPrivate *d;
    friend class SessionedObexClientPrivate;
};

#endif
