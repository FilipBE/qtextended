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

#ifndef QPIMSOURCEMODEL_H
#define QPIMSOURCEMODEL_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <QAbstractListModel>

#include <qpimsource.h>

class QPimSource;
class QPimContext;
class QPimSourceModelData;
class QTOPIAPIM_EXPORT QPimSourceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QPimSourceModel(QWidget *parent = 0);
    ~QPimSourceModel();

    // sets the internal data
    void setContexts(const QList<QPimContext *> &);
    // for convenience
    void setContexts(const QList<QContactContext *> &);
    void setContexts(const QList<QAppointmentContext *> &);
    void setContexts(const QList<QTaskContext *> &);

    void setCheckedSources(const QSet<QPimSource> &);
    QSet<QPimSource> checkedSources() const;

    QPimSource source(const QModelIndex &) const;
    QPimContext *context(const QModelIndex &) const;
    using QAbstractListModel::index;
    QModelIndex index(const QPimSource &) const;

    // for normal model work.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &, const QVariant &, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &) const;

    void addSource(const QPimSource &);
    void removeSource(const QPimSource &);
    void updateSource(const QModelIndex &index, const QPimSource&);
private:
    QPimSourceModelData *d;
};

#endif
