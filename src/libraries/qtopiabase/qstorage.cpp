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

#include <qstorage.h>

#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QHash>
#include <QTimer>
#include <QProcess>
#include <QReadWriteLock>
#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
#include <qwsdisplay_qws.h>
#elif defined(Q_WS_X11)
#include <qx11info_x11.h>
#endif
#endif

#include <qtopiachannel.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QtopiaIpcEnvelope>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <mntent.h>

#ifdef QT_LSB
#include <sys/statvfs.h>
#endif

/*!
  \class QStorageMetaInfo
    \inpublicgroup QtBaseModule

  \brief The QStorageMetaInfo class describes the disks mounted on the file system.

  This class provides access to the mount information for the Linux
  filesystem. Each mount point is represented by the QFileSystem class.
  To ensure this class has the most up to date size information, call update().

  \ingroup io
  \ingroup content
  \sa QFileSystem
*/

class QStorageMetaInfoPrivate
{
public:
    QStorageMetaInfoPrivate()
        : channel( 0 )
        , documentsFileSystem( 0 )
        , applicationsFileSystem( 0 )
        , suppressMessages(false)
    {
    }

    QList<QFileSystem*> fileSystems;
    QtopiaChannel *channel;
    QFileSystem *documentsFileSystem;
    QFileSystem *applicationsFileSystem;
    QMap<QString, QFileSystem *> typeFileSystems;
    bool suppressMessages;
};

/*! Constructor that determines the current mount points of the filesystem.
  The standard \a parent parameters is passed on to QObject. QStorageMetaInfo::instance()
  is the preferred method of obtaining a copy of this class.

  \sa instance()
 */
QStorageMetaInfo::QStorageMetaInfo( QObject *parent )
    : QObject( parent )
{
    d = new QStorageMetaInfoPrivate;

#ifndef QTOPIA_HOST
#if defined(Q_WS_QWS)
    if ( qt_fbdpy )
#elif defined(Q_WS_X11)
    if (QX11Info::display())
#endif
    {
        d->channel = new QtopiaChannel( "QPE/QStorage", this );
        connect( d->channel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(cardMessage(QString,QByteArray)) );
    }
#endif
    update();
}

/*!
  Destroys the QStorageMetaInfo object.
*/
QStorageMetaInfo::~QStorageMetaInfo()
{
    foreach(QFileSystem *f, d->fileSystems)
        delete f;

    delete d;
}

/*! Returns the longest matching QFileSystem that starts with the
   same prefix as \a filename as its mount point. Use \a connectedOnly to search
   only the filesystems that are connected.
*/
const QFileSystem *QStorageMetaInfo::fileSystemOf( const QString &filename, bool connectedOnly )
{
    // The filename way (doesn't understand symlinks)
    QFileSystem *bestMatch = 0;
    int bestLen = 0;
    int currLen;

    foreach ( QFileSystem *fs, fileSystems( NULL, connectedOnly) ) {
        QString path = !fs->path().isEmpty() ? fs->path() : fs->prevPath();
        if ( path.length() > bestLen && filename.startsWith( path ) ) {
            currLen = path.length();
            if ( currLen == 1 )
                currLen = 0; // Fix checking '/' root mount which is a special case
            if ( filename.length() == currLen ||
                 filename[currLen] == '/' ||
                 filename[currLen] == '\\') {
                bestMatch = fs;
                bestLen = path.length();
            }
        }
    }
    if (bestMatch != NULL)
        bestMatch->update();
    return bestMatch;
}

/*!
    Returns a pointer to the file system on which the default documents path is located.
*/
const QFileSystem *QStorageMetaInfo::documentsFileSystem()
{
    return d->documentsFileSystem;
}

/*!
    Returns a pointer to the file system on which the default applications path is located.
 */
const QFileSystem *QStorageMetaInfo::applicationsFileSystem()
{
    return d->applicationsFileSystem;
}

/*!
    Returns the QFileSystem that is the default storage location for files of \a type.
*/
const QFileSystem *QStorageMetaInfo::typeFileSystem(const QString &type)
{
    return d->typeFileSystems.value(type);
}

/*!
    \internal System hook to listen for notifications that a new file system has been connected
    and we need to refresh our internal information.
*/

void QStorageMetaInfo::cardMessage( const QString& message, const QByteArray& data )
{
    if ( message == "updateStorage()" )
        update();
    else if( message == "mounting(QString)" || message == "unmounting(QString)" ) {
        QDataStream in(data);
        QString mountpoint;
        in >> mountpoint;

        update(mountpoint, message == "mounting(QString)" ? true : false);
    }
}

/*! Updates the mount and free space information for each mount
  point. This function is automatically called when a mount point is mounted or
  unmounted.
*/
void QStorageMetaInfo::update()
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("Storage"));
#if !defined(QT_LSB)
    mntent *me;
    FILE *mntfp = NULL;
    QHash<QString, QString> mountEntries;
    QString disk, path, prevPath, options, name, documentsPath, applicationsPath;
    bool removable, applications, documents, contentDatabase, connected;

    mntfp = setmntent( "/proc/mounts", "r" );
    me = getmntent(mntfp);
    while(me != NULL)
    {
        mountEntries[me->mnt_fsname] = me->mnt_dir;
        me = getmntent(mntfp);
    }
    endmntent(mntfp);

    cfg.beginGroup(QLatin1String("MountTable"));
    QStringList mountpointslist;

    if (cfg.contains("MountPoints"))
        mountpointslist = cfg.value(QLatin1String("MountPoints")).toString().split(QLatin1String(","));

    QString documentsMount = cfg.value( QLatin1String( "DocumentsDefault" ), QLatin1String("HOME") ).toString();
    QString applicationsMount = cfg.value( QLatin1String( "ApplicationsDefault" ), QLatin1String("HOME") ).toString();

    const QString defaultString(QLatin1String("Default"));

    QMap<QString, QString> defaultMap;

    foreach (QString key, cfg.childKeys()) {
        if (key.endsWith(defaultString)) {
            QString name = key;
            name.chop(defaultString.length());
            defaultMap.insert(cfg.value(key).toString(), name);
        }
    }

    qLog(DocAPI) << "cfg.mountpoints =" << cfg.value(QLatin1String("MountPoints")).toString();
    qLog(DocAPI) << "mountpointslist =" << mountpointslist;
    cfg.endGroup();

    if(d->fileSystems.count() != mountpointslist.count() + 2)
    {
        d->typeFileSystems.clear();

        // save the path info (so we can record the "previous path")
        QHash<QString, QString> prevPaths;
        foreach(QFileSystem *f, d->fileSystems) {
            prevPaths[f->disk()] = (!f->path().isEmpty()?f->path():f->prevPath());
            delete f;
        }
        d->fileSystems.clear();
        // refresh d->fileSystems
        foreach(QString entry, mountpointslist)
        {
            cfg.beginGroup(entry);
            disk            = cfg.value(QLatin1String("Path")).toString();
            if ( mountEntries.contains(disk) ) {
                path        = mountEntries[disk];
                prevPath    = mountEntries[disk];
            } else {
                path        = QLatin1String("");
                prevPath    = prevPaths[disk];
            }
            options         = cfg.value(QLatin1String("Options"), QLatin1String("rw")).toString();
            name            = cfg.value(QLatin1String("Name[]"), disk).toString();
            documentsPath    = cfg.value(QLatin1String("DocumentsPath")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath")).toString();
            removable       = cfg.value(QLatin1String("Removable"), false).toBool();
            applications    = cfg.value(QLatin1String("Applications"), false).toBool();
            documents       = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase = cfg.value(QLatin1String("ContentDatabase"), documents).toBool();
            connected       = removable?false:true;

            QMap<QString, QVariant> values;
            foreach (QString key, cfg.childKeys())
                values[key] = cfg.value(key);

            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                            "documents, contentDatabase, connected ="
                         << disk << path << options << name << documentsPath << applicationsPath << removable
                         << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, prevPath, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected, values));

            if( entry == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( entry == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

            foreach (QString defaultKey, defaultMap.values(entry))
                d->typeFileSystems[defaultKey] = d->fileSystems.last();

            cfg.endGroup();
        }

        // add [HOME]
        do {
            cfg.beginGroup(QLatin1String("HOME"));
            disk            = cfg.value(QLatin1String("Path"), QLatin1String("HOME")).toString();
            // If your HOME is on a partition that's specified by Storage.conf and it doesn't contain
            // Applications or Documents you might want to hide it. Set "Path = HIDE" to do this.
            if ( disk == QLatin1String("HIDE") ) {
                cfg.endGroup();
                break;
            } else {
                disk        = QLatin1String("HOME");
            }
            path             = QDir::homePath();
            options          = cfg.value(QLatin1String("Options"), QLatin1String("rw")).toString();
            name             = cfg.value(QLatin1String("Name[]"), QLatin1String("HOME")).toString();
            documentsPath    = cfg.value(QLatin1String("DocumentsPath"), QLatin1String("/Documents")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath"), QLatin1String("/Applications")).toString();
            removable        = cfg.value(QLatin1String("Removable"), false).toBool();
            applications     = cfg.value(QLatin1String("Applications"), true).toBool();
            documents        = cfg.value(QLatin1String("Documents"), true).toBool();
            contentDatabase  = false; // This setting makes no sense because HOME's database is always loaded.
            connected        = removable?false:true;

            QMap<QString, QVariant> values;
            foreach (QString key, cfg.childKeys())
                values[key] = cfg.value(key);

            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                    "documents, contentDatabase, connected ="
                    << disk << path << options << name << documentsPath << applicationsPath << removable
                    << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, path, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected, values));

            if( QLatin1String("HOME") == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( QLatin1String("HOME") == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

            foreach (QString defaultKey, defaultMap.values(QLatin1String("HOME")))
                d->typeFileSystems[defaultKey] = d->fileSystems.last();

            cfg.endGroup();
        } while (0);

        // add [PREFIX]
        do {
            cfg.beginGroup(QLatin1String("PREFIX"));
            disk             = QLatin1String("PREFIX");
            path             = QDir::cleanPath( Qtopia::qtopiaDir() );
            options          = cfg.value(QLatin1String("Options"), QLatin1String("ro")).toString();
            name             = cfg.value(QLatin1String("Name[]"), QLatin1String("PREFIX")).toString();
            documentsPath    = cfg.value(QLatin1String("DocumentsPath")).toString();
            applicationsPath = cfg.value(QLatin1String("ApplicationsPath")).toString();
            removable        = cfg.value(QLatin1String("Removable"), false).toBool();
            applications     = cfg.value(QLatin1String("Applications"), false).toBool();
            documents        = cfg.value(QLatin1String("Documents"), false).toBool();
            contentDatabase  = cfg.value(QLatin1String("ContentDatabase"), true).toBool();
            connected        = removable?false:true;

            QMap<QString, QVariant> values;
            foreach (QString key, cfg.childKeys())
                values[key] = cfg.value(key);

            qLog(DocAPI) << "disk, path, options, name, documentsPath, applicationsPath, removable, applications, "
                    "documents, contentDatabase, connected ="
                    << disk << path << options << name << documentsPath << applicationsPath << removable
                    << applications << documents << contentDatabase << connected;
            d->fileSystems.append(new QFileSystem(disk, path, path, options, name, documentsPath, applicationsPath,
                                removable, applications, documents, contentDatabase, connected, values));

            if( QLatin1String("PREFIX") == documentsMount )
                d->documentsFileSystem = d->fileSystems.last();
            if( QLatin1String("PREFIX") == applicationsMount )
                d->applicationsFileSystem = d->fileSystems.last();

            foreach (QString defaultKey, defaultMap.values(QLatin1String("PREFIX")))
                d->typeFileSystems[defaultKey] = d->fileSystems.last();

            cfg.endGroup();
        } while (0);
    }

    foreach(QFileSystem *fs, d->fileSystems)
    {
        bool connected = !fs->isRemovable();
        QString path;

        if (fs->disk() != QLatin1String("HOME") &&
            fs->disk() != QLatin1String("PREFIX") )
        {
            connected = mountEntries.contains(fs->disk());
            if( connected )
                path = mountEntries[fs->disk()];

            fs->update( connected, path );
        }
        else
            fs->update();
    }
    if(d->suppressMessages == false)
        emit disksChanged();
#endif
}

/*!
    Update the system mounted at \a mountpoint, marking whether it is \a connected.
*/
void QStorageMetaInfo::update(QString& mountpoint, bool connected)
{
    d->suppressMessages = true;
    update();
    d->suppressMessages = false;
    foreach(QFileSystem *fs, d->fileSystems)
    {
        if(fs->disk() == mountpoint && fs->isRemovable())
        {
            QString path;
            mntent *me;
            FILE *mntfp = NULL;
            QHash<QString, QString> mountEntries;

            mntfp = setmntent( "/proc/mounts", "r" );
            me = getmntent(mntfp);
            while(me != NULL)
            {
                mountEntries[me->mnt_fsname] = me->mnt_dir;
                me = getmntent(mntfp);
            }
            endmntent(mntfp);
            if(mountEntries.contains(fs->disk()))
                path=mountEntries[fs->disk()];

            fs->update(connected, path);
        }
    }
    emit disksChanged();
}

/*!
  Returns a string containing the name, path, size and read/write parameters
  of all known filesystems

  \sa installLocationsString()
*/
QString QStorageMetaInfo::cardInfoString()
{
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    return infoString( fileSystems( &fsf ), "" );
}

/*!
  Includes the same information as QStorageMetaInfo::cardInfoString() and the path
  where documents can be installed

  \sa cardInfoString()
*/
QString QStorageMetaInfo::installLocationsString()
{
    QFileSystemFilter fsf;
    fsf.applications = QFileSystemFilter::Set;
    return infoString( fileSystems( &fsf ), "/Documents" );
}

/*!
  \internal Returns infostrings for each of the \a filesystems passed, appending \a extension
  to the path.

  \sa cardInfoString(), installLocationsString()
*/
QString QStorageMetaInfo::infoString( QList<QFileSystem*> filesystems, const QString &extension )
{
    //storage->update();
    QString s;
    foreach ( QFileSystem *fs, filesystems ) {
        fs->update();
        s += fs->name() + "=" + fs->path() + extension + " "
             + QString::number( fs->availBlocks() * (fs->blockSize() / 256) / 4 )
             + "K " + fs->options() + ";";
    }
    return s;
}

/*!
  Returns a list of available mounted file systems matching the \a filter. Use \a connectedOnly to return
  a list of only the filesystems that are connected.

  \sa fileSystemNames(), QFileSystemFilter
*/
QList<QFileSystem*> QStorageMetaInfo::fileSystems( QFileSystemFilter *filter, bool connectedOnly )
{
    if(d->fileSystems.count() == 0)
        update();
    QList<QFileSystem*> ret;
    foreach ( QFileSystem *fs, d->fileSystems ) {
        if(fs->isConnected() || connectedOnly == false) {
            fs->update();
            if ( !filter || filter->filter( fs ) )
                ret << fs;
        }
    }
    return ret;
}

/*!
  Returns a list of file system names matching the \a filter. Use \a connectedOnly to return
  a list of only the filesystems that are connected.

  \sa fileSystems(), QFileSystemFilter
*/
QStringList QStorageMetaInfo::fileSystemNames( QFileSystemFilter *filter, bool connectedOnly )
{
    if(d->fileSystems.count() == 0)
        update();
    QStringList strings;
    foreach ( QFileSystem *fs, fileSystems(filter, connectedOnly) )
        strings << fs->name();
    return strings;
}

/*! \fn void QStorageMetaInfo::disksChanged()
  This signal is emitted whenever a disk has been mounted or unmounted, such as when
  a CF card has been inserted or removed.
*/

/*!
    Returns a pointer to a static instance of QStorageMetaInfo.
*/
Q_GLOBAL_STATIC(QStorageMetaInfo, storageMetaInfoInstance);

/*!
    Singleton accessor for an application wide QStorageMetaInfo. This is the preferred means for
    creating a QStorageMetaInfo object.
*/
QStorageMetaInfo *QStorageMetaInfo::instance()
{
    return storageMetaInfoInstance();
}

//---------------------------------------------------------------------------

class QFileSystemPrivate : public QSharedData
{
public:
    QFileSystemPrivate()
        : removable( false )
        , applications( false )
        , documents( false )
        , contentDatabase( false )
        , connected( false )
        , blockSize( 512 )
        , totalBlocks( 0 )
        , availBlocks( 0 )
    {
    }
    QFileSystemPrivate(const QFileSystemPrivate& other) : QSharedData(other) { operator=(other); }

    const QFileSystemPrivate &operator=(const QFileSystemPrivate&);

    QString disk;
    QString path;
    QString prevPath;
    QString options;
    QString name;
    QString documentsPath;
    QString applicationsPath;
    bool removable;
    bool applications;
    bool documents;
    bool contentDatabase;
    bool connected;
    QMap<QString, QVariant> values;

    long blockSize;
    long totalBlocks;
    long availBlocks;
};

const QFileSystemPrivate &QFileSystemPrivate::operator=(const QFileSystemPrivate &other)
{
    disk = other.disk;
    path = other.path;
    prevPath = other.prevPath;
    options = other.options;
    name = other.name;
    documentsPath = other.documentsPath;
    applicationsPath = other.applicationsPath;
    removable = other.removable;
    applications = other.applications;
    documents = other.documents;
    contentDatabase = other.contentDatabase;
    connected = other.connected;
    values = other.values;
    blockSize = other.blockSize;
    totalBlocks = other.totalBlocks;
    availBlocks = other.availBlocks;
    return *this;
}


/*!
    \class QFileSystem
    \inpublicgroup QtBaseModule
    \brief The QFileSystem class describes a single mount point.

    This class is an informational result structure returned by the QStorageMetaInfo class.
    This class should not be created directly, but should rather be used in tandem
    with the QStorageMetaInfo class.

    \ingroup io
    \ingroup content
*/

/*!
    Construct an empty QFileSystem object.
*/
QFileSystem::QFileSystem()
{
    d = new QFileSystemPrivate;
}

/*!
    Construct a copy of \a other.
*/
QFileSystem::QFileSystem( const QFileSystem &other )
    : d( other.d )
{
}

/*!
    \internal Called by QStorageMetaInfo to construct and initialise a QFileSystem object
    for use.
*/

QFileSystem::QFileSystem( const QString &disk, const QString &path, const QString &prevPath, const QString &options,
                        const QString &name, const QString &documentsPath, const QString &applicationsPath, bool removable,
                        bool applications, bool documents, bool contentDatabase, bool connected,
                        const QMap<QString, QVariant> &values)
{
    d = new QFileSystemPrivate;

    d->disk = disk;
    d->path = path;
    d->prevPath = prevPath;
    d->options = options;
    d->name = name;
    d->documentsPath = path + documentsPath;
    d->applicationsPath = path + applicationsPath;
    d->removable = removable;
    d->applications = applications;
    d->documents = documents;
    d->contentDatabase = contentDatabase;
    d->connected = connected;
    d->values = values;
}

/*!
    Destroys this QFileSystem object.
*/
QFileSystem::~QFileSystem()
{
}

/*!
    Assign the contents of \a other to this object.
*/
QFileSystem &QFileSystem::operator =( const QFileSystem &other )
{
    d = other.d;

    return *this;
}

/*!
    \internal Update the size infromation for this QFileSystem.
*/

void QFileSystem::update()
{
    QFileSystemPrivate *_d = const_cast< QFileSystemPrivate * >( d.constData() ); // Circumvent the copy on write.

    _d->blockSize = 0;
    _d->totalBlocks = 0;
    _d->availBlocks = 0;
#ifdef QT_LSB
    struct statvfs fs;
    if (_d->connected && statvfs(_d->path.toLocal8Bit().constData(), &fs) == 0) {
        _d->blockSize = fs.f_bsize;
        _d->totalBlocks = fs.f_blocks;
        _d->availBlocks = fs.f_bavail;
    }
#else
    struct statfs fs;
    if ( _d->connected && statfs( _d->path.toLocal8Bit(), &fs ) ==0 ) {
        _d->blockSize = fs.f_bsize;
        _d->totalBlocks = fs.f_blocks;
        _d->availBlocks = fs.f_bavail;
    }
#endif
}

/*!
    \internal Update the size infromation for this QFileSystem only if the \a connected state has
    changed, passing \a path in, in case of disconnection to keep track of where a filesystem was mounted.
*/
void QFileSystem::update( bool connected, const QString &path )
{
    if( d.constData()->connected != connected )
    {
        QFileSystemPrivate *_d = const_cast< QFileSystemPrivate * >( d.constData() ); // Circumvent the copy on write.

        _d->connected = connected;

        _d->documentsPath = path + _d->documentsPath.mid( _d->path.length() );
        _d->applicationsPath = path + _d->applicationsPath.mid( _d->path.length() );

        _d->prevPath = _d->path;
        _d->path = path;
    }
    update();
}

/*!
    Returns true if the QFileSystem is unitialised.
*/
bool QFileSystem::isNull() const
{
    return d->name.isEmpty();
}

/*!
  Returns the file system name, eg. /dev/hda3
*/
const QString &QFileSystem::disk() const
{
    return d->disk;
}

/*!
  Returns the mount path, eg. /home
*/
const QString &QFileSystem::path() const
{
    return d->path;
}

/*!
    Returns the path of the documents directory, eg. /home/Documents.
*/
const QString &QFileSystem::documentsPath() const
{
    return d->documentsPath;
}

/*!
    Returns the path of the applications directory, eg. /home/Applications.
*/
const QString &QFileSystem::applicationsPath() const
{
    return d->applicationsPath;
}

/*!
    Returns the path for files of \a type stored on a filesystem.

    If no path is defined this returns the root path of the filesystem.

    The return value of this is only valid if \l storesType() returns true.
*/
QString QFileSystem::typePath(const QString &type) const
{
    QMap<QString, QVariant>::const_iterator iterator = d->values.find(type + QLatin1String("Path"));

    return iterator != d->values.end() && iterator.value().type() == QVariant::String
        ? d->path + iterator.value().toString()
        : d->path;
}

/*!
  Returns the previous mount path, eg. /home
  This is useful when a filesystem has been unmounted.
*/
const QString &QFileSystem::prevPath() const
{
    return d->prevPath;
}

/*!
  Returns the translated, human readable name for the mount directory.
*/
const QString &QFileSystem::name() const
{
    return d->name;
}

/*!
  Returns the mount options
*/
const QString &QFileSystem::options() const
{
    return d->options;
}

/*!
  Returns the size of each block on the file system.
*/
long QFileSystem::blockSize() const
{
    return d->blockSize;
}

/*!
  Returns the total number of blocks on the file system
*/
long QFileSystem::totalBlocks() const
{
    return d->totalBlocks;
}

/*!
  Returns the number of available blocks on the file system
 */
long QFileSystem::availBlocks() const
{
    return d->availBlocks;
}

/*!
  Returns flag indicating if the file system can be removed. For example, a CF card would be removable, but the internal memory would not.
*/
bool QFileSystem::isRemovable() const
{
    return d->removable;
}

/*!
  Returns flag indicating if the file system can contain applications.
*/
bool QFileSystem::applications() const
{
    return d->applications;
}

/*!
  Returns flag indicating if the file system can contain documents.
*/
bool QFileSystem::documents() const
{
    return d->documents;
}

/*!
    Returns true if the file system can store files of \a type.

    The path of these files is given by \l {typePath()}.
*/
bool QFileSystem::storesType(const QString &type) const
{
    QMap<QString, QVariant>::const_iterator iterator = d->values.find(type);

    return iterator != d->values.end() && iterator.value().canConvert(QVariant::Bool)
        ? iterator.value().toBool()
        : false;
}

/*!
  Returns flag indicating if the file system is mounted as writable or read-only.
  Returns false if read-only, true if read and write.
*/
bool QFileSystem::isWritable() const
{
    return d->options.contains("rw");
}

/*!
    Connects the filesystem to the content system by sending the QCop message
    QPE/QStorage::mounting(QString).

    \bold {Note:} This call is asynchronous.  QStorageMetaInfo will emit the disksChanged()
    signal once the filesystem has been connected.

    \sa isConnected(), QStorageMetaInfo::disksChanged()
*/
void QFileSystem::connect() const
{
    if (!isConnected()) {
        QtopiaIpcEnvelope e("QPE/QStorage", "mounting(QString)");
        e << disk();
    }
}

/*!
    Disconnects the filesystem from the content system by sending the QCop message
    QPE/QStorage::unmounting(QString).

    \bold {Note:} This call is asynchronous.  QStorageMetaInfo will emit the disksChanged()
    signal once the filesystem has been disconnected.

    \sa isConnected(), QStorageMetaInfo::disksChanged()
*/
void QFileSystem::disconnect() const
{
    if (isConnected()) {
        QtopiaIpcEnvelope e("QPE/QStorage", "unmounting(QString)");
        e << disk();
    }
}

/*!
    Returns true if the filesystem is mounted.

    \sa mount(), unmount()
*/
bool QFileSystem::isMounted() const
{
    FILE *mntfp = setmntent( "/proc/mounts", "r" );
    mntent *me = getmntent(mntfp);
    while(me != NULL)
    {
        if (disk() == me->mnt_fsname)
            return true;
        me = getmntent(mntfp);
    }
    endmntent(mntfp);

    return false;
}

/*!
    Unmounts the filesystem from the system.  Returns true on success, false otherwise.
    Returns false if the filesystem is already unmounted.

    The filesystem must have previously been disconnected from the content system with
    disconnect().

    \sa mount(), disconnect()
*/
bool QFileSystem::unmount() const
{
    if (isConnected())
        return false;

    return QProcess::execute("umount " + path()) == 0;
}

/*!
    Mounts the filesystem.  Returns true on success, false otherwise.
    Returns false if the filesystem is already mounted.

    The filesystem must have a corresponding entry in the \c {/etc/fstab} file.
    The mount point directory must exist.

    After the filesystem is sucessfully mounted it should be connected to the content
    system by calling connect().

    \sa unmount(), connect()
*/
bool QFileSystem::mount() const
{
    if (isMounted())
        return false;

    return QProcess::execute("mount " + path()) == 0;
}

/*!
  Returns flag indicating if the file system has it's own content database stored.
 */
bool QFileSystem::contentDatabase() const
{
    return d->contentDatabase;
}

/*!
    Returns flag indicating if the file system is currently connected.

    \sa connect(), disconnect()
*/
bool QFileSystem::isConnected() const
{
    return d->connected;
}

/*!
    Returns a QFileSystem object describing the file system on which the file with the name \a fileName is located.

    If \a connectedOnly is true the QFileSystem will only be returned if it is currently connected.
*/
QFileSystem QFileSystem::fromFileName( const QString &fileName, bool connectedOnly )
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->fileSystemOf( fileName, connectedOnly );

    return fs ? *fs : QFileSystem();
}

/*!
    Returns the QFileSystem which contains the default document storage path.

    The default document storage path can be obtained by calling \c{QFileSystem::documentsFileSystem().documentsPath()}.

    \sa documentsPath()
*/
QFileSystem QFileSystem::documentsFileSystem()
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->documentsFileSystem();

    return fs ? *fs : QFileSystem();
}

/*!
    Returns the QFileSystem which contains the default application storage path.

    The default application storage path can be obtained by calling \c{QFileSystem::applicationsFileSystem().applicationsPath()}.

    \sa applicationsPath()
 */
QFileSystem QFileSystem::applicationsFileSystem()
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->applicationsFileSystem();

    return fs ? *fs : QFileSystem();
}

/*!
    Returns the QFileSystem that is the default storage location for files of \a type.
*/
QFileSystem QFileSystem::typeFileSystem(const QString &type)
{
    const QFileSystem *fs = QStorageMetaInfo::instance()->typeFileSystem(type);

    return fs ? *fs : QFileSystem();
}

// ====================================================================

/*!
  \class QFileSystemFilter
    \inpublicgroup QtBaseModule
  \brief The QFileSystemFilter class is used to restrict the available filesystems returned from QStorageMetaInfo.

  Extending the filter class is relatively simple.

  \code
class WriteableFileSystemFilter : public QFileSystemFilter
{
public:
    WriteableFileSystemFilter()
        : writable( QFileSystemFilter::Either )
    {
    }

    bool filter( QFileSystem *fs )
    {
        if ( (writable == QFileSystemFilter::Set && !fs->isWritable()) ||
             (writable == QFileSystemFilter::NotSet && fs->isWritable()) )
            return false;
        else
            return QFileSystemFilter::filter(fs);
    }

    QFileSystemFilter::FilterOption writable;
};
  \endcode

  \ingroup io
  \ingroup content
  \sa QStorageMetaInfo
*/

/*!
  \enum QFileSystemFilter::FilterOption

  This enum is used to describe the filter conditions for file systems.

  \value Set The filter matches if the connected condition is set.
  \value NotSet The filter matches if the connected condition is not set
  \value Either The filter always matches.
*/

/*! Constructor that initializes the filter to allow any filesystem.
*/
QFileSystemFilter::QFileSystemFilter()
    : documents(Either), applications(Either), removable(Either),
      content(Either)
{
}

/*! Destructor
*/
QFileSystemFilter::~QFileSystemFilter()
{
}

/*! Returns true if the \a fs matches the filter; otherwise returns false.
*/
bool QFileSystemFilter::filter( QFileSystem *fs )
{
    if ( (documents == Set && !fs->documents()) ||
         (documents == NotSet && fs->documents()) )
        return false;
    if ( (applications == Set && !fs->applications()) ||
         (applications == NotSet && fs->applications()) )
        return false;
    if ( (removable == Set && !fs->isRemovable()) ||
         (removable == NotSet && fs->isRemovable()) )
        return false;
    if ( (content == Set && !fs->contentDatabase()) ||
         (content == NotSet && fs->contentDatabase()) )
        return false;
    return true;
}

