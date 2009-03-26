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

#include <qmultiportmultiplexer.h>
#include <qmap.h>
#include <qset.h>

/*!
    \class QMultiPortMultiplexer
    \inpublicgroup QtBaseModule

    \brief The QMultiPortMultiplexer class provides multiplexing across several serial ports
    \ingroup telephony::serial

    Use this class instead of QGsm0710Multiplexer if the operating system kernel
    has built-in support for multiplexing.

    Instances of QMultiPortMultiplexer are created by multiplexer plug-ins.
    See the \l{Tutorial: Writing a Multiplexer Plug-in} for more information
    on how to write a multiplexer plug-in that uses QMultiPortMultiplexer.

    \sa QSerialIODeviceMultiplexer, QGsm0710Multiplexer, QSerialIODeviceMultiplexerPlugin
*/

class QMultiPortMultiplexerPrivate
{
public:
    ~QMultiPortMultiplexerPrivate();

    QMap<QString, QSerialIODevice *> channels;
    QSet<QSerialIODevice *> devices;
};

QMultiPortMultiplexerPrivate::~QMultiPortMultiplexerPrivate()
{
    QSet<QSerialIODevice *>::ConstIterator iter;
    for ( iter = devices.begin(); iter != devices.end(); ++iter ) {
        delete *iter;
    }
}

/*!
    Construct a new multi-port multiplexer object and attach it to \a parent.
    The \c{primary} channel will be set to \a device.  Further channels
    can be added by calling addChannel().  Ownership of \a device will
    pass to this object; it will be deleted when this object is deleted.
*/
QMultiPortMultiplexer::QMultiPortMultiplexer
        ( QSerialIODevice *device, QObject *parent )
    : QSerialIODeviceMultiplexer( parent )
{
    d = new QMultiPortMultiplexerPrivate();
    addChannel( "primary", device );
}

/*!
    Destruct this multi-port multiplexer object.
*/
QMultiPortMultiplexer::~QMultiPortMultiplexer()
{
    delete d;
}

/*!
    Add a new channel to this multiplexer, with requests for \a name
    being redirected to \a device.  Returns false if \a name already
    exists.

    Ownership of \a device will pass to this object; it will be
    deleted when this object is deleted.  A single device can be added
    for multiple channels (e.g. \c{data} and \c{datasetup}).  This
    object will ensure that such devices will be deleted only once.
*/
bool QMultiPortMultiplexer::addChannel
        ( const QString& name, QSerialIODevice *device )
{
    if ( d->channels.contains( name ) )
        return false;
    d->channels.insert( name, device );
    if ( !d->devices.contains( device ) )
        d->devices.insert( device );
    return true;
}

/*!
    \reimp
*/
QSerialIODevice *QMultiPortMultiplexer::channel( const QString& name )
{
    if ( d->channels.contains( name ) ) {
        return d->channels.value( name );
    } else if ( name == "secondary" && d->channels.contains( "primary" ) ) {
        // No explicit "secondary" channel, so use "primary".
        return d->channels.value( "primary" );
    } else if ( name == "datasetup" && d->channels.contains( "data" ) ) {
        // No explicit "datasetup" channel, so use "data".
        return d->channels.value( "data" );
    } else if ( name.startsWith( "aux" ) && d->channels.contains( "aux" ) ) {
        // No explicit "aux*" channel, so use "aux".
        return d->channels.value( "aux" );
    } else {
        return 0;
    }
}
