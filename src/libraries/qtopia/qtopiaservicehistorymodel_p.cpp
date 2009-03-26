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

#include "qtopiaservicehistorymodel_p.h"
#include <QtopiaSql>
#include <qtopialog.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QtopiaIpcAdaptor>

static const int cacheSize = 40;

void QtopiaServiceHistoryModelPrivate::changed()
{
    countChanged = true;
    //Below is a way of calling reset on the associated model
    QtopiaServiceHistoryModel::SortFlags prev = sortFlags;
    model->setSorting(QtopiaServiceHistoryModel::Frequency);
    model->setSorting(QtopiaServiceHistoryModel::Recent);
    model->setSorting(prev);
}

QtopiaServiceHistoryModelPrivate::QtopiaServiceHistoryModelPrivate(QtopiaServiceHistoryModel *parent)
    : sortFlags(QtopiaServiceHistoryModel::History), cacheStart(-1), countChanged(true), countCache(0), countDups(0)
{
    model=parent;
    adaptor = new QtopiaIpcAdaptor("QPE/ServiceHistory");
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(change()), this, SLOT(changed()));
}

void QtopiaServiceHistoryModelPrivate::setSorting(QtopiaServiceHistoryModel::SortFlags s)
{
    if (s != sortFlags) {
        cacheStart = -1;
        cache.clear();
        sortFlags = s;
    }
}

QtopiaServiceDescription QtopiaServiceHistoryModelPrivate::getItem(int row) const
{

    if (cacheStart >= 0 && row >= cacheStart && row < cacheStart+cache.count()){
        return cache[row-cacheStart];
    }
    cache.clear();
    cacheStart = -1;

    int numItems = cacheSize; // How many items should we fetch

    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if (sortFlags == QtopiaServiceHistoryModel::History) {
        if (!q.prepare("SELECT label,icon,service,message,arguments from servicehistory ORDER BY id desc"
                    " LIMIT :numItems OFFSET :offset"))
            qWarning() << "Prepare get service history failed:" << q.lastError().text();
    } else if (sortFlags == QtopiaServiceHistoryModel::Frequency) {
        if (!q.prepare("SELECT label,icon,service,message,arguments from servicehistory"
                    " GROUP BY service,message,arguments"
                    " ORDER BY COUNT(service) DESC , id DESC"
                    " LIMIT :numItems OFFSET :offset"))
            qWarning() << "Prepare get service history failed:" << q.lastError().text();
    } else if (sortFlags == QtopiaServiceHistoryModel::Recent) {
        if (!q.prepare("SELECT label,icon,service,message,arguments from servicehistory"
                    " GROUP BY service,message,arguments"
                    " ORDER BY id desc"
                    " LIMIT :numItems OFFSET :offset"))
            qWarning() << "Prepare get service history failed:" << q.lastError().text();
    }

    cacheStart = qMax(row-(numItems/2), 0);
    q.bindValue(":numItems", numItems);
    q.bindValue(":offset", cacheStart);

    if (!q.exec()) {
        qWarning() << "Get service history failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        cacheStart = -1;
        return QtopiaServiceDescription();
    }
    while (q.next()) {
        QString label = q.value(0).toString();
        QString icon = q.value(1).toString();
        QString service = q.value(2).toString();
        QString message = q.value(3).toString();
        QByteArray args = q.value(4).toByteArray();

        QtopiaServiceRequest req(service, message);
        QtopiaServiceRequest::deserializeArguments(req, args);
        QtopiaServiceDescription desc(req, label, icon);
        cache.append(desc);
    }
    return cache[row - cacheStart];
}

int QtopiaServiceHistoryModelPrivate::itemCount() const
{
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    if(countChanged){
        QSqlQuery q(db);
        // Simply return the count of items in our table.
        if (q.prepare("select count(*) from (select distinct service,message,arguments from servicehistory)")) {
            if (q.exec() && q.next()) {
                countCache = q.value(0).toInt();
            }
        } else {
            qWarning() << "Prepare count servicehistory failed:" << q.lastError().text();
        }
        if (q.prepare("select count(id) from servicehistory")) {
            if (q.exec() && q.next()) {
                countDups = q.value(0).toInt() - countCache;
            }
        } else {
            qWarning() << "Prepare count servicehistory failed:" << q.lastError().text();
        }
        countChanged=false;
    }
    if (sortFlags == QtopiaServiceHistoryModel::History) {
        return countCache+countDups;
    } else if (sortFlags == QtopiaServiceHistoryModel::Frequency
            || sortFlags == QtopiaServiceHistoryModel::Recent) {
        return countCache;
    }
    return 0;
}

bool QtopiaServiceHistoryModelPrivate::insertItem(const QtopiaServiceDescription &desc)
{
    QtopiaIpcAdaptor tempAdaptor("QPE/ServiceHistory");
    tempAdaptor.send(MESSAGE(change()));

    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if (!q.prepare(QLatin1String("INSERT INTO servicehistory ") +
            QLatin1String("(label, icon, service, message, arguments) ") +
            QLatin1String("VALUES (:label, :icon, :service, :message, :arguments)"))) {
        qWarning() << "Prepare insert servicehistory failed:" << q.lastError().text();
        return false;
    }

    q.bindValue(QLatin1String("label"), desc.label());
    q.bindValue(QLatin1String("icon"), desc.iconName());
    q.bindValue(QLatin1String("service"), desc.request().service());
    q.bindValue(QLatin1String("message"), desc.request().message());
    QByteArray args = QtopiaServiceRequest::serializeArguments(desc.request());
    q.bindValue(QLatin1String("arguments"), args);

    if (!q.exec()) {
        qWarning() << "Insert service history failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return false;
    }

    return true;
}
