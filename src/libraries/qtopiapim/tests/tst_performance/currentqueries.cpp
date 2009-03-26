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

#include "currentqueries.h"
#include "../../qcontactsqlio_p.h"
#include <shared/qtopiaunittest.h>
#include <QDebug>

CurrentQueries::CurrentQueries(FilterCombinations *c)
    : data(c)
{
    QString dbName = QtopiaUnitTest::baseDataPath() + "/currentschema.sqlite";

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "current");
    db.setDatabaseName(dbName);
    if (!db.open())
        qWarning() << "Could not open database" << dbName << db.lastError().text();
    QPimSqlIO::setDatabase(new QSqlDatabase(db));

    io = new ContactSqlIO;
}

CurrentQueries::~CurrentQueries()
{
    delete io;
}

int CurrentQueries::count()
{
    return io->count();
}

void CurrentQueries::applyCurrentFilter()
{
    QSet<int> contextFilterValue = QSet<int>::fromList(data->contextFilter());
    io->setContextFilter(contextFilterValue, QPimSqlIO::RestrictToContexts);
    io->setCategoryFilter(QCategoryFilter(data->categoryFilter()));

    int flags = 0;
    if (data->fieldFilterType() == FilterCombinations::ActionId) {
        QString text = data->fieldFilterText();
        if (text == "dial")
            flags = QContactModel::ContainsPhoneNumber;
        else if (text == "email")
            flags = QContactModel::ContainsEmail;
        else if (text == "chat")
            flags = QContactModel::ContainsChat;
    }
    if (data->labelFilter().isEmpty() && flags == 0)
        io->clearFilter();
    else
        io->setFilter(data->labelFilter(), flags);

}

void CurrentQueries::retrieveRow(int row)
{
    io->simpleContact(row);
}

void CurrentQueries::findRowForLabel(const QString &value)
{
    io->match(QContactModel::Label, QVariant(value), Qt::MatchCaseSensitive | Qt::MatchStartsWith);
}
