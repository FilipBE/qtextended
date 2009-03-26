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

#include "qvaluespace.h"
#include "qmallocpool.h"
#include <strings.h>
#include <QByteArray>
#include <QObject>
#include <QMap>
#include <QPair>
#include <QCoreApplication>
#include <QThread>
#include <QSet>
#include <QString>
#include <QVarLengthArray>
#include <qpacketprotocol.h>

#define VS_CALL_ASSERT Q_ASSERT(!QCoreApplication::instance() || QCoreApplication::instance()->thread() == QThread::currentThread());

///////////////////////////////////////////////////////////////////////////////
// Layer plug-in interface documentation
///////////////////////////////////////////////////////////////////////////////

/*!
  \class IValueSpaceLayer
    \inpublicgroup QtBaseModule
  \internal
  \ingroup ipc
  \brief The IValueSpaceLayer class provides support for adding new logical data
  layers to the Qt Extended Value Space.
  
  For an overview of the Qt Extended Value Space, please see the \l ValueSpace 
  documentation.
  */

/*!
  \typedef IValueSpaceLayer::HANDLE

  The HANDLE type is an opaque, pointer sized contextual handle used to
  represent paths within value space layers.  HANDLES are only ever created by
  IValueSpaceLayer::item() and are always released by calls to
  IValueSpaceLayer::remHandle().  The special, \c {InvalidHandle} is reserved to
  represent an invalid handle.
 */

/*!
  \enum IValueSpaceLayer::Properties

  To allow for efficient layer implementations, expensive handle operations,
  currently only monitoring for changes, are enabled and disabled as needed on
  a per-handle basis.  The Properties enumeration is a bitmask representing
  the different properties that can exist on a handle.

  \value Publish Enable change notification for the handle.  When set, the layer
         should emit IValueSpaceLayer::handleChanged() signals when appropriate
         for the handle.
 */

/*!
  \enum IValueSpaceLayer::Type

  Value Space layers are initialized in either a "Server" or a "Client"
  context.  There is only a single server in the value space architecture, and
  its layers are always initialized before any clients.  This distinction allows
  layers to implement Client/Server architecture \i {if required}.  If not,
  layers are free to treat Server and Client contexts identically.

  \value Server The layer is being initialized in the "server" context.
  \value Client The layer is being initialized in the "client" context.
 */

/*!
  \fn QString IValueSpaceLayer::name()

  Returns the name of the Value Space layer.  This name is only used for
  diagnostics purposes.
 */

/*!
  \fn bool IValueSpaceLayer::startup(Type type)

  Called by the Value Space system to initialize each layer.  The \a type
  parameter will be set accordingly, and layer implementors can use this to
  implement a client/server architecture if desired.
  Returns true upon success; otherwise returns false.
 */

/*!
  \fn bool IValueSpaceLayer::restart()

  Called by the Value Space system to restart each layer.
  Returns true upon success; otherwise returns false.
 */

/*!
  \fn void IValueSpaceLayer::shutdown()

  Called by the Value Space system to uninitialize each layer.  The shutdown
  call can be used to release any resources in use by the layer.
 */

/*!
  \fn QUuid IValueSpaceLayer::id()

  Return a globally unique id for the layer.  This id is used to break ordering
  ties.
 */

/*!
  \fn unsigned int IValueSpaceLayer::order()

  Return the position in the Value Space layer stack that this layer should
  reside.  Higher numbers mean the layer has a higher precedence and its values
  will "shadow" those below it.  If two layers specify the same ordering, the
  IValueSpaceLayer::id() value is used to break the tie.
 */

/*!
  \fn bool IValueSpaceLayer::value(HANDLE handle, QVariant *data)

  Returns the value for a particular \a handle.  If a value is available, the
  layer will set \a data and return true.  If no value is available, false is
  returned.
  */

/*!
  \fn bool IValueSpaceLayer::value(HANDLE handle, const QByteArray &subPath, QVariant *data)

  Returns the value for a particular \a subPath of \a handle.  If a value is
  available, the layer will set \a data and return true.  If no value is
  available, false is returned.
  */

/*!
  \fn bool IValueSpaceLayer::remove(HANDLE handle)

  Process a client side QValueSpaceItem::remove() for the specified \a handle.
  Return true on success and false on failure.
 */

/*!
  \fn bool IValueSpaceLayer::remove(HANDLE handle, const QByteArray &subPath)

  Process a client side QValueSpaceItem::remove() for the specified \a subPath
  of \a handle.  Return true on success and false on failure.
 */

/*!
  \fn bool IValueSpaceLayer::setValue(HANDLE handle, const QVariant &value)

  Process a client side QValueSpaceItem::setValue() for the specified \a handle
  and \a value.  Return true on success and false on failure.
  */

/*!
  \fn bool IValueSpaceLayer::setValue(HANDLE handle, const QByteArray &subPath, const QVariant &value)

  Process a client side QValueSpaceItem::setValue() for the specified \a subPath
  of \a handle and the provided \a value.  Return true on success and false on
  failure.
  */

/*!
  \fn bool IValueSpaceLayer::syncChanges()

  Commit any changes made through setValue().  Return true on success and false
  on failure.
  */

/*!
  \fn QSet<QByteArray> IValueSpaceLayer::children(HANDLE handle)

  Returns the set of children of \a handle.  For example, in a layer providing
  the following items:

  \code
  /Device/Configuration/Applications/FocusedApplication
  /Device/Configuration/Buttons/PrimaryInput
  /Device/Configuration/Name
  \endcode

  a request for children of "/Device/Configuration" will return
  { "Applications", "Buttons", "Name" }.
 */

/*!
  \fn HANDLE IValueSpaceLayer::item(HANDLE parent, const QByteArray &subPath)

  Returns a new opaque handle for the requested \a subPath of \a parent.  If
  \a parent is an InvalidHandle, \a subPath is an absolute path.
  */

/*!
  \fn void IValueSpaceLayer::setProperty(HANDLE handle, Properties property)

  Apply the specified \a property mask to \a handle.
 */

/*!
  \fn void IValueSpaceLayer::remHandle(HANDLE handle)

  Releases a \a handle previously returned from IValueSpaceLayer::item().
 */

/*!
  \fn void IValueSpaceLayer::handleChanged(unsigned int handle)

  Emitted whenever the \a handle's value, or any sub value, changes.
  */

///////////////////////////////////////////////////////////////////////////////
// declare QValueSpaceManager
///////////////////////////////////////////////////////////////////////////////
class QValueSpaceManager
{
public:
    QValueSpaceManager();

    void initServer();
    void init();
    void reinit();
    bool isServer() const;

    void install(IValueSpaceLayer * layer);
    void install(QValueSpace::LayerCreateFunc func);
    QList<IValueSpaceLayer *> const & getLayers();

private:
    void commonInit(IValueSpaceLayer::Type);
    void commonReinit();
    bool initLayer(IValueSpaceLayer* layer);
    bool reinitLayer(IValueSpaceLayer* layer);

    enum { Uninit, Server, Client } type;
    QList<IValueSpaceLayer *> layers;
    QList<QValueSpace::LayerCreateFunc> funcs;
};

Q_GLOBAL_STATIC(QValueSpaceManager, valueSpaceManager);

///////////////////////////////////////////////////////////////////////////////
// define QValueSpace
///////////////////////////////////////////////////////////////////////////////
/*!
  \namespace QValueSpace
  \ingroup ipc
  \inpublicgroup QtBaseModule

  \brief The QValueSpace namespace provides methods that are useful to Value
  Space layer implementors.

  Value Space layers that are available at link time can be automatically
  installed using \c {QVALUESPACE_AUTO_INSTALL_LAYER(name)} macro.  The method
  \c {IValueSpaceLayer * name::instance()} must exist and return a pointer to
  the layer to install.  The \c {QVALUESPACE_AUTO_INSTALL_LAYER(name)} macro
  will only invoke this method \i {after} QApplication has been constructed,
  making it safe to use any Qt class in your layer's constructor.
 */

/*!
  \typedef QValueSpace::LayerCreateFunc
  \internal
  */
/*!
  \class QValueSpace::AutoInstall
    \inpublicgroup QtBaseModule

  \internal
  */
/*!
  Initialize the value space.  This method only needs to be called by the value
  space manager process, and should be called before any process in the system
  uses a value space class.
 */
void QValueSpace::initValuespaceManager()
{
    valueSpaceManager()->initServer();
}

/*!
  Initialize the value space.  This method only needs to be called by layer
  implementers to force initialization of the value space.
  */
void QValueSpace::initValuespace()
{
    valueSpaceManager()->init();
}

/*!
  Re-initialize the value space.  This method only needs to be called by layer
  implementers to force re-initialization of the value space.
  */
void QValueSpace::reinitValuespace()
{
    valueSpaceManager()->reinit();
}

/*!
  Used by value space layer implementations to install themselves into the
  system.  \a layer should be a pointer to the layer to install.
  */
void QValueSpace::installLayer(IValueSpaceLayer * layer)
{
    valueSpaceManager()->install(layer);
}

/*!
  \internal
  */
void QValueSpace::installLayer(LayerCreateFunc func)
{
    valueSpaceManager()->install(func);
}

///////////////////////////////////////////////////////////////////////////////
// define QValueSpaceManager
///////////////////////////////////////////////////////////////////////////////
QValueSpaceManager::QValueSpaceManager()
: type(Uninit)
{
}

void QValueSpaceManager::initServer()
{
    Q_ASSERT(Uninit == type);

    commonInit(IValueSpaceLayer::Server);
}

void QValueSpaceManager::init()
{
    if(Uninit != type)
        return; // Already initialized

    commonInit(IValueSpaceLayer::Client);
}

void QValueSpaceManager::reinit()
{
    if(Uninit == type)
        return; // Not already initialized

    commonReinit();
}

void QValueSpaceManager::commonInit(IValueSpaceLayer::Type vsltype)
{
    Q_ASSERT(Uninit == type);

    // Install all the dormant layers
    for(int ii = 0; ii < funcs.count(); ++ii)
        install(funcs[ii]());
    funcs.clear();

    type = (vsltype == IValueSpaceLayer::Server)?Server:Client;

    for(int ii = 0; ii < layers.count(); ++ii) {
        if(!initLayer(layers.at(ii))) {
            layers.removeAt(ii);
            --ii;
        }
    }
}

void QValueSpaceManager::commonReinit()
{
    Q_ASSERT(Uninit != type);

    for(int ii = 0; ii < layers.count(); ++ii) {
        if(!reinitLayer(layers.at(ii))) {
            layers.removeAt(ii);
            --ii;
        }
    }
}

bool QValueSpaceManager::isServer() const
{
    return (Server == type);
}

void QValueSpaceManager::install(IValueSpaceLayer * layer)
{
    Q_ASSERT(Uninit == type);
    Q_ASSERT(layer);
    unsigned int cOrder = layer->order();
    int inserted = -1;
    for(int ii = 0; !inserted && ii < layers.count(); ++ii) {
        unsigned int lOrder = layers.at(ii)->order();
        Q_ASSERT(layer != layers.at(ii));
        if(lOrder < cOrder) {
            // Do nothing
        } else if(lOrder == cOrder) {
            if(layers.at(ii)->id() > layer->id()) {
                layers.insert(ii, layer);
                inserted = ii;
            }
        } else if(lOrder > cOrder) {
            layers.insert(ii, layer);
            inserted = ii;
        }
    }

    if(-1 == inserted) {
        inserted = layers.count();
        layers.append(layer);
    }
}

void QValueSpaceManager::install(QValueSpace::LayerCreateFunc func)
{
    Q_ASSERT(Uninit == type);
    funcs.append(func);
}

QList<IValueSpaceLayer *> const & QValueSpaceManager::getLayers()
{
    init(); // Fallback init

    return layers;
}

bool QValueSpaceManager::initLayer(IValueSpaceLayer* layer)
{
    Q_ASSERT(Uninit != type);

    return layer->startup((type==Client)?IValueSpaceLayer::Client:
                                         IValueSpaceLayer::Server);
}

bool QValueSpaceManager::reinitLayer(IValueSpaceLayer* layer)
{
    Q_ASSERT(Uninit != type);

    return layer->restart();
}

///////////////////////////////////////////////////////////////////////////////
// define QValueSpaceItem
///////////////////////////////////////////////////////////////////////////////
class QValueSpaceItemPrivateProxy : public QObject
{
Q_OBJECT
signals:
    void changed();

public slots:
    void handleChanged(unsigned int handle)
    {
        for(int ii = 0; ii < readers.count(); ++ii)
            if(readers.at(ii).second == handle) {
                emit changed();
                return;
            }
    }

    void objDestroyed()
    {
        connections.remove((QValueSpaceItem *)sender());
    }

public:
    QList<QPair<IValueSpaceLayer *, IValueSpaceLayer::HANDLE> > readers;
    QHash<QValueSpaceItem *,int> connections;
};

struct QValueSpaceItemPrivate
{
    enum Type { Data, Write };
    QValueSpaceItemPrivate(Type _t) : type(_t) {}

    Type type;
};

struct QValueSpaceItemPrivateData : public QValueSpaceItemPrivate
{
    QValueSpaceItemPrivateData(const QByteArray &_path)
        : QValueSpaceItemPrivate(Data), refCount(0), connections(0)
    {
        if(*_path.constData() != '/')
            path.append("/");
        path.append(_path);
        if(path.length() != 1 && '/' == *(path.constData() + path.length() - 1))
            path.truncate(path.length() - 1);

        QValueSpaceManager * man = valueSpaceManager();
        if(!man)
            return;

        const QList<IValueSpaceLayer *> & readerList = man->getLayers();

        for(int ii = 0; ii < readerList.count(); ++ii) {
            IValueSpaceLayer * read = readerList.at(ii);
            if(!read) continue;

            IValueSpaceLayer::HANDLE handle = read->item(IValueSpaceLayer::InvalidHandle, path);
            if(IValueSpaceLayer::InvalidHandle != handle) {
                readers.append(qMakePair(read, handle));
            }
        }
    }

    ~QValueSpaceItemPrivateData()
    {
        for(int ii = 0; ii < readers.count(); ++ii)
            readers[ii].first->remHandle(readers[ii].second);

        if(connections)
            delete connections;

    }

    void connect(QValueSpaceItem * space)
    {
        if(!connections) {
            connections = new QValueSpaceItemPrivateProxy;
            connections->readers = readers;
            connections->connections.insert(space,1);
            QObject::connect(space, SIGNAL(destroyed(QObject*)),
                             connections, SLOT(objDestroyed()));
            QObject::connect(connections, SIGNAL(changed()),
                             space, SIGNAL(contentsChanged()));
            for(int ii = 0; ii < readers.count(); ++ii) {
                readers.at(ii).first->setProperty(readers.at(ii).second, IValueSpaceLayer::Publish);
                QObject::connect(readers.at(ii).first, SIGNAL(handleChanged(uint)), connections, SLOT(handleChanged(uint)));
            }
        } else if(!connections->connections.contains(space)) {
            connections->connections[space] = 1;

            QObject::connect(space, SIGNAL(destroyed(QObject*)),
                    connections, SLOT(objDestroyed()));
            QObject::connect(connections, SIGNAL(changed()),
                    space, SIGNAL(contentsChanged()));
        } else {
            ++connections->connections[space];
        }
    }

    bool disconnect(QValueSpaceItem * space)
    {
        if(connections) {
            QHash<QValueSpaceItem *, int>::Iterator iter =
                connections->connections.find(space);
            if(iter != connections->connections.end()) {
                --(*iter);
                if(!*iter) {
                    QObject::disconnect(space, SIGNAL(destroyed(QObject*)),
                                        connections, SLOT(objDestroyed()));
                    QObject::disconnect(connections, SIGNAL(changed()),
                                        space, SIGNAL(contentsChanged()));
                    connections->connections.erase(iter);
                }
                return true;
            }
        }
        return false;
    }

    void AddRef()
    {
        ++refCount;
    }

    void Release()
    {
        Q_ASSERT(refCount);
        --refCount;
        if(!refCount)
            delete this;
    }

    unsigned int refCount;
    QByteArray path;
    QList<QPair<IValueSpaceLayer *, IValueSpaceLayer::HANDLE> > readers;
    QValueSpaceItemPrivateProxy * connections;
};

struct QValueSpaceItemPrivateWrite : public QValueSpaceItemPrivate
{
    QValueSpaceItemPrivateWrite(const QValueSpaceItemPrivateWrite &other)
        : QValueSpaceItemPrivate(other.type), data(other.data), ops(other.ops)
        {}
    QValueSpaceItemPrivateWrite() : QValueSpaceItemPrivate(Write), data(0) {}
    QValueSpaceItemPrivateData * data;

    struct Op {
        enum Type { Set, Remove };
        Type type;
        QByteArray path;
        QVariant value;
        Op(const QByteArray &p, const QVariant &v)
            : type(Set), path(p), value(v) {}
        Op(const QByteArray &p)
            : type(Remove), path(p) {}
    };
    QList<Op> ops;
};

/*!
  \page qtopiavaluespace.html
  \title Qt Extended Value Space
  \brief The Qt Extended Value Space allows inter-process publication of hierarchical
  data.

  \section1 Overview

  The Qt Extended Value Space unifies various sources of hierarchical data into a
  single consistent model.  Conceptually the Value Space is a hierarchical tree
  of which each node or leaf can optionally contain a QVariant value.  A
  serialized version of a simple example Value Space might look like this.

  \code
  /Device/Buttons = 3
  /Device/Buttons/1/Name = Context
  /Device/Buttons/1/Usable = true
  /Device/Buttons/2/Name = Select
  /Device/Buttons/2/Usable = false
  /Device/Buttons/3/Name = Back
  /Device/Buttons/3/Usable = true
  \endcode

  Programmers access the Value Space through the QValueSpaceItem class.  This
  class allows applications to read item values, navigate through and subscribe
  to change notifications for items within the space.

  Nodes in the Value Space can be thought of as representing schema objects.
  Obviously this is a conceptual differentiation and not a physical one, as
  internally the Value Space is treated as one large tree.  By applying
  structured schema to the space "explorability" is increased.  For example,
  the \c {/Device/Buttons} schema can be defined as containing a value
  representing the number of mappable buttons on a device, and a sub-item for
  each adhering to the \c {MappableButton} schema.  The \c {MappableButton}
  schema itself may be defined as containing two attributes \c {Name} and
  \c {Usable}.  Change notification is modeled in this fashion also.  Where the
  \c {/Device/Buttons/1/Name} item is to change, the \c {/Device/Buttons/1} item
  would be marked as changed, and so on up the tree.  This allows, for example,
  subscription to just \c {/Device/Buttons} to be notified when anything
  "button" related changes.

  Internally, the Value Space consists of an arbitrary number of data source
  trees, or layers, which are stacked on top of each other to form the final
  unified view.  If a "higher" layer contains an item, it shadows the value of
  items in the layers below it.  The two most important Value Space layers are:

  \list 1
  \i {The Application Object layer}

     The Application Object layer allows applications to add and remove
     "transient" data from the Value Space.  Applications access the Application
     Object layer through the QValueSpaceObject class.

  \i {The INI layer}

     The INI layer maps regular INI files into the Value Space.  This mapping
     allows values stored in configuration files to be used interchangably with
     other values in the Value Space.  The INI layer is described in the
     architecture section below.

  \endlist

  Consider the Value Space item \c {/Device/Buttons}.  If both the Application
  Object layer and the INI layer contained this item, the value in the
  Application Object layer would shadow that from the INI layer.  However, if
  only the INI layer contained this item, it would be visible through the
  QValueSpaceItem class, even if the Application Object layer contained
  sub-items such as \c {/Device/Buttons/1}.  That is, layer shadowing occurs by
  value not by path.

  \section1 Architecture

  Layers in the Qt Extended Value Space are provided by objects implementing the
  IValueSpaceLayer interface.  The two internal layers, the Application Object
  layer and the INI layer, are implemented in this fashion.  The Value Space
  supports adding new, 3rd party layers through calls to the
  QValueSpace::installLayer() method.  All layers must be installed prior to the
  first Value Space usage.  Although it is legal to have a different set of
  layers installed in each process (a process local layer, for example) this
  asymmetric model is discouraged as it might cause confusion if these two
  processes communicate.

  The Qt Extended Value Space system is divided into two parts: a single server and
  zero or more clients.  Internally the Value Space system draws no distinction
  between the two - the server is also a client, for example - but certain
  layers, in particular the Application Object layer, do.  The Value Space
  server \bold {must} be initialized via a call to
  QValueSpace::initValuespaceManager() prior to any use of the Value Space.
  Value Space clients will automatically initialize the first time Value Space
  functionionality is used.

  \section2 Application Object layer

  The external use of the Application Object layer is described in
  QValueSpaceObject.  The Application Object layer stores all values in a
  10MB block of shared memory which is reserved when the Value Space
  initializes.  As the layer creates this region at startup, it is assumed that
  the operating system lazily commits memory.  If this assumption is invalid,
  the Application Object layer will unnecessarily consume 10MB of memory.

  Value Space clients read from the Application Object layer's shared memory
  region directly.  A kernel lock is acquired for each read to prevent
  corruption.  While the layer supports concurrent readers, it is possible that
  a faulty or malicious application could acquire and refuse to release this
  lock causing any layer updates to be delayed indefinately.

  Only the Value Space server ever writes to the shared memory region.  When
  clients attempt to add items to the layer, their changes are transmitted via
  the \c {/tmp/qtopia-N/valuespace_applayer} Unix domain socket to the server where the
  update is performed.  Updates are batched in-process and sent when the process
  re-enters the Qt event loop.  Transmission and synchronization of changes can
  be forced manually by the QValueSpaceObject::sync() call, although as this
  requires a round trip between the client and server, doing so frequently may
  significantly degrade performance.

  Change notifications are transmitted to clients in the form of "something has
  changed" messages.  Nodes within the shared memory region are versioned, which
  allows clients to quickly determine exactly what has changed without the need
  for a bulkier change notification protocol.

  \section2 INI layer

  The INI layer maps INI files from disk into the Value Space.  The INI layer
  supports arbitrary mappings, fallback paths and partial change notification.

  As the unified path structure of the Value Space doesn't allow the INI layer
  to transparently determine which INI file to access off disk, a INI layer
  configuration file (an INI file itself) is used to dictate how
  INI files are located.  The INI file identified by
  \c {Trolltech/IniValueSpace} using the QSettings resolution rules is used for
  this purpose.

  The general form of the INI layer configuration file is:

  \code
  [General]
  Translations=<Directory to INI translations>
  LanguageItem=<Value Space Item for current language>
  Mappings=<Number of mappings that follow>

  [Mapping<x>]
  ValueSpacePath=(Required)
  FileSystemPath=(One of FileSystemPath or FileSystemPaths required)
  FileSystemPaths=(One of FileSystemPath or FileSystemPaths required)
  FileSystemPath<x>=(Required if FileSystemPaths specified)
  FileSystemExtension=(Optional.  Required if DirectoryDepth is specified)
  DirectoryDepth=(Optional.  Only allowed if FileSystemExtension specified)
  \endcode

  The \c {General/Mappings} key simply specifies the number of mappings that
  follow, which are grouped as \c {Mapping0} - \c {Mapping<n>}.  Each mapping
  consists of a single required field, \c {ValueSpacePath}, and a number of
  optional and interdependant keys.

  The {ValueSpacePath} key specifies the point of mapping in the value space.
  This is called a "terminal" point.  There may be only a single mapping for
  each distinct terminal point.  Two \c {/Device} mappings are not allowed, but
  a \c {/Device} and a \c {/Device/Buttons} mapping is fine.  Each terminal
  mapping is either a "depth mapping" or a "file mapping".

  File mappings are identified by the lack of the \c {FileSystemExtension} and
  \c {DirectoryDepth} keys.  File mappings map a single INI file to a single
  Value Space node.  For example, in following mapping

  \code
  [Mapping0]
  ValueSpacePath=/Device/Buttons
  FileSystemPath=/opt/Qtopia/etc/defaultbuttons.conf
  \endcode

  if the \c {/opt/Qtopia/etc/defaultbuttons.conf} file contained a
  \c {Mode/Type} key, the corresponding Value Space item
  \c {/Device/Buttons/Mode/Type} would exist.

  Using the FileSystemPaths list, fallback file mappings may be created.  For
  example

  \code
  [Mapping0]
  ValueSpacePath=/Device/Buttons
  FileSystemPaths=2
  FileSystemPath0=/tmp/qtembedded-0/defaultbuttons.conf
  FileSystemPath1=/opt/Qtopia/etc/defaultbuttons.conf
  \endcode

  will first attempt to map \c {/tmp/qtembedded-0/defaultbuttons.conf} and then, if this
  doesn't exist, \c {/opt/Qtopia/etc/defaultbuttons.conf}.  Fallback mappings
  completely obscure each other.  That is, the two files
  \c {/tmp/qtembedded-0/defaultbuttons.conf} and
  \c {/opt/Qtopia/etc/defaultbuttons.conf} are not unified, but whichever exists
  is used and the other ignored.  Fallbacks are monitored for change, so if the
  \c {/tmp/qtembedded-0/defaultbuttons.conf} file is created sometime later the INI
  layer will update accordingly.

  Depth mappings allow groups of ini files to be mapped into the Value Space
  dynamically.  Depth mappings are identified by the presence of the
  \c {FileExtension} key.  In the case of depth mappings, the
  \c {ValueSpacePath} point specifies the beginning of the mapping.  The next
  \c {DirectoryDepth} sub-paths are treated as directory specifiers, and the
  subsequent sub-path a file name with the \c {FileSystemExtension} extension.
  For example

  \code
  [Mapping0]
  ValueSpacePath=/Applications
  FileSystemPath=/opt/Qtopia/apps
  FileSystemExtension=desktop
  DirectoryDepth=1

  [Mapping1]
  ValueSpacePath=/GamesApplications
  FileSystemPath=/opt/Qtopia/apps/Games
  FileSystemExtension=desktop
  DirectoryDepth=0
  \endcode

  creates two mappings.  In the above both the Value Space items
  \c {/Applications/Games/parashoot} and \c {/GamesApplications/parashoot} map
  to the \c {/opt/Qtopia/apps/Games/parashoot.desktop} INI file.  Thus if this
  file contained \c {Desktop Entry/Name}, the corresponding Value Space paths
  \c {/Applications/Games/parashoot/Desktop Entry/Name} and
  \c {/GamesApplications/parashoot/Desktop Entry/Name} would also exist.

  As with file mappings, depth mapping support fallback paths.  For example

  \code
  [Mapping0]
  ValueSpacePath=/Settings
  FileSystemPaths=2
  FileSystemPath0=/home/username/.config
  FileSystemPath1=/etc/
  FileSystemExtension=conf
  DirectoryDepth=1
  \endcode

  will map \c {/Settings/Trolltech/qpe} first to
  \c {/home/username/.config/Trolltech/qpe.conf} and then to
  \c {/etc/Trolltech/qpe.conf}.  Like file mappings, the fallback paths are
  monitored in case the file is later created.

  Keys within an INI file can be marked as translatable by appending the "[]"
  token to them.  For example, the following file contains one translatable
  key, \c {/Example/Translatable} and one non-translatable key,
  \c {/Example/NonTranslatable}.

  \code
  [Translation]
  File=ExampleTranslation
  Context=ExampleContext

  [Example]
  Translatable[]=Translatable Value
  NonTranslatable=Non-Translatable Value
  \endcode

  In both cases the "[]" token is omitted from the key name when accessed
  through the QValueSpaceItem class.

  INI translation files must be stored under a directory structure rooted at
  the \c {General/Translations} directory provided in the INI layer's
  configuration file.  If this configuration entry is omitted, translation is
  disabled.  To provide accurate translations, the INI layer also needs to know
  the current system language.  This information is accesses through the Value
  Space itself, from the item path specified by \c {General/LanguageItem}.
  Together the translations directory, and the current system language are
  combined to form the root under which the INI layer looks for translations
  files.

  INI files that include translatable keys, must also include the special
  \c {Translation} INI group that dictates how the INI layer locates
  translations for the key's value.  The \c {Translation/File} key specifies the
  file to open under the translations root directory discussed above, and the
  \c {Translation/Context} key the Qt translation context to use within that
  file.  The INI layer supports change notifications on language change.

  Currently the INI layer only supports partial change notification.  Change
  notifications will occur for keys that exist within mapped INI files, but
  these notifications will not be propagated up the Value Space tree.  For
  example with the previous mapping, should the \c {qpe.conf} file change,

  \code
  // Will emit QValueSpaceItem::contentsChanged()
  QValueSpaceItem item("/Settings/Trolltech/qpe/Desktop Entry/Name");

  // Will emit QValueSpaceItem::contentsChanged()
  QValueSpaceItem item2("/Settings/Trolltech/qpe");

  // Will NOT emit QValueSpaceItem::contentsChanged()
  QValueSpaceItem item3("/Settings/Trolltech");
  \endcode.

  This limitation may be removed in future versions.
 */

/*!
  \class QValueSpaceItem
    \inpublicgroup QtBaseModule

  \brief The QValueSpaceItem class allows access to Value Space items.
  \ingroup ipc

  The Value Space is an inter-application hierarchy of readable, writable and
  subscribable data.  The QValueSpaceItem class allows applications to read
  and subscribe to this data. 

  Conceptually, the Value Space is a hierarchical tree of which each item can 
  optionally contain a QVariant value and sub-items.  A serialized version of a 
  simple example might look like this.

  \code
  /Device/Buttons = 3
  /Device/Buttons/1/Name = Context
  /Device/Buttons/1/Usable = true
  /Device/Buttons/2/Name = Select
  /Device/Buttons/2/Usable = false
  /Device/Buttons/3/Name = Back
  /Device/Buttons/3/Usable = true
  \endcode

  Any application in Qt Extended can read values from the Value Space, or be notified
  asynchronously when they change using the QValueSpaceItem class.  

  Items in the Value Space can be thought of as representing "objects" adhering
  to a well known schema.  This is a conceptual differentiation, not a physical
  one, as internally the Value Space is treated as one large tree.  In the 
  sample above, the \c {/Device/Buttons} schema can be defined as containing a
  value representing the number of mappable buttons on a device, and a sub-item
  for each.  Likewise, the sub-item object schema contains two attributes - 
  \c {Name} and \c {Usable}.  
  
  Applications may use the QValueSpaceObject class to create a schema object 
  within the Value Space.  Objects remain in the Value Space as long as the
  QValueSpaceObject instance exists - that is, they are not persistant.  If
  the object is destroyed, or the application containing it exits (or crashes)
  the items are removed.

  Change notification is modelled in a similar way.  Applications subscribe to
  notifications at a particular object (ie. item) in the tree.  If anything in
  that object (ie. under that item) changes, the application is notified.  This
  allows, for example, subscription to just the \c {/Device/Buttons} item to
  receive notification when anything "button" related changes.

  For example, 

  \code
  QValueSpaceItem *buttons = new QValueSpaceItem("/Device/Buttons");
  qWarning() << "There are" << buttons->value().toUInt() << "buttons";
  QObject::connect(buttons, SIGNAL(contentsChanged()),
                   this, SLOT(buttonInfoChanged()));
  \endcode

  will invoke the \c {buttonInfoChanged()} slot whenever any item under
  \c {/Device/Buttons} changes.  This includes the value of \c {/Device/Buttons}
  itself, a change of a sub-object such as \c {/Device/Buttons/2/Name} or the
  creation (or removal) of a new sub-object, such as \c {/Device/Buttons/4}.

  \i {Note:} The QValueSpaceItem class is not thread safe and may only be used from 
  an application's main thread.
 */

#define QVALUESPACEITEM_D(d) QValueSpaceItemPrivateData *md = (QValueSpaceItemPrivate::Data == d->type)?static_cast<QValueSpaceItemPrivateData *>(d):static_cast<QValueSpaceItemPrivateWrite *>(d)->data;

/*!
  \fn QValueSpaceItem::contentsChanged()

  Emitted whenever the value of this item, or any sub-items changes.
 */
/*!
  Construct a new QValueSpaceItem with the specified \a parent that refers to the same path as \a other.
 */
QValueSpaceItem::QValueSpaceItem(const QValueSpaceItem &other, QObject* parent)
: QObject(parent), d(other.d)
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(other.d);
    if(QValueSpaceItemPrivate::Data == other.d->type) {
        d = md;
    } else {
        d = new QValueSpaceItemPrivateWrite(*static_cast<QValueSpaceItemPrivateWrite *>(other.d));
    }

    md->AddRef();
}

/*!
  Construct a new QValueSpaceItem with the specified \a parent that refers to the root path .
 */
QValueSpaceItem::QValueSpaceItem( QObject* parent )
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    d = new QValueSpaceItemPrivateData("/");
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  \overload

  Construct a new QValueSpaceItem with the specified parent that refers to the sub-\a path of \a base.
  This constructor is equivalent to \c {QValueSpaceItem(base, path.toUtf8())}.
 */
QValueSpaceItem::QValueSpaceItem(const QValueSpaceItem &base,
                                 const QString &path,
                                 QObject* parent)
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(base.d);

    if("/" == md->path)
        d = new QValueSpaceItemPrivateData(md->path + path.toUtf8());
    else
        d = new QValueSpaceItemPrivateData(md->path + "/" + path.toUtf8());
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  \overload

  Construct a new QValueSpaceItem with the specified \a parent that refers to the sub-\a path of \a base.
  This constructor is equivalent to
  \c {QValueSpaceItem(base, QByteArray(path))}.
 */
QValueSpaceItem::QValueSpaceItem(const QValueSpaceItem &base,
                                 const char *path,
                                 QObject* parent)
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(base.d);

    if("/" == md->path)
        d = new QValueSpaceItemPrivateData(md->path + QByteArray(path));
    else
        d = new QValueSpaceItemPrivateData(md->path + "/" + QByteArray(path));
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  Construct a new QValueSpaceItem with the specified \a parent that refers to the sub-\a path of \a base.
 */
QValueSpaceItem::QValueSpaceItem(const QValueSpaceItem &base,
                                 const QByteArray &path,
                                 QObject* parent)
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(base.d);
    if("/" == md->path)
        d = new QValueSpaceItemPrivateData(md->path + path);
    else
        d = new QValueSpaceItemPrivateData(md->path + "/" + path);
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  Assign \a other to this.
  */
QValueSpaceItem &QValueSpaceItem::operator=(const QValueSpaceItem& other)
{
    VS_CALL_ASSERT;
    if(other.d == d)
        return *this;

    bool reconnect = false;
    {
        QVALUESPACEITEM_D(d);
        reconnect = md->disconnect(this);
        if(d->type == QValueSpaceItemPrivate::Data) {
            md->Release();
            d = 0;
        } else {
            md->Release();
            delete d;
        }
    }

    {
        QVALUESPACEITEM_D(other.d);
        md->AddRef();

        if(other.d->type == QValueSpaceItemPrivate::Data) {
            d = other.d;
        } else {
            d = new QValueSpaceItemPrivateWrite(*static_cast<QValueSpaceItemPrivateWrite *>(other.d));
        }
    }

    if(reconnect) {
        QVALUESPACEITEM_D(d);
        md->connect(this);
    }

    return *this;
}

/*!
  \overload

  Construct a new QValueSpaceItem with the specified \a parent that refers to \a path.  This constructor is
  equivalent to \c {QValueSpaceItem(path.toUtf8())}.
 */
QValueSpaceItem::QValueSpaceItem(const QString &path, QObject* parent)
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    d = new QValueSpaceItemPrivateData(path.toUtf8());
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  \overload

  Construct a new QValueSpaceItem with the specified \a parent that refers to \a path.  This constructor is
  equivalent to \c {QValueSpaceItem(QByteArray(path))}.
 */
QValueSpaceItem::QValueSpaceItem(const char *path, QObject* parent)
: QObject( parent ), d(0)
{
    VS_CALL_ASSERT;
    d = new QValueSpaceItemPrivateData(path);
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}
/*!
  Construct a new QValueSpaceItem with the specified \a parent that refers to \a path.
 */
QValueSpaceItem::QValueSpaceItem(const QByteArray &path, QObject* parent)
: QObject(parent), d(0)
{
    VS_CALL_ASSERT;
    d = new QValueSpaceItemPrivateData(path);
    static_cast<QValueSpaceItemPrivateData *>(d)->AddRef();
}

/*!
  Destroys the QValueSpaceItem
  */
QValueSpaceItem::~QValueSpaceItem()
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(d);
    if(d->type == QValueSpaceItemPrivate::Write)
        delete d;

    md->Release();
}

/*!
  Returns the item name of this QValueSpaceItem.
  */
QString QValueSpaceItem::itemName() const
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(d);

    return QString::fromUtf8(md->path.constData(), md->path.length());
}

/*!
  Request that the item be removed.  The provider of the item determines whether
  the request is honored or ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.
  
  \sa QValueSpaceObject::itemRemove()
 */
void QValueSpaceItem::remove()
{
    VS_CALL_ASSERT;
    remove(QByteArray());
}

/*!
  \overload

  Request that the \a subPath of item be removed.  The provider of the sub path
  determines whether the request is honored or ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.
  
  \sa QValueSpaceObject::itemRemove()
 */
void QValueSpaceItem::remove(const QByteArray &subPath)
{
    VS_CALL_ASSERT;
    if(QValueSpaceItemPrivate::Data == d->type) {
        QValueSpaceItemPrivateWrite * write = new QValueSpaceItemPrivateWrite();
        write->data = static_cast<QValueSpaceItemPrivateData *>(d);
        d = write;
    }
    QValueSpaceItemPrivateWrite * write =
        static_cast<QValueSpaceItemPrivateWrite *>(d);

    write->ops.append(QValueSpaceItemPrivateWrite::Op(subPath));
}

/*!
  \overload

  Request that the \a subPath of item be removed.  The provider of the sub path
  determines whether the request is honored or ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.
  
  \sa QValueSpaceObject::itemRemove()
 */
void QValueSpaceItem::remove(const char *subPath)
{
    VS_CALL_ASSERT;
    remove(QByteArray(subPath));
}

/*!
  \overload

  Request that the \a subPath of item be removed.  The provider of the sub path
  determines whether the request is honored or ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.
  
  \sa QValueSpaceObject::itemRemove()
 */
void QValueSpaceItem::remove(const QString &subPath)
{
    VS_CALL_ASSERT;
    remove(subPath.toUtf8());
}

/*!
  Request that the value of this item be changed to \a value.  The provider of
  the item determines whether the request is honored or ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.

  \sa QValueSpaceObject::itemSetValue()
  */
void QValueSpaceItem::setValue(const QVariant &value)
{
    VS_CALL_ASSERT;
    setValue(QByteArray(), value);
}

/*!
  \overload

  Request that the value of the \a subPath of this item be changed to \a value.
  The provider of the sub path determines whether the request is honored or
  ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.

  \sa QValueSpaceObject::itemSetValue()
  */
void QValueSpaceItem::setValue(const QByteArray &subPath,
                               const QVariant &value)
{
    VS_CALL_ASSERT;
    if(QValueSpaceItemPrivate::Data == d->type) {
        QValueSpaceItemPrivateWrite * write = new QValueSpaceItemPrivateWrite();
        write->data = static_cast<QValueSpaceItemPrivateData *>(d);
        d = write;
    }
    QValueSpaceItemPrivateWrite * write =
        static_cast<QValueSpaceItemPrivateWrite *>(d);


    write->ops.append(QValueSpaceItemPrivateWrite::Op(subPath, value));
}

/*!
  \overload

  Request that the value of the \a subPath of this item be changed to \a value.
  The provider of the sub path determines whether the request is honored or
  ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.

  \sa QValueSpaceObject::itemSetValue()
  */
void QValueSpaceItem::setValue(const char * subPath,
                               const QVariant &value)
{
    VS_CALL_ASSERT;
    setValue(QByteArray(subPath), value);
}

/*!
  \overload

  Request that the value of the \a subPath of this item be changed to \a value.
  The provider of the sub path determines whether the request is honored or
  ignored.

  \i {Note:} This call asynchronously \i asks the current provider of the object to 
  change the value.  To explicitly make a change use QValueSpaceObject.

  \sa QValueSpaceObject::itemSetValue()
  */
void QValueSpaceItem::setValue(const QString & subPath,
                               const QVariant &value)
{
    VS_CALL_ASSERT;
    setValue(subPath.toUtf8(), value);
}

/*!
  Commit all changes made by calls to setValue() or remove().  The return value
  is reserved for future use.
 */
bool QValueSpaceItem::sync()
{
    VS_CALL_ASSERT;
    if(d->type == QValueSpaceItemPrivate::Data)
        return true;

    QValueSpaceItemPrivateWrite * write = static_cast<QValueSpaceItemPrivateWrite *>(d);
    QValueSpaceItemPrivateData * md = write->data;
    QList<QValueSpaceItemPrivateWrite::Op> ops = write->ops;
    write->ops.clear();

    bool rv = true;
    for(int ii = 0; ii < ops.count(); ++ii) {
        const QByteArray & path = ops.at(ii).path;
        const QVariant & value = ops.at(ii).value;

        if(ops.at(ii).type == QValueSpaceItemPrivateWrite::Op::Set) {
            if(path.isEmpty()) {
                for(int ii = md->readers.count(); ii > 0; --ii) {
                    if(!md->readers[ii - 1].first->setValue(md->readers[ii - 1].second,
                            value)) rv = false;
                }
            } else {
                QByteArray vpath =
                    ((*path.constData()) == '/')?path:(QByteArray("/") + path);
                for(int ii = md->readers.count(); ii > 0; --ii) {
                    if(!md->readers[ii - 1].first->setValue(md->readers[ii - 1].second,
                            vpath, value)) rv = false;
                }
            }
        } else {
            if(path.isEmpty()) {
                for(int ii = md->readers.count(); ii > 0; --ii) {
                    if(!md->readers[ii - 1].first->remove(md->readers[ii - 1].second)) rv = false;
                }
            } else {
                QByteArray vpath =
                    ((*path.constData()) == '/')?path:(QByteArray("/") + path);
                for(int ii = md->readers.count(); ii > 0; --ii) {
                    if(!md->readers[ii - 1].first->remove(md->readers[ii - 1].second, vpath)) rv = false;
                }
            }
        }
    }

    for(int ii = md->readers.count(); ii > 0; --ii) {
        if(!md->readers[ii - 1].first->syncChanges())
            rv = false;
    }
    return rv;
}

/*!
  \overload

  This is a convenience overload and is equivalent to
  \c {value(subPath.toUtf8(), def)}.
 */
QVariant QValueSpaceItem::value(const QString & subPath, const QVariant &def) const
{
    VS_CALL_ASSERT;
    return value(subPath.toUtf8(), def);
}

/*!
  \overload

  This is a convenience overload and is equivalent to
  \c {value(QByteArray(subPath), def)}.
 */
QVariant QValueSpaceItem::value(const char * subPath, const QVariant &def) const
{
    VS_CALL_ASSERT;
    return value(QByteArray(subPath), def);
}


/*!
   Returns the value of sub-item \a subPath of this item, or the value of this
   item if \a subPath is empty.  The following code shows how the item and
   \a subPath relate.

   \code

   QValueSpaceItem base("/Settings");
   QValueSpaceItem equiv("/Settings/Trolltech/IniValueSpace/General/Mappings);

   // Is true
   equiv.value() == base.value("Trolltech/IniValueSpace/General/Mapping");
   \endcode

   If the item does not exist, \a def is returned.
   */
QVariant QValueSpaceItem::value(const QByteArray & subPath, const QVariant &def) const
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(d);
    QVariant value;
    if(subPath.isEmpty()) {
        for(int ii = md->readers.count(); ii > 0; --ii) {
            if(md->readers[ii - 1].first->value(md->readers[ii - 1].second,
                                                &value))
                return value;
        }
    } else {
        QByteArray vpath =
            ((*subPath.constData()) == '/')?subPath:(QByteArray("/") + subPath);
        for(int ii = md->readers.count(); ii > 0; --ii) {
            if(md->readers[ii - 1].first->value(md->readers[ii - 1].second,
                                                vpath, &value))
                return value;
        }
    }
    return def;
}

/*! \internal */
void QValueSpaceItem::connectNotify(const char *signal)
{
    VS_CALL_ASSERT;
    if(QLatin1String(signal) == SIGNAL(contentsChanged())) {
        QVALUESPACEITEM_D(d);
        md->connect(this);
    } else {
        QObject::connectNotify(signal);
    }
}

/*! \internal */
void QValueSpaceItem::disconnectNotify(const char *signal)
{
    VS_CALL_ASSERT;
    if(QLatin1String(signal) == SIGNAL(contentsChanged())) {
        QVALUESPACEITEM_D(d);
        md->disconnect(this);
    } else {
        QObject::disconnectNotify(signal);
    }
}

/*!
  Returns a list of sub-paths for this item.  For example, given a Value Space
  tree containing:

  \code
  /Settings/Trolltech/IniValueSpace
  /Settings/Trolltech/Other
  /Settings/Qtopia
  /Device/Buttons
  \endcode

  \c { QValueSpaceItem("/Settings").subPaths() } will return a list containing
  \c { { Trolltech, Qtopia } } in no particular order.
 */
QList<QString> QValueSpaceItem::subPaths() const
{
    VS_CALL_ASSERT;
    QVALUESPACEITEM_D(d);
    QSet<QByteArray> rv;
    for(int ii = 0; ii < md->readers.count(); ++ii)
        rv.unite(md->readers[ii].first->children(md->readers[ii].second));

    QList<QString> rvs;
    for(QSet<QByteArray>::ConstIterator iter = rv.begin();
            iter != rv.end();
            ++iter)
        rvs.append(QString::fromUtf8(iter->constData(), iter->length()));

    return rvs;
}

#include "qvaluespace.moc"
