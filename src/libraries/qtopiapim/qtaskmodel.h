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

#ifndef QTASKMODEL_H
#define QTASKMODEL_H

#include <QStyleOptionViewItem>
#include <QSharedDataPointer>
#include <QPimModel>

#include <qcategorymanager.h>
#include <qtask.h>
#include <qpimsource.h>

class QTaskModelData;
class QTOPIAPIM_EXPORT QTaskModel : public QPimModel
{
    Q_OBJECT
public:

    enum Field {
        Invalid = -1,

        Description,
        Priority,
        Completed,
        PercentCompleted,
        Status,
        DueDate,
        StartedDate,
        CompletedDate,
        Notes,

        Identifier,
        Categories,

        Alarm,

        RepeatRule,
        RepeatEndDate,
        RepeatFrequency,
        RepeatWeekFlags
    };

    explicit QTaskModel(QObject *parent = 0);
    virtual ~QTaskModel();

    static QString fieldLabel(Field key);
    static QIcon fieldIcon(Field key);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

    int columnCount(const QModelIndex & = QModelIndex()) const;

    // overridden so can change later and provide drag-n-drop (via vcard)
    // later without breaking API compatibility.
    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QVariant data(const QModelIndex &, int) const;
    bool setData(const QModelIndex &, const QVariant &, int);
    QMap<int, QVariant> itemData ( const QModelIndex & index ) const;
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &);
    QVariant headerData(int, Qt::Orientation orientation, int = Qt::DisplayRole ) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QTask task(const QModelIndex &) const;
    QTask task(const QUniqueId &) const;
    QTask task(int) const;

    static QVariant taskField(const QTask &c, QTaskModel::Field k);
    static bool setTaskField(QTask &c, QTaskModel::Field k,  const QVariant &);

    bool updateTask(const QTask&);
    bool removeTask(const QTask &);
    bool removeTask(const QUniqueId &);
    QUniqueId addTask(const QTask& task, const QPimSource &source = QPimSource());

    bool removeList(const QList<QUniqueId> &);

    QUniqueId addRecord(const QByteArray &, const QPimSource &, const QString &format = QString());
    bool updateRecord(const QUniqueId &id, const QByteArray &, const QString &format = QString());
    QByteArray record(const QUniqueId &id, const QString &format = QString()) const;

    bool removeRecord(const QUniqueId &id) { return removeTask(id); }

    // convienience.
    // does not affect other filter roles.
    void setFilterCompleted(bool);
    bool filterCompleted() const;

    bool updateRecurringTasks();

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &,
            int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

private:
    void setSortField(Field);
    Field sortField() const;

    static void initMaps();
    static QMap<Field, QString> k2t;
    static QMap<Field, QString> k2i;
    static QMap<QString, Field> i2k;

    QTaskModelData *d;
};

#endif
