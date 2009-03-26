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

#ifndef QSERIALIODEVICEMULTIPLEXER_H
#define QSERIALIODEVICEMULTIPLEXER_H

#include <qserialiodevice.h>

class QSerialIODeviceMultiplexerPrivate;

class QTOPIACOMM_EXPORT QSerialIODeviceMultiplexer : public QObject
{
    Q_OBJECT
public:
    explicit QSerialIODeviceMultiplexer( QObject *parent = 0 );
    ~QSerialIODeviceMultiplexer();

    virtual QSerialIODevice *channel( const QString& name ) = 0;

    static bool chat( QSerialIODevice *device, const QString& cmd );
    static QString chatWithResponse
            ( QSerialIODevice *device, const QString& cmd );
    static QSerialIODeviceMultiplexer *create( QSerialIODevice *device = 0 );
};

class QNullSerialIODeviceMultiplexerPrivate;

class QTOPIACOMM_EXPORT QNullSerialIODeviceMultiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
public:
    QNullSerialIODeviceMultiplexer
        ( QSerialIODevice *device, QObject *parent = 0 );
    ~QNullSerialIODeviceMultiplexer();

    QSerialIODevice *channel( const QString& name );

private slots:
    void dataOpened();
    void dataClosed();

private:
    QNullSerialIODeviceMultiplexerPrivate *d;
};

#endif
