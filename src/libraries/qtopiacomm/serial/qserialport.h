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

#ifndef QSERIALPORT_H
#define QSERIALPORT_H


#include <qserialiodevice.h>

class QSerialPortPrivate;

class QTOPIACOMM_EXPORT QSerialPort : public QSerialIODevice
{
    Q_OBJECT
public:
    explicit QSerialPort( const QString& device, int rate = 38400, bool trackStatus = false );
    ~QSerialPort();

    int fd() const;

    // Override QIODevice methods.
    bool open( OpenMode mode );
    void close();
    bool flush();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    // Get or set the CTS/RTS flow control mode.
    bool flowControl() const;
    void setFlowControl( bool value );

    bool keepOpen() const;
    void setKeepOpen( bool value );

    // Override QSerialIODevice methods.
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
    QProcess *run( const QStringList& arguments, bool addPPPdOptions );

    // Create and open a serial device from a "device:rate" name.
    static QSerialPort *create( const QString& name, int defaultRate=115200,
                                bool flowControl=false );

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void statusTimeout();
    void pppdStateChanged( QProcess::ProcessState state );
    void pppdDestroyed();

private:
    QSerialPortPrivate *d;
};

#endif
