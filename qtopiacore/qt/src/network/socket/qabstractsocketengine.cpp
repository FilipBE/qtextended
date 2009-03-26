/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qabstractsocketengine_p.h"
#include "qnativesocketengine_p.h"
#include "qmutex.h"

QT_BEGIN_NAMESPACE

class QSocketEngineHandlerList : public QList<QSocketEngineHandler*>
{
public:
    QMutex mutex;
};

Q_GLOBAL_STATIC(QSocketEngineHandlerList, socketHandlers)

QSocketEngineHandler::QSocketEngineHandler()
{
    if (!socketHandlers())
        return;
    QMutexLocker locker(&socketHandlers()->mutex);
    socketHandlers()->prepend(this);
}

QSocketEngineHandler::~QSocketEngineHandler()
{
    if (!socketHandlers())
        return;
    QMutexLocker locker(&socketHandlers()->mutex);
    socketHandlers()->removeAll(this);
}

QAbstractSocketEnginePrivate::QAbstractSocketEnginePrivate()
    : socketError(QAbstractSocket::UnknownSocketError)
    , hasSetSocketError(false)
    , socketErrorString(QLatin1String(QT_TRANSLATE_NOOP(QSocketLayer, "Unknown error")))
    , socketState(QAbstractSocket::UnconnectedState)
    , socketType(QAbstractSocket::UnknownSocketType)
    , socketProtocol(QAbstractSocket::UnknownNetworkLayerProtocol)
    , localPort(0)
    , peerPort(0)
    , receiver(0)
{
}

QAbstractSocketEngine::QAbstractSocketEngine(QObject *parent)
    : QObject(*new QAbstractSocketEnginePrivate(), parent)
{
}

QAbstractSocketEngine::QAbstractSocketEngine(QAbstractSocketEnginePrivate &dd, QObject* parent)
    : QObject(dd, parent)
{
}

QAbstractSocketEngine *QAbstractSocketEngine::createSocketEngine(const QHostAddress &address, QAbstractSocket::SocketType socketType, QObject *parent)
{
    QMutexLocker locker(&socketHandlers()->mutex);
    for (int i = 0; i < socketHandlers()->size(); i++) {
        if (QAbstractSocketEngine *ret = socketHandlers()->at(i)->createSocketEngine(address, socketType, parent))
            return ret;
    }
    return new QNativeSocketEngine(parent);
}

QAbstractSocketEngine *QAbstractSocketEngine::createSocketEngine(int socketDescripter, QObject *parent)
{
    QMutexLocker locker(&socketHandlers()->mutex);
    for (int i = 0; i < socketHandlers()->size(); i++) {
        if (QAbstractSocketEngine *ret = socketHandlers()->at(i)->createSocketEngine(socketDescripter, parent))
            return ret;
    }
    return new QNativeSocketEngine(parent);
}

QAbstractSocket::SocketError QAbstractSocketEngine::error() const
{
    return d_func()->socketError;
}

QString QAbstractSocketEngine::errorString() const
{
    return d_func()->socketErrorString;
}

void QAbstractSocketEngine::setError(QAbstractSocket::SocketError error, const QString &errorString) const
{
    Q_D(const QAbstractSocketEngine);
    d->socketError = error;
    d->socketErrorString = errorString;
}

void QAbstractSocketEngine::setReceiver(QAbstractSocketEngineReceiver *receiver)
{
    d_func()->receiver = receiver;
}

void QAbstractSocketEngine::readNotification()
{
    if (QAbstractSocketEngineReceiver *receiver = d_func()->receiver)
        receiver->readNotification();
}

void QAbstractSocketEngine::writeNotification()
{
    if (QAbstractSocketEngineReceiver *receiver = d_func()->receiver)
        receiver->writeNotification();
}

void QAbstractSocketEngine::exceptionNotification()
{
    if (QAbstractSocketEngineReceiver *receiver = d_func()->receiver)
        receiver->exceptionNotification();
}

#ifndef QT_NO_NETWORKPROXY
void QAbstractSocketEngine::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
    if (QAbstractSocketEngineReceiver *receiver = d_func()->receiver)
        receiver->proxyAuthenticationRequired(proxy, authenticator);
}
#endif


QAbstractSocket::SocketState QAbstractSocketEngine::state() const
{
    return d_func()->socketState;
}

void QAbstractSocketEngine::setState(QAbstractSocket::SocketState state)
{
    d_func()->socketState = state;
}

QAbstractSocket::SocketType QAbstractSocketEngine::socketType() const
{
    return d_func()->socketType;
}

void QAbstractSocketEngine::setSocketType(QAbstractSocket::SocketType socketType)
{
    d_func()->socketType = socketType;
}

QAbstractSocket::NetworkLayerProtocol QAbstractSocketEngine::protocol() const
{
    return d_func()->socketProtocol;
}

void QAbstractSocketEngine::setProtocol(QAbstractSocket::NetworkLayerProtocol protocol)
{
    d_func()->socketProtocol = protocol;
}

QHostAddress QAbstractSocketEngine::localAddress() const
{
    return d_func()->localAddress;
}

void QAbstractSocketEngine::setLocalAddress(const QHostAddress &address)
{
    d_func()->localAddress = address;
}

quint16 QAbstractSocketEngine::localPort() const
{
    return d_func()->localPort;
}

void QAbstractSocketEngine::setLocalPort(quint16 port)
{
    d_func()->localPort = port;
}

QHostAddress QAbstractSocketEngine::peerAddress() const
{
    return d_func()->peerAddress;
}

void QAbstractSocketEngine::setPeerAddress(const QHostAddress &address)
{
   d_func()->peerAddress = address;
}

quint16 QAbstractSocketEngine::peerPort() const
{
    return d_func()->peerPort;
}

void QAbstractSocketEngine::setPeerPort(quint16 port)
{
    d_func()->peerPort = port;
}

QT_END_NAMESPACE
