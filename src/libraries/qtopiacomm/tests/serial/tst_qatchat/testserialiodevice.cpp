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

//QTEST_SKIP_TEST_DOC

#include "testserialiodevice.h"
#include <QTimer>

TestSerialIODevice::TestSerialIODevice( QObject *parent )
    : QSerialIODevice( parent )
{
    _dtr = true;
    _dsr = true;
    _carrier = false;
    _rts = true;
    _cts = true;
}

TestSerialIODevice::~TestSerialIODevice()
{
}

bool TestSerialIODevice::open( OpenMode mode )
{
    setOpenMode( mode );
    return true;
}

void TestSerialIODevice::close()
{
    setOpenMode( NotOpen );
}

qint64 TestSerialIODevice::bytesAvailable() const
{
    return incoming.size();
}

int TestSerialIODevice::rate() const
{
    return 115200;
}

bool TestSerialIODevice::dtr() const
{
    return _dtr;
}

void TestSerialIODevice::setDtr( bool value )
{
    _dtr = value;
}

bool TestSerialIODevice::dsr() const
{
    return _dsr;
}

bool TestSerialIODevice::carrier() const
{
    return _carrier;
}

bool TestSerialIODevice::rts() const
{
    return _rts;
}

void TestSerialIODevice::setRts( bool value )
{
    _rts = value;
}

bool TestSerialIODevice::cts() const
{
    return _cts;
}

void TestSerialIODevice::discard()
{
    outgoing = QByteArray();
}

bool TestSerialIODevice::isValid() const
{
    return true;
}

void TestSerialIODevice::setDsr( bool value )
{
    if ( _dsr != value ) {
        _dsr = value;
        emit dsrChanged( value );
    }
}

void TestSerialIODevice::setCts( bool value )
{
    if ( _cts != value ) {
        _cts = value;
        emit ctsChanged( value );
    }
}

void TestSerialIODevice::setRemoteCarrier( bool value )
{
    if ( _carrier != value ) {
        _carrier = value;
        emit carrierChanged( value );
    }
}

void TestSerialIODevice::addIncomingData( const QByteArray& data )
{
    incoming += data;
    QTimer::singleShot( 0, this, SLOT(internalReadyRead()) );
}

QByteArray TestSerialIODevice::readOutgoingData()
{
    QByteArray temp = outgoing;
    outgoing = QByteArray();
    return temp;
}

void TestSerialIODevice::respond( const QString& cmd, const QString& resp )
{
    responses.insert( cmd, resp );
}

void TestSerialIODevice::removeRespond( const QString& cmd )
{
    responses.remove( cmd );
}

qint64 TestSerialIODevice::readData( char *data, qint64 maxlen )
{
    int size = incoming.size();
    if ( size > maxlen )
        size = (int)maxlen;
    memcpy( data, incoming.constData(), size );
    incoming = incoming.mid( size );
    return size;
}

qint64 TestSerialIODevice::writeData( const char *data, qint64 len )
{
    QString cmd, resp;

    if ( len >= 2 && data[0] == 'A' && data[1] == 'T' &&
         data[((int)len) - 1] == '\r' ) {
        // This looks like it may be a command.  If we have a standard
        // response for it, then take care of that now.
        cmd = QString::fromLatin1( data, (int)(len - 1) );
        if ( responses.contains( cmd ) ) {
            resp = responses[cmd] + "\r\n";
            addIncomingData( resp.toLatin1() );
            return len;
        }

        // Check for wildcarded responses.
        int tlen = (int)(len - 2);
        while ( tlen > 2 ) {
            cmd = QString::fromLatin1( data, tlen ) + "*";
            if ( responses.contains( cmd ) ) {
                resp = responses[cmd] + "\r\n";
                addIncomingData( resp.toLatin1() );
                return len;
            }
            --tlen;
        }
    }
    outgoing += QByteArray( data, (int)len );
    return len;
}
