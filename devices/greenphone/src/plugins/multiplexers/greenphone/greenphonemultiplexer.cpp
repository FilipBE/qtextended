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

#include "greenphonemultiplexer.h"
#include <qatchat.h>
#include <qtopialog.h>

#include <QtopiaChannel>
#include <QTimer>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

QTOPIA_EXPORT_PLUGIN( GreenphoneMultiplexerPlugin )

// Size of GSM 07.10 frames to use with the multiplexer.
#ifndef GREENPHONE_FRAME_SIZE
#define GREENPHONE_FRAME_SIZE       31
#endif

// Number of seconds until the modem goes to sleep if data has
// not been sent to it in the meantime.
#ifndef GREENPHONE_MODEM_SLEEP
#define GREENPHONE_MODEM_SLEEP      30
#endif

GreenphoneMultiplexerPlugin::GreenphoneMultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
}

GreenphoneMultiplexerPlugin::~GreenphoneMultiplexerPlugin()
{
}

bool GreenphoneMultiplexerPlugin::detect( QSerialIODevice *device )
{
    int wakeupFd = ::open("/dev/omega_bcm2121", O_RDWR);
    if ( wakeupFd < 0 ) {
        ::perror("open /dev/omega_bcm2121");
    } else {
        // Power on the modem. If it is already on it will be reset.
        ::ioctl(wakeupFd, 0x5402, 0);
        usleep(250000);
        ::ioctl(wakeupFd, 0x5401, 1);
        usleep(1000);
        ::ioctl(wakeupFd, 0x5400, 1);
        ::close(wakeupFd);

        // We cannot tell when the modem has finished powering up.
        // If we don't wait long enough the cmuxChat call below will
        // fail and Qtopia will end up using the default GSM multiplexer
        // without power management features of this plug-in.
        usleep(500000);
    }

    // Issue the AT+CMUX command to determine if this device
    // uses Greenphone-style multiplexing.
    return QGsm0710Multiplexer::cmuxChat( device, GREENPHONE_FRAME_SIZE );
}

QSerialIODeviceMultiplexer *GreenphoneMultiplexerPlugin::create
        ( QSerialIODevice *device )
{
    return new GreenphoneMultiplexer( new QGreenphoneWakeupSerialIODevice(device, this), GREENPHONE_FRAME_SIZE );
}

QGreenphoneWakeupSerialIODevice::QGreenphoneWakeupSerialIODevice
        ( QSerialIODevice *device, QObject *parent )
    : QSerialIODevice( parent ), wakeupFd(-1)
{
    this->device = device;
    this->mux = 0;

    // Pass through signals from the underlying device.
    connect( device, SIGNAL(readyRead()), this, SLOT(haveReady()) );
    connect( device, SIGNAL(dsrChanged(bool)), this, SIGNAL(dsrChanged(bool)) );
    connect( device, SIGNAL(ctsChanged(bool)), this, SIGNAL(ctsChanged(bool)) );
    connect( device, SIGNAL(carrierChanged(bool)),
             this, SIGNAL(carrierChanged(bool)) );

    // If the device is open, then we need to be as well.
    if ( device->isOpen() )
    {
        wakeupFd = ::open("/dev/omega_bcm2121", O_RDWR);
        if ( wakeupFd < 0 ) {
            ::perror( "open /dev/omega_bcm2121" );
        } else { 
            // Power on modem then immediately sleep. The modem needs
            // a positive edge on the control line to trigger a wakeup.
            // The first write will generate the positive edge.
            ::ioctl(wakeupFd, 0x5404, 0); // power on modem, only if it is off
            sleep();
        }

        setOpenMode( ReadWrite | Unbuffered );
    }

    // Create a timer that will put the modem to sleep if we don't
    // read or write any data.
    sleepTimer = new QTimer( this );
    sleepTimer->setSingleShot( true );
    connect( sleepTimer, SIGNAL(timeout()), this, SLOT(sleep()) );

    connect( new QtopiaChannel( "QPE/GreenphoneModem", this ),
             SIGNAL(received(QString,QByteArray)),
             this, SLOT(received(QString,QByteArray)) );
}

QGreenphoneWakeupSerialIODevice::~QGreenphoneWakeupSerialIODevice()
{
    delete device;
}

bool QGreenphoneWakeupSerialIODevice::open( OpenMode mode )
{
    if ( device->open( mode ) ) {
        wakeupFd = ::open("/dev/omega_bcm2121", O_RDWR);
        if ( wakeupFd < 0 )
            ::perror( "open /dev/omega_bcm2121" );

        setOpenMode( mode | Unbuffered );
        return true;
    } else {
        return false;
    }
}

void QGreenphoneWakeupSerialIODevice::close()
{
    device->close();

    if ( wakeupFd >= 0 ) {
        sleep();
        ::ioctl( wakeupFd, 0x5403, 1 ); // power down modem
        ::close( wakeupFd );
    }

    setOpenMode( NotOpen );
}

bool QGreenphoneWakeupSerialIODevice::waitForReadyRead(int msecs)
{
    return device->waitForReadyRead( msecs );
}

qint64 QGreenphoneWakeupSerialIODevice::bytesAvailable() const
{
    return device->bytesAvailable();
}

int QGreenphoneWakeupSerialIODevice::rate() const
{
    return device->rate();
}

bool QGreenphoneWakeupSerialIODevice::dtr() const
{
    return device->dtr();
}

void QGreenphoneWakeupSerialIODevice::setDtr( bool value )
{
    device->setDtr( value );
}

bool QGreenphoneWakeupSerialIODevice::dsr() const
{
    return device->dsr();
}

bool QGreenphoneWakeupSerialIODevice::carrier() const
{
    return device->carrier();
}

bool QGreenphoneWakeupSerialIODevice::setCarrier( bool value )
{
    return device->setCarrier( value );
}

bool QGreenphoneWakeupSerialIODevice::rts() const
{
    return device->rts();
}

void QGreenphoneWakeupSerialIODevice::setRts( bool value )
{
    device->setRts( value );
}

bool QGreenphoneWakeupSerialIODevice::cts() const
{
    return device->cts();
}

void QGreenphoneWakeupSerialIODevice::discard()
{
    device->discard();
}

bool QGreenphoneWakeupSerialIODevice::isValid() const
{
    return device->isValid();
}

qint64 QGreenphoneWakeupSerialIODevice::readData( char *data, qint64 maxlen )
{
    qint64 len = device->read( data, maxlen );
    if ( len >= 10 && data[0] == 0x0D && data[1] == 0x0A &&
         data[2] == '*' && data[3] == 'M' && data[4] == 'R' &&
         data[5] == 'D' && data[6] == 'Y' && data[7] == ':' &&
         data[8] == ' ' && data[9] == '1' ) {

        // The modem has dropped out of multiplexing mode and sent us
        // "*MRDY: 1" to tell us it is ready again.  Re-initialize it.
        qLog(Modem) << "re-initializing multiplexing mode";
        if ( mux )
            mux->callReinit();

        // Discard the bogus data that was sent outside multiplexing mode.
        len = 0;

    }
    return len;
}

qint64 QGreenphoneWakeupSerialIODevice::writeData( const char *data, qint64 len )
{
    // Make sure that the modem is awake.
    wakeup(true);

    // Now send the data.
    return device->write( data, len );
}

void QGreenphoneWakeupSerialIODevice::sleep()
{
    if ( wakeupFd >= 0 ) {
        ::ioctl( wakeupFd, 0x5401, 1 );
        modemAsleep = true;
    }
}

void QGreenphoneWakeupSerialIODevice::wakeup(bool delay)
{
    if ( wakeupFd >= 0) {
        sleepTimer->stop();
        if (modemAsleep) {
            ::ioctl( wakeupFd, 0x5400, 1);
            modemAsleep = false;
            if (delay)
                usleep(10000);
        }
        sleepTimer->start( GREENPHONE_MODEM_SLEEP * 1000 );
    }
}

void QGreenphoneWakeupSerialIODevice::forceWakeup()
{
    if ( wakeupFd >= 0 ) {
        sleepTimer->stop();
        ::ioctl( wakeupFd, 0x5401, 1 );
        usleep(1000);
        ::ioctl( wakeupFd, 0x5400, 1 );
        modemAsleep = false;
        sleepTimer->start( GREENPHONE_MODEM_SLEEP * 1000 );
    }
}

GreenphoneMultiplexer::GreenphoneMultiplexer
        ( QGreenphoneWakeupSerialIODevice *device,
          int frameSize, bool advanced, QObject *parent )
    : QGsm0710Multiplexer( device, frameSize, advanced, parent )
{
    device->mux = this;
}

GreenphoneMultiplexer::~GreenphoneMultiplexer()
{
}

void QGreenphoneWakeupSerialIODevice::haveReady()
{
    wakeup();
    internalReadyRead();
}

void QGreenphoneWakeupSerialIODevice::received(const QString &message, const QByteArray &data)
{
    QDataStream in(data);
    if (message == "sleep()") {
        sleep();
    } else if (message == "wakeup()") {
        wakeup();
    } else if (message == "forceWakeup()") {
        forceWakeup();
    }
}

