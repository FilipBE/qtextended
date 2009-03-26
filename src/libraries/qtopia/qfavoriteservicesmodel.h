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

#ifndef QFAVORITESERVICESMODEL_H
#define QFAVORITESERVICESMODEL_H

#include <qtopiaglobal.h>
#include <QAbstractListModel>
#include <QVariant>
#include <QModelIndex>
#include <QtopiaServiceDescription>
#include <QString>
#include <QDialog>

class QFavoriteServicesModelPrivate;

class QTOPIA_EXPORT QFavoriteServicesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum QFavoriteServicesModelRole {
        SpeedDialInputRole = Qt::UserRole,
    };

    QFavoriteServicesModel(QObject* parent = 0);
    ~QFavoriteServicesModel();

    QModelIndex indexOf(const QtopiaServiceDescription &desc) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QtopiaServiceDescription description(const QModelIndex &index) const;
    QString speedDialInput(const QModelIndex &index) const;
    bool remove(const QModelIndex & index);
    bool insert(const QModelIndex & index, const QtopiaServiceDescription & desc);
    bool move(const QModelIndex & from, const QModelIndex & to);

private slots:
    void change();

private:
    QFavoriteServicesModelPrivate *d;
};


#endif
