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

#ifndef QOBEXSERVERSESSION_P_H
#define QOBEXSERVERSESSION_P_H

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

#include <qobexserversession.h>
#include <qobexheader.h>
#include "qobexsocket_p.h"

#include <QByteArray>
#include <QBuffer>
#include <QSet>
#include <QPointer>

class QObexSocketPrivate;

class QObexServerSessionPrivate : public QObject
{
    friend class QObexServerSession;
    Q_OBJECT

public:
    // ---- Functions accessed by QObexSocket: ----

    QObex::ResponseCode acceptIncomingRequest(QObex::Request request);
    QObex::ResponseCode receivedRequestFirstPacket(QObex::Request request,
                                                   QObexHeader &header,
                                                   QObexHeader *responseHeader);
    QObex::ResponseCode receivedRequest(QObex::Request request,
                                        QObexHeader &requestHeader,
                                        const QByteArray &nonHeaderData,
                                        QObexHeader *responseHeader);
    void requestDone(QObex::Request request);

    QObex::ResponseCode bodyDataAvailable(const char *data, qint64 size);
    QObex::ResponseCode bodyDataRequired(const char **data, qint64 *size);

    void errorOccurred(QObexServerSession::Error error, const QString &errorString);

private:
    // ---- Members accessed by QObexServerSession: ----

    QObexServerSessionPrivate(QIODevice *device, QObexServerSession *parent);
    ~QObexServerSessionPrivate();
    void close();

    void setNextResponseHeader(const QObexHeader &header);

    QPointer<QObexSocket> m_socket;


private slots:
    void socketDisconnected();

private:
    // ---- Internal memebers: ----
    QObex::ResponseCode processRequestHeader(QObex::Request request,
                                             QObexHeader &requestHeader,
                                             const QByteArray &nonHeaderData);
    QObex::ResponseCode processAuthenticationChallenge(QObexAuthenticationChallenge &challenge);
    QObex::ResponseCode readAuthenticationResponse(const QByteArray &responseBytes);
    bool invokeSlot(const QString &methodName,
                    QObex::ResponseCode *responseCode = 0,
                    QGenericArgument arg1 = QGenericArgument(),
                    QGenericArgument arg2 = QGenericArgument());
    void resetOpData();
    QObex::SetPathFlags getSetPathFlags(const QByteArray &nonHeaderData);
    void initAvailableCallbacks();

    static QLatin1String getRequestSlot(QObex::Request request);

    QObexServerSession *m_parent;
    bool m_closed;
	bool m_socketDisconnected;

    QObexHeader m_nextResponseHeader;
    bool m_invokedRequestSlot;

    QSet<QString> m_implementedCallbacks;

    QByteArray m_challengeNonce;
};

#endif
