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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qeventwatcher_p.h"

#include <QSet>
#include <QPointer>
#include <QCoreApplication>

struct QEventWatcherPrivate
{
    QSet<QObject*>              objects;
    QList<QEventWatcherFilter*> filters;
    int                         count;
};

QEventWatcher::QEventWatcher(QObject* parent)
    : QObject(parent),
      d(new QEventWatcherPrivate)
{
    d->count = 0;
}

QEventWatcher::~QEventWatcher()
{
    qDeleteAll(d->filters);
    delete d;
    d = 0;
}

void QEventWatcher::addObject(QObject* obj)
{
    d->objects << obj;
    qApp->installEventFilter(this);
}

void QEventWatcher::addFilter(QEventWatcherFilter* filter)
{ d->filters << filter; }

int QEventWatcher::count() const
{ return d->count; }

QString QEventWatcher::toString() const
{
    QString ret;
    QString sep;
    for (int i = d->filters.count()-1; i >= 0; --i) {
        ret += sep + d->filters.at(i)->toString();
        sep = ", ";
    }
    return ret;
}

bool QEventWatcher::eventFilter(QObject* obj, QEvent* e)
{
    if (!d->objects.contains(obj)) return false;

    bool accept = (d->filters.count() ? false : true);
    for (int i = d->filters.count()-1; i >= 0 && !accept; --i) {
        accept = d->filters.at(i)->accept(obj,e);
    }

    if (accept) {
        ++d->count;
        emit event(obj, e->type());
    }

    return false;
}

