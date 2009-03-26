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

#ifndef QMALLOCPOOL_H
#define QMALLOCPOOL_H

#include <cstdlib>
#include <QString>

#include "qtopiailglobal.h"

class QMallocPoolPrivate;

class QTOPIAIL_EXPORT QMallocPool
{
public:
    enum PoolType { Owned, NewShared, Shared };
    QMallocPool();
    QMallocPool(void * poolBase, unsigned int poolLength,
                PoolType type = Owned, const QString& name = QString());
    ~QMallocPool();

    size_t size_of(void *);
    void *calloc(size_t nmemb, size_t size);
    void *malloc(size_t size);
    void free(void *ptr);
    void *realloc(void *ptr, size_t size);

    bool isValid() const;

    struct MemoryStats {
        unsigned long poolSize;
        unsigned long maxSystemBytes;
        unsigned long systemBytes;
        unsigned long inuseBytes;
        unsigned long keepCost;
    };
    MemoryStats memoryStatistics() const;
    void dumpStats() const;

private:
    Q_DISABLE_COPY(QMallocPool)
    QMallocPoolPrivate * d;
};

#endif
