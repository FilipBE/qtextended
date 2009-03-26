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

#include <QContent>

#include "contentdevice.h"


class ContentDevicePrivate
{
public:
    QContent*   content;
    QIODevice*  contentDevice;
    QMediaDevice::Info  outputInfo;
};

/*!
    \class ContentDevice
    \internal
*/

ContentDevice::ContentDevice(QString const& filePath):
    d(new ContentDevicePrivate)
{
    // chop URL modifier
    int marker = filePath.indexOf("://");
    QString urlStripped = filePath.mid(marker + 3);

    d->content = new QContent(marker != -1 ? urlStripped : filePath, false);
    d->contentDevice = 0;

    d->outputInfo.type = QMediaDevice::Info::Raw;
}

ContentDevice::~ContentDevice()
{
    delete d->contentDevice;
    delete d->content;
    delete d;
}

QMediaDevice::Info const& ContentDevice::dataType() const
{
    return d->outputInfo;
}

bool ContentDevice::connectToInput(QMediaDevice*)
{
    qWarning("ContentDevice::connectToInput(); ContentDevice is a source - inputs are invalid");

    return false;
}

void ContentDevice::disconnectFromInput(QMediaDevice*)
{
    qWarning("ContentDevice::disconnectFromInput(); ContentDevice is a source - inputs are invalid");
}

bool ContentDevice::open(QIODevice::OpenMode)
{
    bool        rc = false;

    if (d->contentDevice)
        rc = true;
    else
    {
        d->contentDevice = d->content->open(QIODevice::ReadOnly);
        if (d->contentDevice) {
            if (QIODevice::open(QIODevice::ReadOnly | QIODevice::Unbuffered))
            {
                d->outputInfo.dataSize = d->contentDevice->isSequential() ? -1 : d->contentDevice->size();

                connect(d->contentDevice, SIGNAL(aboutToClose()), SIGNAL(aboutToClose()));
                connect(d->contentDevice, SIGNAL(bytesWritten(qint64)), SIGNAL(bytesWritten(qint64)));
                connect(d->contentDevice, SIGNAL(readyRead()), SIGNAL(readyRead()));

                rc = true;
            }
            else {
                delete d->contentDevice;
                d->contentDevice = 0;
            }
        }
    }

    return rc;
}

void ContentDevice::close()
{
    if (d->contentDevice) {
        d->contentDevice->close();

        delete d->contentDevice;
        d->contentDevice = 0;

        QIODevice::close();
    }
}

bool ContentDevice::isSequential() const
{
    if (d->contentDevice)
        return d->contentDevice->isSequential();

    qWarning("Calling isSequential() on an invalid ContentDevice");

    return false;
}

bool ContentDevice::seek(qint64 pos)
{
    if (d->contentDevice)
        return d->contentDevice->seek(pos);

    qWarning("Calling seek() on an invalid ContentDevice");

    return false;
}

qint64 ContentDevice::pos() const
{
    if (d->contentDevice)
        return d->contentDevice->pos();

    qWarning("Calling pos() on an invalid ContentDevice");

    return 0;
}

//protected:
qint64 ContentDevice::readData(char *data, qint64 maxlen)
{
    if (d->contentDevice)
        return d->contentDevice->read(data, maxlen);

    qWarning("Calling readData() on an invalid ContentDevice");

    return 0;
}

qint64 ContentDevice::writeData(const char *, qint64)
{
    return 0;
}


