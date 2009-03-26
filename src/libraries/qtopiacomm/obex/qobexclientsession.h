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

#ifndef QOBEXCLIENTSESSION_H
#define QOBEXCLIENTSESSION_H

#include <qobexnamespace.h>
#include <qobexheader.h>
#include <qobexglobal.h>

#include <QByteArray>

class QIODevice;
class QObexClientSessionPrivate;
class QObexAuthenticationChallenge;
class QObexAuthenticationResponse;

class QTOPIAOBEX_EXPORT QObexClientSession : public QObject
{
    friend class QObexClientSessionPrivate;
    Q_OBJECT

public:
    enum Error {
        NoError,
        ConnectionError,
        RequestFailed,
        InvalidRequest,
        InvalidResponse,
        Aborted,
        AuthenticationFailed,
        UnknownError = 100
    };

    explicit QObexClientSession(QIODevice *device, QObject *parent = 0);
    virtual ~QObexClientSession();

    QIODevice *sessionDevice() const;

    int connect(const QObexHeader &header = QObexHeader());
    int disconnect(const QObexHeader &header = QObexHeader());
    int put(const QObexHeader &header, QIODevice *dev);
    int put(const QObexHeader &header, const QByteArray &data);
    int putDelete(const QObexHeader &header);
    int get(const QObexHeader &header, QIODevice *dev = 0);
    int setPath(const QObexHeader &header, QObex::SetPathFlags flags = 0);

    int currentId() const;
    QObex::Request currentRequest() const;
    QIODevice *currentDevice() const;

    void clearPendingRequests();
    bool hasPendingRequests() const;

    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();
    qint64 bytesAvailable() const;

    QObex::ResponseCode lastResponseCode() const;
    QObexHeader lastResponseHeader() const;

    quint32 connectionId() const;
    bool hasConnectionId() const;

    Error error() const;
    QString errorString() const;

public slots:
    void abort();

signals:
    void requestStarted(int id);
    void requestFinished(int id, bool error);

    void responseHeaderReceived(const QObexHeader &header);
    void dataTransferProgress(qint64 done, qint64 total);
    void readyRead();

    void done(bool error);

    void authenticationRequired(QObexAuthenticationChallenge *challenge);
    void authenticationResponse(const QObexAuthenticationResponse &response, bool *accept);

private:
    QObexClientSessionPrivate *m_data;
    Q_DISABLE_COPY(QObexClientSession)
};

#endif
