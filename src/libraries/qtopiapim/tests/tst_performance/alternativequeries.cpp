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

#include "alternativequeries.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QPair>
#include <QDebug>

#include <stdio.h>

#define QBENCHMARK


AlternativeQueries::AlternativeQueries(const QString &dbName, FilterCombinations *c) : data(c)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "alternative1");
    db.setDatabaseName(dbName);
    qDebug() << "using database" << dbName;
    m_dbFound = db.open();
    if (!m_dbFound) {
        qWarning() << "Could not open database" << dbName << db.lastError().text();
    }
}

AlternativeQueries::~AlternativeQueries() {}

// not entirely honest, prepare time shouldn't count.
int AlternativeQueries::count()
{
    QSqlQuery q(QSqlDatabase::database("alternative1"));
    if (!q.prepare("SELECT count(*) FROM tmp_contacts"))
        qWarning() << "failed to prepare statement" << q.lastError().text();

    if (q.exec() && q.next())
        return q.value(0).toInt();
    else
        qWarning() << "failed to get count";
    return 0;
}

    /* full filter list

        category (not done).
             - INNER JOIN ON contactIntFields AS categoryFilter USING (recordId) WHERE categoryFilter.fieldId = :categoryFieldId AND categoryFilter.value = :categoryId

        context // multiple
             - INNER JOIN ON contactIntFields AS contextFilter USING (recordId) WHERE contextFilter.fieldId = :contextFieldId AND contextFilter.value IN (list);

         can have either action tag _or_ action _or_ field filter, not any combination
        actionTag // always one
            - INNER JOIN contactFieldActions USING(fieldId) INNER JOIN contactActionTags USING(actionId) WHERE actionTag = :actionTag;

         action // always one
            - INNER JOIN contactFieldActions USING(fieldId) WHERE actionId = :actionId;
        fieldTag // always one
            - INNER JOIN contactTextFields AS fieldFilter USING (recordId) WHERE fieldFilter.fieldId = :fieldId;

        label filter
            - WHERE label like :filter

        value filter
            - IN SUB SELECT, 'where value like', and in outer select, "where primary value is not null'


            So we have category and context being both 'recordId" only joins so go before label table.
            And only a choice between action, field, and field tag as the multiple joins.


            */

struct JoinCondition {
    JoinCondition() {}
    JoinCondition(const QString &t, const QString &f, const QString &c = QString())
        : table(t), field(f), conditions(c) {}
    QString name;
    QString table;
    QString field;
    QString conditions;
};

void AlternativeQueries::applyCurrentFilter()
{
    QSqlQuery q(QSqlDatabase::database("alternative1"));
    if (!q.prepare("DROP TABLE tmp_contacts"))
        qWarning() << "failed to prepare statement" << q.lastError().text();
    if (!q.exec())
        qWarning() << "failed to drop temporary table" << q.lastError().text();

    // select statement for temporary table.
    QList<JoinCondition> filters;
    QMap<QString, QVariant> binds;

    QString activeValueSubSelect;
    switch(data->primaryFieldType()) {
    case FilterCombinations::NoField:
        activeValueSubSelect = "SELECT fieldValue FROM contactTextFields INNER JOIN contactFieldActions USING(fieldId) INNER JOIN contactActionTags USING(actionId) WHERE actionTag = :actionTag AND recordId = labelField.recordId";
        binds.insert(":actionTag", "talk");
        break;
    case FilterCombinations::ActionTag:
        activeValueSubSelect = "SELECT fieldValue FROM contactTextFields INNER JOIN contactFieldActions USING(fieldId) INNER JOIN contactActionTags USING(actionId) WHERE actionTag = :actionTag AND recordId = labelField.recordId";
        binds.insert(":actionTag", data->primaryFieldText());
        break;
    case FilterCombinations::ActionId:
        activeValueSubSelect = "SELECT fieldValue FROM contactTextFields INNER JOIN contactFieldActions USING(fieldId) INNER JOIN contactActions USING(actionId) WHERE actionName = :actionName AND recordId = labelField.recordId";
        binds.insert(":actionName", data->primaryFieldText());
        break;
    case FilterCombinations::FieldId:
        activeValueSubSelect = "SELECT fieldValue FROM contactTextFields INNER JOIN contactFields USING(fieldId) WHERE fieldName = :fieldName AND recordId = labelField.recordId";
        binds.insert(":fieldName", data->primaryFieldText());
        break;
    }

    if (!data->valueFilter().isEmpty()) {
        activeValueSubSelect += " AND fieldValue like '%" + data->valueFilter() + "%' ORDER BY contactTextFields.fieldId, fieldValue";
    } else {
        activeValueSubSelect += " ORDER BY contactTextFields.fieldId, fieldValue";
    }

    // no category filtering yet.
    QList<int> contexts = data->contextFilter();
    if (contexts.count()) {
        JoinCondition c;
        c.name = "contextFilter";
        c.table = "contactIntFields";
        c.field = "recordId";
        c.conditions = " contextFilter.fieldId = :contextFieldId AND contextFilter.fieldValue IN (";
        QListIterator<int> it(contexts);
        while(it.hasNext()) {
            c.conditions += QString::number(it.next());
            if (it.hasNext())
                c.conditions += ", ";
        }
        c.conditions += ")";
        filters.append(c);

        binds.insert(":contextFieldId", 48);
    }

    switch(data->fieldFilterType()) {
    case FilterCombinations::NoField:
        break;
    case FilterCombinations::ActionTag:
        filters.append(JoinCondition("contactTextFields", "recordId"));
        filters.append(JoinCondition("contactFieldActions", "fieldId"));
        filters.append(JoinCondition("contactActionTags", "actionId", "actionTag = :actionTagFilter"));
        binds.insert(":actionTagFilter", data->fieldFilterText());
        break;
    case FilterCombinations::ActionId:
        filters.append(JoinCondition("contactTextFields", "recordId"));
        filters.append(JoinCondition("contactFieldActions", "fieldId"));
        filters.append(JoinCondition("contactActions", "actionId", "actionName = :actionId"));
        binds.insert(":actionId", data->fieldFilterText());
        break;
    case FilterCombinations::FieldId:
        filters.append(JoinCondition("contactTextFields", "recordId"));
        filters.append(JoinCondition("contactFields", "fieldId", "fieldName = :fieldName"));
        binds.insert(":fieldName", data->fieldFilterText());
        break;
    }

    if (!data->labelFilter().isEmpty()) {
        JoinCondition c;
        c.conditions = "label like '" + data->labelFilter() + "%'";
        filters.append(c);
    }
    if (!data->valueFilter().isEmpty()) {
        JoinCondition c;
        c.conditions = "primaryValue IS NOT NULL";
        filters.append(c);
    }


    QString selectStatement =
        "SELECT labelField.recordId AS recordId, labelField.fieldValue AS label, (" + activeValueSubSelect + ") AS primaryValue FROM contactTextFields AS labelField";

    foreach(JoinCondition join, filters) {
        if (!join.table.isEmpty()) {
            selectStatement += " INNER JOIN " + join.table;
            if (!join.name.isEmpty())
                selectStatement += " AS " + join.name;
            selectStatement += " USING(" + join.field + ")";
        }
    }

    bool firstCondition = true;
    foreach(JoinCondition condition, filters) {
        if (!condition.conditions.isEmpty()) {
            if (firstCondition) {
                selectStatement += " WHERE ";
                firstCondition = false;
            } else {
                selectStatement += " AND ";
            }
            selectStatement += condition.conditions;
        }
    }

    selectStatement += " ORDER BY labelField.fieldValue, labelField.recordId";

    if (!q.prepare("CREATE TEMPORARY TABLE tmp_contacts AS " + selectStatement))
        qWarning() << "failed to prepare temporary table query" << q.lastError().text();

    QMapIterator<QString, QVariant> bindIt(binds);
    while (bindIt.hasNext()) {
        bindIt.next();
        q.bindValue(bindIt.key(), bindIt.value());
    }
    if (!q.exec())
        qWarning() << "failed to execute temporary table query" << q.lastError().text();

    /*
    if (!q.prepare("CREATE INDEX tmp_contacts_index ON tmp_contacts (label, recordId)"))
        qWarning() << "failed to prepare temporary index query" << q.lastError().text();
    if (!q.exec())
        qWarning() << "failed to execute temporary index query" << q.lastError().text();
        */
}

void AlternativeQueries::retrieveRow(int row)
{
    QSqlQuery q(QSqlDatabase::database("alternative1"));
    if (!q.prepare("select * from tmp_contacts WHERE ROWID = :row"))
        qWarning() << "failed to prepare retrieve row query" << q.lastError().text();

    q.bindValue(":row", row);

    if (!q.exec())
        qWarning() << "failed to execute retrieve row query" << q.lastError().text();
}

void AlternativeQueries::findRowForLabel(const QString &label)
{
    QSqlQuery q(QSqlDatabase::database("alternative1"));
    if (!q.prepare("select count(*) from tmp_contacts WHERE label < :label"))
        qWarning() << "failed to prepare find row query" << q.lastError().text();
    q.bindValue(":label", label);
    if (!q.exec())
        qWarning() << "failed to execute find row query" << q.lastError().text();
}

