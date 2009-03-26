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

#ifndef METHOD_P_H
#define METHOD_P_H

#include "stamp_p.h"

#include <QHash>
#include <QSet>
#include <QByteArray>

// Encapsulates a call count and inclusive stamp of a called method.
struct CalleeInfo
{
    inline CalleeInfo()
        : callCount(0)
    {}

    int callCount;
    Stamp stamp;
};

struct Method
{
    QByteArray signature;
    QHash<Method*,CalleeInfo> callees;

    Stamp exclusiveStamp;

    inline QSet<Method*> allCallees() const
    {
        QSet<Method const*> seen;
        return allCallees(seen);
    }

    inline QSet<Method*> allCallees(QSet<Method const*>& seen) const
    {
        seen << this;
        QSet<Method*> all = callees.keys().toSet();
        foreach (Method* m, all) {
            if (!seen.contains(m))
                all |= m->allCallees(seen);
            seen << m;
        }
        return all;
    }

    QByteArray toCallgrindFormat(QString const&) const;
};

#endif

