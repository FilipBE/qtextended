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

#ifndef QGLOBALPIXMAPCACHE_H
#define QGLOBALPIXMAPCACHE_H

#include <QStringList>
#include <QPixmap>

#include <qtopiaglobal.h>

#include "custom.h"

// The documentation references these lines (see doc/src/syscust/custom.qdoc)
#ifndef QGLOBAL_PIXMAP_CACHE_LIMIT
#define QGLOBAL_PIXMAP_CACHE_LIMIT 1048576     // 1 Mb
#endif

class QTOPIABASE_EXPORT QGlobalPixmapCache
{
public:
    static bool find( const QString &key, QPixmap &pixmap );
    static bool insert( const QString &key, const QPixmap &pixmap );
    static void remove( const QString &key );
};


#endif
