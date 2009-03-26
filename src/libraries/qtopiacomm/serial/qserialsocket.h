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

#ifndef QSERIALSOCKET_H
#define QSERIALSOCKET_H

#include <qserialiodevice.h>

class QTcpSocket;
class QTcpServer;

class QSerialSocketPrivate;

class QTOPIACOMM_EXPORT QSerialSocket : public QSerialIODevice
{
    Q_OBJECT
    friend class QSerialSocketServer;
private:
    QSerialSocket( QTcpSocket *socket );
public:
    QSerialSocket( const QString& host, quint16 port, QObject *parent = 0 );
    ~QSerialSocket();

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool setCarrier( bool value );
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    bool isValid() const;

signals:
    void closed();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void socketReadyRead();
    void socketClosed();

private:
    QSerialSocketPrivate *d;

    void init();

    void sendModemSignal( int ch );
    void sendCommand( const char *buf, int len );

    void sendDo( int option );
    void sendDont( int option );
    void sendWill( int option );
    void sendWont( int option );
    void sendSubOption( int option, const char *buf, int len );

    void initTelnet();

    void receiveModemSignal( int ch );

    void receiveDo( int option );
    void receiveDont( int option );
    void receiveWill( int option );
    void receiveWont( int option );
    void receiveSubOption( int option, const char *buf, int len );
};

class QTOPIACOMM_EXPORT QSerialSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit QSerialSocketServer( quint16 port, bool localHostOnly = true,
                                  QObject *parent = 0 );
    ~QSerialSocketServer();

    bool isListening() const;
    quint16 port() const;

signals:
    void incoming( QSerialSocket *socket );

private slots:
    void newConnection();

private:
    QTcpServer *server;
};

#endif
