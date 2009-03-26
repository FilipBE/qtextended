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

#include "qfavoriteservicesmodel.h"
#include <qtopiaapplication.h>
#include <QtopiaSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QVariant>
#include <QModelIndex>
#include <QtopiaServiceRequest>
#include <QtopiaServiceDescription>
#include <qtranslatablesettings.h>
#include <QString>
#include <QDebug>
#include <QListView>
#include <QVBoxLayout>

class QFavoriteServicesModelPrivate
{
public:
    QFavoriteServicesModelPrivate() :
        changed(true), countChanged(true), descChanged(true) {}
    mutable bool changed;
    mutable bool countChanged;
    mutable bool descChanged;
    mutable int rowCountCache;
    mutable QList<QString> displayCache;
    mutable QList<QString> iconCache;
    mutable QList<QString> inputCache;
    mutable QList<QtopiaServiceDescription> descriptionCache;
};

/*!
    \class QFavoriteServicesModel
    \inpublicgroup QtBaseModule

    \brief The QFavoriteServicesModel class provides a data model of the Favorite Services list.

    This model provides functions for accessing and manipulating the favorite services list. If
    you merely need to add or remove a service, use the FavoritesService service.

    The Qt Extended favorite services list is a list of service requests which have been selected by the
    user. The list can be reordered by the user, and items can have an optional Speed dial number
    associated with them. List items are unique, each distinct service description will only appear once.

    The favorites list designed to contain services which the user would wish to quickly access. The
    favorites list will be easily accessible and easily manipulated by the user. This is similar to
    the concept of bookmarks in a web browser.

    Note that any optional properties in the service description are not considered when determining
    whether two descriptions are distinct.
    \sa FavoritesService
    \sa QSpeedDial
    \since 4.4

    \ingroup model-view
*/

/*!
    \enum QFavoriteServicesModel::QFavoriteServicesModelRole

    Extends Qt::ItemDataRole

    \value SpeedDialInputRole
    A string containing the speed dial input associated with this favorite, if any.

*/
/*!
    Constructs a QFavoriteServicesModel with the given \a parent.
 */
QFavoriteServicesModel::QFavoriteServicesModel(QObject* parent) : QAbstractListModel(parent)
{
    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor("QPE/FavoriteServices", this);
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(change()),
                              this, SLOT(change()));
    d = new QFavoriteServicesModelPrivate();
}

/*!
    Destroys the model.
 */
QFavoriteServicesModel::~QFavoriteServicesModel()
{
    delete d;
}

/*!
    \internal
    called when changes to the data occur, and sets flags so the
    caches are reloaded
 */
void QFavoriteServicesModel::change()
{
    d->changed = true;
    d->countChanged = true;
    d->descChanged = true;
    reset();
}

/*!
    Returns the index of the entry with the QtopiaServiceDescription \a desc. Otherwise
    returns an invalid QModelIndex.
*/

QModelIndex QFavoriteServicesModel::indexOf(const QtopiaServiceDescription &desc) const
{
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.prepare("SELECT sortIndex FROM favoriteservices WHERE label=:label AND icon=:icon AND service=:service AND message=:message AND arguments=:arguments")){
        qWarning() << "Prepare Find index favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return QModelIndex();
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
        return QModelIndex();
    }

    if(q.next())
        return index(q.value(0).toInt());

    return QModelIndex();
}

/*!
    Returns the number of rows in the model, which is independent of the \a parent.
 */
int QFavoriteServicesModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    if(d->countChanged){
        QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
        QSqlQuery q(db);
        if(!q.exec("SELECT COUNT(id) from favoriteservices")){
            qWarning() << "Count favoriteservices failed:" << q.lastError().text();
            return 0;
        }
        q.first();
        d->rowCountCache=q.value(0).toInt();
        d->countChanged = false;
    }
    return d->rowCountCache;
}

/*!
    Returns the data stored under the given \a role for the item referred to by the \a index.
    The display role contains the service description's label, and the decoration role contains
    the service desription's icon (as a QIcon). The other roles used are defined in QFavoriteServicesModelRole.
 */
QVariant QFavoriteServicesModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(d->changed){
        QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
        QSqlQuery q(db);
        if(!q.exec(QLatin1String("SELECT label,icon,speedDial FROM ") +
            QLatin1String("favoriteservices ORDER BY sortIndex asc"))){
            qWarning() << "Select from favoriteservices failed:" << q.lastError().text();
            return QVariant(tr("An Error Has Occured"));
        }
        d->iconCache.clear();
        d->displayCache.clear();
        d->inputCache.clear();
        while (q.next()) {
            QString input = q.value(2).toString();
            QString label = q.value(0).toString();
            QString icon = q.value(1).toString();

            d->displayCache << label;
            d->iconCache << icon;
            d->inputCache << input;
        }
        d->changed = false;
    }
    if(role==Qt::DisplayRole){
        return QVariant(d->displayCache[index.row()]);
    }else if(role==Qt::DecorationRole){
        return QVariant(QIcon(QLatin1String(":icon/") + d->iconCache[index.row()]));
    }else if(role==QFavoriteServicesModel::SpeedDialInputRole){
        return QVariant(d->inputCache[index.row()]);
    }
    return QVariant();
}

/*!
    Returns the QtopiaServiceDescription of the item at \a index.
*/
QtopiaServiceDescription QFavoriteServicesModel::description(const QModelIndex &index) const
{
    if(!index.isValid())
        return QtopiaServiceDescription();

    if(d->changed) //Need to update other caches, which is done in data()
        data(index);
    /*Note that in order to have the right request for the label and icon any change in
      sorted order MUST set descChanged and changed*/
    if(d->descChanged){
        QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
        QSqlQuery q(db);
        if(!q.exec(QLatin1String("SELECT service,message,arguments,optionalMap FROM ") +
            QLatin1String("favoriteservices ORDER BY sortIndex asc"))){
            qWarning() << "Select from favoriteservices failed:" << q.lastError().text();
            return QtopiaServiceDescription();
        }
        d->descriptionCache.clear();
        int c = 0;
        while(q.next()){
            QString service = q.value(0).toString();
            QString message = q.value(1).toString();
            QByteArray args = q.value(2).toByteArray();
            QtopiaServiceRequest req(service, message);
            QtopiaServiceRequest::deserializeArguments(req, args);
            QByteArray map = q.value(3).toByteArray();
            QVariantMap optionalMap;
            if(!map.isEmpty()){
                QDataStream in(map);
                in >> optionalMap;
            }
            QtopiaServiceDescription desc(req, d->displayCache[c],
                    d->iconCache[c],optionalMap);
            c++;
            d->descriptionCache << desc;
        }
        d->descChanged = false;
    }
    return d->descriptionCache[index.row()];

}

/*!
  Returns the speed dial input for the item at \a index if it has one. Otherwise
  returns a null string.
*/
QString QFavoriteServicesModel::speedDialInput(const QModelIndex &index) const
{
    return data(index, QFavoriteServicesModel::SpeedDialInputRole).toString();
}

/*!
  Removes the action currently at the given \a index. Returns true if successful,
  false otherwise.

  \sa FavoritesService::remove()
  */
bool QFavoriteServicesModel::remove(const QModelIndex &index)
{
    if(!index.isValid())
        return false;

    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery query(db);

    if(!query.prepare("DELETE FROM favoriteservices WHERE sortIndex = :position")){
        qWarning() << "Prepare delete favoriteservices failed:" << query.lastError().text();
        return false;
    }
    query.bindValue("position",index.row());
    if(!query.exec()){
        qWarning() << "Exec delete favoriteservices failed:" << query.lastError().text();
        qLog(Sql) << query.executedQuery();
        return false;
    }

    if(!query.prepare("UPDATE favoriteservices SET sortIndex = sortIndex - 1 WHERE sortIndex > :position")){
        qWarning() << "Prepare delete favoriteservices failed:" << query.lastError().text();
        return false;
    }
    query.bindValue("position",index.row());
    if(!query.exec()){
        qWarning() << "Exec delete favoriteservices failed:" << query.lastError().text();
        qLog(Sql) << query.executedQuery();
        return false;
    }
    change();
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
    return true;
}

/*!
  Adds the given QtopiaServiceDescription, \a desc, to the Favorite Services list. It
  will be placed above the item at \a index. If \a index is invalid it will place it at the
  end of the list, behaving the same as FavoritesService::add. If the service description
  \a desc already exists in the list, it will be moved from its current position to the position
  above the item at \a index. Returns true if it successfully inserts or moves the specified
  service description, false otherwise.

  \sa FavoritesService::add()
 */
bool QFavoriteServicesModel::insert(const QModelIndex & index, const QtopiaServiceDescription& desc)
{
    if(desc.isNull())
        return false;
    //Only one of each description can exist in the list
    QModelIndex exists = indexOf(desc);
    if(exists.isValid()){
        return move(exists, index);
    }
    //First move things down so the row is clear (unless added on the end)
    //Then it's inserted in that row
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    int positionTo;
    int positionFrom;
    if(index.isValid()){
        positionTo = index.row();
        positionFrom = rowCount();

        if(!q.prepare(QLatin1String("UPDATE favoriteservices SET sortIndex = sortIndex+1 ") +
            QLatin1String ("WHERE sortIndex <= :positionHigh AND sortIndex >= :positionLow"))){
            qWarning() << "Prepare move favoriteservices failed:" << q.lastError().text();
            return false;
        }
        q.bindValue("positionHigh",positionFrom);
        q.bindValue("positionLow",positionTo);
        if(!q.exec()){
            qWarning() << "Exec move favoriteservices failed:" << q.lastError().text();
            qLog(Sql) << q.executedQuery();
            return false;
        }
    }else{
        positionTo = rowCount();
    }
    if (!q.prepare(QLatin1String("INSERT INTO favoriteservices ") +
        QLatin1String("(sortIndex, speedDial, label, icon, service, message, arguments, optionalMap) ") +
        QLatin1String("VALUES (:index, NULL, :label, :icon, :service, :message, :arguments, :optionalMap)"))) {
        qWarning() << "Prepare insert favoriteservices failed:" << q.lastError().text();
        return false;
    }
    q.bindValue(QLatin1String("index"),positionTo);
    q.bindValue(QLatin1String("label"), desc.label());
    q.bindValue(QLatin1String("icon"), desc.iconName());
    q.bindValue(QLatin1String("service"), desc.request().service());
    q.bindValue(QLatin1String("message"), desc.request().message());
    QByteArray args = QtopiaServiceRequest::serializeArguments(desc.request());
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
        return false;
    }
    change();//Need immediate reload within this model
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
    return true;
}

/*!
  Changes the order of the Favorite Service entries by moving the entry at index \a from to index \a to.
  This will also change the row of entries between these rows, but not their
  relative position. Returns true if the move is successful, false otherwise.
 */
bool QFavoriteServicesModel::move(const QModelIndex & from, const QModelIndex & to)
{
    if(!from.isValid() || !to.isValid() || from==to)
        return false;

    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);

    int positionTo = to.row();
    int positionFrom = from.row();

    int nextIndex = rowCount();
    if(positionTo>nextIndex)
        positionTo=nextIndex;

    if(!q.prepare(QLatin1String("SELECT id FROM favoriteservices WHERE sortIndex = :positionFrom"))){
        qWarning() << "Prepare move favoriteservices failed:" << q.lastError().text();
        return false;
    }
    q.bindValue("positionFrom",positionFrom);
    if(!q.exec()){
        qWarning() << "Exec move favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return false;
    }
    if(!q.first())
        return false;
    int idFrom = q.value(0).toInt();
    if(!q.prepare(QLatin1String("UPDATE favoriteservices SET sortIndex = sortIndex+:positionInc WHERE sortIndex <= :positionHigh AND sortIndex >= :positionLow"))){
        qWarning() << "Prepare move favoriteservices failed:" << q.lastError().text();
        return false;
    }
    q.bindValue("positionInc",qMin(positionFrom,positionTo)==positionFrom?-1:1);
    q.bindValue("positionHigh",qMax(positionFrom,positionTo));
    q.bindValue("positionLow",qMin(positionFrom,positionTo));
    if(!q.exec()){
        qWarning() << "Exec move favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return false;
    }
    if(!q.prepare(QLatin1String("UPDATE favoriteservices SET sortIndex = :positionTo WHERE id = :idFrom"))){
        qWarning() << "Prepare move favoriteservices failed:" << q.lastError().text();
        return false;
    }
    q.bindValue("positionTo",positionTo);
    q.bindValue("idFrom",idFrom);
    if(!q.exec()){
        qWarning() << "Exec move favoriteservices failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return false;
    }
    change();//Need immediate reload within this model
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
    return true;
}

