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

#include <qgsm0710multiplexer.h>
#include <qtopialog.h>
#include <qmap.h>
#include <alloca.h>
#include "gsm0710_p.h"

/*!
    \class QGsm0710Multiplexer
    \inpublicgroup QtBaseModule

    \brief The QGsm0710Multiplexer class provides a multiplexing implementation based on 3GPP TS 07.10/27.010
    \ingroup telephony::serial

    This is the default multiplexer implementation that is tried if a multiplexer
    plug-in override is not available.

    By default, this class uses the 3GPP TS 07.10/27.010 basic multiplexing mode,
    with a frame size of 31 bytes.  The mode and/or frame size can be modified
    by writing a multiplexer plug-in.  See the \l{Tutorial: Writing a Multiplexer Plug-in}
    for more information on how to write a multiplexer plug-in that modifies the
    3GPP TS 07.10/27.010 parameters.

    \sa QSerialIODeviceMultiplexer, QMultiPortMultiplexer, QSerialIODeviceMultiplexerPlugin
*/

class QGsm0710MultiplexerChannel;

class QGsm0710MultiplexerPrivate
{
public:
    QGsm0710MultiplexerPrivate( QSerialIODevice *device,
                                int frameSize, bool advanced,
                                bool server )
    {
        this->device = device;
        this->server = server;
        gsm0710_initialize( &ctx );
        ctx.frame_size = frameSize;
        ctx.mode = ( advanced ? GSM0710_MODE_ADVANCED : GSM0710_MODE_BASIC );
        ctx.port_speed = device->rate();
        ctx.server = (int)server;
        ctx.user_data = (void *)this;
        ctx.at_command = at_command;
        ctx.read = read;
        ctx.write = write;
        ctx.deliver_data = deliver_data;
        ctx.deliver_status = deliver_status;
        ctx.debug_message = debug_message;
        ctx.open_channel = open_channel;
        ctx.close_channel = close_channel;
        ctx.terminate = terminate;
    }
    ~QGsm0710MultiplexerPrivate();

    void closeAll();

    QSerialIODevice *device;
    bool server;
    struct gsm0710_context ctx;
    QMap<int,QGsm0710MultiplexerChannel *> channels;
    QGsm0710Multiplexer *mux;

    static int at_command(struct gsm0710_context *ctx, const char *cmd);
    static int read(struct gsm0710_context *ctx, void *data, int len);
    static int write(struct gsm0710_context *ctx, const void *data, int len);
    static void deliver_data(struct gsm0710_context *ctx, int channel,
                             const void *data, int len);
    static void deliver_status(struct gsm0710_context *ctx,
                               int channel, int status);
    static void debug_message(struct gsm0710_context *ctx, const char *msg);
    static void open_channel(struct gsm0710_context *ctx, int channel);
    static void close_channel(struct gsm0710_context *ctx, int channel);
    static void terminate(struct gsm0710_context *ctx);
};

class QGsm0710MultiplexerChannel : public QSerialIODevice
{
public:
    QGsm0710MultiplexerChannel( QGsm0710Multiplexer *mux, int channel );
    ~QGsm0710MultiplexerChannel();

    int chan() const { return channel; }

    // Override methods from QIODevice.
    bool open( OpenMode mode );
    void close();
    qint64 bytesAvailable() const;
    bool waitForReadyRead( int msecs );

    // Override methods from QSerialIODevice.
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

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

public:
    void add( const char *data, uint len );
    void setStatus( int status );

private:
    void updateStatus();

private:
    QGsm0710Multiplexer *mux;
    int channel;
    char incomingStatus;
    char outgoingStatus;
    bool previouslyOpened;
    bool currentlyOpen;
    bool waitingForReadyRead;
    int used;
    QByteArray buffer;
};

QGsm0710MultiplexerPrivate::~QGsm0710MultiplexerPrivate()
{
    if ( !server )      // Don't delete the device when in server mode.
        delete device;
}

void QGsm0710MultiplexerPrivate::closeAll()
{
    QMap<int,QGsm0710MultiplexerChannel *>::Iterator iter;
    for ( iter = channels.begin(); iter != channels.end(); ++iter ) {
        delete iter.value();
    }
}

#define MUXP(ctx)   ((QGsm0710MultiplexerPrivate *)((ctx)->user_data))

int QGsm0710MultiplexerPrivate::at_command
        (struct gsm0710_context *ctx, const char *cmd)
{
    return QSerialIODeviceMultiplexer::chat( MUXP(ctx)->device, QString(cmd) );
}

int QGsm0710MultiplexerPrivate::read
        (struct gsm0710_context *ctx, void *data, int len)
{
    len = (int)(MUXP(ctx)->device->read( (char *)data, len ));
    if ( qLogEnabled(Mux) ) {
        qLog(Mux) << "QGsm0710MultiplexerPrivate::read()";
        for ( int i = 0; i < len; ++i ) {
            if( i >= 16 && (i % 16) == 0 )
                fprintf(stdout, "\n");
            fprintf(stdout, "%02x ", ((char *)data)[i] & 0xFF);
        }
        fprintf(stdout, "\n");
    }
    return (int)len;
}

int QGsm0710MultiplexerPrivate::write
        (struct gsm0710_context *ctx, const void *data, int len)
{
    if ( qLogEnabled(Mux) ) {
        qLog(Mux) << "QGsm0710MultiplexerPrivate::write()";
        for ( int i = 0; i < len; ++i ) {
            if( i >= 16 && (i % 16) == 0 )
                fprintf(stdout, "\n");
            fprintf(stdout, "%02x ", ((const char *)data)[i] & 0xFF);
        }
        fprintf(stdout, "\n");
    }
    return MUXP(ctx)->device->write( (const char *)data, len );
}

void QGsm0710MultiplexerPrivate::deliver_data
        (struct gsm0710_context *ctx, int channel, const void *data, int len)
{
    QGsm0710MultiplexerPrivate *d = MUXP(ctx);
    if ( d->channels.contains( channel ) ) {
        QGsm0710MultiplexerChannel *chan = d->channels.value( channel );
        chan->add( (const char *)data, len );
    }
}

void QGsm0710MultiplexerPrivate::deliver_status
        (struct gsm0710_context *ctx, int channel, int status)
{
    QGsm0710MultiplexerPrivate *d = MUXP(ctx);
    if ( d->channels.contains( channel ) ) {
        QGsm0710MultiplexerChannel *chan = d->channels.value( channel );
        chan->setStatus( status );
    }
}

void QGsm0710MultiplexerPrivate::debug_message
        (struct gsm0710_context *ctx, const char *msg)
{
    Q_UNUSED(ctx);
    qLog(Mux) << msg;
}

void QGsm0710MultiplexerPrivate::open_channel
        (struct gsm0710_context *ctx, int channel)
{
    MUXP(ctx)->mux->open( channel );
}

void QGsm0710MultiplexerPrivate::close_channel
        (struct gsm0710_context *ctx, int channel)
{
    MUXP(ctx)->mux->close( channel );
}

void QGsm0710MultiplexerPrivate::terminate(struct gsm0710_context *ctx)
{
    MUXP(ctx)->mux->terminate();
}

QGsm0710MultiplexerChannel::QGsm0710MultiplexerChannel
        ( QGsm0710Multiplexer *mux, int channel )
{
    this->mux = mux;
    this->channel = channel;
    this->incomingStatus = GSM0710_DSR | GSM0710_CTS;
    this->outgoingStatus = GSM0710_DTR | GSM0710_RTS | GSM0710_DCD | 0x01;
    this->previouslyOpened = false;
    this->currentlyOpen = false;
    this->waitingForReadyRead = false;
    this->used = 0;
}

QGsm0710MultiplexerChannel::~QGsm0710MultiplexerChannel()
{
    // Send a close request for this channel if it was ever open.
    if ( previouslyOpened )
        gsm0710_close_channel( &(mux->d->ctx), channel );
}

bool QGsm0710MultiplexerChannel::open( OpenMode mode )
{
    QIODevice::setOpenMode( ( mode & ReadWrite ) | Unbuffered );
    if ( !previouslyOpened && !mux->d->server ) {
        // Send an open request for this channel.
        gsm0710_open_channel( &(mux->d->ctx), channel );
        previouslyOpened = true;
    }
    currentlyOpen = true;
    return true;
}


void QGsm0710MultiplexerChannel::close()
{
    setOpenMode( NotOpen );
    currentlyOpen = false;
    used = 0;
}

qint64 QGsm0710MultiplexerChannel::bytesAvailable() const
{
    return used;
}

bool QGsm0710MultiplexerChannel::waitForReadyRead( int msecs )
{
    // Wait for the underlying device to have data and process it.
    // We must loop around because we could see data for other channels.
    if ( used > 0 ) {
        // We already have data ready to be read.
        return true;
    }
    waitingForReadyRead = true;
    while ( mux->d->device->waitForReadyRead( msecs ) ) {
        mux->incoming();
        if ( used > 0 ) {
            waitingForReadyRead = false;
            return true;
        }
    }
    waitingForReadyRead = false;
    return false;
}

int QGsm0710MultiplexerChannel::rate() const
{
    return mux->d->device->rate();
}

bool QGsm0710MultiplexerChannel::dtr() const
{
    return ( ( outgoingStatus & GSM0710_DTR ) != 0 );
}

void QGsm0710MultiplexerChannel::setDtr( bool value )
{
    if ( value != dtr() ) {
        if ( value )
            outgoingStatus |= GSM0710_DTR;
        else
            outgoingStatus &= ~GSM0710_DTR;
        updateStatus();
    }
}

bool QGsm0710MultiplexerChannel::dsr() const
{
    return ( ( incomingStatus & GSM0710_DSR ) != 0 );
}

bool QGsm0710MultiplexerChannel::carrier() const
{
    return ( ( incomingStatus & GSM0710_DCD ) != 0 );
}

bool QGsm0710MultiplexerChannel::setCarrier( bool value )
{
    if ( value )
        outgoingStatus |= GSM0710_DCD;
    else
        outgoingStatus &= ~GSM0710_DCD;
    updateStatus();
    return true;
}

bool QGsm0710MultiplexerChannel::rts() const
{
    return ( ( outgoingStatus & GSM0710_RTS ) != 0 );
}

void QGsm0710MultiplexerChannel::setRts( bool value )
{
    if ( value != rts() ) {
        outgoingStatus &= ~( GSM0710_RTS | GSM0710_FC );
        if ( value )
            outgoingStatus |= GSM0710_RTS;
        else
            outgoingStatus |= GSM0710_FC;
        updateStatus();
    }
}

bool QGsm0710MultiplexerChannel::cts() const
{
    return ( ( incomingStatus & GSM0710_CTS ) != 0 );
}

void QGsm0710MultiplexerChannel::discard()
{
    used = 0;
}

qint64 QGsm0710MultiplexerChannel::readData( char *data, qint64 maxlen )
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

qint64 QGsm0710MultiplexerChannel::writeData( const char *data, qint64 len )
{
    qint64 result = len;
    uint framelen;
    while ( len > 0 ) {
        framelen = len;
        if ( framelen > (uint)( mux->d->ctx.frame_size ) ) {
            framelen = (uint)( mux->d->ctx.frame_size );
        }
        gsm0710_write_data( &(mux->d->ctx), channel, data, (int)framelen );
        data += framelen;
        len -= framelen;
    }
    return result;
}

void QGsm0710MultiplexerChannel::add( const char *data, uint len )
{
    if ( !currentlyOpen ) {
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

// Set the incoming status on this channel and emit the relevant signals.
void QGsm0710MultiplexerChannel::setStatus( int status )
{
    int lastStatus = incomingStatus;
    status &= ( GSM0710_DSR | GSM0710_DCD | GSM0710_CTS );
    incomingStatus = (char)status;
    if ( ( ( status ^ lastStatus ) & GSM0710_DSR ) != 0 ) {
        emit dsrChanged( ( status & GSM0710_DSR ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & GSM0710_DCD ) != 0 ) {
        emit carrierChanged( ( status & GSM0710_DCD ) != 0 );
    }
    if ( ( ( status ^ lastStatus ) & GSM0710_CTS ) != 0 ) {
        emit ctsChanged( ( status & GSM0710_CTS ) != 0 );
    }
}

// Update the outgoing status of DTR and RTS on this channel.
void QGsm0710MultiplexerChannel::updateStatus()
{
    gsm0710_set_status( &(mux->d->ctx), channel, outgoingStatus );
}

/*!
    Construct a new GSM 07.10 multiplexer around \a device and attach
    it to \a parent.  The size of frames is \a frameSize.  If \a advanced
    is true, then use the Advanced multiplexing option; otherwise use
    the Basic multiplexing option.

    Ownership of \a device will pass to this object; it will be
    deleted when this object is deleted.
*/
QGsm0710Multiplexer::QGsm0710Multiplexer( QSerialIODevice *device,
                                          int frameSize, bool advanced,
                                          QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QGsm0710MultiplexerPrivate( device, frameSize, advanced, false );
    d->mux = this;
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );

    // Create the control channel (0).
    gsm0710_startup( &(d->ctx), 0 );
}

// This constructor is called from QGsm0710MultiplexerServer only.
QGsm0710Multiplexer::QGsm0710Multiplexer( QSerialIODevice *device,
                                          int frameSize, bool advanced,
                                          QObject *parent, bool server )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QGsm0710MultiplexerPrivate( device, frameSize, advanced, server );
    d->mux = this;
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );
}

/*!
    Destruct this GSM 07.10 multiplexer, closing the session.
*/
QGsm0710Multiplexer::~QGsm0710Multiplexer()
{
    // Close all of the active channels.
    d->closeAll();

    // Send the terminate command to exit AT+CMUX multiplexing in client mode.
    gsm0710_shutdown( &(d->ctx) );

    // Clean up everything else.
    delete d;
}

/*!
    \reimp
*/
QSerialIODevice *QGsm0710Multiplexer::channel( const QString& name )
{
    // Convert the name into a channel number.
    int number = channelNumber( name );
    if ( number == -1 )
        return 0;

    // Look up the channel's device by its number.
    if ( d->channels.contains( number ) )
        return d->channels.value( number );

    // Create a new channel with the specified number.
    QGsm0710MultiplexerChannel *channel =
            new QGsm0710MultiplexerChannel( this, number );
    d->channels.insert( number, channel );
    return channel;
}

/*!
    Construct a \c{AT+CMUX} command with the specified parameters and
    send it to \a device.  Returns true if the command succeeded, or
    false if the command failed.  This is typically called by plug-ins
    that use GSM 07.10 with a different frame size or operating mode.

    The size of frames is \a frameSize.  If \a advanced is true, then use
    the Advanced multiplexing option; otherwise use the Basic multiplexing
    option.  The baud rate is acquired from \a device, and the subset is
    always set to zero.

    This is a convenience function for the most common options that
    are passed to \c{AT+CMUX}.  If more involved options are required
    (e.g. subset or timeout values), then the plug-in should call
    QSerialIODeviceMultiplexer::chat() instead.

    \sa QSerialIODeviceMultiplexer::chat()
*/
bool QGsm0710Multiplexer::cmuxChat( QSerialIODevice *device,
                                    int frameSize, bool advanced )
{
    int portSpeed;
    switch ( device->rate() ) {
        case 9600:      portSpeed = 1; break;
        case 19200:     portSpeed = 2; break;
        case 38400:     portSpeed = 3; break;
        case 57600:     portSpeed = 4; break;
        case 115200:    portSpeed = 5; break;
        case 230400:    portSpeed = 6; break;
        default:        portSpeed = 5; break;
    }
    QString cmd = "AT+CMUX=";
    if ( advanced )
        cmd += "1,0,";
    else
        cmd += "0,0,";
    cmd += QString::number( portSpeed ) + "," + QString::number( frameSize );
    return chat( device, cmd );
}

/*!
    Returns the GSM 07.10 channel number associated with the channel \a name.
    Returns -1 if the channel name is not recognized.  The default channel
    assignment is as follows:

    \table
    \header \o Name \o Number
    \row \o \c{primary} \o 1
    \row \o \c{secondary} \o 2
    \row \o \c{data} \o 3
    \row \o \c{datasetup} \o 3
    \row \o \c{aux*} \o 4
    \endtable

    This assignment causes data call setup commands to be sent on the data
    channel.  If a modem needs data call setup on the primary AT command
    channel, then it should override this method and return 1 for \c{datasetup}.

    The current implementation is limited to channel numbers between 1 and 63.
    Numbers outside this range should not be returned.

    \sa channel()
*/
int QGsm0710Multiplexer::channelNumber( const QString& name ) const
{
    if ( name == "primary" )
        return 1;
    else if ( name == "secondary" )
        return 2;
    else if ( name == "data" || name == "datasetup" )
        return 3;
    else if ( name.startsWith( "aux" ) )
        return 4;
    else
        return -1;
}

/*!
    Re-initialize the multiplexing session.  This is called by
    subclasses that have detected that the modem has dropped out
    of multiplexing mode.
*/
void QGsm0710Multiplexer::reinit()
{
    // Restart the GSM 07.10 implementation, and recreate all channels.
    gsm0710_startup( &(d->ctx), 1 );
}

void QGsm0710Multiplexer::incoming()
{
    gsm0710_ready_read( &(d->ctx) );
}

void QGsm0710Multiplexer::terminate()
{
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    QMap<int,QGsm0710MultiplexerChannel *>::Iterator iter;
    for ( iter = d->channels.begin(); iter != d->channels.end(); ++iter ) {
        emit server->closed( iter.value()->chan(), iter.value() );
        delete iter.value();
    }
    d->channels.clear();
    emit server->terminated();
}

void QGsm0710Multiplexer::open( int channel )
{
    // If there is already a channel open with this number, then ignore.
    if ( d->channels.contains( channel ) )
        return;

    // Create a new channel device and register it.
    QGsm0710MultiplexerChannel *device =
            new QGsm0710MultiplexerChannel( this, channel );
    d->channels.insert( channel, device );

    // Make sure that the channel device is properly opened.
    device->open( QIODevice::ReadWrite );

    // Let interested parties know about the new channel.
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    emit server->opened( channel, device );
}

void QGsm0710Multiplexer::close( int channel )
{
    // If this channel is not currently active, then ignore.
    if ( !d->channels.contains( channel ) )
        return;

    // Remove the device from the channel list.
    QGsm0710MultiplexerChannel *device = d->channels.value( channel );
    d->channels.remove( channel );

    // Close the device for the channel.
    device->close();

    // Let interested parties know that the channel is closing.
    QGsm0710MultiplexerServer *server = (QGsm0710MultiplexerServer *)this;
    emit server->closed( channel, device );

    // Delete the device as it is no longer required.
    delete device;
}

/*!
    \class QGsm0710MultiplexerServer
    \inpublicgroup QtBaseModule

    \brief The QGsm0710MultiplexerServer class provides a server-side multiplexing implementation based on 3GPP TS 07.10/27.010
    \ingroup telephony::serial

    This class is used by incoming AT connection managers such as the \l{Modem Emulator}
    to emulate 3GPP TS 07.10/27.010 multiplexing for the remote device that is connecting.

    When the remote device opens a channel, the opened() signal is emitted, providing a
    QSerialIODevice that can be used by the AT connection manager to communicate
    with the remote device on that channel.

    When the remote device closes a channel, the closed() signal is emitted to allow
    the AT connection manager to clean up the QSerialIODevice for the channel and
    any other associated channel-specific context information.

    When the remote device terminates the 3GPP TS 07.10/27.010 session, closed() signals
    for all open channels will be emitted, and then the terminated() signal will be
    emitted.

    \sa QGsm0710Multiplexer, {Modem Emulator}
*/

/*!
    Construct a new GSM 07.10 multiplexer in server mode around \a device
    and attach it to \a parent.  The size of frames is \a frameSize.
    If \a advanced is true, then use the Advanced multiplexing option;
    otherwise use the Basic multiplexing option.

    Unlike the base class, QGsm0710Multiplexer, the ownership of
    \a device will not pass to this object and it will not be deleted
    when this object is deleted.  It will still exist after destruction.
    This is because the device will typically return to normal AT command
    mode after the multiplexer exits.
*/
QGsm0710MultiplexerServer::QGsm0710MultiplexerServer
            ( QSerialIODevice *device, int frameSize,
              bool advanced, QObject *parent )
    : QGsm0710Multiplexer( device, frameSize, advanced, parent, true )
{
    // Nothing to do here at present.
}

/*!
    Destruct this GSM 07.10 server instance.
*/
QGsm0710MultiplexerServer::~QGsm0710MultiplexerServer()
{
    // Nothing to do here at present.
}

/*!
    \fn void QGsm0710MultiplexerServer::opened( int channel, QSerialIODevice *device )

    Signal that is emitted when the client opens a new GSM 07.10
    \a channel.  The \a device object allows access to the raw
    data and modem control signals on the channel.

    \sa closed()
*/

/*!
    \fn void QGsm0710MultiplexerServer::closed( int channel, QSerialIODevice *device )

    Signal that is emitted when the client closes a GSM 07.10 \a channel.
    After this signal is emitted, \a device will no longer be valid.

    \sa opened()
*/

/*!
    \fn void QGsm0710MultiplexerServer::terminated()

    Signal that is emitted when the client terminates the GSM 07.10
    session and wishes to return to normal command mode.

    This signal will be preceded by closed() signals for all channels
    that were still open when the terminate command was received.

    A slot that is connected to this signal should use QObject::deleteLater()
    to clean up this object.
*/
