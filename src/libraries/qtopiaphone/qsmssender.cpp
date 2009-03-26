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

#include <qsmssender.h>
#include <QUuid>

/*!
    \class QSMSSender
    \inpublicgroup QtTelephonyModule

    \brief The QSMSSender class provides facilities to send SMS messages.
    \ingroup telephony

    Client applications construct an SMS message using QSMSMessage and then call
    the send() method.  Once the message has been delivered, or delivery failed,
    the finished() signal is emitted.

    \sa QSMSMessage, QSMSReader, QCommInterface
*/

/*!
    Construct a new SMS sending object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SMS sending.  If there is more
    than one service that supports SMS sending, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSMSSender objects for each.

    \sa QCommServiceManager::supports()
*/
QSMSSender::QSMSSender( const QString& service, QObject *parent,
                        QCommInterface::Mode mode )
    : QCommInterface( "QSMSSender", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SMS sending object.
*/
QSMSSender::~QSMSSender()
{
}

/*!
    Send the SMS message \a msg and return an identifier for it.
    This is a convenience method around
    QSMSSender::send(const QString&, const QSMSMessage&).

    \sa finished()
*/
QString QSMSSender::send( const QSMSMessage& msg )
{
    QString id = QUuid::createUuid().toString();
    send( id, msg );
    return id;
}

/*!
    Send the SMS message \a msg, and associate the finished() response
    with the identifier \a id.

    \sa finished()
*/
void QSMSSender::send( const QString& id, const QSMSMessage& msg )
{
    invoke( SLOT(send(QString,QSMSMessage)), id, qVariantFromValue( msg ) );
}

/*!
    \fn void QSMSSender::finished( const QString& id, QTelephony::Result result )
    Signal that is emitted when the message associated with \a id has
    been sent, or the send failed for some reason.  The reason for the
    failure is specified by \a result.

    \sa send()
*/
