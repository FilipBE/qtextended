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

#include "contentserver.h"

#include <qstorage.h>
#include <qtopiaapplication.h>
#include <qmimetype.h>
#include <qcontent.h>
#include <qtopiaipcenvelope.h>
#include <qtopialog.h>
#include <qcategorymanager.h>
#include <qtopiasql.h>
#include <qdrmcontentplugin.h>


#include <QTimer>
#include <QDebug>
#include <QStyle>
#include <QSemaphore>
#include <QValueSpaceObject>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QContentPlugin>

// Making this larger will cause the scanner to go more deeply into
// subdirectories
static const int MaxSearchDepth = 10;

/*
  Recursive threaded background directory scanner for advanced use only.
  This class is used behind the scenes in the ContentServer and should
  not be needed unless undertaking advanced customization.
 */

class DirectoryScanner : public QObject
{
    Q_OBJECT
public:
    DirectoryScanner();
    ~DirectoryScanner();

public slots:
    void scan(const QString &path, int priority);

signals:
    void scanning(bool scanning);

private slots:
    void scan();

private:
    void addPath(const QString &path, int depth, int priority);
    void scanPath(const QString& path, int depth, int priority);
    void cleanupThumbnails(const QDateTime &threshold);
    QString thumbnailDir(const QString &path) const;
    QString thumbnailPath(const QString &thumbnailDir, const QString &fileName) const;

    void install(const QFileInfo &fi);
    void uninstall(QContentId id);
    void flushInstalls();
    void flushUninstalls();

    struct PendingPath {
        QString path;
        int priority;
        int depth;

        bool operator<(const PendingPath &pp) const {
            return priority < pp.priority;
        }
    };
    QList<PendingPath> m_pendingPaths;
    QDateTime m_startTime;
    QFileInfoList m_pendingInstalls;
    QContentIdList m_pendingUninstalls;

    bool m_scanning;
};

// This value controls how many doclinks are processed at one go before firing off a single shot timer to yield processing
// before continuing processing
const int docsPerShot = 200;

/*!
  \class ContentServer
    \inpublicgroup QtBaseModule
  \brief The ContentServer class provides the Documents API.
  \internal

  Server process implementing the ContentServerInterface, running in
  a separate thread.  Handles updating ContentSet objects in reponse
  to database updates.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

////////////////////////////////////////////////////////////////////////
/////
/////  ContentServer implementation

ContentServer::ContentServer( QObject *parent )
    : QThread(parent)
{
    qLog(DocAPI) << "content server constructed";

    requestQueue = new QtopiaIpcAdaptor(QLatin1String("QPE/DocAPI"), this);

    QtopiaIpcAdaptor::connect(requestQueue, MESSAGE(scanPath(QString,int)), this, SIGNAL(scan(QString,int)));

    scannerVSObject = new QValueSpaceObject("/Documents", this);

    // force early initialisation of QMimeType to remove the chance of issues later on when we're in threads.
    QContentPlugin::preloadPlugins();
    QDrmContentPlugin::initialize();
    QMimeType::updateApplications();
}

ContentServer::~ContentServer()
{
}

/*!
  \internal
  This code runs in a separate thread to that in which the ContentServer
  object is created.  It sets up the initial scanning requests by adding
  them to the queue object, then enters the thread's event loop.

  Inside the event loop changes to the QContentSet's internal QStorageMetaInfo
  object, linkChanged(...) messages, and new requests will cause new scans
  to be launched.  New requests may be posted by sending
  QtopiaIpcAdaptor::send(SIGNAL(scanPath(QString,int),path,priority)
  to send to the QPE/DocAPI channel, with path being the path to be scanned.
*/
void ContentServer::run()
{
    DirectoryScanner scanner;

    connect(this, SIGNAL(scan(QString,int)), &scanner, SLOT(scan(QString,int)));

    connect(&scanner, SIGNAL(scanning(bool)), this, SLOT(scanning(bool)));

    exec();
}

void ContentServer::scanAll()
{
    emit scan(QLatin1String("all"), 0);
}

void ContentServer::scanning(bool scanning)
{
    scannerVSObject->setAttribute(QLatin1String("Scanning"), scanning);
}

static bool binaryStringLessThan(const QString &string1, const QString &string2)
{
  int length = qMin(string1.length(), string2.length()) * 2;

  int c = memcmp(string1.unicode(), string2.unicode(), length);

  if (c < 0)
      return true;
  else if (c == 0)
      return string1.length() - string2.length() < 0;
  else
      return false;
}

static bool binaryStringCompare(const QString &string1, const QString &string2)
{
    int length = qMin(string1.length(), string2.length()) * 2;

    int c = memcmp(string1.unicode(), string2.unicode(), length);

    return c == 0 ? string1.length() - string2.length() : c;
}

/*!
   Construct a directory scanner object representing one thread scanning
   one directory
*/
DirectoryScanner::DirectoryScanner()
    : m_scanning(false)
{
}

/*!
    Destroy this directory scanner object, releasing a thread resource so
    that other scans may start up.  A semaphore is used to ensure no more
    than MaxDirThreads (typically 10) threads are scanning at once.
*/
DirectoryScanner::~DirectoryScanner()
{
}

void DirectoryScanner::scan(const QString &path, int priority)
{
    if (path == QLatin1String("all")) {
        QFileSystemFilter fsf;
        fsf.documents = QFileSystemFilter::Set;
        foreach (QFileSystem *fs, QStorageMetaInfo::instance()->fileSystems(&fsf, true))
            addPath(fs->documentsPath(), 0, priority);

        if (m_startTime.isNull())
            m_startTime = QDateTime::currentDateTime();
    } else {
        addPath(path, 0, priority);
    }
}

void DirectoryScanner::addPath(const QString &path, int depth, int priority)
{
    PendingPath pp;
    pp.path = path;
    pp.priority = priority;
    pp.depth = depth;
    for (int i = 0; i < m_pendingPaths.count(); ++i) {
        if (m_pendingPaths.at(i).path == path) {
            m_pendingPaths.removeAt(i);
            break;
        }
    }
    m_pendingPaths.append(pp);
    qSort(m_pendingPaths);

    if (!m_scanning) {
        emit scanning(m_scanning = true);

        QTimer::singleShot(0, this, SLOT(scan()));
    }
}

void DirectoryScanner::scan()
{
    if (!m_pendingPaths.isEmpty()) {
        PendingPath pending = m_pendingPaths.takeLast();

        scanPath(pending.path, pending.depth, pending.priority);

        QTimer::singleShot(0, this, SLOT(scan()));
    } else {
        flushInstalls();
        flushUninstalls();

        if (!m_startTime.isNull()) {
            cleanupThumbnails(m_startTime);

            m_startTime = QDateTime();

            QTimer::singleShot(0, this, SLOT(scan()));
        } else {
            emit scanning(m_scanning = false);

            qLog(DocAPI) << "finished scanning";
        }
    }
}

void DirectoryScanner::scanPath(const QString& path, int depth, int priority)
{
    const QString cleanPath = QDir::cleanPath(path);
    const QString dirPath = cleanPath + QLatin1Char('/');
    const QString unknownType(QLatin1String("application/octet-stream"));
    qLog(DocAPI) << "scan called for path" << dirPath << "depth" << depth;

    if (depth==0) { // only do it for the top level of this directory
        // Now scan for all files under this location, and check if they're valid or not, if not, add them to the removelist too.
        QStringList paths = QContentFilter(QContentFilter::Location, dirPath)
                .argumentMatches(QContentFilter::Directory, QString());

        QContentFilter removedPaths;

        foreach (QString path, paths) {
            if (!QFile::exists(path)) {
                QContentIdList contentIds = QContentSet(QContentFilter::Directory, path).itemIds();

                foreach (QContentId contentId, contentIds)
                    uninstall(contentId);
            }
        }
    }

    const QString thumbDir = thumbnailDir(cleanPath);

    QContentSet dirContents;
    dirContents.setCriteria(QContentFilter::Directory, cleanPath);
    dirContents.setSortCriteria(QContentSortCriteria(QContentSortCriteria::FileName));

    QStringList fileNames = QDir(dirPath).entryList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    qSort(fileNames.begin(), fileNames.end(), binaryStringLessThan);

    int db = 0;
    int fs = 0;

    const int dbCount = dirContents.count();
    const int fsCount = fileNames.count();

    char buffer[16];

    while (db < dbCount && fs < fsCount) {
        QContent content = dirContents.content(db);
        const QString fsName = fileNames.at(fs);
        const QString dbName = content.fileName().mid(dirPath.length());

        int comparison = binaryStringCompare(dbName, fsName);

        if (comparison == 0) {
            if (content.type() == unknownType && QMimeType::fromFileName(content.fileName()).id() != unknownType) {
                content.setName(QString());
                content.setType(QString());
                content.commit();
            } else if (QFileInfo(content.fileName()).lastModified() > content.lastUpdated()) {
                content.commit();
            } else if (!thumbDir.isNull()) {
                QFile thumb(thumbnailPath(thumbDir, content.fileName()));
 
                 if (thumb.exists() && thumb.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
                     thumb.read(buffer, 16);
                     thumb.close();
                 }
            }
            ++db;
            ++fs;
        } else if (comparison < 0) {
            uninstall(content.id());

            ++db;
        } else if (comparison > 0) {
            install(QFileInfo(dirPath + fsName));

            ++fs;
        }
    }

    while(db < dbCount)
        uninstall(dirContents.contentId(db++));
    while(fs < fsCount)
        install(QFileInfo(dirPath + fileNames.at(fs++)));

    if (depth < MaxSearchDepth)
        foreach (QString fileName, QDir(dirPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            addPath(dirPath + fileName, depth + 1, priority - 1);
}

void DirectoryScanner::cleanupThumbnails(const QDateTime &startTime)
{
    const QString thumbnails(QLatin1String("Thumbnails"));

    QFileSystemFilter filter;

    QStringList paths;

    foreach (const QFileSystem *fs, QStorageMetaInfo::instance()->fileSystems(&filter))
        if (fs->storesType(thumbnails))
            paths.append(fs->typePath(thumbnails));

    if (!QStorageMetaInfo::instance()->typeFileSystem(thumbnails))
        paths.append(Qtopia::homePath() + QLatin1String("/Thumbnails"));

    foreach (QString path, paths) {
        QDirIterator iterator(path, QDir::Files);

        while (iterator.hasNext()) {
            QString path = iterator.next();

            QFileInfo fileInfo = iterator.fileInfo();

            if (fileInfo.lastRead().secsTo(startTime) > 0)
                QFile::remove(path);
        }
    }
}

QString DirectoryScanner::thumbnailDir(const QString &path) const
{
    const QString thumbnails(QLatin1String("Thumbnails"));

    QFileSystem fs = QFileSystem::fromFileName(path);

    if (!fs.storesType(thumbnails))
        fs = QFileSystem::typeFileSystem(thumbnails);

    QString dir = !fs.isNull()
            ? fs.typePath(thumbnails)
            : Qtopia::homePath() + QLatin1String("/Thumbnails");

    return QFile::exists(dir) ? dir + QLatin1Char('/') : QString();
}

QString DirectoryScanner::thumbnailPath(const QString &thumbnailDir, const QString &fileName) const
{
    QByteArray hash = QCryptographicHash::hash(
            fileName.toLocal8Bit(), QCryptographicHash::Md5).toHex();

    return thumbnailDir + QString::fromLatin1(hash.constData(), hash.length())
            + QLatin1String(".png");
}

void DirectoryScanner::install(const QFileInfo &fi)
{
    if (!QtopiaSql::instance()->isDatabase(fi.absoluteFilePath())) {
        m_pendingInstalls.append(fi);

        if (m_pendingInstalls.count() == docsPerShot) {
            QContent::installBatch(m_pendingInstalls);

            m_pendingInstalls.clear();
        }
    }
}

void DirectoryScanner::DirectoryScanner::flushInstalls()
{
    if (!m_pendingInstalls.isEmpty()) {
        QContent::installBatch(m_pendingInstalls);

        m_pendingInstalls.clear();
    }
}

void DirectoryScanner::uninstall(QContentId id)
{
    m_pendingUninstalls.append(id);

    if (m_pendingUninstalls.count() == docsPerShot) {
        QContent::uninstallBatch(m_pendingUninstalls);

        m_pendingUninstalls.clear();
    }
}

void DirectoryScanner::flushUninstalls()
{
    if (!m_pendingUninstalls.isEmpty()) {
        QContent::uninstallBatch(m_pendingUninstalls);

        m_pendingUninstalls.clear();
    }
}

// define ContentServerTask
/*!
  \class ContentServerTask
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The ContentServerTask class manages the Documents API's document scanning functionality.

  The content server is responsible for periodically scanning document 
  directories to maintain the integrity of the documents database.
  The content server is also responsible for scanning newly detected media,
  such as extenal memory cards, for content and integrating them into the 
  document model.

  The ContentServerTask class provides the \c {ContentServer} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */
QTOPIA_TASK(ContentServer, ContentServerTask);
QTOPIA_TASK_PROVIDES(ContentServer, SystemShutdownHandler);
QTOPIA_TASK(DocumentServer,DocumentServerTask);
QTOPIA_TASK_PROVIDES(DocumentServer, SystemShutdownHandler);

/*!
    Constructs a new content server task.

    \internal
*/
ContentServerTask::ContentServerTask()
{
    m_server.start(QThread::LowPriority);

    QTimer::singleShot(4000, &m_server, SLOT(scanAll())); 
}

/*!
    \reimp
 */
bool ContentServerTask::systemRestart()
{
    doShutdown();
    return false;
}

/*!
    \reimp
*/
bool ContentServerTask::systemShutdown()
{
    doShutdown();
    return false;
}

/*! \internal */
void ContentServerTask::doShutdown()
{
    QObject::connect(&m_server, SIGNAL(finished()), this, SIGNAL(proceed()));

    m_server.quit();
}

/*!
    \class DocumentServerTask
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::Task
    \brief The DocumentServerTask class manages the lifetime of the Qt Extended Document Server.

    The \l{QtopiaDocumentServer}{Qt Extended Document Server} provides a secure interface for applications to access
    the functionality of the \l{Document System}{Qt Extended Document System} without having access to the document
    databases or the file system.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
    \sa QtopiaDocumentServer
*/

/*!
    Constructs a new document server task.

    \internal
*/
DocumentServerTask::DocumentServerTask()
{
    connect( &m_server, SIGNAL(shutdownComplete()), this, SIGNAL(proceed()) );
}

/*!
    \reimp
*/
bool DocumentServerTask::systemRestart()
{
    m_server.shutdown();

    return false;
}

/*!
    \reimp
*/
bool DocumentServerTask::systemShutdown()
{
    m_server.shutdown();

    return false;
}

#include "contentserver.moc"
