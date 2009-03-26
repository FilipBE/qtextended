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

#include <qcommservicemanager.h>
#include <qvaluespace.h>

/*!
    \class QCommServiceManager
    \inpublicgroup QtBaseModule

    \brief The QCommServiceManager class provides a method to discover the active communications services and interfaces.

    Each service has a unique name, such as \c{GSM}, \c{VoIP}, etc.  Within
    each service is a list of interfaces for functionality areas, which have
    names such as QNetworkRegistration, QSMSSender, etc.

    Interface names correspond to class names elsewhere in the Qt Extended Communications and Telephony API's.  To use an interface, the caller
    creates an instance of the corresponding class.  The caller can specify
    an explicit service name, if more than one service implements the same
    interface, or leave the service name empty to use the default service
    for that interface.

    See the documentation for QAbstractIpcInterface for more information on
    writing and using interface classes.

    \sa QCommInterface

    \ingroup ipc
    \ingroup telephony
*/

/*!
    Construct a new communications service manager and attach it to \a parent.
*/
QCommServiceManager::QCommServiceManager( QObject *parent )
    : QAbstractIpcInterfaceGroupManager( "/Communications", parent )
{
    // Give the signals more convenient names.
    connect( this, SIGNAL(groupsChanged()),
             this, SIGNAL(servicesChanged()) );
    connect( this, SIGNAL(groupAdded(QString)),
             this, SIGNAL(serviceAdded(QString)) );
    connect( this, SIGNAL(groupRemoved(QString)),
             this, SIGNAL(serviceRemoved(QString)) );
}

/*!
    Destroy this communications service manager.
*/
QCommServiceManager::~QCommServiceManager()
{
}

/*!
    Get the list of all services that are currently active within the system.

    \sa servicesChanged(), serviceAdded(), serviceRemoved()
*/
QStringList QCommServiceManager::services() const
{
    return groups();
}

/*!
    \fn void QCommServiceManager::servicesChanged()

    Signal that is emitted when the list of services changes, if the interfaces
    on a service has changed, or if the priority assignments have changed.

    \sa services()
*/

/*!
    \fn void QCommServiceManager::serviceAdded( const QString& service )

    Signal that is emitted when \a service is added.  A service is considered
    to have been added when its first interface is constructed.

    \sa services()
*/

/*!
    \fn void QCommServiceManager::serviceRemoved( const QString& service )

    Signal that is emitted when \a service is removed.  A service is
    considered to have been removed when its last interface is deleted.

    \sa services()
*/
