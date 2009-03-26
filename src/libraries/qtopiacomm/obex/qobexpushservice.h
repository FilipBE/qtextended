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

#ifndef QOBEXPUSHSERVICE_H
#define QOBEXPUSHSERVICE_H

#include <qobexnamespace.h>

#include <QObject>
#include <QString>

class QIODevice;
class QObexPushServicePrivate;

class QTOPIAOBEX_EXPORT QObexPushService : public QObject
{
    Q_OBJECT

public:

    enum State {
        Ready,
        Connecting,
        Disconnecting,
        Streaming,
        Closed = 100
    };

    enum Error {
        NoError,
        ConnectionError,
        Aborted,
        UnknownError = 100
    };

    explicit QObexPushService(QIODevice *device, QObject *parent = 0);
    ~QObexPushService();

    State state() const;
    Error error() const;

    QIODevice *sessionDevice() const;

    void setBusinessCard(const QByteArray &vCard);
    virtual QByteArray businessCard() const;

    QIODevice *currentDevice() const;

public slots:
    void abort();

protected:
    virtual QIODevice *acceptFile(const QString &name, const QString &type, qint64 size, const QString &description);

signals:
    void putRequested(const QString &name, const QString &type, qint64 size, const QString &description);
    void businessCardRequested();
    void stateChanged(QObexPushService::State);
    void dataTransferProgress(qint64 done, qint64 total);
    void requestFinished(bool error);
    void done(bool error);

private:
    friend class QObexPushServicePrivate;
    QObexPushServicePrivate *m_data;
    Q_DISABLE_COPY(QObexPushService)
};

#endif
