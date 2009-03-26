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

#ifndef CONTENTDEVICE_H
#define CONTENTDEVICE_H

#include <QMediaDevice>

class ContentDevicePrivate;

class ContentDevice : public QMediaDevice
{
public:
    ContentDevice(QString const& filePath);
    ~ContentDevice();

    QMediaDevice::Info const& dataType() const;

    bool connectToInput(QMediaDevice* input);
    void disconnectFromInput(QMediaDevice* input);

    bool open(QIODevice::OpenMode mode);
    void close();
    bool isSequential() const;
    bool seek(qint64 pos);
    qint64 pos() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private:
    ContentDevicePrivate*   d;
};

#endif
