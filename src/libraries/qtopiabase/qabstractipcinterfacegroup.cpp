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

#include <qabstractipcinterfacegroup.h>
#include <qmap.h>
#include <QValueSpaceObject>

/*!
    \class QAbstractIpcInterfaceGroup
    \inpublicgroup QtBaseModule

    \brief The QAbstractIpcInterfaceGroup class provides a convenient wrapper to create the interfaces within a server group.
    \ingroup ipc

    This class is used to collect together several related QAbstractIpcInterface
    instances into a single logical group.  It provides a standard pattern for
    creating all interfaces in the group via the initialize() method, and provides
    discovery mechanisms to allow one interface in a group to quickly locate
    another via the interface() method.

    The most extensive use of this class is for Qt Extended telephony services, which
    create a large number of related interfaces for network registration, phone
    call management, call forwarding settings, SIM file contents, and so on.
    See QTelephonyService for more information.

    It isn't necessary that QAbstractIpcInterface instances be part of a group;
    they can be standalone.

    The QAbstractIpcInterfaceGroupManager class can be used to receive run time
    notification of when groups are added and removed from the system.

    \sa QAbstractIpcInterface, QAbstractIpcInterfaceGroupManager, QTelephonyService
    \ingroup ipc
*/

class QAbstractIpcInterfaceGroupPrivate
{
public:
    QAbstractIpcInterfaceGroupPrivate( const QString& groupName )
    {
        this->groupName = groupName;
    }

    QString groupName;
    QMap<QString, QAbstractIpcInterface *> interfaces;
    QStringList suppressed;
};

/*!
    Create a new server interface group called \a groupName and attach
    it to \a parent.

    A call to the constructor should be followed by a call to
    initialize() to complete the initialization process.

    \sa initialize()
*/
QAbstractIpcInterfaceGroup::QAbstractIpcInterfaceGroup
        ( const QString& groupName, QObject *parent )
    : QObject( parent )
{
    d = new QAbstractIpcInterfaceGroupPrivate( groupName );
}

/*!
    Destroy this server interface group.
*/
QAbstractIpcInterfaceGroup::~QAbstractIpcInterfaceGroup()
{
    delete d;
}

/*!
    Returns the name of this server interface group.
*/
QString QAbstractIpcInterfaceGroup::groupName() const
{
    return d->groupName;
}

/*!
    Initializes this server interface group by creating all of the interfaces
    that it supports.  In subclasses, interfaces should be created using
    the following pattern:

    \code
    if ( !supports<T>() )
        addInterface( new MyT(this) );
    \endcode

    where \c{T} is the name of the interface being created and \c{MyT}
    is the name of the class that implements the interface.

    At the end of a subclass initialize(), it must call its immediate
    superclass.  The superclass follows the same pattern to create
    missing interfaces that were not explicitly overridden.

    The base implementation calls QAbstractIpcInterface::groupInitialized()
    on all of the interfaces that were created by subclasses.

    \sa supports(), addInterface(), QAbstractIpcInterface::groupInitialized()
*/
void QAbstractIpcInterfaceGroup::initialize()
{
    // Notify all interfaces that the group has been initialized.
    QMap<QString, QAbstractIpcInterface *>::ConstIterator iter;
    for ( iter = d->interfaces.begin(); iter != d->interfaces.end(); ++iter ) {
        iter.value()->groupInitialized( this );
    }

    // Flush all of the changes to the value space so this
    // server interface group becomes visible to clients.
    QValueSpaceObject::sync();
}

/*!
    \fn bool QAbstractIpcInterfaceGroup::supports<T>() const

    Returns true if this server interface group supports an interface with
    the name \c{T}; otherwise returns false.
*/

/*!
    \fn T *QAbstractIpcInterfaceGroup::interface<T>() const

    Get the object that implements the interface \c{T} on this
    server interface group, or null if there is no such interface.
*/

/*!
    Adds \a interface to the list of interfaces that is supported by
    this server interface group.
*/
void QAbstractIpcInterfaceGroup::addInterface
        ( QAbstractIpcInterface *interface )
{
    if ( interface ) {
        QString name = interface->interfaceName();
        interface->setObjectName( name );
        d->interfaces.insert( name, interface );
        connect( interface, SIGNAL(destroyed()),
                 this, SLOT(interfaceDestroyed()) );
    }
}

/*!
    \fn void QAbstractIpcInterfaceGroup::suppressInterface<T>()

    Suppresses the interface \c{T} from being created during initialize().
    This causes the supports() function to return true for \c{T} even
    if no interface object for \c{T} has been added yet.

    The main use of this method is to stop a parent class from creating a default
    implementation of \c{T} when the subclass does not create their own override,
    but the default implementation would cause problems if used.
*/

void QAbstractIpcInterfaceGroup::interfaceDestroyed()
{
    d->interfaces.remove( sender()->objectName() );
}

bool QAbstractIpcInterfaceGroup::_supports( const char *interfaceName ) const
{
    QString name = QString( interfaceName );
    if ( d->suppressed.contains( name ) )
        return true;
    return d->interfaces.contains( name );
}

QAbstractIpcInterface *QAbstractIpcInterfaceGroup::_interface
        ( const char *interfaceName ) const
{
    QMap<QString, QAbstractIpcInterface *>::ConstIterator iter;
    iter = d->interfaces.find( QString( interfaceName ) );
    if ( iter != d->interfaces.end() )
        return iter.value();
    else
        return 0;
}

void QAbstractIpcInterfaceGroup::_suppressInterface
        ( const char *interfaceName )
{
    d->suppressed += QString( interfaceName );
}
