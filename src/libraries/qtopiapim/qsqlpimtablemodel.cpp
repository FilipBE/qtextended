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

#include "qsqlpimtablemodel_p.h"
#include <qtopialog.h>
#include <QSqlResult>
#include <QSqlError>
#include <QVariant>
#include <QTimer>
#include <QDebug>
#include <QVector>
#include <QTimerEvent>
#include <QtopiaSql>
#include <QMap>

#include "qpimsqlio_p.h"

#ifdef GREENPHONE_EFFECTS
static const int rowStep(10); // where to grab key frames
static const int cacheSize(1000); // how many indexes to grab.
static const int keyCacheSize(1000); // how many key frames to grab
// interval by lookahead gives approximate 'busy' time after starting list.
static const int cacheTimerInterval(5);
static const int cacheTimerLookAhead(900);
#else
// non-greenphone effects numbers. Keep processing time and memory usage down.
static const int rowStep(20); // where to grab key frames
static const int cacheSize(200); // how many indexes to grab.
static const int keyCacheSize(500); // how many key frames to grab
static const int cacheTimerInterval(5);
static const int cacheTimerLookAhead(100);
#endif

QSqlPimTableModel::QSqlPimTableModel(const QString &table, const QString &categoryTable)
    : QObject(), cCatFilter(QCategoryFilter::All), mExcludeContexts(true), cachedCount(-1),
    cachedIndexes(cacheSize), cachedKeyValues(cacheSize), cachedKeys(keyCacheSize), mSimpleCache(0),
    tableText(table),
    catUnfiledText(" LEFT JOIN " + categoryTable + " AS cat ON t1.recid = cat.recid"),
    catSelectedText(categoryTable),idByRowValid(false), cacheTimerTarget(0),
    lastCachedRow(-1)
{
    mOrderBy << "recid";
}

QSqlPimTableModel::~QSqlPimTableModel()
{
}

QList<QVariant> QSqlPimTableModel::orderKey(const QUniqueId &id) const
{
    QPreparedSqlQuery keyQuery(QPimSqlIO::database());
    keyQuery.prepare("SELECT " + sortColumn()
                + ", t1.recid FROM "
                + tableText + " AS t1 WHERE t1.recid = :id");
    keyQuery.bindValue(":id", id.toUInt());

    QList<QVariant> list;
    if (keyQuery.exec()) {
        if (keyQuery.next()) {
            for (int i = 0; i < mOrderBy.count() + 1; i++)
                list << keyQuery.value(i);
        }
    } else {
        qWarning("QSqlPimTableModel::orderKey() - Could not execute query: %s",
                keyQuery.lastError().text().toLocal8Bit().constData());
    }

    return list;
}

QString QSqlPimTableModel::selectText(const QStringList &temporaryFilters) const
{
    return selectText("distinct t1.recid", temporaryFilters);
}

QString QSqlPimTableModel::selectText(const QString &retrieve, const QStringList &temporaryFilters, const QStringList & temporaryJoins) const
{
    /*
       select
            ( id | count(id) | * )
       from records as t1
            join records as t2 on (record = :id)
            where t1.key > t2.key etc.
            [, records as t2]

            -- multiple categories --
            inner join contactcategories as cat1 on (cat1.recid = t1.recid and cat1.categoryid='Personal')
            inner join contactcategories as cat2 on (cat2.recid = t1.recid and cat2.categoryid='Business')

            -- unfiled --
            | left join recordcategories as cat1 on t1.id = cat1.recid )]
            where cat.recid is null

       where

            [t1.sortcolumn <= t2.sortcolumn and t2.id = id and t1.id < id]
            [recid is null]
            [order by sortcolumn, id]
     */

    QString qtext = "SELECT " + retrieve + " from " + tableText + " as t1";

    QStringList j = joins();
    foreach(QString join, j) {
        // contactpresence sometimes needs a left join
        if (join.startsWith("LEFT ")) {
            join = join.mid(5);
            qtext += " LEFT";
        }

        // Special case for simcardidmap (different recid column name)
        if (join == "simcardidmap")
            qtext += " JOIN simcardidmap ON (t1.recid = simcardidmap.sqlid) ";
        else
            qtext += " JOIN " + join + " ON (t1.recid = " + join + ".recid) ";
    }
    foreach (QString join, temporaryJoins) {
        qtext += join;
    }

    /* THE JOINS */
    if (cCatFilter.acceptUnfiledOnly())
        qtext += catUnfiledText;
    else if (!cCatFilter.acceptAll()) {
        QStringList cats = cCatFilter.requiredCategories();
        QString cname("c%1");
        int cid = 1;
        foreach (QString ctj, cats) {
            qtext += " JOIN " + catSelectedText + " AS " + cname.arg(cid) + " ON (t1.recid = "
                + cname.arg(cid) + ".recid and "
                + cname.arg(cid) + ".categoryid='" + ctj + "') ";

            cid++;
        }
    }

    /* THE CONDITIONS */
    bool isFirstCondition = true;
    // sort out category filtering later
    if (cCatFilter.acceptUnfiledOnly()) {
        qtext += " WHERE cat.recid IS NULL";
        isFirstCondition = false;
    }

    /*
       Exclusion filter.  e.g. if its in the list, don't show it
       makes it easier for different contexts to exclude or include themselves,
       don't have to know what the entire set of numbers is.
     */
    if (!mContextFilter.isEmpty()) {
        if (isFirstCondition) {
            qtext += " WHERE"; // No tr
            isFirstCondition=false;
        } else {
            qtext += " AND"; // No tr
        }
        if (mExcludeContexts)
            qtext += " t1.context NOT IN (";
        else
            qtext += " t1.context IN (";
        bool first = true;
        foreach(int item, mContextFilter) {
            if (item >= 0) {
                if (!first) qtext += ", ";
                qtext += QString::number(item);
                first = false;
            }
        }
        qtext += ")";
    }

    QStringList ofs = filters() + temporaryFilters;
    foreach (QString of, ofs) {
        if (isFirstCondition) {
            qtext += " WHERE "; // No tr
            isFirstCondition=false;
        } else {
            qtext += " AND "; // No tr
        }
        qtext += of;
        qtext += " ";
    }

    return qtext;
}

/*!
  \internal
  Returns number of records shown in filtered view of data.
*/
int QSqlPimTableModel::count() const
{
    if (cachedCount != -1)
        return cachedCount;

    qLog(Sql) << " QSqlPimTable::count() - not cached";
    // may not be sqlite compatible...
    QPreparedSqlQuery countQuery;
    countQuery.prepare(selectText("count(distinct t1.recid)"));

    if (!countQuery.exec()) {
        qWarning("QSqlPimTableModel::count() - Could not execute query: %s",
                countQuery.lastError().text().toLocal8Bit().constData());
        return 0;
    }
    if (countQuery.next() && !countQuery.isNull(0)) {
        cachedCount = countQuery.value(0).toInt();
        qLog(Sql) << "QSqlPimTable::count() - result:" << cachedCount;
        return cachedCount;
    }
    qLog(Sql) << "QSqlPimTable::count() - failed";
    return 0;
}

/*!
  \internal
  Sets the category filter used to filter records to \a f.
  Records that are not accepted by the category filter \a f will not be shown.
*/
void QSqlPimTableModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (cCatFilter != f) {
        cCatFilter = f;
        invalidateQueries();
    }
}

/*!
  \internal
  Returns the category filter used to filter records.
*/
QCategoryFilter QSqlPimTableModel::categoryFilter() const
{
    return cCatFilter;
}

/*!
  \internal
  Sets the context filter used to filter records to \a f.
  Records in the context list \a f will not be shown in the list of records.
*/
void QSqlPimTableModel::setContextFilter(const QSet<int> &f, bool b)
{
    if (mContextFilter != f || b != mExcludeContexts) {
        mContextFilter = f;
        mExcludeContexts = b;
        invalidateQueries();
    }
}

/*!
  \internal
  Returns the context filter used to filter records.
*/
QSet<int> QSqlPimTableModel::contextFilter() const
{
    return mContextFilter;
}

/*!
  \internal
  Returns whether the context filter excludes or restricts to set
*/
bool QSqlPimTableModel::contextFilterExcludes() const
{
    return mExcludeContexts;
}

/*!
  \internal
  Returns the identifier for the record at \a row in the filtered records.
*/
QUniqueId QSqlPimTableModel::id(int row) const
{
    // works now, not later.
    if (!cachedIndexes.contains(row)) {
        buildCache(row);
    }

    // should detect if forwards or backwards, assume forwards.
    if (cacheTimerInterval != -1 && !cacheTimer.isActive()) {
        cacheRow = ((row/rowStep)+1)*rowStep; // first target.

        cacheTimerTarget = cacheRow + cacheTimerLookAhead;

        if (lastCachedRow != -1)
            cacheRow = ((lastCachedRow/rowStep)+1)*rowStep;

        if (cachedCount > 0)
            cacheTimerTarget = qMin(cachedCount-1, cacheTimerTarget);

        if (cacheRow < cacheTimerTarget && !cachedIndexes.contains(cacheTimerTarget))
            cacheTimer.start(cacheTimerInterval, (QObject *)this);
    }

    // don't assume it was a valid row.
    if (cachedIndexes.contains(row))
        return *cachedIndexes[row];
    return QUniqueId();
}

void QSqlPimTableModel::setSimpleQueryCache(QPimQueryCache *c)
{
    mSimpleCache = c;
    mSimpleCache->setMaxCost(cacheSize);
}

/*
   Ensures that \a row is in the cache, assuming its in the current filter at.
 */
void QSqlPimTableModel::buildCache(int row) const
{
// first thing, work out if we can jump some rows.
    QUniqueId keyJumpId;
    QVariant keyValue;
    int rowBlockStart = row/rowStep;
    for (;rowBlockStart > 0; rowBlockStart--) {
        if (cachedKeys.contains(rowBlockStart)) {
            // we have a jump row
            keyJumpId = *cachedKeys[rowBlockStart];
            keyValue = *cachedKeyValues[rowBlockStart];
            break;
        }
    }
    int current = rowBlockStart*rowStep;
    int rowStart = (row/rowStep)*rowStep;
    int rowEnd = (row/rowStep+1)*rowStep;

    if (!idByRowValid)
        prepareRowQueries();

    uint recid = keyJumpId.toUInt();
    if (keyValue.isNull()) {
        idByRowQuery.prepare();
        idByRowQuery.exec();
#ifdef GREENPHONE_EFFECTS
        cacheRows(idByRowQuery, current, current, rowEnd, recid);
#else
        cacheRows(idByRowQuery, current, rowStart, rowEnd, recid);
#endif

        idByRowQuery.reset();
    } else {
        idByJumpedRowQuery.prepare();
        idByJumpedRowQuery.bindValue(":key", keyValue);
        idByJumpedRowQuery.exec();

#ifdef GREENPHONE_EFFECTS
        cacheRows(idByJumpedRowQuery, current, current, rowEnd, recid);
#else
        cacheRows(idByJumpedRowQuery, current, rowStart, rowEnd, recid);
#endif

        idByJumpedRowQuery.reset();
    }
}

// if recid != -1, then current refers to that row, and we won't be able to increment it till we hit it.
void QSqlPimTableModel::cacheRows(QPreparedSqlQuery &q, int current, int cacheStart, int cacheEnd, uint recid) const
{
    // can't use next, Qt caches on that.  use seek instead.
    while(q.next()) {
        if (current > cacheEnd)
            break;

        uint rowid = q.value(0).toUInt();

        if (rowid == recid) // cache all rows including this one from here
            recid = 0;

        if (recid == 0) {
            // e.g. current is accurate
            if (!(current % rowStep) && !cachedKeys.contains(current/rowStep)) {
                cachedKeys.insert(current/rowStep, new QUniqueId(QUniqueId::fromUInt(rowid)));
                cachedKeyValues.insert(current/rowStep, new QVariant(q.value(1)));
            }

            if (current >= cacheStart) {
                cachedIndexes.insert(current, new QUniqueId(QUniqueId::fromUInt(q.value(0).toUInt())));
                if ( mSimpleCache )
                    mSimpleCache->cacheRow(current, q);
            }

            current++;
        }
    }
    lastCachedRow = current;
    if (cachedCount == -1 && !q.next())
        cachedCount = current;
}

void QSqlPimTableModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == cacheTimer.timerId())
    {
        cacheRow += rowStep;
        if (cachedCount != -1)
            cacheRow = qMin(cachedCount-1, cacheRow);
        if (!cachedIndexes.contains(cacheRow)) {
            buildCache(cacheRow);
        }
        if (cacheRow >= cacheTimerTarget ||
                cacheRow >= cachedCount-1 && cachedCount != -1) {
            cacheTimer.stop();
            cacheRow = 0;
            cacheTimerTarget = 0;
        }
    }
}

void QSqlPimTableModel::prepareRowQueries() const
{
    QString sortColumnString = sortColumn();
    QStringList keyJumpFilter;
    QStringList joinJumpFilter;

    QString k;
    if (orderBy().isEmpty())
        k = "recid";
    else {
        k = orderBy()[0];
    }

    keyJumpFilter << "t1." + k + " >= :key";

    QString targetFields, postString;
    targetFields = "DISTINCT t1.recid, t1." + k;
    if (mSimpleCache)
        targetFields += ", " + mSimpleCache->fields();

    if (!sortColumnString.isNull())
        postString = " ORDER BY " + sortColumnString + ", t1.recid";
    else
        postString = " ORDER BY t1.recid";

    idByJumpedRowQuery.prepare(
                selectText( targetFields, keyJumpFilter, joinJumpFilter )
                + postString );
    idByRowQuery.prepare(
                selectText( targetFields )
                + postString );

    idByRowValid = true;
}

QString QSqlPimTableModel::sortColumn() const
{
    return "t1." + orderBy().join(", t1.");
}

/*!
  \internal
  Returns the row for the record with identifier \a id in the filtered records.
  returns -1 if the record isn't in the filtered records.
*/
int QSqlPimTableModel::row(const QUniqueId & tid) const
{
    if (tid.isNull())
        return -1;

    QList<QVariant> keys = orderKey(tid);
    int r = predictedRow(keys)-1;
    if (r > -1 && id(r) == tid)
        return r;
    return -1;
}

bool QSqlPimTableModel::contains(const QUniqueId &id) const
{
    QStringList filter("t1.recid = :id");
    QPreparedSqlQuery q;
    q.prepare(selectText(filter));
    q.bindValue(":id", id.toUInt());
    q.exec();
    return q.next();
}

int QSqlPimTableModel::predictedRow(const QList<QVariant> &keys) const
{
    QStringList sc = orderBy();
    QString k;
    QString filter;
    int i = 0;

    // We have a dependence on the recid being the last value in keys, by using i below.
    if (keys.count() == 0)
        return -1;

    foreach(k, sc) {
        if (i >= keys.count())
            break;
        // For tasks, we often have NULL fields (these get sorted to the top in ASC order, bottom in DESC)
        // so if we have a non null, key, try to include NULL keys in the output as well as non null keys
        // that would come before this key.  If it is null, we only include NULL keys since non null keys
        // would never get sorted before this one.
        if (!keys[i].isNull())
            filter += "(t1." + k + " IS NULL or t1." + k + " < ? or t1." + k + " = ? and ";
        else
            filter += "(t1." + k + " IS NULL and ";
        i++;
    }
    filter += "(t1.recid < ? or t1.recid = ?)";
    i = 0;
    foreach(k, sc) {
        if (i >= keys.count())
            break;

        filter += ")";
        i++;
    }
    QStringList filters(filter);
    QString querytext = selectText("count(distinct t1.recid)", filters);

    QPreparedSqlQuery rowByIdQuery(QPimSqlIO::database());
    rowByIdQuery.prepare(querytext);

    int pos = 0;
    foreach(QVariant v, keys) {
        if (!v.isNull()) {
            rowByIdQuery.bindValue(pos++, v);
            rowByIdQuery.bindValue(pos++, v);
        }
    }


    rowByIdQuery.exec();
    if (!rowByIdQuery.next())
        return -1; // record doesn't exists
    return rowByIdQuery.value(0).toInt();
}

void QSqlPimTableModel::reset()
{
    invalidateQueries();
}

void QSqlPimTableModel::invalidateCache()
{
    cacheTimer.stop();
    cacheRow = 0;
    cacheTimerTarget = 0;
    lastCachedRow = -1;

    cachedCount = -1;
    cachedIndexes.clear();
    cachedKeyValues.clear();
    cachedKeys.clear();
    if (mSimpleCache)
        mSimpleCache->clear();
    idByRowValid = false;
}

void QSqlPimTableModel::invalidateQueries()
{
    invalidateCache();
}

void QSqlPimTableModel::setFilters(const QStringList &f)
{
    if (f == mFilters)
        return;
    mFilters = f;
    invalidateQueries();
}

void QSqlPimTableModel::setJoins(const QStringList &j)
{
    if (j == mJoins)
        return;
    mJoins = j;
    invalidateQueries();
}

void QSqlPimTableModel::setOrderBy(const QStringList &sortorder)
{
    if (sortorder == mOrderBy)
        return;
    mOrderBy = sortorder;
    invalidateQueries();
}
