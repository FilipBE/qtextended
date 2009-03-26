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

#ifndef QGSM0710MULTIPLEXER_H
#define QGSM0710MULTIPLEXER_H

#include <qserialiodevicemultiplexer.h>

class QGsm0710MultiplexerPrivate;

class QTOPIACOMM_EXPORT QGsm0710Multiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
    friend class QGsm0710MultiplexerPrivate;
    friend class QGsm0710MultiplexerChannel;
    friend class QGsm0710MultiplexerServer;
public:
    explicit QGsm0710Multiplexer( QSerialIODevice *device,
                                  int frameSize = 31, bool advanced = false,
                                  QObject *parent = 0 );
    ~QGsm0710Multiplexer();

    QSerialIODevice *channel( const QString& name );

    static bool cmuxChat( QSerialIODevice *device, int frameSize = 31,
                          bool advanced = false );

protected:
    virtual int channelNumber( const QString& name ) const;
    void reinit();

private slots:
    void incoming();

private:
    QGsm0710MultiplexerPrivate *d;

    QGsm0710Multiplexer( QSerialIODevice *device,
                         int frameSize, bool advanced,
                         QObject *parent, bool server );

    void terminate();
    void open( int channel );
    void close( int channel );
};

class QTOPIACOMM_EXPORT QGsm0710MultiplexerServer : public QGsm0710Multiplexer
{
    Q_OBJECT
    friend class QGsm0710Multiplexer;
public:
    explicit QGsm0710MultiplexerServer( QSerialIODevice *device,
                                        int frameSize = 31, bool advanced = false,
                                        QObject *parent = 0 );
    ~QGsm0710MultiplexerServer();

signals:
    void opened( int channel, QSerialIODevice *device );
    void closed( int channel, QSerialIODevice *device );
    void terminated();
};

#endif
