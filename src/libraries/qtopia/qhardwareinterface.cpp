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
#include "qhardwareinterface.h"
#include "qhardwareinterface_p.h"

#include <QValueSpaceItem>

// Qt includes
#include <QSettings>

// ============================================================================
//
// QHardwareInterface
//
// ============================================================================

/*!
    \class QHardwareInterface
    \inpublicgroup QtBaseModule

    \brief The QHardwareInterface class is the base class of all hardware abstraction classes.

    QHardwareInterface and the abstraction classes are part of the
    \l {Qt Extended Hardware Abstraction}, which provides information on the
    availability of physical device and an API for controlling those
    devices.  The device API is split into two sets of classes:
    iabstraction provider classes and abstraction client classes.  Both sets are
    subclasses of QHardwareInterface.
    
    The provider classes implement device-specific code for
    controlling the hardware feature and manage the state that is reported to
    the rest of Qt Extended through the client API.  Provider classes are
    created by passing QAbstractIpcInterface::Server as the mode parameter when
    constructing QHardwareInterface derived classes.

    The client classes provide an API for querying the state of and
    controlling devices.  Client classes communicate with provider classes
    through an Inter Process Communication (IPC) mechanism.  Multiple client
    instances can connect to a single provider.  Client classes are
    created by passing QAbstractIpcInterface::Client as the mode parameter when
    constructing QHardwareInterface derived classes.

    Qt Extended automatically recognizes any subclass of QHardwareInterface as a Qt Extended hardware abstraction.
    Each abstraction follows the principal of client and provider split whereby
    the provider is a subclass of the client class. See the documentation for 
    QAbstractIpcInterface for more information on writing and using interface classes.

    The system supports multiple providers of the same
    interface type.  For example, a device could have a primary and secondary
    battery.  Each battery would be exposed as a separate instance of a
    QPowerSourceProvider derived class.  Default devices can be defined in
    the \c {Trolltech/HardwareAccessories.conf} configuration file.  The file
    contains a list of interface names and the identity of the default
    device, for example:

    \code
    [Defaults]
    QPowerSource = DefaultBattery
    QSignalSource = DefaultSignal
    QVibrateAccessory = Modem
    \endcode

    \sa QHardwareManager, QAbstractIpcInterface

    \ingroup hardware
*/

/*!
    Constructs a new QHardwareInterface object with interface type \a name,
    identity \a id and operates in \a mode.  The object is attached to
    \a parent.

    If \a id is empty the default device for \a name will be
    automatically selected.
*/
QHardwareInterface::QHardwareInterface( const QString& name,
                                        const QString& id,
                                        QObject* parent,
                                        QAbstractIpcInterface::Mode mode )
: QAbstractIpcInterface( HARDWAREINTERFACE_VALUEPATH,
                         name,
                         id,
                         parent,
                         mode )
{
    if ( mode == QAbstractIpcInterface::Server ) {
        QSettings defaults( "Trolltech", "HardwareAccessories" );
        defaults.beginGroup( "Defaults" );
        if ( defaults.value( name ) == id )
            setPriority( 1 );
        defaults.endGroup();
    }
}

/*!
    Destroys the hardware interface object.
*/
QHardwareInterface::~QHardwareInterface()
{
}
