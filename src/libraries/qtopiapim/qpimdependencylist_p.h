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

#ifndef QPIMDEPENDENCYLIST_P_H
#define QPIMDEPENDENCYLIST_P_H

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

#include <qtopiasql.h>
#include <qsqlquery.h>
#include <qvariant.h>
#include <qmap.h>
#include <qlist.h>
#include <qcache.h>

#include "quniqueid.h"

class QPimDependencyList {
    public:
        // ro
        static QList<QUniqueId> typedChildrenRecords(const QUniqueId& recid, const QString& type);
        static QMap<QString, QUniqueId>  childrenRecords(const QUniqueId &recid);
        static QUniqueId parentRecord(const QUniqueId& recid);
        static QString parentDependencyType(const QUniqueId& recid);

        // rw
        static bool addDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype);
        static bool updateDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype);
        static bool removeDependency(const QUniqueId& srcid, const QUniqueId& destid, const QString& deptype);
};

#endif
