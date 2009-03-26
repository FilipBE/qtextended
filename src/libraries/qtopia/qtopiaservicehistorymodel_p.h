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

#ifndef QTOPIASERVICEHISTORYMODEL_P_H
#define QTOPIASERVICEHISTORYMODEL_P_H

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

#include <QObject>
#include <QList>
#include <QtopiaServiceDescription>
#include "qtopiaservicehistorymodel.h"

class QtopiaIpcAdaptor;

class QtopiaServiceHistoryModelPrivate : public QObject
{
    Q_OBJECT
public:
    QtopiaServiceHistoryModelPrivate(QtopiaServiceHistoryModel *parent);

    void setSorting(QtopiaServiceHistoryModel::SortFlags s);
    QtopiaServiceHistoryModel::SortFlags sorting() { return sortFlags; }
    QtopiaServiceDescription getItem(int row) const;
    int itemCount() const;

    static bool insertItem(const QtopiaServiceDescription &desc);
public slots:
    void changed();
private:
    QtopiaServiceHistoryModel::SortFlags sortFlags;
    QtopiaServiceHistoryModel *model;
    mutable QList<QtopiaServiceDescription> cache;
    mutable int cacheStart;
    QtopiaIpcAdaptor *adaptor;
    mutable bool countChanged;
    mutable int countCache;
    mutable int countDups;//Number of duplicate service calls
};

#endif
