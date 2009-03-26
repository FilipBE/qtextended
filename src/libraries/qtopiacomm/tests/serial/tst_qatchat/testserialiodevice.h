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

#ifndef TESTSERIALIODEVICE_H
#define TESTSERIALIODEVICE_H

#include <qserialiodevice.h>
#include <QMap>

class TestSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
public:
    TestSerialIODevice( QObject *parent = 0 );
    ~TestSerialIODevice();

    bool open( OpenMode mode );
    void close();
    qint64 bytesAvailable() const;
    int rate() const;
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    bool isValid() const;

    void setDsr( bool value );
    void setCts( bool value );
    void setRemoteCarrier( bool value );
    void addIncomingData( const QByteArray& data );
    QByteArray readOutgoingData();

    void respond( const QString& cmd, const QString& resp );
    void removeRespond( const QString& cmd );

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private:
    bool _dtr;
    bool _dsr;
    bool _carrier;
    bool _rts;
    bool _cts;
    QByteArray incoming;
    QByteArray outgoing;
    QMap<QString, QString> responses;
};

#endif
