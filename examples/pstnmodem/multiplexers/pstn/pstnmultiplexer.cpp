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

#include "pstnmultiplexer.h"
#include <qatchat.h>
#include <qtimer.h>
#include <qtopialog.h>
#include <stdio.h>

/*

This multiplexer plugin understands embedded DLE sequences according
to ITU V.253 and converts them into regular AT-like commands and
notifications.  This is intended for use with QAtChat when accessing
PSTN-capable Data/Fax/Voice modems.

If the modem vendor plugin sends a "AT+DLE=x" command to the multiplexer,
it will convert it into "<DLE>x" and respond with OK.  When an embedded
"<DLE>y" sequence is received by the multiplexer, it will convert it
into an unsolicited "+DLE: y" sequence for the modem vendor plugin.

If the DLE sequence is longer than one character, the extended form of
DLE encoding will be used: "<DLE>Xabc<DLE>." where "abc" is the message.

*/

QTOPIA_EXPORT_PLUGIN( PstnMultiplexerPlugin )

PstnMultiplexerPlugin::PstnMultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
    rockwell = false;
}

PstnMultiplexerPlugin::~PstnMultiplexerPlugin()
{
}

bool PstnMultiplexerPlugin::detect( QSerialIODevice *device )
{
    // Try the AT+FCLASS=8 command from V.253 to turn on voice command mode.
    if ( QSerialIODeviceMultiplexer::chat( device, "AT+FCLASS=8" ) ) {
	rockwell = false;
	return true;
    }

    // Rockwell modems may use AT#CLS=8 to turn on voice command mode.
    if ( QSerialIODeviceMultiplexer::chat( device, "AT#CLS=8" ) ) {
	rockwell = true;
	return true;
    }
    return false;
}

QSerialIODeviceMultiplexer *PstnMultiplexerPlugin::create( QSerialIODevice *device )
{
    if ( rockwell )
	return new PstnMultiplexer( device, "AT#CLS=0", "AT#CLS=8" );
    else
	return new PstnMultiplexer( device, "AT+FCLASS=0", "AT+FCLASS=8" );
}

class PstnMultiplexerPrivate
{
public:
    PstnMultiplexerPrivate()
    {
	used = 0;
    }
    ~PstnMultiplexerPrivate()
    {
        delete device;
    }

    QSerialIODevice *device;
    PstnCommandChannel *command;
    PstnDataChannel *data;
    char buffer[4096];
    int used;
    QString switchOut;
    QString switchIn;
};

PstnMultiplexer::PstnMultiplexer
            ( QSerialIODevice *device, const QString& switchOut,
	      const QString& switchIn, QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new PstnMultiplexerPrivate();
    d->device = device;
    d->command = new PstnCommandChannel( device, this );
    d->data = new PstnDataChannel( device, this );
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );
    connect( d->data, SIGNAL(opened()), this, SLOT(dataOpened()) );
    connect( d->data, SIGNAL(closed()), this, SLOT(dataClosed()) );
    d->switchOut = switchOut;
    d->switchIn = switchIn;
}

PstnMultiplexer::~PstnMultiplexer()
{
    delete d;
}

QSerialIODevice *PstnMultiplexer::channel( const QString& name )
{
    if ( name == "primary" || name == "secondary" )
        return d->command;
    else if ( name == "data" || name == "datasetup" )
        return d->data;
    else
        return 0;
}

void PstnMultiplexer::incoming()
{
    // Ignore the incoming bytes if the data channel is handling them.
    if ( !d->command->enabled )
	return;

    // Read more data from the serial device.
    int len = d->device->read
        ( d->buffer + d->used, sizeof(d->buffer) - d->used );
    if ( len <= 0 )
        return;

    // Log the new data to the debug stream.
    if ( qLogEnabled(Mux) ) {
        qLog(Mux) << "PstnMultiplexer::incoming()";
        for ( int i = d->used; i < d->used + len; ++i ) {
            if( i >= 16 && (i % 16) == 0 )
                fprintf(stdout, "\n");
            fprintf(stdout, "%02x ", d->buffer[i] & 0xFF);
        }
        fprintf(stdout, "\n");
    }

    // Update the buffer size.
    d->used += len;

    // Extract DLE sequences from the incoming data stream.
    int posn = 0;
    while ( posn < d->used ) {
	char *ptr = (char *)memchr( d->buffer + posn, '\020', d->used - posn );
	if ( !ptr ) {
	    // No DLE sequence in the remainder of the buffer.
	    d->command->add( d->buffer + posn, d->used - posn );
	    posn = d->used;
	} else if ( d->buffer[posn] != '\020' ) {
	    // Pass through characters up until the next DLE sequence.
	    len = (int)(ptr - (d->buffer + posn));
	    d->command->add( d->buffer + posn, len );
	    posn += len;
	} else if ( posn == (d->used - 1) ) {
	    // DLE without a following character: wait for more data.
	    break;
	} else if ( d->buffer[posn + 1] == '\020' ) {
	    // Two DLE's in a row are converted into a single raw DLE.
	    d->command->add( "\020", 1 );
	    posn += 2;
	} else if ( d->buffer[posn + 1] == '\032' ) {
	    // DLE followed by a SUB corresponds to two raw DLE's.
	    d->command->add( "\020\020", 2 );
	    posn += 2;
	} else if ( d->buffer[posn + 1] == 'X' ) {
	    // Extended DLE sequence.  Search for the "<DLE>." at the end.
	    len = 2;
	    while ( ( posn + len + 2 ) <= d->used ) {
		if ( d->buffer[posn + len] == '\020' &&
		     d->buffer[posn + len + 1] == '.' ) {
		    break;
		} else {
		    ++len;
		}
	    }
	    if ( ( posn + len + 2 ) > d->used ) {
		// The sequence is incomplete, so wait for more data.
		break;
	    }
	    d->command->add( "\r\n+DLE: ", 8 );
	    d->command->add( d->buffer + posn + 2, len - 2 );
	    d->command->add( "\r\n", 2 );
	    posn += len + 2;
	} else {
	    // One-byte DLE sequence.  Turn it into a "+DLE:" notification.
	    d->command->add( "\r\n+DLE: ", 8 );
	    d->command->add( d->buffer + posn + 1, 1 );
	    d->command->add( "\r\n", 2 );
	    posn += 2;
	}
    }
    if ( posn < d->used ) {
        memmove( d->buffer, d->buffer + posn, d->used - posn );
        d->used -= posn;
    } else {
        d->used = 0;
    }

    // If we decoded some data, then emit the readyRead() signal.
    d->command->emitReadyRead();
}

void PstnMultiplexer::dataOpened()
{
    // Allow the data channel to take over the modem.
    d->command->setEnabled(false);
    d->data->setEnabled(true);

    // Switch out of class 8 voice mode.
    QSerialIODeviceMultiplexer::chat( d->device, d->switchOut );
}

void PstnMultiplexer::dataClosed()
{
    // Turn off the data channel and re-enable the command channel.
    d->data->setEnabled(false);
    d->command->setEnabled(true);

    // Switch back into class 8 voice mode.
    QSerialIODeviceMultiplexer::chat( d->device, d->switchIn );
}

PstnCommandChannel::PstnCommandChannel
            ( QSerialIODevice *device, QObject *parent )
    : QSerialIODevice( parent )
{
    this->device = device;
    this->used = 0;
    this->waitingForReadyRead = false;
    this->enabled = true;
}

PstnCommandChannel::~PstnCommandChannel()
{
}

bool PstnCommandChannel::open( OpenMode mode )
{
    setOpenMode( ( mode & ReadWrite ) | Unbuffered );
    return true;
}

void PstnCommandChannel::close()
{
    setOpenMode( NotOpen );
    used = 0;
}

qint64 PstnCommandChannel::bytesAvailable() const
{
    return used;
}

bool PstnCommandChannel::waitForReadyRead( int msecs )
{
    // Wait for the underlying device to have data and process it.
    // We must loop around because we could see data for other channels.
    if ( used > 0 ) {
        // We already have data ready to be read.
        return true;
    }
    if ( !enabled ) {
	return false;
    }
    waitingForReadyRead = true;
    while ( device->waitForReadyRead( msecs ) ) {
        ((PstnMultiplexer *)parent())->incoming();
        if ( used > 0 ) {
            waitingForReadyRead = false;
            return true;
        }
    }
    waitingForReadyRead = false;
    return false;
}

bool PstnCommandChannel::dtr() const
{
    if ( enabled )
	return device->dtr();
    else
	return true;
}

void PstnCommandChannel::setDtr( bool value )
{
    if ( enabled )
	device->setDtr( value );
}

bool PstnCommandChannel::dsr() const
{
    if ( enabled )
	return device->dsr();
    else
	return true;
}

bool PstnCommandChannel::carrier() const
{
    if ( enabled )
	return device->carrier();
    else
	return true;
}

bool PstnCommandChannel::rts() const
{
    if ( enabled )
	return device->rts();
    else
	return true;
}

void PstnCommandChannel::setRts( bool value )
{
    if ( enabled )
	device->setRts( value );
}

bool PstnCommandChannel::cts() const
{
    if ( enabled )
	return device->cts();
    else
	return true;
}

void PstnCommandChannel::discard()
{
    used = 0;
}

void PstnCommandChannel::add( const char *data, int len )
{
    if ( !isOpen() ) {
        // The upper layers haven't opened for this channel yet,
        // so we discard any input.  This prevents unused channels
        // from increasing their buffer sizes indefinitely to
        // cache data that will never be read.
        return;
    }
    if ( ( used + len ) > buffer.size() ) {
        buffer.resize( used + len );
    }
    memcpy( buffer.data() + used, data, len );
    used += len;
}

void PstnCommandChannel::emitReadyRead()
{
    if ( used > 0 && !waitingForReadyRead ) {
        internalReadyRead();
    }
}

qint64 PstnCommandChannel::readData( char *data, qint64 maxlen )
{
    int size;
    if ( used == 0 ) {
        size = 0;
    } else if ( maxlen >= (qint64)used ) {
        memcpy( data, buffer.data(), used );
        size = used;
        used = 0;
    } else {
        memcpy( data, buffer.data(), (int)maxlen );
        memmove( buffer.data(), buffer.data() + (int)maxlen,
                 used - (int)maxlen );
        used -= (int)maxlen;
        size = (int)maxlen;
    }
    return size;
}

qint64 PstnCommandChannel::writeData( const char *data, qint64 len )
{
    if ( !enabled )
	return len;	// Discard all AT commands while data channel is open.
    if (len >= 8 && !memcmp( data, "AT+DLE=", 7 ) &&
        data[((int)len) - 1] == '\r') {
	// This is a special AT command that should be turned
	// into a DLE sequence.
	if ( len == 9 ) {
	    // Send a single-character DLE sequence.
	    device->write( "\020", 1);
	    device->write( data + 7, 1 );
	} else {
	    // Send an extended DLE sequence.
	    device->write( "\020X", 2);
	    device->write( data + 7, (int)(len - 8) );
	    device->write( "\020.", 2);
	}

	// Fake an "OK" in response to the AT+DLE command so that the
	// QAtChat layer can proceed to the next command in the queue.
	add( "OK\r\n", 4 );
	QTimer::singleShot( 0, this, SLOT(emitReadyRead()) );
	return len;
    } else {
	// Copy the data to the underlying device directly.
	return device->write( data, len );
    }
}

PstnDataChannel::PstnDataChannel
	( QSerialIODevice *device, QObject *parent )
    : QSerialIODevice( parent )
{
    this->device = device;
    this->enabled = false;
    connect( device, SIGNAL(readyRead()), this, SLOT(deviceReadyRead()) );
}

PstnDataChannel::~PstnDataChannel()
{
}

bool PstnDataChannel::open( OpenMode mode )
{
    setOpenMode( mode | QIODevice::Unbuffered );
    emit opened();
    return true;
}

void PstnDataChannel::close()
{
    setOpenMode( NotOpen );
    emit closed();
}

qint64 PstnDataChannel::bytesAvailable() const
{
    if ( enabled )
	return device->bytesAvailable();
    else
	return 0;
}

bool PstnDataChannel::waitForReadyRead( int msecs )
{
    if ( enabled )
	return device->waitForReadyRead( msecs );
    else
	return false;
}

bool PstnDataChannel::dtr() const
{
    if ( enabled )
	return device->dtr();
    else
	return true;
}

void PstnDataChannel::setDtr( bool value )
{
    if ( enabled )
	device->setDtr( value );
}

bool PstnDataChannel::dsr() const
{
    if ( enabled )
	return device->dsr();
    else
	return true;
}

bool PstnDataChannel::carrier() const
{
    if ( enabled )
	return device->carrier();
    else
	return true;
}

bool PstnDataChannel::rts() const
{
    if ( enabled )
	return device->rts();
    else
	return true;
}

void PstnDataChannel::setRts( bool value )
{
    if ( enabled )
	device->setRts( value );
}

bool PstnDataChannel::cts() const
{
    if ( enabled )
	return device->cts();
    else
	return true;
}

void PstnDataChannel::abortDial()
{
    if ( enabled )
	device->abortDial();
}

void PstnDataChannel::discard()
{
    if ( enabled )
	device->discard();
}

QProcess *PstnDataChannel::run
    ( const QStringList& arguments, bool addPPPdOptions )
{
    if ( enabled )
	return device->run( arguments, addPPPdOptions );
    else
	return 0;
}

void PstnDataChannel::deviceReadyRead()
{
    if ( enabled )
	internalReadyRead();
}

qint64 PstnDataChannel::readData( char *data, qint64 maxlen )
{
    if ( enabled )
	return device->read( data, maxlen );
    else
	return 0;
}

qint64 PstnDataChannel::writeData( const char *data, qint64 len )
{
    if ( enabled )
	return device->write( data, len );
    else
	return len;
}
