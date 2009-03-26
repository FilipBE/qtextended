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

#include "qsharedmemorycache_p.h"
#include "qglobalpixmapcache.h"
#include "qtopianamespace.h"
#include <qtopialog.h>
#include <QPixmapCache>
#include <QCache>
#include <QObject>
#include <QVector>
#include <QDebug>
#include <private/qlock_p.h>
#include <private/qpixmapdata_p.h>
#include <QStringList>
#include <qglobal.h>

#include <qwindowdefs.h>
#include <qbitmap.h>

#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE

#ifdef DEBUG_SHARED_MEMORY_CACHE
# define TEST_SHM_AT_RANDOM_LOCATION
#endif

#ifndef QTOPIA_NO_PAGE_SIZE_MASK
#define THROW_AWAY_UNUSED_PAGES
#endif

// Only define this if there really isn't going to be enough room for
// a full cache, in which case it really means the cache is set too large
// for the system, instead consider making the cache smaller.
#undef WHEN_APP_CLOSES_DESTROY_ANY_PIXMAPS_WITH_ZERO_REFERENCES

#include <QApplication>
#include <qscreen_qws.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#ifdef THROW_AWAY_UNUSED_PAGES
# include <sys/mman.h> // madvise
# include <asm/page.h> // PAGE_SIZE,PAGE_MASK,PAGE_ALIGN
# ifndef PAGE_SIZE
# define PAGE_SIZE QTOPIA_TEST_PAGE_SIZE
# endif
# ifndef PAGE_MASK
# define PAGE_MASK QTOPIA_TEST_PAGE_MASK
# endif
# ifndef PAGE_ALIGN
# define PAGE_ALIGN(addr)       (((addr)+PAGE_SIZE-1)&PAGE_MASK)
# endif // PAGE_ALIGN
#endif // THROW_AWAY_UNUSED_PAGES


char *qt_sharedMemoryData = 0;
QSharedMemoryManager *qt_getSMManager();


/*
  Structure used to link together the chunks of shared memory allocated or freed
  This is used to manage the shared memory
*/
class QSMemNode {
public:
    QSMemNode(QSMemPtr prev, QSMemPtr next) :
        signature(0xF00C0DE), freeFlag(1), prevNode(prev), nextNode(next) {}
    bool isValid() { return signature == 0xF00C0DE; }
    void invalidate() { signature = 0; }
    QSMemNode *prev() { return (QSMemNode*)(char*)prevNode; }
    QSMemNode *next() { return (QSMemNode*)(char*)nextNode; }
    QSMemNode *nextFree() { return (QSMemNode*)(char*)nextFreeNode; }
    QSMemPtr prevPtr() { return prevNode; }
    QSMemPtr nextPtr() { return nextNode; }
    QSMemPtr nextFreePtr() { return nextFreeNode; }
    void setPrev(QSMemNode *prev) { prevNode = QSMemPtr((char*)prev); }
    void setNext(QSMemNode *next) { nextNode = QSMemPtr((char*)next); }
    void setNextFree(QSMemNode *nextFree) { nextFreeNode = QSMemPtr((char*)nextFree); }
    void setPrev(QSMemPtr prev) { prevNode = prev; }
    void setNext(QSMemPtr next) { nextNode = next; }
    void setNextFree(QSMemPtr nextFree) { nextFreeNode = nextFree; }
    int size() { return (int)nextNode - (int)QSMemPtr((char*)this) - sizeof(QSMemNode); }
    void setFree(bool free) { freeFlag = (free ? 1 : 0); }
    bool isFree() { return (freeFlag == 1); }
private:
    uint        signature : 31;
    uint        freeFlag : 1;
    QSMemPtr    prevNode;
    QSMemPtr    nextNode;
    QSMemPtr    nextFreeNode;
};


// Double hashing algorithm is employed by the shared memory pixmap cache
#define SHM_ITEMS               1537        // 1537 entries
#define MAX_SHM_ITEMS           1200        // only add 1200 entries,
                                            // don't fill so hash has spaces so it works
#define MAX_SHM_FREE_ITEMS      100         // 100 entries

#define MAGIC_HASH_DELETED_VAL  -2

#ifdef DEBUG_SHARED_MEMORY_CACHE
# define CHECK_CONSISTENCY_A(msg) \
    Q_ASSERT( checkConsistency(msg, __FILE__, __LINE__) )
# define CHECK_CONSISTENCY(msg) \
    Q_ASSERT( qt_getSMManager()->checkConsistency(msg, __FILE__, __LINE__) )
# define CHECK_CACHE_CONSISTENCY() \
    Q_ASSERT( checkCacheConsistency() );
#else
# define CHECK_CONSISTENCY_A(msg)
# define CHECK_CONSISTENCY(msg)
# define CHECK_CACHE_CONSISTENCY()
#endif

/*
class QSharedMemoryHashTable {
public:
    QSMCacheItemPtr newItem(const char *key, int size);
    QSMCacheItemPtr findItem(const char *key, bool ref);
};
*/

class QSharedMemoryCacheData {
public:
    // Cache version
    int version;

    // Offsets and sizes for arrays
    int dataOffset;
    int dataSize;
    int offsetsOffset;
    int offsetsSize;    // Size of the hash table
    int freeItemsOffset;
    int freeItemsSize;

    // Counts
    int maxItems;       // Max items allowed in the hash table
    int items;          // Current number of items in the hash table
    int freeItemCount;  // Current number of items marked for deletion

    // Profiling data
    int cacheFindHits;
    int cacheFindMisses;
    int cacheFindStrcmps;
    int cacheInsertHits;
    int cacheInsertMisses;
    int cacheInsertStrcmps;

    QSMemPtr offsets[SHM_ITEMS];                // Hash table
    QSMemPtr freeItems[MAX_SHM_FREE_ITEMS+1];   // List of items marked for deletion
};


class QSharedMemoryCache {
public:
    QSharedMemoryCache(QSharedMemoryCacheData *data) : d(data) { }
    ~QSharedMemoryCache() { }

    void init() {
        d->version = 100;
        d->dataOffset = sizeof(QSharedMemoryCacheData) + 2*sizeof(QSMemNode);
        d->dataSize = QGLOBAL_PIXMAP_CACHE_LIMIT;
        d->offsetsOffset = (int)((char*)&d->offsets[0] - (char*)d);
        d->offsetsSize = SHM_ITEMS;
        d->freeItemsOffset = (int)((char*)&d->freeItems[0] - (char*)d);
        d->freeItemsSize = MAX_SHM_FREE_ITEMS;
        d->maxItems = MAX_SHM_ITEMS;
        d->items = 0;
        d->freeItemCount = 0;
        d->cacheFindHits = 0;
        d->cacheFindMisses = 0;
        d->cacheFindStrcmps = 0;
        d->cacheInsertHits = 0;
        d->cacheInsertMisses = 0;
        d->cacheInsertStrcmps = 0;
        for (int i = 0; i < SHM_ITEMS; i++)
            d->offsets[i] = -1;
        //memset(offsets,-1,SHM_ITEMS*sizeof(QSMemPtr));
    }

    QSMCacheItemPtr newItem(const char *key, int size, int type);
    QSMCacheItemPtr findItem(const char *key, bool ref, int type);
    void freeItem(QSMCacheItem *item) {

        CHECK_CONSISTENCY("Pre free item");
        CHECK_CACHE_CONSISTENCY();
        {
            QLockHandle lh(qt_getSMManager()->lock(),QLock::Write);
            if ( d->freeItemCount >= d->freeItemsSize )
                if ( !cleanUp(false) )
                    // all the items have been referenced again, clear the list
                    d->freeItemCount = 0;
#ifdef DEBUG_SHARED_MEMORY_CACHE
            for (int i = 0; i < d->freeItemCount; i++) {
                QSMemPtr ptr = d->freeItems[i];
                QSMCacheItemPtr ptr2 = (QSMCacheItemPtr)ptr;
                if ( ptr2.count() )
                    qFatal("found item with references in free list: %s", ptr2.key() );
                if ((char*)ptr == (char*)item)
                    qFatal("found item already in free list: %s", ptr2.key() );
            }
#endif // DEBUG_SHARED_MEMORY_CACHE
            d->freeItems[d->freeItemCount] = QSMemPtr((char*)item);
            d->freeItemCount++;
        }
        CHECK_CACHE_CONSISTENCY();
        CHECK_CONSISTENCY("Post free item");
    }

    bool cleanUp(bool needLock=true);

#ifdef DEBUG_SHARED_MEMORY_CACHE
    bool checkCacheConsistency();
#endif

private:
    void hash(const char *key, int &hash, int &inc);
    bool cleanUp_internal();

    QSharedMemoryCacheData *d;
};


bool QSharedMemoryCache::cleanUp(bool needLock)
{
    CHECK_CONSISTENCY("Pre CleanUp");
    CHECK_CACHE_CONSISTENCY();
    bool ret = false;
    if ( needLock ) {
        QLockHandle lh(qt_getSMManager()->lock(),QLock::Write);
        ret = cleanUp_internal();
    } else {
        ret = cleanUp_internal();
    }
    CHECK_CACHE_CONSISTENCY();
    CHECK_CONSISTENCY("Post CleanUp");
    return ret;
}


bool QSharedMemoryCache::cleanUp_internal()
{
    if ( !d->freeItemCount )
       return false;

    int releasedItems = 0;
    for ( int i = 0; i < d->freeItemCount; i++ ) {
       QSMCacheItemPtr item(d->freeItems[i]);
        if (!item.count()) {
           bool found = false;
           for ( int j = 0; j < SHM_ITEMS; j++ ) {
                if ( (char*)d->freeItems[i] == (char*)d->offsets[j] ) {
                    // See if next in hash table is null
                    int hashIndex, hashInc;
                    QSMemPtr memPtr = d->offsets[j];
                    QSMCacheItemPtr delItem(memPtr);
                    hash(delItem.key(), hashIndex, hashInc);
                    hashIndex = (j + hashInc) % SHM_ITEMS;
                    memPtr = d->offsets[hashIndex];

                    if ( memPtr )
                        d->offsets[j] = MAGIC_HASH_DELETED_VAL;
                    else
                        d->offsets[j] = -1;

                    d->items--;
                    found = true;
                }
           }
           item.free();
           memmove(&d->freeItems[i],&d->freeItems[i+1],(d->freeItemCount-i)*sizeof(QSMemPtr));
           d->freeItemCount--;
           releasedItems++;
           if ( releasedItems >= 3 )
               break;
        }
     }

    return (releasedItems >= 1);
}


class QSharedMemoryData {
public:
    QSharedMemoryCacheData cacheData;

    QSMemNode head;
    QSMemNode first;
    char data[QGLOBAL_PIXMAP_CACHE_LIMIT];
    QSMemNode tail;
};

static QSharedMemoryManager *qt_SMManager = 0;

static void clean_shared_mem_manager()
{
    if ( qt_SMManager )
        delete qt_SMManager;
    qt_SMManager = 0;
}

QSharedMemoryManager *qt_getSMManager()
{
    if ( !qt_SMManager ) {
        qt_SMManager = new QSharedMemoryManager;
        qAddPostRoutine( clean_shared_mem_manager );
    }
    return qt_SMManager;
}


QSMCacheItemPtr QSharedMemoryManager::newItem(const char *key, int size, int type)
{
    return qt_getSMManager()->cache->newItem(key, size, type);
}


QSMCacheItemPtr QSharedMemoryManager::findItem(const char *key, bool ref, int type)
{
    return qt_getSMManager()->cache->findItem(key, ref, type);
}


void QSharedMemoryManager::freeItem(QSMCacheItem *item)
{
    return qt_getSMManager()->cache->freeItem(item);
}


void QSMCacheItemPtr::free()
{
    if (d) {
        if ( d->count ) {
            qWarning("Can't free null item or item with references");
            return;
        }
        qt_getSMManager()->free(d->key, false);
        qt_getSMManager()->free(QSMemPtr((char*)d), false);
        d = 0;
    } else {
        qWarning("QSMCacheItemPtr::free(): QSMCacheItem has been freed");
    }
}


void QSMCacheItemPtr::ref()
{
    if (!d) {
        qWarning("QSMCacheItemPtr::ref(): QSMCacheItem has been freed");
    } else {
        d->ref();
    }
}


void QSMCacheItemPtr::deref()
{
    if (!d) {
        qWarning("QSMCacheItemPtr::deref(): QSMCacheItem has been freed");
        return;
    }
    if ( !d->count ) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
        qWarning("QSMCacheItem ref count already zero (%s)", (char*)d->key);
#endif
        return;
    }
    d->deref();
    if ( !d->count ) {
        QSharedMemoryManager::freeItem(d);
    }
}


#ifdef USE_EVIL_SIGNAL_HANDLERS
int signalList[14] = {
    SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGKILL,
    SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2, SIGBUS
};


typedef void (*sighandler_t)(int);
sighandler_t oldHandlers[sizeof(signalList)/sizeof(int)];


void cleanUpCacheSignalHandler(int sig)
{
    for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ )
        if ( signalList[i] == sig ) {
            if ( oldHandlers[i] )
                oldHandlers[i](sig);
            else
                abort();
        }
}
#endif // USE_EVIL_SIGNAL_HANDLERS


typedef void (*_qt_image_cleanup_hook_64)(qint64);
extern _qt_image_cleanup_hook_64 qt_image_cleanup_hook_64;

// Attach/create shared memory cache
QSharedMemoryManager::QSharedMemoryManager()
{
    shm = 0;
    shmId = -1;

    bool server = qApp->type() == QApplication::GuiServer;

    QString tmp = Qtopia::tempDir() + "sharedCache";
    QFile f(tmp);
    if ( !f.exists() ) {
        if (!f.open(QIODevice::WriteOnly))
            qFatal("%s creating shared memory key file %s",
                    ( f.error() == QFile::PermissionsError ? "Permissions error" :
                      "Error" ), qPrintable(tmp));
        f.close();
    }

    QByteArray ipcFile = tmp.toLatin1();
    key_t key = ftok(ipcFile.data(), 0xEEDDCCC2);

    l = new QLock(ipcFile.data(), 't', server);

    if ( server ) {
        shmId = shmget(key, 0, 0);
        if ( shmId != -1 )
            shmctl(shmId, IPC_RMID, 0); // Delete if exists already
    }

    shmId = shmget(key, 0, 0);

    if ( shmId == -1 ) { // It needs to be created
        shmId = shmget(key, sizeof(QSharedMemoryData), IPC_CREAT|0600);
        if ( shmId != -1 ) {
            struct shmid_ds shmInfo;
            shmctl(shmId, IPC_STAT, &shmInfo);
            shm = (QSharedMemoryData*)shmat( shmId, 0, 0 );
        }
    } else {
#ifdef TEST_SHM_AT_RANDOM_LOCATION
        srand((int)clock());
        shm = (QSharedMemoryData*)shmat(shmId, (void*)rand(), SHM_RND);
#else
        shm = (QSharedMemoryData*)shmat(shmId, 0, 0);
#endif
    }

    if ( shmId == -1 || (long)shm == -1 )
        qFatal("Cannot attach to shared memory");

    qt_sharedMemoryData = shm->data;
    cache = new QSharedMemoryCache(&shm->cacheData);

    if ( server ) {
        shm->head = QSMemNode(QSMemPtr(),QSMemPtr((char*)&shm->first));
        shm->head.setFree(true);
        shm->head.setNextFree((char*)&shm->first);
        shm->first = QSMemNode(QSMemPtr((char*)&shm->head),QSMemPtr((char*)&shm->tail));
        shm->head.setFree(true);
        shm->first.setNextFree(QSMemPtr());
        shm->tail = QSMemNode(QSMemPtr((char*)&shm->first),QSMemPtr());
        shm->tail.setFree(false);
        shm->tail.setNextFree(QSMemPtr());
#ifdef THROW_AWAY_UNUSED_PAGES
        long freePageStart = PAGE_ALIGN((long)&shm->first + sizeof(QSMemNode));
        long freePagesLength = PAGE_ALIGN((long)&shm->tail) - freePageStart;
        if ( freePagesLength ) {
            if ( madvise((void*)freePageStart, freePagesLength, MADV_DONTNEED) ) {
                perror("madvise of shared memory");
                qWarning("madvise error with marking shared memory pages as not needed");
            }
        }
#endif // THROW_AWAY_UNUSED_PAGES
        cache->init();
    }

    CHECK_CONSISTENCY_A("Initial");

#ifdef USE_EVIL_SIGNAL_HANDLERS
    for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ ) {
        struct sigaction act;
        struct sigaction oldact;
        act.sa_handler = cleanUpCacheSignalHandler;
        act.sa_flags = SA_ONESHOT | SA_ONSTACK;
        if ( sigaction(i, &act, &oldact) == 0 )
            oldHandlers[i] = oldact.sa_handler;
        else
            oldHandlers[i] = 0;
    }
#endif // USE_EVIL_SIGNAL_HANDLERS
    if (!qt_image_cleanup_hook_64) {
        qLog(SharedMemCache) << "Setup image cleanup hook";
        qt_image_cleanup_hook_64 = QSharedMemoryManager::derefSharedMemoryPixmap;
    } 
}


QSharedMemoryManager::~QSharedMemoryManager()
{
    delete cache;
    // Detach from shared memory
    if ( qApp->type() == QApplication::GuiServer )
        shmctl(shmId, IPC_RMID, 0); // Mark for deletion
    shmdt((char*)shm);
    shm = 0;
    delete l;
}


#ifdef DEBUG_SHARED_MEMORY_CACHE

#define SHARED_MEMORY_CHECK( condition ) \
    if ( !(condition) ) {\
        qWarning("%s Consistency Check (line %i) fails test: %s (line %i)", msg, line, #condition, __LINE__ );\
        return false;\
    }

bool QSharedMemoryManager::checkConsistency(const char *msg, const char * /*file*/, int line)
{
    SHARED_MEMORY_CHECK( shm->head.isValid() );
    SHARED_MEMORY_CHECK( shm->first.isValid() );
    SHARED_MEMORY_CHECK( shm->tail.isValid() );

    SHARED_MEMORY_CHECK( !shm->head.prev() );
    SHARED_MEMORY_CHECK( shm->head.isFree() );
    SHARED_MEMORY_CHECK( shm->head.next() == &shm->first );
    SHARED_MEMORY_CHECK( !shm->tail.isFree() );
    SHARED_MEMORY_CHECK( !shm->tail.nextFree() );
    SHARED_MEMORY_CHECK( !shm->tail.next() );

    QSMemNode *prev = &shm->head;
    QSMemNode *node = &shm->first;
    QSMemNode *nextFree = prev->nextFree();
    while (node) {
        SHARED_MEMORY_CHECK( (bool)prev );
        SHARED_MEMORY_CHECK( (bool)node );
        SHARED_MEMORY_CHECK( prev->isValid() );
        SHARED_MEMORY_CHECK( node->isValid() );
        SHARED_MEMORY_CHECK( prev->next() == node );
        if ( node->isFree() ) {
            SHARED_MEMORY_CHECK( node == nextFree );
            nextFree = node->nextFree();
        } else {
            SHARED_MEMORY_CHECK( node != nextFree );
        }
        prev = node;
        node = node->next();
    }
    SHARED_MEMORY_CHECK( !(bool)nextFree );
    SHARED_MEMORY_CHECK( prev == &shm->tail );
    return true;
}


void QSharedMemoryManager::rebuildFreeList()
{
    // rebuild free list
    QSMemNode *node = &shm->tail;
    node = node->prev();
    QSMemNode *lastFree = 0;
    while ( node && node != &shm->head ) {
        Q_ASSERT(node->isValid());
        if ( node->isFree() ) {
            node->setNextFree(lastFree);
            lastFree = node;
        }
        node = node->prev();
    }
    Q_ASSERT( node == &shm->head );
    node->setNextFree(lastFree);
    return;
}
#endif // DEBUG_SHARED_MEMORY_CACHE


QSMemPtr QSharedMemoryManager::alloc(int size, bool needLock)
{
    CHECK_CONSISTENCY_A("Pre Alloc");
    QSMemPtr ret;
    if ( needLock ) {
        QLockHandle lh(lock(),QLock::Write);
        ret = internal_alloc(size);
    } else {
        ret = internal_alloc(size);
    }
    CHECK_CONSISTENCY_A("Post Alloc");
    return ret;
}


QSMemPtr QSharedMemoryManager::internal_alloc(int size)
{
    if ( !size )
        return QSMemPtr();

    size = (size+sizeof(QSMemNode)+3)&~0x3;
    QSMemNode *node = &shm->head;
    QSMemNode *prevFree = node;
    while ( node ) {
        if ( !node->isValid() || !node->isFree() ) {
            qWarning("QSharedMemoryManager::alloc inconsistency error, possible memory corruption");
        } else {
            if ( node->size() == size ) {
                prevFree->setNextFree(node->nextFreePtr());
                node->setFree(false);
                return QSMemPtr((char*)node + sizeof(QSMemNode));
            } else if ( node->size() >= size + (int)sizeof(QSMemNode) ) {

                QSMemNode *newNode = (QSMemNode*)((char*)node + size);
                *newNode = QSMemNode(QSMemPtr((char*)node),node->nextPtr());
                if ( node->next() )
                    node->next()->setPrev(QSMemPtr((char*)newNode));
                node->setNext(QSMemPtr((char*)newNode));

                newNode->setNextFree(node->nextFreePtr());
                prevFree->setNextFree(newNode);

                node->setFree(false);
                return QSMemPtr((char*)node + sizeof(QSMemNode));
            }
        }
        prevFree = node;
        node = node->nextFree();
    }

    qWarning("no free holes in shm");
    return QSMemPtr();
}


void QSharedMemoryManager::free(QSMemPtr ptr, bool needLock)
{
    CHECK_CONSISTENCY_A("Pre Free");
    if ( needLock ) {
        QLockHandle lh(lock(),QLock::Write);
        internal_free(ptr);
    } else {
        internal_free(ptr);
    }
    CHECK_CONSISTENCY_A("Post Free");
}


void QSharedMemoryManager::internal_free(QSMemPtr ptr)
{
    QSMemNode *node = (QSMemNode *)((char*)ptr - sizeof(QSMemNode));
    if ( !node->isValid() ) {
        qWarning("free of non QSharedMemoryManager::alloc'd block");
        return;
    }

    node->setFree(true);

    // Consolidate to the left
    while ( node != &shm->first && node->isFree() && node->prev() && node->prev()->isFree() ) {
        QSMemNode *prev = node->prev();

        QSMemNode *next = node->next();
        if ( !node->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed A");
        if ( next && !next->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed B");
        if ( !prev->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed C");
        if ( next )
            next->setPrev(prev);
        prev->setNext(next);
        node->invalidate();

        node = prev;
    }

    QSMemNode *newFreeNode = node;

    // Consolidate to the right
    while ( node->isFree() && node->next() && node->next()->isFree() ) {
        QSMemNode *prev = node;
        node = node->next();

        QSMemNode *next = node->next();
        if ( !node->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed D");
        if ( next && !next->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed E");
        if ( !prev->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed F");
        if ( next )
            next->setPrev(prev);
        prev->setNext(next);
        node->invalidate();
    }

    // fix up free list
    // set the new free node to point to the next one
    node = newFreeNode->next();

#ifdef THROW_AWAY_UNUSED_PAGES
    long freePageStart = PAGE_ALIGN((long)newFreeNode+sizeof(QSMemNode));
    long freePagesLength = PAGE_ALIGN((long)node) - freePageStart;
    if ( freePagesLength ) {
        if ( madvise((void*)freePageStart, freePagesLength, MADV_DONTNEED) ) {
            perror("madvise of shared memory");
            qWarning("madvise error with marking shared memory pages as not needed");
        }
    }
#endif // THROW_AWAY_UNUSED_PAGES

    newFreeNode->setNextFree(QSMemPtr());
    while ( node ) {
        if ( !node->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed G");
        if ( node->isFree() ) {
            newFreeNode->setNextFree(node);
            break;
        }
        node = node->next();
    }

    // set the first free node before the new one to point to the new one
    node = newFreeNode->prev();
    while ( node ) {
        if ( !node->isValid() )
            qWarning("QSharedMemoryManager::free consistency check failed H");

        if ( node->isFree() ) {
            node->setNextFree(newFreeNode);
            return;
        }
        node = node->prev();
    }

    // We should have eventually found the head node which should be free
    qWarning("QSharedMemoryManager::free consistency check failed I");
}


// Double Hashing is what is implemented, we use increment as our probe variable
void QSharedMemoryCache::hash(const char *key, int &hash, int &increment)
{
    int i = 0;
    hash = 1;
    while (key[i]) {
        hash += i * 131 + key[i];
        i++;
    }
    increment = hash + 31;
    if (i)
        increment -= key[i-1];
    hash %= SHM_ITEMS;
    increment %= SHM_ITEMS - 1;
    if ( increment == 0 )
        increment = 31 % (SHM_ITEMS - 1);
}


#ifdef DEBUG_SHARED_MEMORY_CACHE
bool QSharedMemoryCache::checkCacheConsistency()
{
    for ( int i = 0; i < SHM_ITEMS; i++ ) {
        QSMemPtr memPtr = d->offsets[i];
        if ( (int)memPtr == MAGIC_HASH_DELETED_VAL )
            continue;
        if ( !memPtr )
            continue;
        QSMemNode *node = (QSMemNode *)((char *)memPtr - sizeof(QSMemNode));
        if ( !node->isValid() ) {
            qWarning( "cache inconsistent");
            return false;
        }
    }
    return true;

}
#endif // DEBUG_SHARED_MEMORY_CACHE


QSMCacheItemPtr QSharedMemoryCache::newItem(const char *key, int size, int type)
{
    CHECK_CACHE_CONSISTENCY();

    qLog(SharedMemCache) << "allocate for" <<  key;
    bool retry = true;
    while ( retry ) {
        if ( d->items + 1 < d->maxItems ) {
            QSMemPtr memPtr = qt_getSMManager()->alloc(size + sizeof(QSMCacheItem));
            if ( memPtr ) {
                qLog(SharedMemCache) << "Alloc mem at" << (uchar*)memPtr;
                int strLen = strlen(key);
                QSMCacheItem *item = (QSMCacheItem*)(char*)memPtr;
                item->count = 1;
                item->type = (QSMCacheItem::QSMCacheItemType)type;
                item->key = qt_getSMManager()->alloc(strLen+1);
                qLog(SharedMemCache) << "key at" << (uchar*)item->key << (int)item->key;
                if (!item->key) {
                    cleanUp();
                    item->key = qt_getSMManager()->alloc(strLen+1);
                    if (!item->key) {
                        qt_getSMManager()->free(memPtr);
#ifdef PROFILE_SHARED_MEMORY_CACHE
                        d->cacheInsertMisses++;
#endif
                        return QSMCacheItemPtr();
                    }
                }
                d->items++;
                memcpy((char*)item->key, key, strLen+1);
                QLockHandle lh(qt_getSMManager()->lock(),QLock::Write);
                int hashIndex, hashInc;
                hash(key, hashIndex, hashInc);
                qLog(SharedMemCache) << "hash for"<< key << "is" <<hashIndex;
                QSMemPtr memPtr2 = d->offsets[hashIndex];
                qLog(SharedMemCache) << "new: starting item" << hashIndex << "has value"<<(int)memPtr2;
                int firstReplaceHashIndex = -1;
                while (memPtr2 && (int)memPtr2 != MAGIC_HASH_DELETED_VAL) {
#ifdef PROFILE_SHARED_MEMORY_CACHE
                    if ( firstReplaceHashIndex == -1 )
                        d->cacheInsertStrcmps++;
#endif
                    QSMCacheItemPtr item(memPtr2);
                    if ( firstReplaceHashIndex == -1 && !qstrcmp(key, item.key()) ) {
                        // Well actually, can't just remove it because other processess may
                        // have references to it, they perhaps have active pixmap data connected
                        // to this shared memory, but it will go away when they release it and
                        // it's refcount reaches 0. What we can do instead is make sure when
                        // we look up in the cache we hit this new one first instead of the
                        // old one, so we insert it earlier, ie swap the positions around in the
                        // hash table to where we would put it otherwise.
                        firstReplaceHashIndex = hashIndex;

                        // As we are replacing the old pixmap, deference it as there should
                        // only be one reference for both pixmaps after an
                        // QGlobalPixmapCache::insert(), as QGlobalPixmapCache::remove()
                        // must remove it from the cache.
                        item.deref();
                    }

                    hashIndex = (hashIndex + hashInc) % SHM_ITEMS;
                    memPtr2 = d->offsets[hashIndex];
                    qLog(SharedMemCache) << "new: next item" << hashIndex << "has value" << (int)memPtr2;
                }
                qLog(SharedMemCache) << "new: using item" << hashIndex << "with value" << (int)memPtr2;
                if ( firstReplaceHashIndex != -1 ) {
                    d->offsets[hashIndex] = d->offsets[firstReplaceHashIndex];
                    d->offsets[firstReplaceHashIndex] = memPtr;
                } else {
                    d->offsets[hashIndex] = memPtr;
                }

                CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
                d->cacheInsertHits++;
#endif
                qLog(SharedMemCache) << "allocated" << key << "to index" <<hashIndex;
                return QSMCacheItemPtr(item);
            }
        }
        qWarning("error allocing %i bytes", size);
        retry = cleanUp();
    }

    CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
    d->cacheInsertMisses++;
#endif
    return QSMCacheItemPtr();
}


// Optimization: "type" is currently ignored, but could be used to find the hash table to look in
QSMCacheItemPtr QSharedMemoryCache::findItem(const char *keyStr, bool ref, int /*type*/)
{
    QLockHandle lh(qt_getSMManager()->lock(),QLock::Read);

    CHECK_CACHE_CONSISTENCY();

    qLog(SharedMemCache) << "searching for" << keyStr;
    // search the shared memory
    int hashIndex, hashInc;
    hash(keyStr, hashIndex, hashInc);
    QSMemPtr memPtr = d->offsets[hashIndex];
    qLog(SharedMemCache) << "find: starting item " << hashIndex << " has value " << (int)memPtr;
    while (memPtr) {
        if ( (int)memPtr != MAGIC_HASH_DELETED_VAL ) {
            QSMCacheItemPtr item(memPtr);
            qLog(SharedMemCache) << "key" << keyStr << "index:"<< hashIndex << "item" << (int)memPtr << "mem" << (uchar*)memPtr;
            qLog(SharedMemCache) << "comparing" << keyStr << "with" << item.key() << "(" << (int)((QSMCacheItem*)item)->key << (uchar*)((QSMCacheItem*)item)->key << ") (hash:" << hashIndex << ")";
#ifdef PROFILE_SHARED_MEMORY_CACHE
            d->cacheFindStrcmps++;
#endif
            if ( !qstrcmp(keyStr, item.key()) ) {
                // Item was deref'd and put in free list, remove from free list
                if ( item.count() == 0 ) {
                    for ( int i = 0; i < d->freeItemCount; i++ ) {
                        if ((int)d->freeItems[i] == (int)memPtr) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
                            qWarning("found %s in the free list, fixing up the free list", item.key());
#endif
                            memmove(&d->freeItems[i],&d->freeItems[i+1],(d->freeItemCount-i)*sizeof(QSMemPtr));
                            d->freeItemCount--;
                            break;
                        }
                    }
                }
                if ( ref )
                    item.ref();
                qLog(SharedMemCache) << "using: using item " << hashIndex << " with value " << (int)memPtr;
                qLog(SharedMemCache) << "found " << item.key() << " (hash: " << hashIndex << " refcount: " << item.count();
                CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
                d->cacheFindHits++;
#endif
                return item;
            }
        }
        hashIndex = (hashIndex + hashInc) % SHM_ITEMS;
        memPtr = d->offsets[hashIndex];
        qLog(SharedMemCache) << "find: next item" << hashIndex << "has value" << (int)memPtr;
    }

    CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
    d->cacheFindMisses++;
#endif
    qLog(SharedMemCache) << "didn't find" << keyStr << "(hash:" << hashIndex << ")";
    return QSMCacheItemPtr();
}


class PixmapShmItem {
public:
    uint    w : 16;
    uint    h : 16;
    uint    d : 6;
    uint    numCols : 8;
    QImage::Format format;      //compiler should should pack this correctly
    //uint    mostRecentId; // relative age of the pixmap
};

void QSharedMemoryManager::derefSharedMemoryPixmap(qint64 s)
{
    int ser_no = s >> 32;

    if (qt_SMManager && qt_getSMManager()->localSerialMap.contains(ser_no)) {
        QSMCacheItemPtr item(qt_getSMManager()->localSerialMap[ser_no]);
        item.deref();
        qt_getSMManager()->localSerialMap.remove(ser_no);
    }
}


// Nasty - use a friend class not in E to access QPixmap private data
class QX11PaintEngine {
public:
    static QPixmapData *getPixmapData(const QPixmap &pm) {
        return pm.data;
    }
};

bool QSharedMemoryManager::findPixmap(const QString &k, QPixmap &pm, bool ref) const
{
    qLog(SharedMemCache) << "search for" << k;

    if ( isGlobalPixmap(k) ) {
        // search the shared memory
        PixmapShmItem *item = (PixmapShmItem*)(char*)cache->findItem(k.toLatin1().data(), ref, QSMCacheItem::Pixmap);

        if ( item ) {
            QPixmapData *data = pm.data_ptr();
            QImage newimage((uchar*)item + sizeof(PixmapShmItem),
                                item->w, item->h, item->format);
            localSerialMap.insert(newimage.serialNumber(), (char*)item);
            qLog(SharedMemCache) << "Created image" << newimage.width() << "x" << newimage.height() << "format" << newimage.format();
            if (item->d <= 8) {
                newimage.setNumColors(item->numCols);
                QVector<QRgb> clut(item->numCols);
                memcpy(clut.data(), (char*)item+sizeof(PixmapShmItem)+newimage.numBytes(), item->numCols*sizeof(QRgb));
                newimage.setColorTable(clut);
            }
            data->fromImage(newimage,0);
            qLog(SharedMemCache) << "Found pixmap" << pm.width() << "x" << pm.height();
            return true;
        }
    }
    return false;
}

bool QSharedMemoryManager::insertPixmap(const QString& k, const QPixmap &pm)
{

    if (isGlobalPixmap(k)) {
        QPixmapData *data = const_cast<QPixmap&>(pm).data_ptr();
        const QImage &img = data->toImage();

        int size = img.numBytes() + sizeof(PixmapShmItem);
        if (img.depth() <= 8)
            size += img.numColors() * sizeof(QRgb);

        QSMCacheItemPtr cacheItem = cache->newItem(k.toLatin1().data(), size, QSMCacheItem::Pixmap);
        PixmapShmItem *item = (PixmapShmItem*)(char*)cacheItem;

        if (item) {
            qLog(SharedMemCache) << "allocated" << size << "bytes for" << img.numBytes() << "byte image";
            cacheItem.setCount(1);  // one reference, the app inserting this pixmap holds a local
                                    // refcount, when the local refcount is 0, it will cause the
                                    // shared global refcount to be deref'ed, when that is 0, it is
                                    // marked for freeing from shared memory
            item->w = img.width();
            item->h = img.height();
            item->d = img.depth();
            item->format = img.format();
            item->numCols = 0;

            memcpy((char*)item+sizeof(PixmapShmItem), img.bits(), img.numBytes());
            if (item->d <= 8) {
                item->numCols = img.numColors();
                memcpy((char*)item+sizeof(PixmapShmItem)+img.numBytes(),
                        img.colorTable().data(), img.numColors() * sizeof(QRgb));
            }

            // Now replace pixmap's internal image with the one in shared mem
            QImage newimage((uchar*)item + sizeof(PixmapShmItem),
                                item->w, item->h, item->format);
            if (item->d <= 8) {
                newimage.setNumColors(item->numCols);
                QVector<QRgb> clut(item->numCols);
                memcpy(clut.data(), (char*)item+sizeof(PixmapShmItem)+img.numBytes(), item->numCols*sizeof(QRgb));
                newimage.setColorTable(clut);
            }
            localSerialMap.insert(newimage.serialNumber(), (char*)item);
            data->fromImage(newimage,0);

            return true;
        }
    }
    return false;
}

// removes a pixmap from the shared memory cache
void QSharedMemoryManager::removePixmap(const QString &k)
{
    qLog(SharedMemCache) << "search for" << k;

    if ( isGlobalPixmap(k) ) {
        // Get the cache item, but don't increment the reference count
        QSMCacheItemPtr cacheItem = cache->findItem(k.toLatin1().data(),
                                                    false,
                                                    QSMCacheItem::Pixmap);
        if ( (QSMCacheItem*)cacheItem == 0 ) {
            qLog(SharedMemCache) << "trying to remove non-global pixmap";
            return;
        }
        cacheItem.deref();
    }
}

// ============================================================================
//
// QGlobalPixmapCache
//
// ============================================================================

/*!
    \class QGlobalPixmapCache
    \inpublicgroup QtBaseModule


    \brief The QGlobalPixmapCache class provides a system-wide cache for pixmaps.

    This class is a tool for caching of pixmaps which are likely to be
    used by multiple processes, or are expensive to generate and may
    benefit from caching between application executions.
    For an application-wide QPixmap cache use QPixmapCache.

    Use insert() to insert pixmaps, find() to find them, and remove()
    to remove them from the global cache.

    QGlobalPixmapCache contains no member data, only static functions to
    access the global pixmap cache.

    The global cache associates a pixmap with a string (key). If two pixmaps
    are inserted into the global cache using equal keys, then the last pixmap
    will hide the first pixmap.

    When the global cache becomes full, insert() will fail until adequate
    space is made available by removing unneeded pixmaps from the global
    cache, or by deleting all references to the global pixmap's data.

    \sa QPixmapCache, QPixmap, QGLOBAL_PIXMAP_CACHE_LIMIT
    \ingroup multimedia
*/

/*!
    Looks for a cached pixmap associated with the \a key in the global cache.
    If the pixmap is found, the function sets \a pixmap to that pixmap and
    returns true; otherwise it leaves \a pixmap alone and returns false.

    Internally the global cache ensures that the pixmap is not removed until
    the \a pixmap is destroyed, therefore \a pixmap is directly associated
    with the QPixmap in the global cache and should not be reused.

    Example of correct use:
    \code
        {
            QPixmap pm;
            if (!QGlobalPixmapCache::find("background_image", pm)) {
                pm.load("backgroundimage.png");
                QGlobalPixmapCache::insert("background_image", pm);
            }
            painter->drawPixmap(0, 0, pm);
        }

        {
            QPixmap pm;
            if (!QGlobalPixmapCache::find("wallpaper_image", pm)) {
                pm.load("wallpaper.png");
                QGlobalPixmapCache::insert("wallpaper_image", pm);
            }
            painter->drawPixmap(0, 0, pm);
        }
    \endcode

    Example of incorrect use:
    \code
        QPixmap pm;
        if (!QGlobalPixmapCache::find("background_image", pm)) {
            pm.load("backgroundimage.png");
            QGlobalPixmapCache::insert("background_image", pm);
        }
        painter->drawPixmap(0, 0, pm);

        if (!QGlobalPixmapCache::find("wallpaper_image", pm)) {
            pm.load("wallpaper.png");
            QGlobalPixmapCache::insert("wallpaper_image", pm);
        }
        painter->drawPixmap(0, 0, pm);
    \endcode
*/
bool QGlobalPixmapCache::find( const QString &key, QPixmap &pixmap)
{
    return qt_getSMManager()->findPixmap( key, pixmap );
}

/*!
    Inserts the pixmap \a pixmap associated with the \a key into
    the global cache.

    All pixmaps inserted by Qt Extended have a key starting with "_$QTOPIA",
    so your own pixmap keys should never begin with "_$QTOPIA".

    The function returns true if the object was inserted into the
    global cache; otherwise it returns false.

    When the pixmap is no longer required in the global cache, remove()
    may be called to free space in the global cache.

    If two pixmaps are inserted into the global cache using equal keys,
    then the last pixmap will hide the first pixmap.
*/
bool QGlobalPixmapCache::insert( const QString &key, const QPixmap &pixmap)
{
    if ( qt_getSMManager()->insertPixmap( key, pixmap ) ) {
        return true;
    } else {
        // Cache might be full of unreferenced pixmaps, so try doing
        // a cleanup first
        while ( qt_getSMManager()->pixmapCache()->cleanUp() );
        if ( qt_getSMManager()->insertPixmap( key, pixmap ) ) {
            return true;
        }
    }

    return false;
}

/*!
    Removes the pixmap associated with the \a key from the global cache.

    Space in the global cache will not be reclaimed unless remove() is called
    on pixmaps that are not required or all pixmaps referencing the global
    pixmap data are deleted.

    Note that calling remove marks the image for removal, but may not 
    necessarily remove it until some time later.  Other callers are permitted
    to re-reference the pixmap until that time.  Consequently, the mere 
    presence of an image in the global pixmap cache should not be used to 
    indicate application state.
*/
void QGlobalPixmapCache::remove( const QString &key )
{
    qt_getSMManager()->removePixmap( key );
}

#endif // QT_NO_QWS_SHARED_MEMORY_CACHE
