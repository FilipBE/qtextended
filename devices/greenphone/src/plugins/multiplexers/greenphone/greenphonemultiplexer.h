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

#ifndef GREENPHONEMUTLIPLEXER_H
#define GREENPHONEMUTLIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>
#include <qgsm0710multiplexer.h>

class QTimer;
class GreenphoneMultiplexer;

class GreenphoneMultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    GreenphoneMultiplexerPlugin( QObject* parent = 0 );
    ~GreenphoneMultiplexerPlugin();

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

class QGreenphoneWakeupSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
    friend class GreenphoneMultiplexer;
public:
    QGreenphoneWakeupSerialIODevice
        ( QSerialIODevice *device, QObject *parent = 0 );
    ~QGreenphoneWakeupSerialIODevice();

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    int rate() const;
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

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void sleep();

    void haveReady();
    void received(const QString &message, const QByteArray &data);

private:
    QSerialIODevice *device;
    int wakeupFd;
    bool modemAsleep;
    QTimer *sleepTimer;
    GreenphoneMultiplexer *mux;

    void wakeup(bool delay = false);
    void forceWakeup();
};

class GreenphoneMultiplexer : public QGsm0710Multiplexer
{
    Q_OBJECT
public:
    GreenphoneMultiplexer( QGreenphoneWakeupSerialIODevice *device,
                           int frameSize = 31, bool advanced = false,
                           QObject *parent = 0 );
    ~GreenphoneMultiplexer();

    void callReinit() { reinit(); }
};

#endif
