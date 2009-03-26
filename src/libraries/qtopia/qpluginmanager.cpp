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

#include "qpluginmanager.h"
#include <signal.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qtopianamespace.h>

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPluginLoader>
#include <QSettings>
#include <QTimer>
#include <QTranslator>
#include <QMap>
#include <QPointer>
#include <QtDebug>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

// Qt Extended can try to disabled broken plugins automatically
// on an individual basis. This has performance implications.
//
//#define QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS

Q_DECLARE_METATYPE(QPluginLoader *);

#ifdef SINGLE_EXEC
typedef QObject *(*qtopiaPluginCreateFunc_t)();
typedef QMap <QString, QPointer<QObject> > PluginInstanceDict;
typedef QMap <QString, PluginInstanceDict> PluginTypeInstanceDict;
typedef QMap <QString, qtopiaPluginCreateFunc_t> PluginNameDict;
typedef QMap <QString, PluginNameDict> PluginTypeNameDict;
static PluginTypeNameDict &se_ptd()
{
    static PluginTypeNameDict *d = 0;
    if ( !d )
        d = new PluginTypeNameDict();
    return *d;
}
static PluginTypeInstanceDict &se_pid()
{
    static PluginTypeInstanceDict *d = 0;
    if ( !d )
        d = new PluginTypeInstanceDict();
    return *d;
}
QTOPIA_EXPORT void registerPlugin(const char *name, const char *type, qtopiaPluginCreateFunc_t createFunc)
{
    se_ptd()[QLatin1String(type)][QLatin1String(name)] = createFunc;
}
#else
class PluginLibraryManager : public QObject
{
    Q_OBJECT
public:
    PluginLibraryManager();
    ~PluginLibraryManager();

    QPluginLoader *refLibrary( const QString &file );

public slots:
    void derefLibrary( QPluginLoader *lib );

private:
    QMap<QString,QPluginLoader*> libs;
};

static QPointer<PluginLibraryManager> manager = 0;

PluginLibraryManager *pluginLibraryManagerInstance()
{
    if ( !manager )
        manager = new PluginLibraryManager;

    return manager;
}
#endif

static QString configFilename( const QString &name )
{
    QString homeDirPath = Qtopia::homePath();

    QDir dir = (homeDirPath + "/Settings");
    if ( !dir.exists() ) {
        mkdir(dir.path().toLocal8Bit(),0700);
    }

    return dir.path() + "/" + name + ".conf";
}

static bool lockFile( QFile &f )
{
    if (!f.isOpen())
        return false;

    struct flock fileLock;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = f.size();
    fileLock.l_type = F_WRLCK;

    return (::fcntl(f.handle(), F_SETLKW, &fileLock) == 0);
}

static bool unlockFile( QFile &f )
{
    if (!f.isOpen())
        return false;

    struct flock fileLock;

    fileLock.l_whence = SEEK_SET;
    fileLock.l_start = 0;
    fileLock.l_len = f.size();
    fileLock.l_type = F_UNLCK;

    return (::fcntl(f.handle(), F_SETLK, &fileLock) == 0);
}

static const char *cfgName()
{
    return "PluginManager";
}


//===========================================================================

/*!
  \class QPluginManager
    \inpublicgroup QtBaseModule

  \brief The QPluginManager class simplifies plug-in loading and allows plugins to be
  enabled/disabled.

  The most common use is to
  iterate over the list of plugins and load each one as follows:

  \code
    QPluginManager pluginManager( "Effects" );
    QStringList list = pluginManager.list();
    QStringList::Iterator it;
    QList<EffectsInterface*> effectList;
    for ( it = list.begin(); it != list.end(); ++it ) {
        QObject *instance = pluginManager->instance(*it);
        EffectsInterface *iface = 0;
        iface = qobject_cast<EffectsInterface*>(instance);
        if (iface) {
            effectList.append( iface );
        }
    }
  \endcode

  The list() function returns a list of plugins, using the filenames
  of the plugins in the plugin directory.  This does not require
  any plugins to be loaded, so it is very lightweight.

  In order to load a plugin, call the instance() function with the
  name of the plugin to load.  qobject_cast() may then be used to query
  for the desired interface.

  If the application loading a plugin crashes during while loading the
  plugin, the plugin will be disabled.  This prevents a plugin from permanently
  rendering an application unusable.  Disabled plugins can be queried
  using disabledList() and reenabled using setEnabled().

  \ingroup plugins
*/

class QPluginManagerPrivate
{
public:
    QString type;
    QStringList plugins;
    QStringList disabled;
    QMap<QObject*,QPluginLoader*> interfaces;
};

/*!
  Creates a QPluginManager for plugins of \a type with the given
  \a parent.

  The plugins must be installed in the [qt_prefix]/plugins/\i{type}  directory.
*/

QPluginManager::QPluginManager(const QString &type, QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QPluginLoader *>("QPluginLoader *");
    d = new QPluginManagerPrivate;
    d->type = type;
    init();
    initType();
}

/*!
  Destroys the QPluginManager and releases any resources allocated by
  the PluginManager.
*/
QPluginManager::~QPluginManager()
{
    clear();
    delete d;
}

/*!
  Releases all resources allocated by the QPluginManager
*/
void QPluginManager::clear()
{
    QMutableMapIterator<QObject*,QPluginLoader*> it(d->interfaces);
    while (it.hasNext()) {
        it.next();
        QObject *iface = it.key();
        delete iface;
    }
}

/*! \internal
*/
void QPluginManager::init()
{
#ifndef SINGLE_EXEC
    pluginLibraryManagerInstance();
#endif
}

/*!
  Returns the list of plugins that are available.

  The plugin list is derived from the filenames of the plugins and
  does not force any plugins to be loaded.
*/
const QStringList &QPluginManager::list() const
{
    return d->plugins;
}

/*!
  Returns the list of plugins that have been disabled.
*/
const QStringList &QPluginManager::disabledList() const
{
    return d->disabled;
}

/*!
  Load the plug-in specified by \a name.

  Returns the plugin interface if found, otherwise 0.

  \code
    QObject *instance = pluginManager->instance("name");
    if (instance) {
        EffectsInterface *iface = 0;
        iface = qobject_cast<EffectsInterface*>(instance);
        if (iface) {
            // We have an instance of the desired type.
        }
    }
  \endcode

  If an instance is no longer required and resources need to be released,
  simply delete the returned instance.
*/
QObject *QPluginManager::instance( const QString &name )
{
    QObject *iface = 0;
#ifndef SINGLE_EXEC
    QString lname = stripSystem( name );

    QString libFile;
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        libFile = *qit + "plugins/" + d->type + "/lib" + lname + ".so";
        if ( QFile::exists(libFile) )
            break;
    }
    QPluginLoader *lib = pluginLibraryManagerInstance()->refLibrary( libFile );
    if ( !lib ) // error reported by refLibrary
        return 0;

#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    bool enabled = isEnabled( name );
    if (enabled)
        setEnabled( name, false );
#endif
    if ( (iface = lib->instance()) ) {
        loaded(iface, lib, lname);
    } else {
        pluginLibraryManagerInstance()->derefLibrary( lib );
    }
#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    if (enabled)
        setEnabled( name, true );
#endif
#else
    iface = se_pid()[d->type][name];
    if ( !iface ) {
        qtopiaPluginCreateFunc_t createFunc = se_ptd()[d->type][name];
        if ( createFunc ) {
            se_pid()[d->type][name] = createFunc();
            iface = se_pid()[d->type][name];
            if ( iface )
                loaded(iface, 0, name);
        }
    }
#endif

    return iface;
}

void QPluginManager::instanceDestroyed()
{
    QObject *iface = sender();
    QPluginLoader *lib = d->interfaces.take(iface);
#ifndef SINGLE_EXEC
    QMetaObject::invokeMethod(pluginLibraryManagerInstance(), "derefLibrary",
        Qt::QueuedConnection, Q_ARG(QPluginLoader *, lib));
#else
    Q_UNUSED(lib);
#endif
}

void QPluginManager::initType()
{
#ifndef SINGLE_EXEC
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        QString path = *qit + "plugins/";
        path += d->type;
        DIR *dir = opendir( path.toLatin1() );
        if ( !dir )
            continue;

        QStringList list;
        dirent *file;
        while ( (file = readdir(dir)) ) {
            if ( !strncmp( file->d_name, "lib", 3 ) ) {
                if ( !strcmp( file->d_name+strlen(file->d_name)-3, ".so" ) )
                    list.append( file->d_name );
            }
        }
        closedir(dir);

        bool safeMode = false;

        QString cfgFilename( configFilename(cfgName()) + ".lock" );
        QFile lf( cfgFilename );
        lf.open( lf.exists() ? QIODevice::ReadOnly : QIODevice::WriteOnly );
        lockFile( lf );
        {
            QSettings cfg("Trolltech",cfgName());
            cfg.beginGroup( "Global" );
            safeMode = cfg.value( "Mode", "Normal" ).toString() == "Safe";
            cfg.endGroup();
            cfg.beginGroup( d->type );
            d->disabled = cfg.value( "Disabled").toString().split( ',' );
        }
        unlockFile( lf );

        QStringList required;
        if ( QFile::exists( path + "/.directory" ) ) {
            QSettings config(path + "/.directory", QSettings::IniFormat);
            required = config.value( "Required").toString().split( ',' );
        }

        QStringList::Iterator it;
        for ( it = list.begin(); it != list.end(); ++it ) {
            QString name = stripSystem(*it);
            if ( (!safeMode && isEnabled(name)) || required.contains(name) )
                // Discard duplicate plugins preferring those found first
                if( !d->plugins.contains( name ) )
                    d->plugins += name;
        }
    }
#else
    QStringList *pnsl = new QStringList();
    const PluginNameDict &pnd = se_ptd()[d->type];
    foreach ( const QString &key, pnd.keys() )
        pnsl->append(key);
    d->plugins = *pnsl;
#endif
}

QStringList QPluginManager::languageList() const
{
    return Qtopia::languageList() << QLatin1String("en_US");
}

/*!
  Enables or disables plug-in \a name depending on the value of \a enabled.
  A disabled plug-in can still be loaded, but it will not be returned by list().
*/
void QPluginManager::setEnabled( const QString &name, bool enabled )
{
    QString lname = stripSystem(name);
    QString cfgFilename( configFilename(cfgName()) + ".lock" );
    QFile lf( cfgFilename );
    lf.open( lf.exists() ? QIODevice::ReadOnly : QIODevice::WriteOnly );
    lockFile( lf );
    {
        QSettings cfg("Trolltech",cfgName());
        cfg.beginGroup( d->type );
        d->disabled = cfg.value( "Disabled").toString().split( ',' );
        bool wasEnabled = d->disabled.contains( lname ) == 0;
        if ( wasEnabled != enabled ) {
            if ( enabled ) {
                d->disabled.removeAll( lname );
            } else {
                d->disabled += lname;
            }
            cfg.setValue("Disabled", d->disabled.join(QString(',' )));
        }
    }
    unlockFile( lf );
}

/*!
  Returns true if the plug-in \a name is enabled.
*/
bool QPluginManager::isEnabled( const QString &name ) const
{
    QString lname = stripSystem(name);
    return d->disabled.indexOf( lname ) == -1;
}

/*!
  Returns true if Qt Extended is currently in \i{Safe Mode}.  In safe mode
  list() will return an empty list and no plugins should be loaded.  This
  is to allow misbehaving plugins to be disabled.
*/
bool QPluginManager::inSafeMode()
{
    QSettings cfg("Trolltech",cfgName());
    cfg.beginGroup( "Global" );
    QString mode = cfg.value( "Mode", "Normal" ).toString();
    return ( mode == "Safe" ); // No tr
}

QString QPluginManager::stripSystem( const QString &libFile ) const
{
    QString name = libFile;
    if ( libFile.lastIndexOf(".so") == (int)libFile.length()-3 ) {
        name = libFile.left( libFile.length()-3 );
        if ( name.indexOf( "lib" ) == 0 )
            name = name.mid( 3 );
    }

    return name;
}

void QPluginManager::loaded( QObject *iface, QPluginLoader *lib, QString name )
{
    d->interfaces.insert( iface, lib );
    connect(iface, SIGNAL(destroyed()), this, SLOT(instanceDestroyed()));
    QString type = QLatin1String("/lib") + name + QLatin1String(".qm");
    QStringList langs = languageList();
    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        QString path(*qit+QLatin1String("i18n/"));
        for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
            QString tfn = path + *lit + QLatin1Char('/') + type;
            if ( QFile::exists(tfn) ) {
                QTranslator * trans = new QTranslator(qApp);
                if ( trans->load( tfn ))
                    qApp->installTranslator( trans );
                else
                    delete trans;
            }
        }
    }
}

#ifndef SINGLE_EXEC
//===========================================================================
// Only compile this under single process
#if !(defined(SINGLE_EXEC) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(QPE_NO_COMPAT) && defined(PLUGINLOADER_INTERN))

PluginLibraryManager::PluginLibraryManager() : QObject( qApp )
{
}

PluginLibraryManager::~PluginLibraryManager()
{
    if ( qApp->type() == QApplication::GuiServer ) {
        QSettings cfg("Trolltech",cfgName());
        cfg.beginGroup( "Global" );
        cfg.setValue( "Mode", "Normal" );
    }
}

QPluginLoader *PluginLibraryManager::refLibrary( const QString &file )
{
    QPluginLoader *lib = 0;
    QMap<QString,QPluginLoader*>::const_iterator it = libs.find(file);
    if (it != libs.end())
        lib = *it;
    if ( !lib ) {
        lib = new QPluginLoader( file );
        lib->load();
        if ( !lib->isLoaded() ) {
            qWarning() << "Could not load" << file << "errorString()" << lib->errorString();
            delete lib;
            return 0;
        }
    }
    libs.insertMulti( file, lib );

    return lib;
}

void PluginLibraryManager::derefLibrary( QPluginLoader *lib )
{
    if ( !lib )
        return;

    QString file = lib->fileName();
    libs.take( file );
    if ( !libs.contains(file) ) {
        lib->unload();
        delete lib;
    }
}
#endif
#endif

#include "qpluginmanager.moc"
