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

#ifndef QCOPCHANNEL_X11_P_H
#define QCOPCHANNEL_X11_P_H

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

#include <qobject.h>
#include <qunixsocket_p.h>
#include <qunixsocketserver_p.h>

#if defined(Q_WS_X11)

class QEventLoop;

class QCopX11Client : public QObject
{
    Q_OBJECT
public:
    QCopX11Client();
    QCopX11Client( QIODevice *device, QUnixSocket *socket );
    ~QCopX11Client();

    void registerChannel( const QString& ch );
    void detachChannel( const QString& ch );
    void sendChannelCommand( int cmd, const QString& ch );
    void send( const QString& ch, const QString& msg, const QByteArray& data );
    void forward( const QString& ch, const QString& msg, const QByteArray& data,
                  const QString& forwardTo);
    void isRegisteredReply( const QString& ch, bool known );
    void requestRegistered( const QString& ch );
    void flush();
    bool waitForIsRegistered();

    bool isClient() const { return !server; }
    bool isServer() const { return server; }

    static const int minPacketSize = 256;

private slots:
    void readyRead();
    void disconnected();
    void connectToServer();
    void connectSignals();

private:
    bool server;
    QIODevice *device;
    QUnixSocket *socket;

    void init();

    char outBuffer[minPacketSize];
    char inBuffer[minPacketSize];
    char *inBufferPtr;
    int inBufferUsed;
    int inBufferExpected;
    bool isRegisteredResponse;
    QEventLoop *isRegisteredWaiter;
    QByteArray pendingData;
    int retryCount;
    bool connecting;

    void write( const char *buf, int len );
};

class QCopX11Server : public QUnixSocketServer
{
    Q_OBJECT
public:
    QCopX11Server();
    ~QCopX11Server();

protected:
    void incomingConnection(int socketDescriptor);
};

#endif // Q_WS_X11

#endif
