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

#ifndef QTASKSQLIO_P_H
#define QTASKSQLIO_P_H

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

#include <qtask.h>
#include <qtopiasql.h>
#include <qsqlquery.h>
#include <QHash>
#include "qtaskmodel.h"
#include "qpimsource.h"
#include "qpimsqlio_p.h"

class QTaskSqlIO;
class QTaskDefaultContextData;
class QTaskDefaultContext : public QTaskContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QTaskDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;

    using QTaskContext::title;
    QString title() const;

    // better to be flags ?
    using QTaskContext::editable;
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QTaskContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateTask(const QTask &);
    bool removeTask(const QUniqueId &);
    QUniqueId addTask(const QTask &, const QPimSource &);

    void processRecurringTasks();

private:
    QTaskDefaultContextData *d;
};

class QTaskSqlIO : public QPimSqlIO
{
    Q_OBJECT
public:
    explicit QTaskSqlIO(QObject *parent = 0, const QString &name = QString());
    ~QTaskSqlIO();

    QUuid contextId() const;
    int count() const { return QPimSqlIO::count(); }

    void setSortKey(QTaskModel::Field k);
    QTaskModel::Field sortKey() const;

    bool completedFilter() const;
    void setCompletedFilter(bool);

    QTask task(const QUniqueId &) const;
    QTask task(int row) const;

    QVariant taskField(int row, QTaskModel::Field k) const;
    bool setTaskField(int row, QTaskModel::Field k,  const QVariant &);

    bool removeTask(int row);
    bool removeTask(const QUniqueId & id);
    bool removeTask(const QTask &);
    bool removeTasks(const QList<int> &rows);
    bool removeTasks(const QList<QUniqueId> &ids);

    bool updateTask(const QTask &t);
    QUniqueId addTask(const QTask &t, const QPimSource &s)
    { return addTask(t, s, true); }
    QUniqueId addTask(const QTask &, const QPimSource &, bool);

protected:
    void bindFields(const QPimRecord &, QPreparedSqlQuery &) const;
    void invalidateCache();
    QStringList sortColumns() const;
    QStringList otherFilters() const;

private:
    bool cCompFilter;
    QTaskModel::Field cSort;

    mutable bool taskByRowValid;
    mutable QTask lastTask;
    mutable QPreparedSqlQuery repeatFieldQuery;
};

#endif
