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

#ifndef QPIMSQLIO_P_H
#define QPIMSQLIO_P_H

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

#include <qcategorymanager.h>
#include <qpimrecord.h>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

#include <QCache>
#include <QObject>

#include "qsqlpimtablemodel_p.h"

class QFile;
class QTimer;
class QPimSource;
class QPimQueryCache;
class QValueSpaceItem;
class QValueSpaceObject;
class QTOPIA_AUTOTEST_EXPORT QPimSqlIO : public QObject
{
    Q_OBJECT
public:
    QPimSqlIO(QObject *parent, const QUuid &scope, const char *table, const char *catTable,
            const char *customTable, const char *updateText, const char *insertText, const char *valueSpaceKey);
    virtual ~QPimSqlIO();

    static int sourceContext(const QPimSource &, bool = true);
    static QPimSource contextSource(int);
    static QDateTime lastSyncTime(const QPimSource &);
    static bool setLastSyncTime(const QPimSource &, const QDateTime &);
    static QSqlDatabase database();

    int context(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    // leave to let old code work
    bool startSyncTransaction(const QPimSource &, const QDateTime &syncTime);

    bool startSyncTransaction(const QSet<QPimSource> &sources, const QDateTime &syncTime);
    bool abortSyncTransaction() { return abortTransaction(); }
    bool commitSyncTransaction() { return commitTransaction(); }

    bool startTransaction();
    bool startTransaction(const QDateTime &syncTime);
    bool abortTransaction();
    bool commitTransaction();

    QDateTime currentSyncTransactionTime() const {return mSyncTime;}

    QList<QUniqueId> removed(const QSet<QPimSource> &sources, const QDateTime &) const;
    QList<QUniqueId> added(const QSet<QPimSource> &sources, const QDateTime &) const;
    QList<QUniqueId> modified(const QSet<QPimSource> &sources, const QDateTime &) const;

    enum ContextFilterType {
        ExcludeContexts,
        RestrictToContexts
    };
    QSet<int> contextFilter() const;
    ContextFilterType contextFilterType() const;

    int count() const;

    void setCategoryFilter(const QCategoryFilter &f);
    QCategoryFilter categoryFilter() const;

    QUniqueId id(int row) const;
    QList<QUniqueId> ids(const QList<int> &rows) const;
    int row(const QUniqueId & tid) const;
    bool contains(const QUniqueId & tid) const;
    bool exists(const QUniqueId & id) const;
    void setContextFilter(const QSet<int> &, ContextFilterType = ExcludeContexts);

    /* convenience functions, change sets to invalidate cache? */
    QStringList orderBy() const { return model.orderBy(); }
    void setOrderBy(const QStringList &list) { model.setOrderBy(list); invalidateCache();}


    bool moveRecord(const QUniqueId &, const QPimSource &);

//test:
    static void setDatabase(QSqlDatabase *);

signals:
    void recordsUpdated();

protected slots:
    virtual void invalidateCache();
    virtual void propagateChanges();

protected:

    void setSimpleQueryCache(QPimQueryCache *cache) { model.setSimpleQueryCache(cache); }


    void setFilter(const QString &filter) { model.setFilter(filter); invalidateCache();}
    void setFilters(const QStringList &list) { model.setFilters(list); invalidateCache();}
    void clearFilter() { model.setFilters(QStringList()); invalidateCache(); }
    QStringList filters() const { return model.filters(); }

    void setJoins(const QStringList &list) { model.setJoins(list); invalidateCache();}
    QStringList joins() const { return model.joins(); }

    bool updateRecord(const QPimRecord& t);
    bool removeRecord(int row);
    bool removeRecord(const QUniqueId & id);
    bool removeRecords(const QList<QUniqueId> &ids);
    QUniqueId addRecord(const QPimRecord &, const QPimSource &, bool = true);
    QUniqueId addRecord(const QPimRecord &, int, bool = true);

    bool moveRecord(const QUniqueId &, int context);

    QString selectText(const QStringList &list = QStringList()) const
    { return model.selectText(list); }
    QString selectText(const QString &field, const QStringList &list = QStringList()) const
    { return model.selectText(field, list); }

protected:
    QSqlPimTableModel model;
    QDateTime mSyncTime;
    int mTransactionStack;

    virtual bool updateExtraTables(uint, const QPimRecord &);
    virtual bool insertExtraTables(uint, const QPimRecord &);
    virtual bool removeExtraTables(uint);

    virtual void bindFields(const QPimRecord &r, QPreparedSqlQuery &) const = 0;

    void retrieveRecord(uint, QPimRecord &) const;

    QUniqueIdGenerator idGenerator;

    static QMap<QPimSource, int> sourceMap;

    // yes, I do mean const.
    const QString tableText;
    const QString updateRecordText;
    const QString selectCustomText;
    const QString deleteCustomText;
    const QString insertCustomText;
    const QString selectCategoriesText;
    const QString deleteCategoriesText;
    const QString insertCategoriesText;
    // not done for extra tables, unknown number of.
    const QString deleteRecordText;
    const QString insertRecordText;

    mutable QPreparedSqlQuery retrieveCategoriesQuery;
    mutable QPreparedSqlQuery retrieveCustomQuery;
    mutable QPreparedSqlQuery changeLogInsert;
    mutable QPreparedSqlQuery changeLogUpdate;
    mutable QPreparedSqlQuery changeLogQuery;
    mutable QPreparedSqlQuery addRecordQuery;
    mutable QPreparedSqlQuery contextQuery;
    mutable QPreparedSqlQuery moveRecordQuery;

    QValueSpaceItem *vsItem;
    QValueSpaceObject *vsObject;
    int vsValue;
};

#endif
