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

// We need linux-specific extensions to fcntl and the way to get these
// is to add this define
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "qfilemonitor.h"

#include <QVarLengthArray>
#include <QSocketNotifier>
#include <QString>
#include <QDir>
#include <QMap>
#include <QSet>
#include <QByteArray>
#include <QBasicTimer>
#include <QTimerEvent>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#if defined(QT_NO_INOTIFY)
#include <linux/types.h>

#if defined(__i386__)
# define __NR_inotify_init      291
# define __NR_inotify_add_watch 292
# define __NR_inotify_rm_watch  293
#elif defined(__x86_64__)
# define __NR_inotify_init      253
# define __NR_inotify_add_watch 254
# define __NR_inotify_rm_watch  255
#elif defined(__powerpc__) || defined(__powerpc64__)
# define __NR_inotify_init      275
# define __NR_inotify_add_watch 276
# define __NR_inotify_rm_watch  277
#elif defined (__ia64__)
# define __NR_inotify_init      1277
# define __NR_inotify_add_watch 1278
# define __NR_inotify_rm_watch  1279
#elif defined (__s390__) || defined (__s390x__)
# define __NR_inotify_init      284
# define __NR_inotify_add_watch 285
# define __NR_inotify_rm_watch  286
#elif defined (__alpha__)
# define __NR_inotify_init      444
# define __NR_inotify_add_watch 445
# define __NR_inotify_rm_watch  446
#elif defined (__sparc__) || defined (__sparc64__)
# define __NR_inotify_init      151
# define __NR_inotify_add_watch 152
# define __NR_inotify_rm_watch  156
#elif defined (__arm__)
# define __NR_inotify_init      316
# define __NR_inotify_add_watch 317
# define __NR_inotify_rm_watch  318
#elif defined (__SH4__)
# define __NR_inotify_init      290
# define __NR_inotify_add_watch 291
# define __NR_inotify_rm_watch  292
#elif defined (__SH5__)
# define __NR_inotify_init      318
# define __NR_inotify_add_watch 319
# define __NR_inotify_rm_watch  320
#elif defined (__mips__)
# define __NR_inotify_init      284
# define __NR_inotify_add_watch 285
# define __NR_inotify_rm_watch  286
#elif defined (__hppa__)
# define __NR_inotify_init      269
# define __NR_inotify_add_watch 270
# define __NR_inotify_rm_watch  271
#else
# error "This architecture is not supported."
#endif

#ifdef QT_LSB
// ### the LSB doesn't standardize syscall, need to wait until glib2.4 is standardized
static inline int syscall(...) { return -1; }
#endif

static inline int inotify_init()
{
    return syscall(__NR_inotify_init);
}

static inline int inotify_add_watch(int fd, const char *name, __u32 mask)
{
    return syscall(__NR_inotify_add_watch, fd, name, mask);
}

static inline int inotify_rm_watch(int fd, __u32 wd)
{
    return syscall(__NR_inotify_rm_watch, fd, wd);
}

// the following struct and values are documented in linux/inotify.h
extern "C" {

struct inotify_event {
        __s32           wd;
        __u32           mask;
        __u32           cookie;
        __u32           len;
        char            name[0];
};

#define IN_ACCESS               0x00000001
#define IN_MODIFY               0x00000002
#define IN_ATTRIB               0x00000004
#define IN_CLOSE_WRITE          0x00000008
#define IN_CLOSE_NOWRITE        0x00000010
#define IN_OPEN                 0x00000020
#define IN_MOVED_FROM           0x00000040
#define IN_MOVED_TO             0x00000080
#define IN_CREATE               0x00000100
#define IN_DELETE               0x00000200
#define IN_DELETE_SELF          0x00000400
#define IN_MOVE_SELF            0x00000800
#define IN_UNMOUNT              0x00002000
#define IN_Q_OVERFLOW           0x00004000

#define IN_CLOSE                (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
#define IN_MOVE                 (IN_MOVED_FROM | IN_MOVED_TO)
}

// --------- inotify.h end ----------

#else /* QT_NO_INOTIFY */
#include <sys/inotify.h>
#endif

#define INOTIFY_BUFSIZE 16384
#define POLLMONITOR_PERIOD 5

static bool qualifiedToFilePath(const QByteArray &qualified,
                                QByteArray * file, QByteArray * path)
{
    int fsep = qualified.lastIndexOf('/');
    if(-1 == fsep) return false;

    if(path)
        *path = qualified.left(fsep);
    if(file)
        *file = qualified.mid(fsep + 1);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// declare IMonitorContainer
///////////////////////////////////////////////////////////////////////////////
struct IMonitorContainer
{
    virtual ~IMonitorContainer();
    virtual void AddRef() = 0;
    virtual void Release() = 0;

    virtual unsigned int monitorCount() const = 0;
    virtual QFileMonitorPrivate * monitor(unsigned int) = 0;
};

IMonitorContainer::~IMonitorContainer()
{
}

///////////////////////////////////////////////////////////////////////////////
// declare FileMonitorProvider
///////////////////////////////////////////////////////////////////////////////
struct FileMonitorProvider : public QObject
{
    FileMonitorProvider();

    virtual ~FileMonitorProvider() {}
    virtual bool install(QFileMonitorPrivate *) = 0;
    virtual void remove(QFileMonitorPrivate *) = 0;
    virtual QFileMonitor::Strategy type() const = 0;

protected:
    void enqueueDelayedEmit(IMonitorContainer *);
    void performDelayedEmit();
    virtual void timerEvent(QTimerEvent *);

private:
    time_t delayedEmitTime;
    int timerId;
    QSet<IMonitorContainer *> delayedEmits;
};

///////////////////////////////////////////////////////////////////////////////
// declare INotifyFileMonitor
///////////////////////////////////////////////////////////////////////////////
class INotifyFileMonitor : public FileMonitorProvider
{
Q_OBJECT
public:
    INotifyFileMonitor();

    virtual bool install(QFileMonitorPrivate *);
    virtual void remove(QFileMonitorPrivate *);
    virtual QFileMonitor::Strategy type() const {
        return QFileMonitor::INotify;
    }

private slots:
    void activated();
    void directoryChanged(const QString &);

private:
    int m_fd;

    struct Directory {
        Directory() : dirMonitor(0) {}
        QByteArray path;

        struct File {
            void AddRef();
            void Release();
            QByteArray name;
            QList<QFileMonitorPrivate *> monitors;
        };

        QFileMonitor *dirMonitor;
        QMap<QByteArray, File> files;
    };

    QMap<int, Directory>::Iterator findPath(const QByteArray &);

    QMap<int, Directory> m_monitoredPaths;
    QMap<int, Directory> m_waitingPaths;
    unsigned int m_waitingPathFd;
};
Q_GLOBAL_STATIC(INotifyFileMonitor, iNotifyFileMonitor);
static FileMonitorProvider * iNotifyFileMonitorProv()
{
    return iNotifyFileMonitor();
}

///////////////////////////////////////////////////////////////////////////////
// declare DNotifyFileMonitor
///////////////////////////////////////////////////////////////////////////////
class DNotifyFileMonitor : public FileMonitorProvider
{
Q_OBJECT
public:
    DNotifyFileMonitor();

    virtual bool install(QFileMonitorPrivate *);
    virtual void remove(QFileMonitorPrivate *);

    virtual QFileMonitor::Strategy type() const {
        return QFileMonitor::DNotify;
    }

private slots:
    void activated();
    void directoryChanged(const QString &);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void refresh(int fd);

    struct Directory {
        Directory() : dirMonitor(0) {}
        QByteArray path;

        struct File : public IMonitorContainer {
            virtual ~File() {}
            virtual void AddRef();
            virtual void Release();
            virtual unsigned int monitorCount() const;
            virtual QFileMonitorPrivate * monitor(unsigned int);

            QByteArray name;
            time_t lastWrite;
            QList<QFileMonitorPrivate *> monitors;
        };

        QFileMonitor * dirMonitor;

        // Linear list is fine as we have to manually ::stat anyway
        QList<File> files;
    };

    QMap<int, Directory>::Iterator findPath(const QByteArray &);

    // Maps from directory fd to Directory structure
    QMap<int, Directory> m_monitoredPaths;
    QMap<int, Directory> m_waitingPaths;
    unsigned int m_waitingPathFd;
    int timerId;
};
Q_GLOBAL_STATIC(DNotifyFileMonitor, dNotifyFileMonitor);
static FileMonitorProvider * dNotifyFileMonitorProv()
{
    return dNotifyFileMonitor();
}
///////////////////////////////////////////////////////////////////////////////
// declare PollFileMonitor
///////////////////////////////////////////////////////////////////////////////
class PollFileMonitor : public FileMonitorProvider
{
public:
    PollFileMonitor();

    virtual bool install(QFileMonitorPrivate *);
    virtual void remove(QFileMonitorPrivate *);

    virtual void timerEvent(QTimerEvent *);

    virtual QFileMonitor::Strategy type() const {
        return QFileMonitor::Poll;
    }

private:
    struct File {
        time_t lastModified;
        QList<QFileMonitorPrivate *> monitors;
    };
    QMap<QByteArray, File> m_monitoredFiles;
    int m_timerId;
};
Q_GLOBAL_STATIC(PollFileMonitor, pollFileMonitor);
static FileMonitorProvider * pollFileMonitorProv()
{
    return pollFileMonitor();
}

///////////////////////////////////////////////////////////////////////////////
// declare QFileMonitorPrivate
///////////////////////////////////////////////////////////////////////////////
class QFileMonitorPrivate : public IMonitorContainer
{
public:
    QFileMonitorPrivate(const QString&, QFileMonitor::Strategy,  QFileMonitor*);

    void fileChanged();

    typedef FileMonitorProvider *(*FileMonitorProviderFunc)();

    FileMonitorProviderFunc prov;
    QString fileName;
    QFileMonitor * parent;

    virtual void AddRef();
    virtual void Release();
    virtual unsigned int monitorCount() const;
    virtual QFileMonitorPrivate * monitor(unsigned int);

private:
    friend class GCCWarning;
    virtual ~QFileMonitorPrivate();
    int refCount;
};
// We must do it like this (return a function) as consumers of QFileMonitor
// might be using it in a Q_GLOBAL_STATIC of their own, and shutdown order can
// really mess us up :)
static QFileMonitorPrivate::FileMonitorProviderFunc
fileMonitorProvider(QFileMonitor::Strategy);

static void doFileChanged(QList<QFileMonitorPrivate *> file)
{
    for(int ii = 0; ii < file.count(); ++ii)
        file.at(ii)->fileChanged();
}

///////////////////////////////////////////////////////////////////////////////
// define FileMonitorProvider
///////////////////////////////////////////////////////////////////////////////
FileMonitorProvider::FileMonitorProvider()
: delayedEmitTime(0)
{
}

void FileMonitorProvider::enqueueDelayedEmit(IMonitorContainer *p)
{
    performDelayedEmit();

    if(!delayedEmits.contains(p)) {
        delayedEmits.insert(p);
        p->AddRef();
        if(!timerId)
            timerId = startTimer(1000 /* 1 second */);

        if(0 == delayedEmitTime)
            delayedEmitTime = ::time(0);
    }
}

void FileMonitorProvider::performDelayedEmit()
{
    time_t currentTime = ::time(0);
    if(currentTime <= delayedEmitTime)
        return;

    QSet<IMonitorContainer *> emits = delayedEmits;
    delayedEmits.clear();
    delayedEmitTime = 0;
    if(timerId) {
        killTimer(timerId);
        timerId = 0;
    }

    for(QSet<IMonitorContainer *>::ConstIterator iter = emits.begin();
            iter != emits.end();
            ++iter) {

        for(unsigned int ii = 0; ii < (*iter)->monitorCount(); ++ii)
            (*iter)->monitor(ii)->fileChanged();

        (*iter)->Release();
    }
}

void FileMonitorProvider::timerEvent(QTimerEvent *e)
{
    if(this->timerId && e->timerId() == this->timerId)
        performDelayedEmit();
}

///////////////////////////////////////////////////////////////////////////////
// define QFileMonitorPrivate
///////////////////////////////////////////////////////////////////////////////
void QFileMonitorPrivate::AddRef()
{
    Q_ASSERT(refCount);
    ++refCount;
}

void QFileMonitorPrivate::Release()
{
    Q_ASSERT(refCount);
    --refCount;
    if(!refCount) delete this;
}

unsigned int QFileMonitorPrivate::monitorCount() const
{
    return 1;
}

QFileMonitorPrivate * QFileMonitorPrivate::monitor(unsigned int count)
{
    Q_ASSERT(0 == count);
    Q_UNUSED(count);
    return this;
}

QFileMonitorPrivate::QFileMonitorPrivate(const QString & f,
                                         QFileMonitor::Strategy s,
                                         QFileMonitor *p)
: prov(0), parent(p), refCount(1)
{
    fileName = QDir::cleanPath(f);
    prov = fileMonitorProvider(s);
    FileMonitorProvider * fmp = prov();
    if(fmp)
        if(!fmp->install(this))
            prov = 0;
}


QFileMonitorPrivate::~QFileMonitorPrivate()
{
    if(prov) {
        FileMonitorProvider * p = prov();
        if(p)
            p->remove(this);
    }
}

void QFileMonitorPrivate::fileChanged()
{
    emit parent->fileChanged(fileName);
}

/*!
  \class QFileMonitor
    \inpublicgroup QtBaseModule

  \brief The QFileMonitor class allows applications to asynchronously monitor files for changes.

  Using QFileMonitor, clients will be notified through the fileChanged() signal
  when the contents of a file are changed, the file is deleted or when the file
  is created.

  The QFileMonitor uses different mechanisms for monitoring file changes
  depending on the capabilities of the underlying system.  The following
  methods are presently available:

  \list 1
  \i DNotify

  DNotify is available on all Linux kernels in the 2.4 series and later.
  Monitoring a file using DNotify has the consequence of "pinning" the
  containing directory until the QFileMonitor instance has been destroyed.  A
  pinned directory cannot be removed and the filesystem exporting it cannot be
  unmounted.

  In order to detect directory removal, the DNotify strategy must poll
  directories for existance once every 5 seconds.

  \i INotify

  INotify is available in Linux kernel versions 2.6.13 and later.  Unlike
  DNotify, use of INotify does not pin the containing directory.  If a
  directory is removed, while monitoring contained files the QFileMonitor will
  emit a fileChanged() signal as expected.  However, should the directory be
  recreated the QFileMonitor will not resume monitoring files within it.  The
  QFileMonitor instance must be destroyed and recreated for monitoring to
  resume.

  \i Poll

  While polling files for change is available on all systems, it is the least
  responsive strategy.  Every 5 seconds the QFileMonitor manually polls all
  monitored files for changes.  Polling should only be used as a last-resort
  fallback.

  \endlist

  To avoid race conditions when using QFileMonitor to trigger re-reading of file
  contents, you should always construct QFileMonitor and \i {only then} read the
  initial file contents.

  \ingroup io
 */

/*!
  \enum QFileMonitor::Strategy

  Represents the monitoring strategy being used by the QFileMonitor instance.

  \value Auto The best available monitoring strategy will be automatically
  selected.  The instance will attempt, in order, to use INotify, DNotify and
  finally Polling.
  \value DNotify The DNotify strategy is being used.
  \value INotify The INotify strategy is being used.
  \value Poll The Polling strategy is being used.
  \value None No monitoring strategy is in use.
  */

/*!
  \fn void QFileMonitor::fileChanged(const QString &file)

  Emitted whenever the files contents change, the file is created or the file
  is removed.  \a file is set to the name of the monitored file.
  */

/*!
  Constructs an invalid file monitor with the specified \a parent.
  */
QFileMonitor::QFileMonitor(QObject *parent)
: QObject(parent), d(new QFileMonitorPrivate(QString(),
                                             QFileMonitor::None, this))
{
}

/*!
  Constructs a file monitor for the file \a fileName using the specified file
  monitoring \a strategy and \a parent.
 */
QFileMonitor::QFileMonitor(const QString &fileName, Strategy strategy,
                           QObject *parent)
: QObject(parent), d(new QFileMonitorPrivate(fileName, strategy, this))
{
}

/*!
  Destroys the file monitor.
  */
QFileMonitor::~QFileMonitor()
{
    d->parent = 0;
    d->Release();
    d = 0;
}

/*!
  Returns true if the file is being actively monitored.
  */
bool QFileMonitor::isValid() const
{
    return 0 != d->prov;
}

/*!
  Returns the name of the file being monitored.
  */
QString QFileMonitor::fileName() const
{
    return d->fileName;
}

/*!
  Returns the method used to monitor files.
  */
QFileMonitor::Strategy QFileMonitor::strategy() const
{
    FileMonitorProvider * p = 0;
    if(d->prov) p = d->prov();
    if(!p) return None;
    return p->type();
}


///////////////////////////////////////////////////////////////////////////////
// define DNotifyFileMonitor
///////////////////////////////////////////////////////////////////////////////
static int qfm_fileChanged_pipe[2];
static void (*qfm_old_sigio_handler)(int) = 0;
static void (*qfm_old_sigio_action)(int, siginfo_t *, void *) = 0;
static void qfm_sigio_monitor(int signum, siginfo_t *i, void *v)
{
    ::write(qfm_fileChanged_pipe[1], &i->si_fd, sizeof(int));

    if (qfm_old_sigio_handler && qfm_old_sigio_handler != SIG_IGN)
        qfm_old_sigio_handler(signum);
    if (qfm_old_sigio_action)
        qfm_old_sigio_action(signum, i, v);
}

DNotifyFileMonitor::DNotifyFileMonitor()
: m_waitingPathFd(0), timerId(0)
{
    ::pipe(qfm_fileChanged_pipe);
    ::fcntl(qfm_fileChanged_pipe[0], F_SETFL,
            ::fcntl(qfm_fileChanged_pipe[0], F_GETFL) | O_NONBLOCK);

    QSocketNotifier * notifier =
        new QSocketNotifier(qfm_fileChanged_pipe[0], QSocketNotifier::Read);
    QObject::connect(notifier, SIGNAL(activated(int)), this, SLOT(activated()));

    struct sigaction oldAction;
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = qfm_sigio_monitor;
    action.sa_flags = SA_SIGINFO;
    ::sigaction(SIGIO, &action, &oldAction);
    if (!(oldAction.sa_flags & SA_SIGINFO))
        qfm_old_sigio_handler = oldAction.sa_handler;
    else
        qfm_old_sigio_action = oldAction.sa_sigaction;
}

void DNotifyFileMonitor::directoryChanged(const QString &spath)
{
    QByteArray path = spath.toAscii().constData();

    QMap<int, Directory>::Iterator iter = findPath(path);

    // Attempt to setup directory monitoring
    DIR * d = ::opendir(path.constData());
    if(!d)
        return;

    int fd = ::dirfd(d);
    if(::fcntl(fd, F_SETSIG, SIGIO) ||
       ::fcntl(fd, F_NOTIFY, DN_MODIFY | DN_CREATE | DN_DELETE |
                             DN_RENAME | DN_MULTISHOT)) {
        ::closedir(d);
        return;
    }

    // Sweet - success!
    delete iter->dirMonitor;
    iter->dirMonitor = 0;

    // Insert into monitor paths
    m_monitoredPaths.insert(fd, *iter);
    if(0 == timerId)
        timerId = startTimer(POLLMONITOR_PERIOD * 1000);
    // Remove from waiting list
    m_waitingPaths.erase(iter);

    // Trigger refresh
    refresh(fd);
}

void DNotifyFileMonitor::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == timerId) {
        QVarLengthArray<int> vla(m_monitoredPaths.count());
        int vlacount = 0;
        for(QMap<int, Directory>::ConstIterator iter = m_monitoredPaths.begin();
                iter != m_monitoredPaths.end();
                ++iter) {
            vla[vlacount] = iter.key();
            ++vlacount;
        }

        for(int ii = 0; ii < vlacount; ++ii)
            refresh(vla[ii]);

    } else {
        FileMonitorProvider::timerEvent(e);
    }
}

void DNotifyFileMonitor::activated()
{
    int fd;
    int readrv = ::read(qfm_fileChanged_pipe[0], &fd,sizeof(int));
    Q_ASSERT(readrv == sizeof(int));
    Q_UNUSED(readrv);

    refresh(fd);
}

void DNotifyFileMonitor::refresh(int fd)
{
    // Locate directory structure
    QMap<int, Directory>::Iterator diter = m_monitoredPaths.find(fd);
    if(diter == m_monitoredPaths.end()) return;
    Directory & directory = *diter;
    const QByteArray & path = directory.path;

    // Ensure directory still exists
    struct stat buf;
    memset(&buf, 0, sizeof(struct stat));
    if(-1 == ::stat(path.constData(), &buf)) {
        // Directory has been removed!
        ::close(fd);

        directory.dirMonitor = new QFileMonitor(path);
        QObject::connect(directory.dirMonitor,
                SIGNAL(fileChanged(QString)),
                this,
                SLOT(directoryChanged(QString)));

        m_waitingPaths.insert(m_waitingPathFd++, directory);
        Directory d = directory;
        m_monitoredPaths.erase(diter);

        for(int ii = 0; ii < d.files.count(); ++ii)
            d.files[ii].AddRef();
        for(int ii = 0; ii < d.files.count(); ++ii) {
            if(d.files.at(ii).lastWrite) {
                d.files[ii].lastWrite = 0;
                doFileChanged(d.files.at(ii).monitors);
            }
        }
        for(int ii = 0; ii < d.files.count(); ++ii)
            d.files[ii].Release();

        return;
    }

    // A file is deemed to have changed if its current mtime does not equal the
    // previously seen mtime.  The mtime for a non-existant file is treated as
    // 0.
    // If a file has an mtime equal to the current system time, we must delay
    // its fileChanged() signal until the next clock second to prevent dropping
    // change notifications in the case of multiple changes per second to a
    // single file.
    time_t currentTime;
    currentTime = ::time(0);
    performDelayedEmit();

    QList<Directory::File *> delayedChanges;
    QList<Directory::File *> immediateChanges;

    for(int ii = 0; ii < directory.files.count(); ++ii) {

        QByteArray fileName = path + "/" + directory.files.at(ii).name;
        memset(&buf, 0, sizeof(struct stat));
        ::stat(fileName.constData(), &buf);

        if(buf.st_mtime != directory.files.at(ii).lastWrite) {
            directory.files[ii].lastWrite = buf.st_mtime;

            if(currentTime == buf.st_mtime)
                delayedChanges.append(&directory.files[ii]);
            else
                immediateChanges.append(&directory.files[ii]);
        }
    }

    for(int ii = 0; ii < delayedChanges.count(); ++ii)
        enqueueDelayedEmit(delayedChanges.at(ii));

    for(int ii = 0; ii < immediateChanges.count(); ++ii)
        immediateChanges.at(ii)->AddRef();
    for(int ii = 0; ii < immediateChanges.count(); ++ii)
        doFileChanged(immediateChanges.at(ii)->monitors);
    for(int ii = 0; ii < immediateChanges.count(); ++ii)
        immediateChanges.at(ii)->Release();
}

QMap<int, DNotifyFileMonitor::Directory>::Iterator
DNotifyFileMonitor::findPath(const QByteArray &path)
{
    QMap<int, Directory>::Iterator rv = m_monitoredPaths.begin();
    for(; rv != m_monitoredPaths.end(); ++rv) {
        if(rv.value().path == path)
            return rv;
    }

    rv = m_waitingPaths.begin();
    for(; rv != m_waitingPaths.end(); ++rv) {
        if(rv.value().path == path)
            return rv;
    }
    return rv;
}

bool DNotifyFileMonitor::install(QFileMonitorPrivate * p)
{
    QByteArray qPath = p->fileName.toAscii();
    QByteArray path;
    QByteArray fileName;
    if(!qualifiedToFilePath(qPath, &fileName, &path))
        return false;

    QMap<int, Directory>::Iterator iter = findPath(path);
    if(iter == m_monitoredPaths.end() || iter == m_waitingPaths.end()) {

        // Setup directory monitoring
        DIR * d = ::opendir(path.constData());
        int fd = -1;
        if(d) {
            fd = ::dirfd(d);
            if(::fcntl(fd, F_SETSIG, SIGIO) ||
               ::fcntl(fd, F_NOTIFY, DN_MODIFY | DN_CREATE | DN_DELETE |
                                     DN_RENAME | DN_MULTISHOT)) {
                ::closedir(d);
                d = 0;
            }
        }

        // Create structures
        Directory directory;
        directory.path = path;
        Directory::File file;
        file.name = fileName;
        file.monitors.append(p);

        if(d) {
            struct stat buf;
            memset(&buf, 0, sizeof(struct stat));
            ::stat(qPath.constData(), &buf);
            file.lastWrite = buf.st_mtime;
        } else {
            file.lastWrite = 0;

            // The directory doesn't exist.  We must monitor for its creation.
            directory.dirMonitor = new QFileMonitor(path);
            QObject::connect(directory.dirMonitor,
                             SIGNAL(fileChanged(QString)),
                             this,
                             SLOT(directoryChanged(QString)));
            fd = m_waitingPathFd++;
        }

        directory.files.append(file);

        // Insert new entry
        if(d) {
            m_monitoredPaths.insert(fd, directory);
            if(0 == timerId)
                timerId = startTimer(POLLMONITOR_PERIOD * 1000);
        } else {
            m_waitingPaths.insert(fd, directory);
        }


    } else {

        // Does this particular file exist?
        for(int ii = 0; ii < iter->files.count(); ++ii) {
            if(iter->files.at(ii).name == fileName) {
                iter->files[ii].monitors.append(p);
                return true;
            }
        }

        // Not found
        Directory::File file;
        file.name = fileName;
        file.monitors.append(p);
        struct stat buf;
        memset(&buf, 0, sizeof(struct stat));
        ::stat(qPath.constData(), &buf);
        file.lastWrite = buf.st_mtime;
        iter->files.append(file);
    }

    return true;
}

void DNotifyFileMonitor::remove(QFileMonitorPrivate * p)
{
    QByteArray qPath = p->fileName.toAscii();
    QByteArray path;
    QByteArray fileName;
    bool qtfp = qualifiedToFilePath(qPath, &fileName, &path);
    Q_ASSERT(qtfp);
    Q_UNUSED(qtfp);

    QMap<int, Directory>::Iterator iter = findPath(path);
    Q_ASSERT(iter != m_monitoredPaths.end());

    for(int ii = 0; ii < iter->files.count(); ++ii) {
        if(iter->files.at(ii).name == fileName) {
            iter->files[ii].monitors.removeAll(p);
            if(iter->files.at(ii).monitors.isEmpty()) {
                iter->files.removeAt(ii);
                if(iter->files.isEmpty()) {
                    ::close(iter.key());
                    if(iter->dirMonitor) {
                        delete iter->dirMonitor;
                        m_waitingPaths.erase(iter);
                    } else {
                        m_monitoredPaths.erase(iter);
                    }
                }
            }

            return;
        }
    }

    if(m_monitoredPaths.isEmpty() && timerId) {
        killTimer(timerId);
        timerId = 0;
    }

    Q_ASSERT(!"File monitor shouldn't exist");
}

/*!
  \internal

  Increment the reference count on all monitors in the file.  Reference counting
  the monitors is necessary to prevent problems when one is removed during a
  fileChanged() signal.
  */
void DNotifyFileMonitor::Directory::File::AddRef()
{
    for(int ii = 0; ii < monitors.count(); ++ii)
        monitors.at(ii)->AddRef();
}

unsigned int DNotifyFileMonitor::Directory::File::monitorCount() const
{
    return monitors.count();
}

QFileMonitorPrivate *
DNotifyFileMonitor::Directory::File::monitor(unsigned int count)
{
    return monitors.at(count);
}

/*!
  \internal

  Decrement the reference count on all monitors in the file.  Reference counting
  the monitors is necessary to prevent problems when one is removed during a
  fileChanged() signal.
  */
void DNotifyFileMonitor::Directory::File::Release()
{
    for(int ii = 0; ii < monitors.count(); ++ii)
        monitors.at(ii)->Release();
}

/*!
  \internal

  Return true if DNotify is supported on this system.
  */
static bool dNotifyTest()
{
    static bool tested = false;
    static bool result = false;
    if(tested) return result;

    DIR * d = ::opendir("/");
    if(d) {
        int fd = ::dirfd(d);
        if(-1 != fd) {
            if(-1 != ::fcntl(fd, F_NOTIFY, DN_CREATE))
                result = true;
            ::close(fd);
        }
    }

    tested = true;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// define INotifyFileMonitor
///////////////////////////////////////////////////////////////////////////////
INotifyFileMonitor::INotifyFileMonitor()
: m_fd(-1), m_waitingPathFd(0)
{
    m_fd = inotify_init();
    if(-1 == m_fd) {
        qWarning("INotify not supported.  Please upgrade to kernel 2.6.13 or "
                 "greater.");
        return;
    }

    QSocketNotifier * sock = new QSocketNotifier(m_fd, QSocketNotifier::Read,
                                                 this);
    QObject::connect(sock, SIGNAL(activated(int)), this, SLOT(activated()));
}

void INotifyFileMonitor::activated()
{
    char buffer[INOTIFY_BUFSIZE];
    inotify_event & event = *((inotify_event *)buffer);

    int readrv = ::read(m_fd, &event, INOTIFY_BUFSIZE);
    Q_ASSERT(readrv >= (int)sizeof(inotify_event));
    Q_UNUSED(readrv);

    // Locate the directory this wd was for
    QMap<int, Directory>::Iterator diter = m_monitoredPaths.find(event.wd);
    if(diter == m_monitoredPaths.end()) return;

    const Directory & dir = *diter;

    if(IN_DELETE_SELF & event.mask || IN_UNMOUNT & event.mask) {
        // Directory is gone
        ::inotify_rm_watch(m_fd, diter.key());
        Q_ASSERT(!diter->dirMonitor);
        diter->dirMonitor = new QFileMonitor(diter->path);
        QObject::connect(diter->dirMonitor,
                         SIGNAL(fileChanged(QString)),
                         this, SLOT(directoryChanged(QString)));
        m_waitingPaths.insert(m_waitingPathFd++, *diter);
        QList<QFileMonitorPrivate *> monitors;
        for(QMap<QByteArray, Directory::File>::ConstIterator fiter =
                diter->files.begin();
                fiter != diter->files.end();
                ++fiter)
            monitors += fiter->monitors;
        m_monitoredPaths.erase(diter);
        for(int ii = 0; ii < monitors.count(); ++ii)
            monitors[ii]->AddRef();
        doFileChanged(monitors);
        for(int ii = 0; ii < monitors.count(); ++ii)
            monitors[ii]->Release();

    } else {
        // Regular run-of-the-mill change
        QByteArray file(event.name, event.len);

        QMap<QByteArray, Directory::File>::ConstIterator fiter =
            dir.files.find(file);
        if(fiter == dir.files.end()) return;

        // Emit!
        Directory::File f= *fiter;
        f.AddRef();
        doFileChanged(f.monitors);
        f.Release();
    }
}

void INotifyFileMonitor::directoryChanged(const QString &spath)
{
    QByteArray path = spath.toAscii().constData();

    QMap<int, Directory>::Iterator iter = findPath(path);

    // Attempt to set up watch
    int wd = inotify_add_watch(m_fd, path.constData(),
                               IN_MODIFY | IN_CREATE | IN_DELETE |
                               IN_MOVED_FROM | IN_MOVED_TO |
                               IN_DELETE_SELF | IN_UNMOUNT);
    if(-1 != wd) {
        delete iter->dirMonitor;
        iter->dirMonitor = 0;
        m_monitoredPaths.insert(wd, *iter);
        m_waitingPaths.erase(iter);
    }
}

QMap<int, INotifyFileMonitor::Directory>::Iterator
INotifyFileMonitor::findPath(const QByteArray &path)
{
    QMap<int, Directory>::Iterator rv = m_monitoredPaths.begin();
    for(; rv != m_monitoredPaths.end(); ++rv) {
        if(rv.value().path == path)
            return rv;
    }

    rv = m_waitingPaths.begin();
    for(; rv != m_waitingPaths.end(); ++rv) {
        if(rv.value().path == path)
            return rv;
    }

    return rv;
}
bool INotifyFileMonitor::install(QFileMonitorPrivate *p)
{
    QByteArray qPath = p->fileName.toAscii();
    QByteArray path;
    QByteArray fileName;
    if(!qualifiedToFilePath(qPath, &fileName, &path))
        return false;

    // Attempt to locate path
    QMap<int, Directory>::Iterator iter = findPath(path);

    if(iter == m_monitoredPaths.end() ||
       iter == m_waitingPaths.end()) {
        // Insert a new path
        int wd = inotify_add_watch(m_fd, path.constData(),
                                   IN_MODIFY | IN_CREATE | IN_DELETE |
                                   IN_MOVED_FROM | IN_MOVED_TO |
                                   IN_DELETE_SELF | IN_UNMOUNT);

        Directory d;
        d.path = path;
        Directory::File f;
        f.name = fileName;
        f.monitors += p;
        d.files.insert(fileName, f);

        if(wd != -1) {
            m_monitoredPaths.insert(wd, d);
        } else {
            d.dirMonitor = new QFileMonitor(path);
            m_waitingPaths.insert(m_waitingPathFd++, d);
            QObject::connect(d.dirMonitor,
                             SIGNAL(fileChanged(QString)),
                             this, SLOT(directoryChanged(QString)));
        }
    } else {
        // Attempt to locate file
        QMap<QByteArray, Directory::File>::Iterator fiter =
            iter->files.find(fileName);
        if(fiter != iter->files.end()) {
            fiter->monitors += p;
        } else {
            Directory::File f;
            f.name = fileName;
            f.monitors += p;
            iter->files.insert(fileName, f);
        }
    }

    return true;
}

void INotifyFileMonitor::remove(QFileMonitorPrivate *p)
{
    QByteArray qPath = p->fileName.toAscii();
    QByteArray path;
    QByteArray fileName;
    bool qtfp = qualifiedToFilePath(qPath, &fileName, &path);
    Q_ASSERT(qtfp);
    Q_UNUSED(qtfp);

    QMap<int, Directory>::Iterator iter = findPath(path);
    Q_ASSERT(iter != m_monitoredPaths.end());

    QMap<QByteArray, Directory::File>::Iterator fiter =
        iter->files.find(fileName);
    Q_ASSERT(fiter != iter->files.end());

    fiter->monitors.removeAll(p);
    if(fiter->monitors.isEmpty())
        iter->files.erase(fiter);

    if(iter->files.isEmpty()) {
        ::inotify_rm_watch(m_fd, iter.key());
        if(iter->dirMonitor) {
            delete iter->dirMonitor;
            m_waitingPaths.erase(iter);
        } else {
            m_monitoredPaths.erase(iter);
        }
    }
}

/*!
  \internal

  Increment the reference count on all monitors in the file.  Reference counting
  the monitors is necessary to prevent problems when one is removed during a
  fileChanged() signal.
  */
void INotifyFileMonitor::Directory::File::AddRef()
{
    for(int ii = 0; ii < monitors.count(); ++ii)
        monitors.at(ii)->AddRef();
}

/*!
  \internal

  Decrement the reference count on all monitors in the file.  Reference counting
  the monitors is necessary to prevent problems when one is removed during a
  fileChanged() signal.
  */
void INotifyFileMonitor::Directory::File::Release()
{
    for(int ii = 0; ii < monitors.count(); ++ii)
        monitors.at(ii)->Release();
}

/*!
  \internal

  Returns true if this system supports INotify.
  */
static bool iNotifyTest()
{
#ifdef FILEMONITOR_DISABLE_INOTIFY
    return false;
#else
    static bool tested = false;
    static bool result = false;
    if(tested) return result;

    int testFd = inotify_init();
    if(-1 != testFd) {
        result = true;
        ::close(testFd);
    }

    tested = true;
    return result;
#endif
}

static QFileMonitorPrivate::FileMonitorProviderFunc
fileMonitorProvider(QFileMonitor::Strategy s)
{
    switch(s) {
        case QFileMonitor::Auto:
            if(iNotifyTest())
                return iNotifyFileMonitorProv;
            else if(dNotifyTest())
                return dNotifyFileMonitorProv;
            else
                return pollFileMonitorProv;

        case QFileMonitor::DNotify:
            if(dNotifyTest())
                return dNotifyFileMonitorProv;
            break;

        case QFileMonitor::INotify:
            if(iNotifyTest())
                return iNotifyFileMonitorProv;
            break;

        case QFileMonitor::Poll:
            return pollFileMonitorProv;

        default:
            break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// define PollFileMonitor
///////////////////////////////////////////////////////////////////////////////
PollFileMonitor::PollFileMonitor()
: m_timerId(0)
{
}

bool PollFileMonitor::install(QFileMonitorPrivate *p)
{
    if(!m_timerId)
        m_timerId = startTimer(POLLMONITOR_PERIOD);

    QByteArray qPath = p->fileName.toAscii();

    QMap<QByteArray, File>::Iterator iter = m_monitoredFiles.find(qPath);
    if(iter == m_monitoredFiles.end()) {
        File f;
        struct stat buf;
        memset(&buf, 0, sizeof(struct stat));
        ::stat(qPath.constData(), &buf);
        f.lastModified = buf.st_mtime;
        f.monitors += p;
        m_monitoredFiles.insert(qPath, f);
    } else {
        iter->monitors += p;
    }

    return true;
}

void PollFileMonitor::remove(QFileMonitorPrivate *p)
{
    QByteArray qPath = p->fileName.toAscii();

    QMap<QByteArray, File>::Iterator iter = m_monitoredFiles.find(qPath);
    Q_ASSERT(iter != m_monitoredFiles.end());

    iter->monitors.removeAll(p);

    if(iter->monitors.isEmpty())
        m_monitoredFiles.erase(iter);

    if(m_monitoredFiles.isEmpty()) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

void PollFileMonitor::timerEvent(QTimerEvent *e)
{
    if(m_timerId == e->timerId()) {
        QList<QFileMonitorPrivate *> immediateChanges;
        QList<QFileMonitorPrivate *> delayedChanges;

        time_t currentTime = ::time(0);
        for(QMap<QByteArray, File>::Iterator iter = m_monitoredFiles.begin();
                iter != m_monitoredFiles.end();
                ++iter) {

            struct stat buf;
            memset(&buf, 0, sizeof(struct stat));
            ::stat(iter.key().constData(), &buf);

            if(buf.st_mtime != iter->lastModified) {
                iter->lastModified = buf.st_mtime;

                if(buf.st_mtime >= currentTime)
                    delayedChanges += (*iter).monitors;
                else
                    immediateChanges += (*iter).monitors;
            }
        }

        for(int ii = 0; ii < delayedChanges.count(); ++ii)
            enqueueDelayedEmit(delayedChanges.at(ii));

        for(int ii = 0; ii < immediateChanges.count(); ++ii)
            immediateChanges[ii]->AddRef();
        doFileChanged(immediateChanges);
        for(int ii = 0; ii < immediateChanges.count(); ++ii)
            immediateChanges[ii]->Release();

    } else {
        FileMonitorProvider::timerEvent(e);
    }
}

#include "qfilemonitor.moc"
