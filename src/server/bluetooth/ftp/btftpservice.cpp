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

#include "btftpservice.h"
#include "qtopiaserverapplication.h"
#include <QBluetoothRfcommServer>
#include <QBluetoothRfcommSocket>
#include <QBluetoothAbstractService>
#include <QBluetoothLocalDevice>
#include <QBluetoothAddress>
#include <QBluetoothSdpRecord>
#include <QBuffer>

#include <QObexServerSession>
#include <QObexHeader>

#include <QCommDeviceSession>

#include <qtopianamespace.h>
#include <qtopiaserverapplication.h>

#include <QTimer>
#include <QFile>
#include <QFileInfo>

#include <QStorageMetaInfo>
#include <QFileSystem>
#include <QContent>
#include <QList>
#include <QContentSetModel>
#include <QHash>
#include <QMimeType>
#include <QValueSpaceItem>

static char target_uuid[] = {
    0xF9,
    0xEC,
    0x7B,
    0xC4,
    0x95,
    0x3C,
    0x11,
    0xD2,
    0x98,
    0x4E,
    0x52,
    0x54,
    0x00,
    0xDC,
    0x9E,
    0x09
};

static const char listing_header[] = "<?xml version=\"1.0\"?>\n<!DOCTYPE folder-listing SYSTEM \"obex-folder-listing.dtd\">\n<folder-listing version=\"1.0\">\n";

static const char listing_footer[] = "</folder-listing>\n";

static const char parent_folder_tag[] = "\t<parent-folder />\n";

static const QString LOCAL_TIME_FORMAT = "yyyyMMddThhmmss";
static const QString UTC_TIME_FORMAT = LOCAL_TIME_FORMAT + 'Z';

class BtFtpDirectoryInfo : public QObject
{
    Q_OBJECT

public:
    BtFtpDirectoryInfo(const QString &directory, QObject *parent = 0);
    ~BtFtpDirectoryInfo();

    QByteArray listing() const;
    QContentSetModel *model();
    QString directory() const;
    QContentSet *set();

public slots:
    void contentsChanged();

private:
    mutable bool m_needsUpdate;
    mutable QByteArray m_cachedListing;
    QContentSet *m_set;
    QContentSetModel *m_model;
    QString m_disk;
    QString m_directory;
};

BtFtpDirectoryInfo::BtFtpDirectoryInfo(const QString &directory, QObject *parent)
    : QObject(parent), m_needsUpdate(true), m_directory(directory)
{
    QContentFilter fsFilter = QContentFilter( QContent::Document ) &
                            QContentFilter( QContentFilter::Location, directory);

    m_set = new QContentSet(fsFilter, this);
    m_model = new QContentSetModel(m_set, this);

    connect( m_model, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(layoutChanged()),
                this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(modelReset()),
                this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this , SLOT(contentsChanged()) );
    connect( m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this , SLOT(contentsChanged()) );
}

BtFtpDirectoryInfo::~BtFtpDirectoryInfo()
{
    delete m_model;
    delete m_set;
}

void BtFtpDirectoryInfo::contentsChanged()
{
    m_needsUpdate = true;
}

QByteArray BtFtpDirectoryInfo::listing() const
{
    if (m_needsUpdate) {
        m_cachedListing = listing_header;
        m_cachedListing.append(parent_folder_tag);

        int rowCount = m_model->rowCount();
        for ( int idx = 0; idx < rowCount; idx++ ) {
            QContent content = m_model->content(idx);
            if ( content.isValid() ) {
                QFileInfo fileInfo(content.fileName());
                m_cachedListing.append("\t<file name=\"");
                m_cachedListing.append(fileInfo.fileName().toUtf8());
                m_cachedListing.append("\" size=\"");
                m_cachedListing.append(QString::number(content.size()).toUtf8());
                m_cachedListing.append("\" type=\"");
                m_cachedListing.append(content.type().toUtf8());
                m_cachedListing.append("\" user-perm=\"");

                if (content.permissions() & QDrmRights::Distribute)
                    m_cachedListing.append("RWD");
                else
                    m_cachedListing.append("WD");

                m_cachedListing.append("\" modified=\"");
                m_cachedListing.append(content.lastUpdated().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
                m_cachedListing.append("\" accessed=\"");
                m_cachedListing.append(fileInfo.lastRead().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
                m_cachedListing.append("\" created=\"");
                m_cachedListing.append(fileInfo.created().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
                m_cachedListing.append("\" >");
                m_cachedListing.append(content.name().toUtf8());
                m_cachedListing.append("</file>\n");
            }
        }

        m_cachedListing.append(listing_footer);
        m_needsUpdate = false;
    }

    return m_cachedListing;
}

QString BtFtpDirectoryInfo::directory() const
{
    return m_directory;
}

QContentSetModel *BtFtpDirectoryInfo::model()
{
    return m_model;
}

QContentSet *BtFtpDirectoryInfo::set()
{
    return m_set;
}

class BtFtpContentManager : public QObject
{
    Q_OBJECT

public:
    BtFtpContentManager(QObject *parent = 0);
    ~BtFtpContentManager();

    BtFtpDirectoryInfo *info(const QString &disk);
    QByteArray rootListing();
    QFileSystem *fileSystemForDisk(const QString &disk);

private slots:
    void disksChanged();

private:
    QHash<QString, BtFtpDirectoryInfo *> m_disks;
    QHash<QString, QFileSystem *> m_fs;
    QStorageMetaInfo *m_storage;

    bool m_rootListingChanged;
    QByteArray m_cachedRootListing;
};

BtFtpContentManager::BtFtpContentManager(QObject *parent)
    : QObject(parent)
{
    m_storage = new QStorageMetaInfo(this);
    connect(m_storage, SIGNAL(disksChanged()), this, SLOT(disksChanged()));

    QTimer::singleShot(0, this, SLOT(disksChanged()));
}

BtFtpContentManager::~BtFtpContentManager()
{
    delete m_storage;
}

void BtFtpContentManager::disksChanged()
{
    QFileSystemFilter filter;
    filter.documents = QFileSystemFilter::Set;

    QList<QFileSystem *> fslist = m_storage->fileSystems(&filter);

    QList<QString> oldDiskList = m_disks.uniqueKeys();
    QSet<QString> oldDisks = QSet<QString>::fromList(oldDiskList);

    m_fs.clear();
    foreach (QFileSystem *fs, fslist) {
        m_fs.insert(fs->name(), fs);
    }

    QList<QString> newDiskList = m_fs.uniqueKeys();
    QSet<QString> newDisks = QSet<QString>::fromList(newDiskList);

    QSet<QString> added = newDisks - oldDisks;

    foreach (QString disk, added) {
        QHash<QString, QFileSystem *>::iterator it = m_fs.find(disk);
        Q_ASSERT(it != m_fs.end());

        BtFtpDirectoryInfo *info = new BtFtpDirectoryInfo(it.value()->documentsPath(), this);
        m_disks.insert(disk, info);
    }

    QSet<QString> removed = oldDisks - newDisks;

    foreach (QString disk, removed) {
        QHash<QString, BtFtpDirectoryInfo *>::iterator it = m_disks.find(disk);
        Q_ASSERT(it != m_disks.end());
        delete it.value();
        m_disks.erase(it);
    }

    m_rootListingChanged = true;
}

QFileSystem *BtFtpContentManager::fileSystemForDisk(const QString &disk)
{
    QHash<QString, QFileSystem *>::iterator it = m_fs.find(disk);

    if (it != m_fs.end()) {
        return it.value();
    }

    return NULL;
}

QByteArray BtFtpContentManager::rootListing()
{
    if (m_rootListingChanged) {
        m_cachedRootListing = listing_header;
        foreach (QFileSystem *fs, m_fs) {
            QFileInfo fileInfo(fs->documentsPath());

            m_cachedRootListing.append("\t<folder name=\"");
            m_cachedRootListing.append(fs->name().toUtf8());
            m_cachedRootListing.append("\" size=\"0\" user-perm=\"RWD\" modified=\"");
            m_cachedRootListing.append(fileInfo.lastModified().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
            m_cachedRootListing.append("\" accessed=\"");
            m_cachedRootListing.append(fileInfo.lastRead().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
            m_cachedRootListing.append("\" created=\"");
            m_cachedRootListing.append(fileInfo.created().toUTC().toString(UTC_TIME_FORMAT).toUtf8());
            m_cachedRootListing.append("\" />");
        }
        m_cachedRootListing.append(listing_footer);

        m_rootListingChanged = false;
    }

    return m_cachedRootListing;
}

BtFtpDirectoryInfo *BtFtpContentManager::info(const QString &disk)
{
    QHash<QString, BtFtpDirectoryInfo *>::iterator it = m_disks.find(disk);

    if (it != m_disks.end()) {
        return it.value();
    }

    return NULL;
}

class BtFtpSession : public QObexServerSession
{
    Q_OBJECT

public:
    BtFtpSession(QIODevice *device, BtFtpContentManager *mgr, QObject *parent = 0);
    ~BtFtpSession();

signals:
    void disconnected();

protected:
    void error(QObexServerSession::Error error, const QString &errorString);

    QObex::ResponseCode dataAvailable(const char *data, qint64 size);
    QObex::ResponseCode provideData(const char **data, qint64 *size);

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header);
    QObex::ResponseCode disconnect(const QObexHeader &header);
    QObex::ResponseCode put(const QObexHeader &header);
    QObex::ResponseCode putDelete(const QObexHeader &header);
    QObex::ResponseCode get(const QObexHeader &header);
    QObex::ResponseCode setPath(const QObexHeader &header, QObex::SetPathFlags flags);

private:
    int generateConnectionId() const;
    QObex::ResponseCode listDirectory(const QString &name);

    static QAtomicInt nextConnectionId;

    BtFtpContentManager *m_manager;
    QString m_currentPath;
    QByteArray m_currentListing;

    QIODevice *m_provider;
    QIODevice *m_consumer;

    QContent *m_content;
    char m_buf[4096];
};

QAtomicInt BtFtpSession::nextConnectionId(1);

BtFtpSession::BtFtpSession(QIODevice *device, BtFtpContentManager *manager, QObject *parent)
    : QObexServerSession(device, parent), m_manager(manager)
{
    m_provider = 0;
    m_consumer = 0;
    m_content = 0;
}

BtFtpSession::~BtFtpSession()
{
    delete m_provider;
    delete m_consumer;
    delete m_content;
}

int BtFtpSession::generateConnectionId() const
{
    register int id;
    for (;;) {
        id = nextConnectionId;
        if (nextConnectionId.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
}

void BtFtpSession::error(QObexServerSession::Error error, const QString &)
{
    if (m_provider) {
        m_provider->close();
        delete m_provider;
        m_provider = 0;
    }

    if (m_consumer) {
        m_consumer->close();
        delete m_consumer;
        m_consumer = 0;
    }

    if (m_content) {
        m_content->removeFiles();
        delete m_content;
        m_content = 0;
    }

    switch (error) {
        case QObexServerSession::Aborted:
            break;
        default:
            QTimer::singleShot(0, this, SIGNAL(disconnected()));
    };
}

QObex::ResponseCode BtFtpSession::dataAvailable(const char *data, qint64 size)
{
    if (m_consumer) {
        if (size > 0) {
            if (m_consumer->write(data, size) <= 0) {
                m_consumer->close();
                delete m_consumer;
                m_consumer = 0;

                m_content->removeFiles();
                delete m_content;
                m_content = 0;

                return QObex::InternalServerError;
            }
        }
        else {
            m_consumer->close();
            delete m_consumer;
            m_consumer = 0;
            m_content->setRole(QContent::Document);
            m_content->commit();
            delete m_content;
            m_content = 0;
        }
    }

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::provideData(const char **data, qint64 *size)
{
    qint64 len = m_provider->read(m_buf, 4096);

    if (len <= 0) {
        m_provider->close();
        delete m_provider;
        m_provider = 0;
    }

    if (len < 0)
        return QObex::InternalServerError;

    *data = m_buf;
    *size = len;

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::connect(const QObexHeader &header)
{
    qLog(Bluetooth) << "BtFtpSession: CONNECT request:" << header;

    if (header.target().isEmpty() || header.target() != QByteArray(target_uuid, sizeof(target_uuid)))
        return QObex::ServiceUnavailable;

    QObexHeader response;
    response.setConnectionId(generateConnectionId());

    if (!header.who().isEmpty()) {
        response.setTarget(header.who());
    }

    response.setWho(QByteArray(target_uuid, sizeof(target_uuid)));

    setNextResponseHeader(response);

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::disconnect(const QObexHeader &header)
{
    qLog(Bluetooth) << "BtFtpSession: DISCONNECT request:" << header;

    QTimer::singleShot(0, this, SIGNAL(disconnected()));

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::put(const QObexHeader &header)
{
    qLog(Bluetooth) << "BtFtpSession: PUT request:" << header;

    if (header.type() == "x-obex/folder-listing") {
        //TODO:
        // Clients can send this to customize their listing
        // output.  We just silently ignore it for now
        return QObex::Success;
    }

    if (header.name().isEmpty())
        return QObex::BadRequest;

    if (m_currentPath.isEmpty())
        return QObex::Unauthorized;

    BtFtpDirectoryInfo *info = m_manager->info(m_currentPath);
    if (!info) {
        return QObex::NotFound;
    }

    QString stripPath = header.name();
    int pos = stripPath.lastIndexOf("/");
    if ( pos != -1 )
        stripPath = stripPath.mid( pos + 1 );

    // Handle case where we're overwriting a file
    QContentSet *set = info->set();
    QContent oldContent = set->findFileName(stripPath);

    if (oldContent.isValid()) {
        oldContent.removeFiles();
        oldContent.commit();
    }

    m_content = new QContent;
    QMimeType mimetype;

    if (header.type().isEmpty())
        mimetype = QMimeType::fromFileName(stripPath);
    else
        mimetype = QMimeType::fromId(header.type());

    if (mimetype.isNull())
        m_content->setType("application/octet-stream");
    else
        m_content->setType(mimetype.id());

    pos = stripPath.lastIndexOf( "." );
    if ( pos != -1 )
        stripPath = stripPath.left( pos );

    // Doc system is weird, so open up a file with filename given in stripPath,
    // then if the description exists, use it to set the file name

    m_content->setName(stripPath);
    m_content->setMedia(info->directory());
    m_consumer = m_content->open(QIODevice::WriteOnly);

    if (!m_consumer) {
        delete m_content;
        m_content = 0;

        return QObex::Unauthorized;
    }

    if (!header.description().isEmpty())
        m_content->setName(header.description());

    m_content->setRole(QContent::Data);
    m_content->commit();

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::putDelete(const QObexHeader &header)
{
    qLog(Bluetooth) << "BtFtpSession: PUT-DELETE request:" << header;

    if (header.name().isEmpty())
        return QObex::BadRequest;

    BtFtpDirectoryInfo *info;

    if (m_currentPath.isEmpty()) {
        info = m_manager->info(header.name());

        if (info) {
            QContentSetModel *model = info->model();
            if (model->rowCount() == 0)
                return QObex::Unauthorized;

            return QObex::PreconditionFailed;
        }

        return QObex::NotFound;
    }

    info = m_manager->info(m_currentPath);
    if (!info)
        return QObex::NotFound;

    QContentSet *set = info->set();

    QString stripPath = header.name();
    int pos = stripPath.lastIndexOf("/");
    if ( pos != -1 )
        stripPath = stripPath.mid( pos + 1 );

    QContent content = set->findFileName(stripPath);

    if (!content.isValid())
        return QObex::NotFound;

    content.removeFiles();
    content.commit();

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::listDirectory(const QString &name)
{
    if (name.isEmpty()) {
        if (m_currentPath.isEmpty()) {
            m_currentListing = m_manager->rootListing();
            QBuffer *buffer = new QBuffer;
            buffer->setData(m_currentListing);
            buffer->open(QIODevice::ReadOnly);
            m_provider = buffer;
            return QObex::Success;
        }

        BtFtpDirectoryInfo *info = m_manager->info(m_currentPath);
        if (info) {
            m_currentListing = info->listing();
        }
        else {
            m_currentListing = listing_header;
            m_currentListing.append(parent_folder_tag);
            m_currentListing.append(listing_footer);
        }

        QBuffer *buffer = new QBuffer;
        buffer->setData(m_currentListing);
        buffer->open(QIODevice::ReadOnly);
        m_provider = buffer;
        return QObex::Success;
    }

    if (!m_currentPath.isEmpty()) {
        return QObex::NotFound;
    }

    BtFtpDirectoryInfo *info = m_manager->info(m_currentPath);
    if (!info) {
        return QObex::NotFound;
    }

    m_currentListing = info->listing();

    QBuffer *buffer = new QBuffer;
    buffer->setData(m_currentListing);
    buffer->open(QIODevice::ReadOnly);
    m_provider = buffer;

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::get(const QObexHeader &header)
{
    qLog(Bluetooth) << "BtFtpSession: GET request:" << header;

    if (header.type() == "x-obex/folder-listing") {
        return listDirectory(header.name());
    }

    if (header.name().isEmpty())
        return QObex::NotFound;

    // If we're in the root directory, no files should be
    // present anyway, send a NotFound
    if (m_currentPath.isEmpty())
        return QObex::NotFound;

    BtFtpDirectoryInfo *info = m_manager->info(m_currentPath);
    if (!info)
        return QObex::NotFound;

    QContentSet *set = info->set();

    QString stripPath = header.name();
    int pos = stripPath.lastIndexOf("/");
    if ( pos != -1 )
        stripPath = stripPath.mid( pos + 1 );

    QContent content = set->findFileName(stripPath);

    if (!content.isValid()) {
        qLog(Bluetooth) << "Cannot find requested file:" << stripPath;
        return QObex::NotFound;
    }

    if (!(content.permissions() & QDrmRights::Distribute))
        return QObex::Unauthorized;

    m_provider = content.open();

    if (m_provider == 0)
        return QObex::InternalServerError;

    QObexHeader response;

    response.setType(content.type());
    response.setDescription(content.name());
    response.setLength(content.size());
    response.setName(stripPath);
    response.setTime(content.lastUpdated());

    setNextResponseHeader(response);

    return QObex::Success;
}

QObex::ResponseCode BtFtpSession::setPath(const QObexHeader &header,
                                          QObex::SetPathFlags flags)
{
    qLog(Bluetooth) << "BtFtpSession: SETPATH request:" << header << flags;

    if (header.name().isEmpty() &&
        (flags & QObex::NoPathCreation) &&
        !(flags & QObex::BackUpOneLevel)) {
        m_currentPath = QByteArray();
        return QObex::Success;
    }

    else if ( header.name().isEmpty() &&
               (flags & QObex::BackUpOneLevel) &&
               (flags & QObex::NoPathCreation) ) {
        if (m_currentPath.isEmpty()) {
            qLog(Bluetooth) << "BtFtpSession: cannot change to parent dir, already at root";
            return QObex::NotFound;
        }

        m_currentPath = QByteArray();
        return QObex::Success;
    }

    else if ( !header.name().isEmpty() &&
               (flags & QObex::NoPathCreation) &&
               !(flags & QObex::BackUpOneLevel) ) {

        if (!m_currentPath.isEmpty()) {
            qLog(Bluetooth) << "BtFtpSession: cannot find path" << header.name();
            return QObex::NotFound;
        }

        BtFtpDirectoryInfo *info = m_manager->info(header.name());

        if (!info) {
            qLog(Bluetooth) << "BtFtpSession: cannot find dir info";
            return QObex::NotFound;
        }

        m_currentPath = header.name();

        return QObex::Success;
    }

    else if ( !header.name().isEmpty() &&
                !(flags & QObex::NoPathCreation) &&
                !(flags & QObex::BackUpOneLevel) ) {
        qLog(Bluetooth) << "BtFtpSession: no directory name provided";
        return QObex::Unauthorized;
    }

    qLog(Bluetooth) << "BtFtpSession: bad SETPATH request";
    return QObex::BadRequest;
}

class BtFtpRfcommSession : public QObject
{
    Q_OBJECT

public:
    BtFtpRfcommSession(BtFtpContentManager *manager, QBluetoothRfcommSocket *socket, QObject *parent = 0);
    ~BtFtpRfcommSession();

signals:
    void disconnected();

private slots:
    void sessionDisconnected();

private:
    QBluetoothRfcommSocket *m_socket;
    BtFtpSession *m_session;
};

BtFtpRfcommSession::BtFtpRfcommSession(BtFtpContentManager *manager,
                                       QBluetoothRfcommSocket *socket, QObject *parent)
    : QObject(parent)
{
    m_socket = socket;
    m_session = new BtFtpSession(socket, manager, this);

    connect(m_session, SIGNAL(disconnected()), this, SLOT(sessionDisconnected()));
}

BtFtpRfcommSession::~BtFtpRfcommSession()
{
    delete m_session;
    delete m_socket;
}

void BtFtpRfcommSession::sessionDisconnected()
{
    delete m_session;
    m_session = 0;

    connect(m_socket, SIGNAL(disconnected()), m_socket, SLOT(deleteLater()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    m_socket->disconnect();
    m_socket = 0;

    emit disconnected();
}

class BtFtpService : public QBluetoothAbstractService
{
    Q_OBJECT

public:
    BtFtpService(QObject *parent = 0);
    ~BtFtpService();

    void start();
    void stop();

    void setSecurityOptions(QBluetooth::SecurityOptions options);

private slots:
    void newFtpConnection();
    void sessionEnded();

private:
    void close();

    QBluetoothRfcommServer *m_server;
    QBluetooth::SecurityOptions m_securityOptions;
    QBluetoothLocalDevice *m_local;
    int m_numBtSessions;
    QCommDeviceSession *m_session;
    quint32 m_sdpRecordHandle;
    BtFtpContentManager *m_manager;
};

BtFtpService::BtFtpService(QObject *parent)
    : QBluetoothAbstractService("OBEXFTP", tr("OBEX File Transfer"), parent),
      m_server(0),
      m_securityOptions(0),
      m_local(new QBluetoothLocalDevice(this)),
      m_numBtSessions(0),
      m_session(0),
      m_sdpRecordHandle(0),
      m_manager(0)
{
}

BtFtpService::~BtFtpService()
{
    delete m_session;
    delete m_manager;
    delete m_local;
}

void BtFtpService::close()
{
    qLog(Bluetooth) << "BtFtpService close";

    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = 0;
    }

    delete m_session;
    m_session = 0;
}

void BtFtpService::start()
{
    qLog(Bluetooth) << "BtFtpService start";
    if (m_server)
        close();

    if (!m_local->isValid()) {
        delete m_local;
        m_local = new QBluetoothLocalDevice(this);
        if (!m_local->isValid()) {
            emit started(true, tr("Cannot access local bluetooth device"));
            return;
        }
    }

    m_sdpRecordHandle = 0;
    QBluetoothSdpRecord sdpRecord;

    // register the SDP service
    QFile sdpRecordFile(Qtopia::qtopiaDir() + "etc/bluetooth/sdp/ftp.xml");
    if (sdpRecordFile.open(QIODevice::ReadOnly)) {
        sdpRecord = QBluetoothSdpRecord::fromDevice(&sdpRecordFile);
        if (!sdpRecord.isNull())
            m_sdpRecordHandle = registerRecord(sdpRecord);
    }

    if (sdpRecord.isNull())
        qWarning() << "BtFtpService: cannot read" << sdpRecordFile.fileName();

    if (m_sdpRecordHandle == 0) {
        emit started(true, tr("Error registering with SDP server"));
        return;
    }

    m_server = new QBluetoothRfcommServer(this);
    connect(m_server, SIGNAL(newConnection()),
            this, SLOT(newFtpConnection()));

    if (!m_server->listen(m_local->address(),
                QBluetoothSdpRecord::rfcommChannel(sdpRecord))) {
        unregisterRecord(m_sdpRecordHandle);
        close();
        emit started(true, tr("Error listening on OBEX Push Server"));
        return;
    }
    m_server->setSecurityOptions(m_securityOptions);
    emit started(false, QString());

    if (!m_session) {
        QBluetoothLocalDevice dev;
        m_session = new QCommDeviceSession(dev.deviceName().toLatin1());
    }
}

void BtFtpService::newFtpConnection()
{
    if (!m_server->hasPendingConnections())
        return;

    QBluetoothRfcommSocket *rfcommSocket =
            qobject_cast<QBluetoothRfcommSocket*>(m_server->nextPendingConnection());
    if (!m_manager)
        m_manager = new BtFtpContentManager(this);
    BtFtpRfcommSession *session = new BtFtpRfcommSession(m_manager, rfcommSocket);
    connect(session, SIGNAL(disconnected()), this, SLOT(sessionEnded()));
    connect(session, SIGNAL(disconnected()), session, SLOT(deleteLater()));

    m_numBtSessions++;

    if (m_numBtSessions == 1) { // First session
        qLog(Bluetooth) << "BtFtpService starting BT Session";
        m_session->startSession();
    }
}

void BtFtpService::sessionEnded()
{
    m_numBtSessions--;
    qLog(Bluetooth) << "Bluetooth FTP session finished, m_numBtSessions: " << m_numBtSessions;

    if (m_numBtSessions == 0) {
        qLog(Bluetooth) << "BtFtpService: Ending Bluetooth FTP session";
        m_session->endSession();
        qLog(Bluetooth) << "BtFtpService: Session ended";
    }
}

void BtFtpService::stop()
{
    qLog(Bluetooth) << "BtFtpService stop";

    if (m_server && m_server->isListening())
        close();

    if (!unregisterRecord(m_sdpRecordHandle))
        qLog(Bluetooth) << "BtFtpService::stop() error unregistering SDP service";

    emit stopped();
}

void BtFtpService::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_securityOptions = options;
    if (m_server && m_server->isListening()) {
        m_server->setSecurityOptions(options);
    }
}

/*!
  \class BtFtpServiceTask
    \inpublicgroup QtBluetoothModule
  \brief The BtFtpServiceTask class provides server side support for the Bluetooth
  FTP profile.
  \ingroup QtopiaServer::Task::Bluetooth

  This task listens for incoming Bluetooth FTP connections and handles them.

  The BtFtpServiceTask class provides the \c {BtFtpServiceTask} task.
  This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
  */

/*!
  Constructs the BtFtpServiceTask instance with the given \a parent.
  */
BtFtpServiceTask::BtFtpServiceTask( QObject* parent )
    : QObject( parent )
{
    qLog(Bluetooth) << "BtFtpService: initializing";
    m_service = new BtFtpService(this);
}

/*!
  Destroys the BtDialupServiceTask instance.
  */
BtFtpServiceTask::~BtFtpServiceTask()
{
    delete m_service;
}

QTOPIA_TASK(BtFtpServiceTask,BtFtpServiceTask);

#include "btftpservice.moc"
