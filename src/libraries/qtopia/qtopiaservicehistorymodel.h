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

#ifndef QTOPIASERVICEHISTORYMODEL_H
#define QTOPIASERVICEHISTORYMODEL_H

#include <qtopiaglobal.h>
#include <qtopiaserviceselector.h>
#include <QAbstractListModel>

class QtopiaServiceHistoryModelPrivate;

class QTOPIA_EXPORT QtopiaServiceHistoryModel : public QAbstractListModel
{
    Q_OBJECT
public:
    QtopiaServiceHistoryModel(QObject* parent = 0);
    ~QtopiaServiceHistoryModel();

    enum SortFlags { History, Frequency, Recent };
    void setSorting(SortFlags sort);
    SortFlags sorting() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QtopiaServiceRequest serviceRequest(const QModelIndex &index) const;
    QtopiaServiceDescription serviceDescription(const QModelIndex &index) const;

    static void insert(const QtopiaServiceDescription&);
    static void insert(const QtopiaServiceRequest &request, const QString &label, const QString &icon);

private:
    QtopiaServiceHistoryModelPrivate *d;
};

#endif
