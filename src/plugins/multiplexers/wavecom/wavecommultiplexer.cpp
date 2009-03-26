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

#include "wavecommultiplexer.h"
#include <qatchat.h>
#include <qtopialog.h>
#include <alloca.h>
#include <stdio.h>

#define Wavecom_DTR     0x80
#define Wavecom_RTS     0x40
#define Wavecom_DSR     0x80
#define Wavecom_DCD     0x40
#define Wavecom_CTS     0x20

QTOPIA_EXPORT_PLUGIN( WavecomMultiplexerPlugin )

WavecomMultiplexerPlugin::WavecomMultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
}

WavecomMultiplexerPlugin::~WavecomMultiplexerPlugin()
{
}

bool WavecomMultiplexerPlugin::detect( QSerialIODevice *device )
{
    // Issue the AT+WMUX=1 command to determine if this device
    // uses Wavecom-style multiplexing.
    return QSerialIODeviceMultiplexer::chat( device, "AT+WMUX=1" );
}

QSerialIODeviceMultiplexer *WavecomMultiplexerPlugin::create( QSerialIODevice *device )
{
    return new WavecomMultiplexer( device );
}

class WavecomMultiplexerPrivate
{
public:
    WavecomMultiplexerPrivate()
    {
        used = 0;
    }
    ~WavecomMultiplexerPrivate()
    {
        delete device;
    }

    QSerialIODevice *device;
    WavecomCommandChannel *command;
    WavecomDataChannel *data;
    char buffer[4096];
    uint used;
};

WavecomMultiplexer::WavecomMultiplexer
            ( QSerialIODevice *device, QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new WavecomMultiplexerPrivate();
    d->device = device;
    d->command = new WavecomCommandChannel( device, this );
    d->data = new WavecomDataChannel( device, this );
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );
}

WavecomMultiplexer::~WavecomMultiplexer()
{
    delete d;
}

QSerialIODevice *WavecomMultiplexer::channel( const QString& name )
{
    if ( name == "primary" || name == "secondary" || name == "datasetup" )
        return d->command;
    else if ( name == "data" )
        return d->data;
    else
        return 0;
}

void WavecomMultiplexer::incoming()
{
    // Read more data from the serial device.
    int len = d->device->read
        ( d->buffer + d->used, sizeof(d->buffer) - d->used );
    if ( len <= 0 )
        return;

    // Log the new data to the debug stream.
    if ( qLogEnabled(Mux) ) {
        qLog(Mux) << "WavecomMultiplexer::incoming()";
        for ( uint i = d->used; i < d->used + (uint)len; ++i ) {
            if( i >= 16 && (i % 16) == 0 )
                fprintf(stdout, "\n");
            fprintf(stdout, "%02x ", d->buffer[i] & 0xFF);
        }
        fprintf(stdout, "\n");
    }

    // Update the buffer size.
    d->used += (uint)len;

    // Break the incoming data up into packets.
    uint posn = 0;
    int sum, temp;
    while ( posn < d->used ) {
        if ( d->buffer[posn] == (char)0xAA ||
             d->buffer[posn] == (char)0xDD ) {

            // We need at least 3 bytes for the header.
            if ( ( posn + 3 ) > d->used )
                break;

            // Get the packet length and validate it.
            len = (d->buffer[posn + 1] & 0xFF) |
                   ((d->buffer[posn + 2] & 0x07) << 8);
            if ( ( posn + 4 + len ) > d->used )
                break;

            // Verify the packet checksum.
            temp = len + 3;
            sum = 0;
            while ( temp > 0 ) {
                --temp;
                sum += d->buffer[posn + temp];
            }
            if ( ( sum & 0xFF ) != ( d->buffer[posn + len + 3] & 0xFF ) ) {
                qLog(Mux) << "*** wavecom checksum check failed ***";
                posn += len + 4;
                continue;
            }

            // Determine what type of packet we have.
            temp = ((d->buffer[posn + 2] & 0xF8) >> 3);
            if ( d->buffer[posn] == (char)0xAA && temp == 0x1D ) {

                // Packet on the command channel.
                d->command->add( d->buffer + posn + 3, len );

            } else if ( d->buffer[posn] == (char)0xDD && temp == 0x00 ) {

                // Packet on the data channel.
                d->data->add( d->buffer + posn + 3, len );

            } else if ( d->buffer[posn] == (char)0xDD && temp == 0x02 ) {

                // Reset indication after data connection establishment.
                // We send a status command to set DTR and RTS.
                static char const set_status[] = "\xDD\x01\x08\xC0\xA6";
                d->device->write( set_status, 5 );

                // The data channel is ready to accept data.
                d->data->emitReady();

            } else if ( d->buffer[posn] == (char)0xDD && temp == 0x01 ) {

                // Status sent for the data channel.
                if ( len > 0 ) {
                    d->data->setStatus( d->buffer[posn + 3] & 0xFF );
                }

            } else if ( d->buffer[posn] == (char)0xDD && temp == 0x03 ) {

                // "Busy" indication on the data channel.  This is
                // sent when the data channel shuts down.  We treat
                // it the same as a carrier drop.
                d->data->busy();

            } else {

                // Unknown packet.
                qLog(Mux) << "*** unknown wavecom packet ***";

            }
            posn += len + 4;

        } else {
            // The connection has probably dropped out of the
            // multiplexing mode due to "AT+CFUN=1".  Pass spurious
            // characters to the command connection until we can
            // re-sync and get back into the multiplexing mode.
            len = 1;
            while ( ( posn + len ) < d->used &&
                    d->buffer[posn + len] != (char)0xAA &&
                    d->buffer[posn + len] != (char)0xDD ) {
                ++len;
            }
            d->command->add( d->buffer + posn, len );
            posn += len;
        }
    }
    if ( posn < d->used ) {
        memmove( d->buffer, d->buffer + posn, d->used - posn );
        d->used -= posn;
    } else {
        d->used = 0;
    }
}

WavecomCommandChannel::WavecomCommandChannel
            ( QSerialIODevice *device, QObject *parent )
    : QSerialIODevice( parent )
{
    this->device = device;
    this->used = 0;
    this->needRestart = false;
    this->waitingForReadyRead = false;
}

WavecomCommandChannel::~WavecomCommandChannel()
{
}

bool WavecomCommandChannel::open( OpenMode mode )
{
    setOpenMode( ( mode & ReadWrite ) | Unbuffered );
    return true;
}

void WavecomCommandChannel::close()
{
    setOpenMode( NotOpen );
    used = 0;
}

qint64 WavecomCommandChannel::bytesAvailable() const
{
    return used;
}

bool WavecomCommandChannel::waitForReadyRead( int msecs )
{
    // Wait for the underlying device to have data and process it.
    // We must loop around because we could see data for other channels.
    if ( used > 0 ) {
        // We already have data ready to be read.
        return true;
    }
    waitingForReadyRead = true;
    while ( device->waitForReadyRead( msecs ) ) {
        ((WavecomMultiplexer *)parent())->incoming();
        if ( used > 0 ) {
            waitingForReadyRead = false;
            return true;
        }
    }
    waitingForReadyRead = false;
    return false;
}

bool WavecomCommandChannel::dtr() const
{
    // Command channel is always up.
    return true;
}

void WavecomCommandChannel::setDtr( bool /*value*/ )
{
    // Command channel is always up.
}

bool WavecomCommandChannel::dsr() const
{
    // Command channel is always up.
    return true;
}

bool WavecomCommandChannel::carrier() const
{
    // Command channel is always up.
    return true;
}

bool WavecomCommandChannel::rts() const
{
    // Command channel is always up.
    return true;
}

void WavecomCommandChannel::setRts( bool /*value*/ )
{
    // Command channel is always up.
}

bool WavecomCommandChannel::cts() const
{
    // Command channel is always up.
    return true;
}

void WavecomCommandChannel::discard()
{
    used = 0;
}

void WavecomCommandChannel::abortDial()
{
    static char const abortString[] = "ATH\r";
    write( abortString, 4 );
}

QAtChat *WavecomCommandChannel::atchat()
{
    QAtChat *ac = QSerialIODevice::atchat();
    ac->setCPINTerminator();
    return ac;
}

void WavecomCommandChannel::add( const char *data, uint len )
{
    if ( !isOpen() ) {
        // The upper layers haven't opened for this channel yet,
        // so we discard any input.  This prevents unused channels
        // from increasing their buffer sizes indefinitely to
        // cache data that will never be read.
        return;
    }
    if ( ((int)( used + len )) > buffer.size() ) {
        buffer.resize( used + len );
    }
    memcpy( buffer.data() + used, data, len );
    used += len;
    if ( len > 0 && !waitingForReadyRead ) {
        internalReadyRead();
    }
}

qint64 WavecomCommandChannel::readData( char *data, qint64 maxlen )
{
    uint size;
    if ( used == 0 ) {
        size = 0;
    } else if ( maxlen >= (qint64)used ) {
        memcpy( data, buffer.data(), used );
        size = used;
        used = 0;
    } else {
        memcpy( data, buffer.data(), (int)maxlen );
        memmove( buffer.data(), buffer.data() + (uint)maxlen,
                 used - (uint)maxlen );
        used -= (uint)maxlen;
        size = (uint)maxlen;
    }
    return size;
}

qint64 WavecomCommandChannel::writeData( const char *data, qint64 len )
{
    if ( needRestart ) {
        // Re-initialize the framed data mode after an "AT+CFUN" reset.
        QSerialIODeviceMultiplexer::chat( device, "AT+WMUX=1" );
        needRestart = false;
    }
    writeBlock( 0xAA, 0x1D, data, len );
    if ( len >= 9 && !memcmp( data, "AT+CFUN=1", 9 ) ) {
        // AT+CFUN=1 causes us to drop out of the framed data mode.
        // But AT+CFUN=0 (for plane mode) doesn't do this.
        needRestart = true;
    }
    return len;
}

void WavecomCommandChannel::writeInner( int lead, int type, const char *data, uint len )
{
    char *buf = (char *)alloca( len + 4 );
    uint posn;
    int sum;
    buf[0] = (char)lead;
    buf[1] = (char)len;
    buf[2] = (char)((len >> 8) | (type << 3));
    if ( len > 0 )
        memcpy( buf + 3, data, len );
    posn = len + 3;
    sum = 0;
    while ( posn > 0 ) {
        --posn;
        sum += buf[posn];
    }
    buf[len + 3] = (char)sum;
    device->write( buf, len + 4 );
}

void WavecomCommandChannel::writeBlock( int lead, int type, const char *data, uint len )
{
    uint start = len;
    while ( len >= 1024 ) {
        writeInner( lead, type, data, 1024 );
        data += 1024;
        len -= 1024;
    }
    if ( len > 0 || start == 0 ) {
        writeInner( lead, type, data, len );
    }
}

WavecomDataChannel::WavecomDataChannel
            ( QSerialIODevice *device, QObject *parent )
    : WavecomCommandChannel( device, parent )
{
    incomingStatus = Wavecom_DSR | Wavecom_CTS;
    outgoingStatus = Wavecom_DTR | Wavecom_RTS;
    updateStatus();
}

WavecomDataChannel::~WavecomDataChannel()
{
}

bool WavecomDataChannel::dtr() const
{
    return ( ( outgoingStatus & Wavecom_DTR ) != 0 );
}

void WavecomDataChannel::setDtr( bool value )
{
    if ( value != dtr() ) {
        if ( value )
            outgoingStatus |= Wavecom_DTR;
        else
            outgoingStatus &= ~Wavecom_DTR;
        updateStatus();
    }
}

bool WavecomDataChannel::dsr() const
{
    return ( ( incomingStatus & Wavecom_DSR ) != 0 );
}

bool WavecomDataChannel::carrier() const
{
    return ( ( incomingStatus & Wavecom_DCD ) != 0 );
}

bool WavecomDataChannel::rts() const
{
    return ( ( outgoingStatus & Wavecom_RTS ) != 0 );
}

void WavecomDataChannel::setRts( bool value )
{
    if ( value != rts() ) {
        if ( value )
            outgoingStatus |= Wavecom_RTS;
        else
            outgoingStatus &= ~Wavecom_RTS;
        updateStatus();
    }
}

bool WavecomDataChannel::cts() const
{
    return ( ( incomingStatus & Wavecom_CTS ) != 0 );
}

bool WavecomDataChannel::waitForReady() const
{
    return true;
}

void WavecomDataChannel::setStatus( int status )
{
    int lastStatus = incomingStatus;
    status &= ( Wavecom_DSR | Wavecom_CTS | Wavecom_DCD );
    incomingStatus = status;
    if ( ( ( status ^ lastStatus ) & Wavecom_DSR ) != 0 ) {
        emit dsrChanged( ( status & Wavecom_DSR ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & Wavecom_DCD ) != 0 ) {
        emit carrierChanged( ( status & Wavecom_DCD ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & Wavecom_CTS ) != 0 ) {
        emit ctsChanged( ( status & Wavecom_CTS ) != 0 );
    }
}

void WavecomDataChannel::busy()
{
    // The Wavecom "busy" signal is treated like a carrier drop.
    incomingStatus &= ~Wavecom_DCD;
    emit carrierChanged( false );
}

qint64 WavecomDataChannel::writeData( const char *data, qint64 len )
{
    writeBlock( 0xDD, 0, data, len );
    return len;
}

void WavecomDataChannel::updateStatus()
{
    char data[1];
    data[0] = (char)outgoingStatus;
    writeBlock( 0xDD, 1, data, 1 );
}
