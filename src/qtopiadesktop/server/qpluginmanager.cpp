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

#include <desktopsettings.h>

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPluginLoader>
#include <QTimer>
#include <QMap>
#include <QPointer>
#include <QApplication>
#include <qdebug.h>

#include <stdlib.h>
#include <signal.h>
#ifndef Q_OS_WIN32
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <dirent.h>
#endif

// Qtopia can try to disabled broken plugins automatically
// on an individual basis. This has performance implications.
//
//#define QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS

Q_DECLARE_METATYPE(QPluginLoader *);

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

//===========================================================================

class QPluginManagerPrivate
{
public:
    QString type;
    QStringList plugins;
    QStringList disabled;
    QMap<QObject*,QPluginLoader*> interfaces;
};

QPluginManager::QPluginManager(const QString &type, QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QPluginLoader *>("QPluginLoader *");
    d = new QPluginManagerPrivate;
    d->type = type;
    init();
    initType();
}

QPluginManager::~QPluginManager()
{
    clear();
    delete d;
}

void QPluginManager::clear()
{
    QMutableMapIterator<QObject*,QPluginLoader*> it(d->interfaces);
    while (it.hasNext()) {
	it.next();
	QObject *iface = it.key();
	delete iface;
    }
}

void QPluginManager::init()
{
    pluginLibraryManagerInstance();
}

const QStringList &QPluginManager::list() const
{
    return d->plugins;
}

const QStringList &QPluginManager::disabledList() const
{
    return d->disabled;
}

QObject *QPluginManager::instance( const QString &name )
{
    QObject *iface = 0;
    QString lname = stripSystem( name );

    QString libFile;
    QStringList qpepaths;
    qpepaths << DesktopSettings::installedDir();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
#ifndef Q_OS_WIN32
	libFile = *qit + "plugins/" + d->type + "/lib" + lname + ".so";
#else
	libFile = *qit + "plugins/" + d->type + "/" + lname + ".dll";
#endif
	if ( QFile::exists(libFile) )
	    break;
    }
    QPluginLoader *lib = pluginLibraryManagerInstance()->refLibrary( libFile );
    if ( !lib )
	return 0;

#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    bool enabled = isEnabled( name );
    if (enabled)
	setEnabled( name, false );
#endif
    if ( (iface = lib->instance()) ) {
        d->interfaces.insert( iface, lib );
        connect(iface, SIGNAL(destroyed()), this, SLOT(instanceDestroyed()));
        QString file = QString("lib%1").arg(name);
        DesktopSettings::loadTranslations( file, iface );
    } else {
	pluginLibraryManagerInstance()->derefLibrary( lib );
    }
#ifdef QTOPIA_DISABLE_ONLY_BROKEN_PLUGINS
    if (enabled)
	setEnabled( name, true );
#endif

    return iface;
}

void QPluginManager::instanceDestroyed()
{
    QObject *iface = sender();
    QPluginLoader *lib = d->interfaces.take(iface);
    QMetaObject::invokeMethod(pluginLibraryManagerInstance(), "derefLibrary",
        Qt::QueuedConnection, Q_ARG(QPluginLoader *, lib));
}

void QPluginManager::initType()
{
    QStringList qpepaths;
    qpepaths << DesktopSettings::installedDir();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
	QString path = *qit + "plugins/";
	path += d->type;
#ifndef Q_OS_WIN32
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
#else
	QDir dir (path, "*.dll");
	QStringList list = dir.entryList();
#endif

	bool safeMode = false;

        DesktopSettings settings("pluginloader");
        safeMode = ( settings.value("Mode", "Normal").toString() == "Safe" );
        settings.beginGroup(d->type);
        d->disabled = settings.value( "Disabled" ).toString().split(",");

	QStringList required;
	if ( QFile::exists( path + "/.directory" ) ) {
	    QSettings config(path + "/.directory", QSettings::IniFormat);
	    required = config.value( "Required").toString().split( ',' );
	}

	QStringList::Iterator it;
	for ( it = list.begin(); it != list.end(); ++it ) {
	    QString name = stripSystem(*it);
	    if ( (!safeMode && isEnabled(name)) || required.contains(name) )
		d->plugins += name;
	}
    }
}

void QPluginManager::setEnabled( const QString &name, bool enabled )
{
    QString lname = stripSystem(name);
    DesktopSettings settings("pluginloader");
    settings.beginGroup(d->type);
    d->disabled = settings.value( "Disabled" ).toString().split(",");
    bool wasEnabled = ( d->disabled.contains( lname ) == 0 );
    if ( wasEnabled != enabled ) {
        if ( enabled ) {
            d->disabled.removeAll( lname );
        } else {
            d->disabled += lname;
        }
        settings.setValue( "Disabled", d->disabled.join(",") );
    }
}

bool QPluginManager::isEnabled( const QString &name ) const
{
    QString lname = stripSystem(name);
    return d->disabled.indexOf( lname ) == -1;
}

bool QPluginManager::inSafeMode()
{
    DesktopSettings settings("pluginloader");
    return ( settings.value("Mode", "Normal").toString() == "Safe" );
}

QString QPluginManager::stripSystem( const QString &libFile ) const
{
    QString name = libFile;
#ifndef Q_OS_WIN32
    if ( libFile.lastIndexOf(".so") == (int)libFile.length()-3 ) {
	name = libFile.left( libFile.length()-3 );
	if ( name.indexOf( "lib" ) == 0 )
	    name = name.mid( 3 );
    }
#else
    if ( libFile.lastIndexOf(".dll") == (int)libFile.length()-4 )
	name = libFile.left( libFile.length()-4 );
#endif

    return name;
}

//===========================================================================
// Only compile this once under Win32 and single process
#if !(defined(Q_OS_WIN32) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(SINGLE_EXEC) && defined(PLUGINLOADER_INTERN)) && \
    !(defined(QPE_NO_COMPAT) && defined(PLUGINLOADER_INTERN))

PluginLibraryManager::PluginLibraryManager() : QObject( qApp )
{
}

PluginLibraryManager::~PluginLibraryManager()
{
}

QPluginLoader *PluginLibraryManager::refLibrary( const QString &file )
{
    QPluginLoader *lib = 0;
    QMap<QString,QPluginLoader*>::const_iterator it = libs.find(file);
    if (it != libs.end())
	lib = *it;
    if ( !lib ) {
	//qDebug() << "Trying to load plugin" << file;
	lib = new QPluginLoader( file );
	if ( !lib->load() ) {
            qWarning() << "QPluginLoader could not load" << file << "errorString()" << lib->errorString();
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

#include "qpluginmanager.moc"
