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

#ifndef QMEDIALIST_H
#define QMEDIALIST_H

#include <QtGui>

#include <QMediaPlaylist>

class QMediaListPrivate;

class QTOPIAMEDIA_EXPORT QMediaList : public QAbstractListModel
{
    Q_OBJECT
public:
    enum SortDirection   { Ascending, Descending, Unsorted };
    enum Properties { ShowEmpty, ShowEmptyAsUnknown, HideEmpty };
    enum Grouping   { ShowAll, ShowGrouped };

    enum Roles      {
        Title      = 0x01,
        Url        = 0x02,
        Artist     = 0x04,
        Album      = 0x08,
        Genre      = 0x10,
        MimeType   = 0x40
    };

    QMediaList();
    QMediaList(const QMediaPlaylist& playlist);
    QMediaList(const QMediaList& other);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;

    void beginFilter();
    void clearFilter();
    bool removeFilter(Roles filterType);
    bool setFilter(Roles filterType, const QString& text);
    bool addFilter(Roles filterType, const QString& text);
    void randomize();
    void setSorting(SortDirection sortType);
    void setSorting(QList<int> roles, SortDirection sortType);
    void endFilter();
    bool isFiltered() const;

    void setDisplayRole(int role, Grouping grouping=ShowGrouped);
    int displayRole() const;
    Grouping displayGrouping() const;
    void showEmpty(Properties role);

    void setModel(const QMediaPlaylist& playlist);

    QMediaPlaylist playlist(const QModelIndex &index=QModelIndex()) const;
    QMediaList &operator=(const QMediaList &medialist);
    bool operator==(const QMediaList &other) const;

protected:
    void refreshData();

private:
    QMediaListPrivate    *d;
};

#endif
