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

#ifndef QSHAREDMEMORYCACHE_P_H
#define QSHAREDMEMORYCACHE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QHash>

//#define DEBUG_SHARED_MEMORY_CACHE

//#if !defined(_WS_QWS_) || defined(QT_NO_QWS_MULTIPROCESS)
//#  define QT_NO_QWS_SHARED_MEMORY_CACHE
//#endif

//#define QT_NO_QWS_SHARED_MEMORY_CACHE 

#ifdef QT_NO_QWS_SHARED_MEMORY_CACHE
#define QT_NO_SHARED_FONT_CACHE
#define QT_NO_SHARED_PIXMAP_CACHE
#endif

#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE

extern char *qt_sharedMemoryData;

class QPixmap;

/*
  Process independant pointer to data in shared memory
  (on some systems a piece of shared memory could be mapped
  to different addresses in each process, therefore references
  to shared memory stored in shared memory need to be
  relative to where the shared memory is mapped)
*/
class QSMemPtr {
public:
    QSMemPtr(char *mem) { index = (mem) ? int(mem - qt_sharedMemoryData) : -1; }
    QSMemPtr() { index = -1; }
    QSMemPtr &operator =(int i) { index = i; return *this; }
    operator char*() const { return index != -1 ? &qt_sharedMemoryData[index] : 0; }
    operator uchar*() const { return index != -1 ? (uchar*)&qt_sharedMemoryData[index] : 0; }
    operator short*() const { return index != -1 ? (short*)&qt_sharedMemoryData[index] : 0; }
    operator ushort*() const { return index != -1 ? (ushort*)&qt_sharedMemoryData[index] : 0; }
    operator int*() const { return index != -1 ? (int*)&qt_sharedMemoryData[index] : 0; }
    operator uint*() const { return index != -1 ? (uint*)&qt_sharedMemoryData[index] : 0; }
    operator bool() const { return index != -1; }
    operator int() const { return index; }
private:
    int index; // offset from start of shared memory
};

/*
  A cached item in shared memory, it has a key which is
  in shared memory and a ref count which will be in shared memory
  also if this object is allocated in shared memory, usually
  it is allocated with data following the reference to the key
*/
class QSMCacheItem {
public:
    QSMCacheItem() { count = 1; }
    void ref() { count++; }
    bool deref() { return !--count; }
    QSMemPtr key;
    enum QSMCacheItemType {
        Global = 0,
        Pixmap = 1,
        GlyphRowIndex = 2,
        GlyphRow = 3
    };
    /*
        Would be nice to have virtual destructors for when
        deref'd and then cleaning up from the shared memory different cached items.
        But this is not really possible in a generic way because it would depend
        on the process which created the item to still be in memory to provide
        the destructor and for the cleanup to get that process to do the work.
        Obviously not very sensible, the alternative is to only have a fixed set
        of types of items with the cleanup for those all provided in the library.
    */
    QSMCacheItemType type;
    uint count;
};

class QSMCacheItemPtr {
public:
    // default
    QSMCacheItemPtr() : d(0) {}
    // construct from pointer to item
    QSMCacheItemPtr(QSMCacheItem *ptr) : d(ptr) {}
    // constructs from an index to the start of the item structure
    QSMCacheItemPtr(QSMemPtr ptr) {
        d = (QSMCacheItem*)(char*)ptr;
    }
    // constructs from a memory ptr to where the data after the item starts
    QSMCacheItemPtr(void *data) {
        char *ptr = (char*)data;
        if ( (long)ptr != (((long)ptr+3)&~3) )
            qWarning("QSMCacheItemPtr: passed a non-aligned data ptr %lx", (long)ptr);
        d = (QSMCacheItem*)(ptr - sizeof(QSMCacheItem));
    }
    // returns a pointer to the data after the item
    operator char*() const {
        return d ? (char*)d + sizeof(QSMCacheItem) : 0;
    }
    // pointer to the item itself
    operator QSMCacheItem*() const { return d; }
    void free();
    void ref();
    void deref();
    char *key() { return (char*)d->key; }
    int count() { return d->count; }
    void setCount(int c) { d->count = c; }
private:
    QSMCacheItem *d;
};

class QSharedMemoryCache;
class QSharedMemoryData;
class QLock;

class QSharedMemoryManager {
public:
    QSharedMemoryManager();
    ~QSharedMemoryManager();

    // finds a pixmap from the shared memory cache
    bool findPixmap(const QString &key, QPixmap &pm, bool ref=true) const;
    // inserts a pixmap to the shared memory cache
    bool insertPixmap(const QString &key, const QPixmap &pm);
    // removes a pixmap from the shared memory cache
    void removePixmap(const QString &key);

    // creates a new item in the cache
    static QSMCacheItemPtr newItem(const char *key, int size, int type = QSMCacheItem::Global);
    // finds an item from the cache
    static QSMCacheItemPtr findItem(const char *key, bool ref, int type = QSMCacheItem::Global);
    // removes an item from the cache, items are ref counted, on last
    // deref this will get called, normally just use item->deref().
    static void freeItem(QSMCacheItem *item);

    QLock *lock() { return l; }
    QSharedMemoryCache *pixmapCache() { return cache; }
    QSharedMemoryData *data() { return shm; }

    QSMemPtr alloc(int size, bool lock=true);
    void free(QSMemPtr ptr, bool lock=true);

#ifdef DEBUG_SHARED_MEMORY_CACHE
    bool checkConsistency(const char *msg, const char *file, int line);
    void rebuildFreeList();
#endif

private:
    QSMemPtr internal_alloc(int size);
    void internal_free(QSMemPtr ptr);

    bool isGlobalPixmap(const QString &) const { return true; }

    int shmId;
    QLock *l;
    QSharedMemoryCache *cache;
    QSharedMemoryData *shm;

    mutable QHash<int,char*> localSerialMap;
    static void derefSharedMemoryPixmap(qint64);
};

extern QSharedMemoryManager *qt_getSMManager();

#endif // !QT_NO_QWS_SHARED_MEMORY_CACHE

#endif
