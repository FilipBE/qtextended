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

// Local includes
#include "qhardwareinterface_p.h"
#include "qhardwaremanager.h"

// ============================================================================
//
// QHardwareManager
//
// ============================================================================

struct QHardwareManagerPrivate
{
    QString interface;
};

/*!
    \class QHardwareManager
    \inpublicgroup QtBaseModule

    \brief The QHardwareManager class finds available hardware abstraction providers for a 
    given interface.

    QHardwareManager is part of the \l{Qt Extended Hardware Abstraction},
    which provides information about the available hardware devices.

    A QHardwareMonitor can be used to monitor the availability of a given
    type of hardware, providing a list of providers through providers() 
    and emitting signals providerAdded() and  providerRemoved() when that list changes.
    The type of hardware monitored is given as a parameter to
    constructing the QHardwareMonitor.

    The following example responds whenever a new QSignalSource becomes available:

    \code
        QHardwareManager* manager = new QHardwareManager( "QSignalSource" );
        connect( manager, SIGNAL(providerAdded(QString)), 
            this, SLOT(newBatteryAdded(QString)));
    \endcode

    The types of a hardware abstraction is the name of the abstraction classes 
    which implement the interface to the device. Any class that is subclassing QHardwareInterface 
    is considered to be a Qt Extended hardware abstraction.

    \sa QHardwareInterface, QPowerSource, QSignalSource, QVibrateAccessory
    \ingroup hardware
*/

/*!
    \fn void QHardwareManager::providerAdded( const QString& id );

    This signal is emitted when provider \a id is added.
*/

/*!
    \fn void QHardwareManager::providerRemoved( const QString& id );

    This signal is emitted when provider \a id is removed.
*/

/*!
    Creates a QHardwareManager object and attaches it to \a parent. \a interface is the
    name of the hardware interface that this object is monitoring.

    The following code assumes that Qt Extended is running on a device that
    has a modem as power source (for more details see QPowerStatus and QPowerSource).
    \code
        QHardwareManager manager( "QPowerSource" );
        QStringList providers = manager.providers();
        
        //Qt Extended always has a virtual power source with id DefaultBattery
        providers.contains( "DefaultBattery" ); //always true
        providers.contains( "modem" ); //true
    \endcode

    Another way to achieve the same as the above example would be:

    \code
        QStringList providers = QHardwareManager::providers<QPowerSource>();
        providers.contains( "DefaultBattery" ); //always true
        providers.contains( "modem" ); //true
    \endcode
*/
QHardwareManager::QHardwareManager( const QString& interface, QObject *parent )
    : QAbstractIpcInterfaceGroupManager( HARDWAREINTERFACE_VALUEPATH, interface, parent )
{
    d = new QHardwareManagerPrivate;
    d->interface = interface;
    connect( this, SIGNAL(groupAdded(QString)),
             this, SIGNAL(providerAdded(QString)) );
    connect( this, SIGNAL(groupRemoved(QString)),
             this, SIGNAL(providerRemoved(QString)) );
}

/*!
    Destroys the QHardwareManager object.
*/
QHardwareManager::~QHardwareManager()
{
}

/*!
  Returns the interface that this object is monitoring.
*/
QString QHardwareManager::interface() const
{
    return d->interface;
}

/*!
    Returns a list of providers that support the interface that this object was initialized
    with.
*/
QStringList QHardwareManager::providers() const
{
    return groups();
}

/*!
  \fn QStringList QHardwareManager::providers<T>()

  Returns a list of providers which support the given interface of type \c{T}. 
  The following example demonstrates how to get the list of providers that supports
  the QPowerSource interface.

  \code
        QStringList providers = QHardwareManager::providers<QPowerSource>();
  \endcode
 */
