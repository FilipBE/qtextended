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

#include <QMetaMethod>
#include <QString>
#include <QTimer>
#include <QtopiaIpcAdaptor>
#include <QValueSpaceItem>
#include <QDebug>

#include "qmediahandle_p.h"
#include "qmediacontent.h"

#include "qmediaabstractcontrol.h"


class QMediaAbstractControlPrivate
{
public:
    QMediaContent*      mediaContent;
    QString             name;
    QValueSpaceItem*    values;
    QtopiaIpcAdaptor*   send;
    QtopiaIpcAdaptor*   recieve;
};

QMediaAbstractControl::QMediaAbstractControl(QMediaContent* mediaContent, QString const& name):
    QObject(mediaContent),
    d(new QMediaAbstractControlPrivate)
{
    d->mediaContent = mediaContent;
    d->name = name;
    d->values = 0;

    QMediaHandle handle = QMediaHandle::getHandle(mediaContent);
    QString baseName = handle.toString() + "/" + name;

    d->send = new QtopiaIpcAdaptor("QPE/Media/Server/Control/" + baseName, this);
    d->recieve = new QtopiaIpcAdaptor("QPE/Media/Library/Control/" + baseName, this);

    connect(mediaContent, SIGNAL(controlAvailable(QString)),
            this, SLOT(controlAvailable(QString)));

    connect(mediaContent, SIGNAL(controlUnavailable(QString)),
            this, SLOT(controlUnavailable(QString)));

    // Check if active server TODO: bit of a hack
    QValueSpaceItem item("/Media/Control/" + handle.toString() + "/Session");

    if (item.value("controls").toStringList().contains(name))
    {
        d->values = new QValueSpaceItem("/Media/Control/" + handle.toString() + "/" + name);
        QTimer::singleShot(0, this, SIGNAL(valid()));
    }
}

QMediaAbstractControl::~QMediaAbstractControl()
{
    delete d->values;
    delete d;
}

QVariant QMediaAbstractControl::value(QString const& name, QVariant const& defaultValue) const
{
    if (d->values == 0)
        return defaultValue;

    return d->values->value(name, defaultValue);
}

void QMediaAbstractControl::setValue(QString const& name, QVariant const& value)
{
    if (d->values == 0)
        qWarning("Attempting to call setValue() on an invalid control");

    d->values->setValue(name, value);
    d->values->sync();
}

void QMediaAbstractControl::proxyAll()
{
    QMetaObject const*  mo = metaObject();
    int                 mc = mo->methodCount();
    int                 offset = mo->methodOffset();

    // Connect server signals to client
    for (int i = offset; i < mc; ++i)
    {
        QMetaMethod method = mo->method(i);

        switch (method.methodType())
        {
        case QMetaMethod::Signal:
            QtopiaIpcAdaptor::connect(d->recieve, QByteArray::number(QMESSAGE_CODE) + method.signature(),
                                      this, QByteArray::number(QSIGNAL_CODE) + method.signature());
            break;

        case QMetaMethod::Slot:
        case QMetaMethod::Method:
            break;
        }
    }
}

void QMediaAbstractControl::forward(QString const& slot, SlotArgs const& args)
{
    d->send->send(slot.toLatin1(), args);
}

void QMediaAbstractControl::controlAvailable(const QString& name)
{
    if (name == d->name && d->values == 0)
    {
        // Connect to our valuespace
        d->values = new QValueSpaceItem("/Media/Control/" + QMediaHandle::getHandle(d->mediaContent).toString() + "/" + name);

        emit valid();
    }
}

void QMediaAbstractControl::controlUnavailable(const QString& name)
{
    if (name == d->name)
    {
        delete d->values;
        d->values = 0;

        emit invalid();
    }
}



