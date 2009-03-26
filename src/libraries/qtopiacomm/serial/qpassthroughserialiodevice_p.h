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

#ifndef QPASSTHROUGHSERIALIODEVICE_P_H
#define QPASSTHROUGHSERIALIODEVICE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qserialiodevice.h>

class QPassThroughSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
public:
    explicit QPassThroughSerialIODevice( QSerialIODevice *device, QObject *parent = 0 );
    ~QPassThroughSerialIODevice();

    bool isEnabled() const { return enabled; }
    void setEnabled( bool flag ) { enabled = flag; }

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

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

    bool isValid() const;

signals:
    void opened();
    void closed();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void deviceReadyRead();

private:
    QSerialIODevice *device;
    bool enabled;
};

#endif
