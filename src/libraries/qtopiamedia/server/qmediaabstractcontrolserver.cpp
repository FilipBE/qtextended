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
#include <QtopiaIpcAdaptor>
#include <QValueSpaceObject>

#include "qmediahandle_p.h"
#include "qmediaabstractcontrolserver.h"


// {{{ QMediaAbstractControlServerPrivate
class QMediaAbstractControlServerPrivate
{
public:
    QMediaHandle            handle;
    QValueSpaceObject*      values;
    QtopiaIpcAdaptor*       send;
    QtopiaIpcAdaptor*       recieve;
};
// }}}

/*!
    \class QMediaAbstractControlServer
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaAbstractControlServer class is used as a base class for
    media server control classes to make then available to clients.

    \sa QMediaServerSession, QMediaVideoControlServer
*/

/*!
    Creaate a QMediaAbstractControlServer with \a handle to the session and
    control \a name, parented to \a parent.
*/

QMediaAbstractControlServer::QMediaAbstractControlServer
(
 QMediaHandle const& handle,
 QString const& name,
 QObject* parent
):
    QObject(parent),
    d(new QMediaAbstractControlServerPrivate)
{
    d->values = new QValueSpaceObject("/Media/Control/" + handle.toString() + "/" + name, this);
    connect(d->values, SIGNAL(itemRemove(QByteArray)),
            this, SLOT(itemRemove(QByteArray)));

    connect(d->values, SIGNAL(itemSetValue(QByteArray,QVariant)),
            this, SLOT(itemSetValue(QByteArray,QVariant)));

    QString baseName = handle.toString() + "/" + name;

    d->send = new QtopiaIpcAdaptor("QPE/Media/Library/Control/" + baseName, this);
    d->recieve = new QtopiaIpcAdaptor("QPE/Media/Server/Control/"+ baseName, this);
}

/*!
    Destroy a QMediaAbstractControlServer object.
*/

QMediaAbstractControlServer::~QMediaAbstractControlServer()
{
    delete d;
}

/*!
    \fn void QMediaAbstractControlServer::controlAvailable(QString const& name);

    This signal is emitted when the control \a name becomes available to the
    client.
*/

/*!
    \fn void QMediaAbstractControlServer::controlUnavailable(QString const& name);

    This signal is emitted when the control \a name is no longer available to
    the client.
*/

/*!
    Set the \a value with the \a name into the Value-space related to this
    media control object.
*/

void QMediaAbstractControlServer::setValue(QString const& name, QVariant const& value)
{
    d->values->setAttribute(name, value);

    QValueSpaceObject::sync();
}

/*!
    Present the objects signals and slots to the IPC system.
*/

void QMediaAbstractControlServer::proxyAll()
{
    QMetaObject const*  mo = metaObject();
    int                 mc = mo->methodCount();
    int                 offset = QMediaAbstractControlServer::staticMetaObject.methodOffset();

    QString className = mo->className();

    for (int i = offset; i < mc; ++i)
    {
        QMetaMethod method = mo->method(i);

        switch (method.methodType())
        {
        case QMetaMethod::Signal:
            QtopiaIpcAdaptor::connect(this, QByteArray::number(QSIGNAL_CODE) + method.signature(),
                                      d->send, QByteArray::number(QMESSAGE_CODE) + method.signature());
            break;

        case QMetaMethod::Slot:
            if (method.access() == QMetaMethod::Public)
            {
                QtopiaIpcAdaptor::connect(d->recieve, QByteArray::number(QMESSAGE_CODE) + method.signature(),
                                          this, QByteArray::number(QSLOT_CODE) + method.signature());
            }
            break;

        case QMetaMethod::Method:
            break;
        }
    }
}

/*!
    \internal
*/

void QMediaAbstractControlServer::itemRemove(const QByteArray &attribute)
{
    d->values->removeAttribute(attribute);
}

/*!
    \internal
*/

void QMediaAbstractControlServer::itemSetValue(const QByteArray &attribute, const QVariant &value)
{
    d->values->setAttribute(attribute, value);
}

