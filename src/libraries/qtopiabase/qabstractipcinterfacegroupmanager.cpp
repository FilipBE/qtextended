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

#include <qabstractipcinterfacegroupmanager.h>
#include <qvaluespace.h>
#include <QMap>
#include <QSet>
#include <qdebug.h>

/*!
    \class QAbstractIpcInterfaceGroupManager
    \inpublicgroup QtBaseModule

    \brief The QAbstractIpcInterfaceGroupManager class provides a method to discover the active IPC interfaces and server interface groups.
    \ingroup ipc

    This class is used to query which IPC groups and interfaces are currently
    present in the system, using groups() and interfaces() respectively.
    It can also be used to receive notification of when IPC groups are
    added to or removed from the system, using groupAdded() and groupRemoved()
    respectively.

    Each server interface group has a unique name, such as \c{modem},
    \c{voip}, etc.  Within each group is a list of interfaces for
    functionality areas, which have names such as
    QNetworkRegistration, QSMSSender, etc.

    Interface names correspond to class names elsewhere in the Qt Extended API's.  To use an interface, the caller creates an instance of the
    corresponding class.  The caller can specify an explicit group name,
    if more than one group implements the same interface, or leave the group
    name empty to use the default group for that interface.

    See the documentation for QAbstractIpcInterface for more information
    on writing and using interface classes.

    \sa QAbstractIpcInterface, QAbstractIpcInterfaceGroup
    \ingroup ipc
*/

class QAbstractIpcInterfaceGroupManagerPrivate
{
public:
    QAbstractIpcInterfaceGroupManagerPrivate( const QString& location )
    {
        valueSpaceLocation = location;
        item = new QValueSpaceItem( location );
        cacheInitialized = false;
    }
    ~QAbstractIpcInterfaceGroupManagerPrivate()
    {
        delete item;
    }

    QString valueSpaceLocation;
    QValueSpaceItem *item;
    bool cacheInitialized;
    QSet<QString> groups;
    QMap<QString,QStringList> interfaces;
    QString interfaceFilter;

    void initCache()
    {
        if ( !cacheInitialized ) {
            cacheInitialized = true;
            rebuildCache();
        }
    }

    void rebuildCache();
};

void QAbstractIpcInterfaceGroupManagerPrivate::rebuildCache()
{
    groups.clear();
    interfaces.clear();

    if ( !interfaceFilter.isEmpty() ) {
        QStringList grps = item->subPaths();
        foreach ( QString group, grps ) {
            this->interfaces[group].append( interfaceFilter );
        }
        groups += QSet<QString>::fromList( grps );
    } else {
        QStringList interfaces = item->subPaths();
        foreach ( QString interface, interfaces ) {
            QValueSpaceItem iface( *item, interface );
            QStringList grps = iface.subPaths();
            foreach ( QString group, grps ) {
                this->interfaces[group].append( interface );
            }
            groups += QSet<QString>::fromList( grps );
        }
    }
}

/*!
    Construct a new server interface group manager and attach it to \a parent.

    The \a valueSpaceLocation parameter specifies the location in the
    value space to place all information about interfaces.
    Subclasses such as QCommServiceManager will set this to a particular
    value meeting their requirements.

    \sa QCommServiceManager
*/
QAbstractIpcInterfaceGroupManager::QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, QObject *parent )
    : QObject( parent )
{
    d = new QAbstractIpcInterfaceGroupManagerPrivate
        ( valueSpaceLocation + QLatin1String("/_channels") );
    connect( d->item, SIGNAL(contentsChanged()),
             this, SLOT(groupsChangedInner()) );
}

/*!
    Construct a new server interface group manager and attach it to \a parent.

    The \a valueSpaceLocation parameter specifies the location in the
    value space to place all information about interfaces.
    Subclasses such as QCommServiceManager will set this to a particular
    value meeting their requirements.

    In some use cases it might be only necessary to monitor the groups which support
    a particular interface. This constructor's additional \a interface 
    parameter acts as such a filter. The constructed object entirely ignores server interface 
    groups which support other interfaces but not the given one.
    
    \code
        QAbstractIpcInterfaceGroupManager* man = new QAbstractIpcInterfaceGroupManager( vsp, "QNetworkRegistration" );

        QStringList groupList = man->groups(); //all groups which support QNetworkRegistration

        QStringList ifaces = man->interfaces( "voip" );
        //ifaces is empty if VoIP doesn't support QNetworkRegistration 
        //otherwise has exactly one entry being QNetworkRegistration
    \endcode

    If \a interface is empty, this manager object will behave as if no filtering 
    is required.

    \sa QCommServiceManager, QHardwareManager
*/
QAbstractIpcInterfaceGroupManager::QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, const QString& interface, QObject* parent )
    : QObject( parent )
{
    if ( interface.isEmpty() ) {
        d = new QAbstractIpcInterfaceGroupManagerPrivate
            ( valueSpaceLocation + QLatin1String("/_channels") );
    } else {
        d = new QAbstractIpcInterfaceGroupManagerPrivate( 
                valueSpaceLocation + QLatin1String("/_channels/") + interface );
        d->interfaceFilter = interface;
    }
    connect( d->item, SIGNAL(contentsChanged()),
             this, SLOT(groupsChangedInner()) );
}


/*!
    Destroy this server interface group manager.
*/
QAbstractIpcInterfaceGroupManager::~QAbstractIpcInterfaceGroupManager()
{
    delete d;
}

/*!
    Returns the list of all groups that are currently active within the system or
    within the particular interface that this object could be limited to.
*/
QStringList QAbstractIpcInterfaceGroupManager::groups() const
{
    d->initCache();
    return d->groups.toList();
}

/*!
    Returns the list of interfaces that are supported by \a group.

    If this object is limited to just one interface this function only ever returns a list
    with either zero or at most one entry.
*/
QStringList QAbstractIpcInterfaceGroupManager::interfaces
        ( const QString& group ) const
{
    d->initCache();
    QMap<QString,QStringList>::ConstIterator it;
    it = d->interfaces.find( group );
    if ( it != d->interfaces.end() )
        return it.value();
    else
        return QStringList();
}

/*!
    \fn QStringList QAbstractIpcInterfaceGroupManager::supports<T>() const

    Returns the list of groups that support the interface \c{T}.  The following
    example demonstrates how to get the list of groups that supports the
    QNetworkRegistration interface:

    \code
    QAbstractIpcInterfaceGroupManager manager("/Communications");
    QStringList list = manager.supports<QNetworkRegistration>();
    \endcode
*/

/*!
    \fn bool QAbstractIpcInterfaceGroupManager::supports<T>( const QString& group ) const

    Determines if \a group supports the interface \c{T}.  The following
    example demonstrates how to determine if the \c modem group supports
    the QNetworkRegistration interface:

    \code
    QAbstractIpcInterfaceGroupManager manager("/Communications");
    if ( manager.supports<QNetworkRegistration>( "modem" ) )
        ...
    \endcode
*/

/*!
    \fn int QAbstractIpcInterfaceGroupManager::priority<T>( const QString& group ) const

    Returns the priority of the interface \c{T} within \a group.  The priority
    determines which group will be selected by default if an explicit
    group name is not supplied when constructing an interface object.

    Returns the default priority of zero if the interface or group name
    does not exist.

    See the documentation for QAbstractIpcInterface for more information on how
    priorities affect the choice of a default interface implementation.

    \sa QAbstractIpcInterface
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupsChanged()

    Signal that is emitted when the list of groups changes, if the interfaces
    on a group has changed, or if the priority assignments have changed.

    \sa groupAdded(), groupRemoved()
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupAdded( const QString& group )

    Signal that is emitted when \a group is added.  A group is considered
    to have been added when its first interface is constructed.
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupRemoved( const QString& group )

    Signal that is emitted when \a group is removed.  A group is
    considered to have been removed when its last interface is deleted.
*/

/*!
    \internal
*/
void QAbstractIpcInterfaceGroupManager::connectNotify( const char *signal )
{
    // Don't cache the group data until someone actually needs it.
    if ( QLatin1String( signal ) == SIGNAL(groupAdded(QString)) ||
         QLatin1String( signal ) == SIGNAL(groupRemoved(QString)) ) {
        d->initCache();
    }
    QObject::connectNotify( signal );
}

void QAbstractIpcInterfaceGroupManager::groupsChangedInner()
{
    if ( d->cacheInitialized ) {
        // Check for changes in the list of services.
        QSet<QString> current = d->groups;
        d->rebuildCache();
        QSet<QString> list = d->groups;
        foreach ( QString newGroup, list ) {
            if ( !current.contains( newGroup ) ) {
                emit groupAdded( newGroup );
            }
        }
        foreach ( QString oldGroup, current ) {
            if ( !list.contains( oldGroup ) ) {
                emit groupRemoved( oldGroup );
            }
        }
    }
    emit groupsChanged();
}

QStringList QAbstractIpcInterfaceGroupManager::supports( const QString& iface ) const
{
    if ( d->interfaceFilter.isEmpty() ) {
        QValueSpaceItem item( *d->item, iface );
        return item.subPaths();
    } else {
        if ( iface != d->interfaceFilter )
            return QStringList();
        return d->item->subPaths();
    }
}

bool QAbstractIpcInterfaceGroupManager::supports
        ( const QString& group, const QString& iface ) const
{
    return supports( iface ).contains( group );
}

int QAbstractIpcInterfaceGroupManager::priority
    ( const QString& group, const QString& iface ) const
{
    if ( d->interfaceFilter.isEmpty() ) 
        return d->item->value( iface + "/" + group + "/priority", (int)0 ).toInt();
    else if ( iface == d->interfaceFilter ) 
        return d->item->value( group + QLatin1String("/priority"), (int)0 ).toInt();
    return 0;
}
