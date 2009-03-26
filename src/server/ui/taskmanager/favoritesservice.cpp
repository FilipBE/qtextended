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
#include "favoritesservice.h"
#include "qfavoriteserviceslist.h"
#include <qtopiaapplication.h>
#include <QDebug>
#include <QtopiaSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtopiaServiceDescription>
#include <qtranslatablesettings.h>
#include <QStringList>
#include <QExpressionEvaluator>
#include <QSpeedDial>
#include <QListView>
#include <QSmoothList>
#include <QFavoriteServicesModel>
#include <QDialog>
#include <QVBoxLayout>
#include <QtopiaServiceRequest>
/*!
    \internal
    Before 4.4 speed dial was stored in a settings file. Now it's stored in a db.
    This function migrates the old data
 */
void importOldSpeedDial()
{
    QTranslatableSettings cfg(QLatin1String("Trolltech"),QLatin1String("SpeedDial"));
    if(cfg.contains("sqlImported"))
        return;
    cfg.setValue("sqlImported",true);
    cfg.beginGroup(QLatin1String("Dial"));

    QStringList d = cfg.value(QLatin1String("Defined")).toString().split( ',');
    for(QStringList::ConstIterator it = d.begin(); it != d.end(); ++it)
    {
        cfg.endGroup();
        cfg.beginGroup(QLatin1String("Dial") + *it);

        QString s = cfg.value(QLatin1String("Service")).toString();

        if (s.isEmpty()
            || cfg.value(QLatin1String("RemoveIfUnavailable")).toBool()
            && QtopiaService::app(s).isEmpty()
           )
            continue;

        QByteArray r = cfg.value(QLatin1String("Requires")).toByteArray();
        QExpressionEvaluator expr(r);
        if ( !r.isEmpty() && !(expr.isValid() && expr.evaluate() && expr.result().toBool()) )
            continue;

        QString m = cfg.value(QLatin1String("Message")).toString();
        QByteArray a = cfg.value(QLatin1String("Args")).toByteArray();
        QString l = cfg.value(QLatin1String("Label")).toString();
        QString ic = cfg.value(QLatin1String("Icon")).toString();
        QMap<QString, QVariant> p = cfg.value(QLatin1String("OptionalProperties")).toMap();

        QtopiaServiceRequest req(s, m.toLatin1());

        if(!a.isEmpty())
            QtopiaServiceRequest::deserializeArguments(req, a);

        QtopiaServiceDescription* t = new QtopiaServiceDescription(req, l, ic, p);
        QSpeedDial::set((*it),*t);
    }
}

/*!
    \service FavoritesService Favorites
    \inpublicgroup QtBaseModule
    \brief The FavoritesService class provides the Qt Extended Favorites service.

    The Favorites service can be used by applications to add and remove entries in the favorite services list.

    For a more detailed explanation of the favorite services list see QFavoriteServicesModel

    \sa QFavoriteServicesModel
*/

/*!
    \internal
*/
FavoritesService::FavoritesService( QObject *parent )
    : QtopiaAbstractService( "Favorites", parent )
{
    importOldSpeedDial();
    qRegisterMetaType<QtopiaServiceDescription>();
    publishAll();
}

/*!
    Adds the given service description, \a desc, to the favorites list, if it is not
    already in the list.

    \sa QFavoriteServicesModel::insert()
*/
void FavoritesService::add(QtopiaServiceDescription desc)
{
    if(desc.isNull())
        return;
    //First check if it's there already
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.prepare(QLatin1String("SELECT sortIndex FROM favoriteservices WHERE label=:label ") +
        QLatin1String("AND icon=:icon AND service=:service AND message=:message AND arguments=:arguments"))){
        qWarning() << "Prepare Find index favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return ;
    }

    QtopiaServiceRequest req = desc.request();
    QByteArray args = QtopiaServiceRequest::serializeArguments(req);
    q.bindValue(QLatin1String("label"),desc.label());
    q.bindValue(QLatin1String("icon"),desc.iconName());
    q.bindValue(QLatin1String("service"),req.service());
    q.bindValue(QLatin1String("message"),req.message());
    q.bindValue(QLatin1String("arguments"), args);

    if(!q.exec()){
        qWarning() << "Exec Find index favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return;
    }
    if(q.next())
        return;
    //Insert into list
    if (!q.prepare(QLatin1String("INSERT INTO favoriteservices ") +
            QLatin1String("(sortIndex, speedDial, label, icon, service, message, arguments, optionalMap) ") +
            QLatin1String("VALUES (:index, NULL, :label, :icon, :service, :message, :arguments, :optionalMap)"))) {
        qWarning() << "Prepare insert favoriteservices failed:" << q.lastError().text();
        return;
    }
    QSqlQuery q2(db);
    q2.exec("SELECT COUNT(id) FROM favoriteservices");
    q2.first();
    int nextIndex = q2.value(0).toInt();
    q.bindValue(QLatin1String("index"),nextIndex);
    q.bindValue(QLatin1String("label"), desc.label());
    q.bindValue(QLatin1String("icon"), desc.iconName());
    //req and args reused from last query
    q.bindValue(QLatin1String("service"), req.service());
    q.bindValue(QLatin1String("message"), req.message());
    q.bindValue(QLatin1String("arguments"), args);
    if(desc.optionalProperties().isEmpty()){
        q.bindValue(QLatin1String("optionalMap"),QVariant());
    }else{
        QByteArray map;
        QDataStream ds(&map, QIODevice::WriteOnly);
        ds << desc.optionalProperties();
        q.bindValue(QLatin1String("optionalMap"),map);
    }

    if (!q.exec()) {
        qWarning() << "Insert favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return;
    }
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
}

/*!
    Removes the given service description, \a desc, from the favorites list,
    if it is already in the list.

    \sa QFavoriteServicesModel::remove()
*/
void FavoritesService::remove(QtopiaServiceDescription desc)
{
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery query(db);
    if(!query.prepare(QLatin1String("SELECT sortIndex FROM favoriteservices WHERE label = :label AND icon = :icon ") +
        QLatin1String("AND service = :service AND message = :message AND arguments = :arguments"))){
        qWarning() << "Prepare delete favoriteservices failed:" << query.lastError().text();
        return;
    }
    query.bindValue(QLatin1String("label"), desc.label());
    query.bindValue(QLatin1String("icon"), desc.iconName());
    query.bindValue(QLatin1String("service"), desc.request().service());
    query.bindValue(QLatin1String("message"), desc.request().message());
    QByteArray args = QtopiaServiceRequest::serializeArguments(desc.request());
    query.bindValue(QLatin1String("arguments"), args);
    if(!query.exec()){
        qWarning() << "Exec delete favoriteservices failed:" << query.lastError().text();
        qLog(Sql) << query.executedQuery();
    }
    if(!query.first())
        return; //Description is Not in favorites
    int position = query.value(0).toInt();
    if(!query.prepare("DELETE FROM favoriteservices WHERE sortIndex = :position")){
        qWarning() << "Prepare delete favoriteservices failed:" << query.lastError().text();
        return;
    }
    query.bindValue("position",position);
    if(!query.exec()){
        qWarning() << "Exec delete favoriteservices failed:" << query.lastError().text();
        qLog(Sql) << query.executedQuery();
    }

    //Need to shift the indexes of those below it down by one
    if(!query.prepare("UPDATE favoriteservices SET sortIndex = sortIndex - 1 WHERE sortIndex > :position")){
        qWarning() << "Prepare delete favoriteservices failed:" << query.lastError().text();
        return;
    }
    query.bindValue("position",position);
    if(!query.exec()){
        qWarning() << "Exec delete favoriteservices failed:" << query.lastError().text();
        qLog(Sql) << query.executedQuery();
    }
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
}

/*!
    Adds the provided description, \a desc,  and opens a dialog allowing the user to manipulate
    the new description in the favorites list.
*/
void FavoritesService::addAndEdit(QtopiaServiceDescription desc)
{
    add(desc);
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QFavoriteServicesList *favoritesList = new QFavoriteServicesList(widget);
    widget->show();
    widget->setWindowTitle("Favorites");
    widget->setAttribute(Qt::WA_DeleteOnClose,true);
    widget->setWindowState(favoritesList->windowState() | Qt::WindowMaximized);
    layout->addWidget(favoritesList);
    favoritesList->setCurrentRow(favoritesList->rowCount()-1);
    widget->raise();
}
/*!
    Provides a dialog form which the user can select and launch a service from the favortites list
*/
void FavoritesService::select()
{
    QDialog dialog;
    dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
    dialog.setWindowModality(Qt::WindowModal);
    QtopiaApplication::setMenuLike(&dialog, true);
    dialog.raise();

    QFavoriteServicesModel *model = new QFavoriteServicesModel(&dialog);
    QSmoothList *list = new QSmoothList(&dialog);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(list);
    list->setModel(model);

    connect(list, SIGNAL(activated(QModelIndex)),
            &dialog, SLOT(accept()));

    if(dialog.exec()){
        QtopiaServiceDescription desc = model->description(list->currentIndex());
        if(!desc.isNull())
            desc.request().send();
    }
}
