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

#include <qservicechecker.h>

/*!
    \class QServiceChecker
    \inpublicgroup QtTelephonyModule

    \brief The QServiceChecker class allows client applications to check to see if a service is valid to use.
    \ingroup telephony

    Some services (e.g. GSM modems) may be available, but unusable because of hardware
    failure.  This class allows the failure to be advertised to client applications.

    All telephony services that inherit from QTelephonyService will have a
    default QServiceChecker instance with isValid() set to true.  This
    can be overridden in subclasses to set isValid() to false if a
    hardware failure is detected during start up.

    \sa QTelephonyService
*/

/*!
    Construct a new service checker object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports service checking.  If there is more
    than one service that supports service checking, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QServiceChecker objects for each.

    \sa QCommServiceManager::supports()
*/
QServiceChecker::QServiceChecker
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QServiceChecker", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this service checker object.
*/
QServiceChecker::~QServiceChecker()
{
}

/*!
    Returns true if the service is valid; otherwise returns false.

    \sa setValid()
*/
bool QServiceChecker::isValid()
{
    return value( "isValid", false ).toBool();
}

/*!
    Sets the isValid() state to \a value.  This is used by server-side
    implementations.

    \sa isValid()
*/
void QServiceChecker::setValid( bool value )
{
    setValue( "isValid", value );
}

/*!
    \class QServiceCheckerServer
    \inpublicgroup QtTelephonyModule

    \brief The QServiceCheckerServer class allows telephony services to easily advertise their hardware failure state.
    \ingroup telephony

    This class is useful for when the validity is known at initialization time.
    If the validity is not known until later, QServiceChecker should be inherited and
    QServiceChecker::setValid() called when the state is known.

    \sa QServiceChecker
*/

/*!
    Construct a server-side service checker for \a service and
    attach it to \a parent.  The isValid() state will be set to \a valid.
*/
QServiceCheckerServer::QServiceCheckerServer
        ( const QString& service, bool valid, QObject *parent )
    : QServiceChecker( service, parent, Server )
{
    setValid( valid );
}

/*!
    Destroy this server-side service checker.
*/
QServiceCheckerServer::~QServiceCheckerServer()
{
}
