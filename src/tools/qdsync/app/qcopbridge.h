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
#ifndef QCOPBRIDGE_H
#define QCOPBRIGDE_H

#include <QObject>

class QCopBridgePrivate;
class QCopBridgePI;
class QCopBridgePIPrivate;
class QTcpSocket;
class QIODevice;

class QCopBridge : public QObject
{
    Q_OBJECT
public:
    QCopBridge( QObject *parent = 0 );
    ~QCopBridge();

    bool startTcp( int port = 0 );
    bool startSerial( const QString &port );

signals:
    void gotConnection();
    void lostConnection();

private slots:
    void newTcpConnection();
    void newSerialConnection();
    void startSerialConnection();
    void serialServerDied();
    void desktopMessage( const QString &message, const QByteArray &data );
    void disconnected( QCopBridgePI *pi );

private:
    void newSocket( QIODevice *socket );

    QCopBridgePrivate *d;
};

class QCopBridgePI : public QObject
{
    Q_OBJECT
public:
    QCopBridgePI( QIODevice *socket, QObject *parent = 0 );
    ~QCopBridgePI();

    enum State { WaitUser, WaitPass, Connected };

    void sendDesktopMessage( const QString &channel, const QString &message, const QByteArray &data );
    QIODevice *socket();

signals:
    void disconnected( QCopBridgePI *pi );
    void gotConnection();

private slots:
    void read();
    void socketDisconnected();
    void helperTimeout();
    void killTimerTimeout();

private:
    void process( const QByteArray &line );
    void send( const QByteArray &line, int _line );

    QCopBridgePIPrivate *d;
};

#endif
