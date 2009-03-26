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

#ifndef QOBEXFTPCLIENT_H
#define QOBEXFTPCLIENT_H

#include <QObject>
#include <QString>
#include <QDateTime>

#include <qobexnamespace.h>
#include <qobexglobal.h>

class QObexFolderListingEntryInfo;
class QObexFtpClientPrivate;
class QIODevice;
class QObexHeader;

class QTOPIAOBEX_EXPORT QObexFtpClient : public QObject
{
    Q_OBJECT

public:

    enum Command {
        None,
        Connect,
        Cd,
        CdUp,
        List,
        Get,
        Put,
        Mkdir,
        Rmdir,
        Remove,
        Disconnect,
    };

    enum State {
        Unconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    enum Error {
        NoError,
        RequestFailed,
        InvalidRequest,
        InvalidResponse,
        ListingParseError,
        Aborted,
        AuthenticationFailed,
        UnknownError = 100
    };

    explicit QObexFtpClient(QIODevice *device, QObject *parent = 0);
    ~QObexFtpClient();

    QIODevice *sessionDevice() const;
    Error error() const;
    QString errorString() const;

    int currentId() const;
    QObexFtpClient::State state() const;
    QObexFtpClient::Command currentCommand() const;
    QIODevice *currentDevice() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    QObex::ResponseCode lastCommandResponse() const;

    int connect();
    int list(const QString &dir = QString());

    int cd(const QString &dir = QString());
    int cdUp();

    int mkdir(const QString &dir);
    int rmdir(const QString &dir);

    int get(const QString &file, QIODevice *dev = 0);
    int put(QIODevice *dev, const QString &file,
            qint64 size = 0,
            const QString &mimetype = QString(),
            const QString &description = QString(),
            const QDateTime &lastModified = QDateTime());
    int put(const QByteArray &data, const QString &file,
            const QString &mimetype = QString(),
            const QString &description = QString(),
            const QDateTime &lastModified = QDateTime());

    int remove(const QString &file);

    int disconnect();

    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();
    qint64 bytesAvailable() const;

public slots:
    void abort();

signals:
    void listInfo(const QObexFolderListingEntryInfo &info);
    void getInfo(const QString &mimetype, const QString &description, const QDateTime &lastModified);
    void done(bool error);
    void dataTransferProgress(qint64, qint64);
    void commandFinished(int id, bool error);
    void commandStarted(int id);
    void readyRead();
    void stateChanged(QObexFtpClient::State state);

private:
    friend class QObexFtpClientPrivate;
    QObexFtpClientPrivate *m_data;
    Q_DISABLE_COPY(QObexFtpClient)
};

#endif
