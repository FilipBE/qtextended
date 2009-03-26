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

#ifndef QAUDIOOUTPUT_H
#define QAUDIOOUTPUT_H

#include <qiodevice.h>

#include <qtopiaglobal.h>

class QAudioOutputPrivate;

class QTOPIAAUDIO_EXPORT QAudioOutput : public QIODevice
{
    Q_OBJECT
public:
    explicit QAudioOutput( QObject *parent = 0 );
    explicit QAudioOutput( const QByteArray &device, QObject *parent = 0 );
    ~QAudioOutput();

    int frequency() const;
    void setFrequency( int value );

    int channels() const;
    void setChannels( int value );

    int bitsPerSample() const;
    void setBitsPerSample( int value );

    bool open( QIODevice::OpenMode mode );
    void close();
    bool isSequential() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void deviceReady( int id );
    void deviceError( int id );

private:
    QAudioOutputPrivate *d;
};

#endif
