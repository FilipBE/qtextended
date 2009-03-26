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

#ifndef QOBEXSERVERSESSION_H
#define QOBEXSERVERSESSION_H

#include <qobexnamespace.h>
#include <QObject>

class QIODevice;
class QObexHeader;
class QObexServerSessionPrivate;
class QObexAuthenticationChallenge;
class QObexAuthenticationResponse;

class QTOPIAOBEX_EXPORT QObexServerSession : public QObject
{
    friend class QObexServerSessionPrivate;
    Q_OBJECT

public:
    enum Error {
        ConnectionError = 1,
        InvalidRequest,
        Aborted,
        AuthenticationFailed,
        UnknownError = 100
    };

    explicit QObexServerSession(QIODevice *device, QObject *parent = 0);
    virtual ~QObexServerSession() = 0;

    QIODevice *sessionDevice();
    void close();

signals:
    void finalResponseSent(QObex::Request request);
    void authenticationRequired(QObexAuthenticationChallenge *challenge);
    void authenticationResponse(const QObexAuthenticationResponse &response, bool *accept);

protected:
    void setNextResponseHeader(const QObexHeader &header);
    virtual void error(QObexServerSession::Error error, const QString &errorString);

    virtual QObex::ResponseCode dataAvailable(const char *data, qint64 size);
    virtual QObex::ResponseCode provideData(const char **data, qint64 *size);

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header);
    QObex::ResponseCode disconnect(const QObexHeader &header);
    QObex::ResponseCode put(const QObexHeader &header);
    QObex::ResponseCode putDelete(const QObexHeader &header);
    QObex::ResponseCode get(const QObexHeader &header);
    QObex::ResponseCode setPath(const QObexHeader &header, QObex::SetPathFlags flags);

private:
    QObexServerSessionPrivate *m_data;
    Q_DISABLE_COPY(QObexServerSession)
};

#endif
