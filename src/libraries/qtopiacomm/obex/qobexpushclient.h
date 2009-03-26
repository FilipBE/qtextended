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

#ifndef QOBEXPUSHCLIENT_H
#define QOBEXPUSHCLIENT_H

#include <qobexnamespace.h>

#include <QObject>
#include <QString>

class QObexPushClientPrivate;
class QByteArray;
class QIODevice;

class QTOPIAOBEX_EXPORT QObexPushClient : public QObject
{
    Q_OBJECT
public:

    enum Command {
        None,
        Connect,
        Disconnect,
        Send,
        SendBusinessCard,
        RequestBusinessCard
    };

    enum Error {
        NoError,
        ConnectionError,
        RequestFailed,
        Aborted,
        UnknownError = 100
    };

    explicit QObexPushClient(QIODevice *device, QObject *parent = 0);
    virtual ~QObexPushClient();

    QIODevice *sessionDevice() const;

    Error error() const;
    QObex::ResponseCode lastCommandResponse() const;

    int currentId() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    int connect();
    int disconnect();

    int send(QIODevice *device,
             const QString &name,
             const QString &type = QString(),
             const QString &description = QString());

    int send(const QByteArray &array,
             const QString &name,
             const QString &type = QString(),
             const QString &description = QString());

    int sendBusinessCard(QIODevice *vcard);
    int requestBusinessCard(QIODevice *vcard);
    void exchangeBusinessCard(QIODevice *mine, QIODevice *theirs,
                                int *putId = 0, int *getId = 0);

public slots:
    void abort();

signals:
    void commandStarted(int id);
    void commandFinished(int id, bool error);
    void dataTransferProgress(qint64, qint64);
    void done(bool error);

private:
    friend class QObexPushClientPrivate;
    QObexPushClientPrivate *m_data;
    Q_DISABLE_COPY(QObexPushClient)
};

#endif
