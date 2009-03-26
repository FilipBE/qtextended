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
#ifndef ALBUMSELECTOR_H
#define ALBUMSELECTOR_H

#include "albummodel.h"
#include <QWidget>

class ThumbCache;
class QContent;
class QModelIndex;
class AlbumModel;
class AlbumRibbon;
class QListView;
class AlbumIconDelegate;
class AlbumListDelegate;
class QSmoothIconView;
class QSmoothList;
class QStackedWidget;

class AlbumSelector : public QWidget
{
    Q_OBJECT
public:

    AlbumSelector(AlbumModel::SortMode sort = AlbumModel::SortByDate, QWidget *parent = 0);
    ~AlbumSelector();

    AlbumModel::SortMode sortMode() const;
    void setSortMode(AlbumModel::SortMode mode);

public slots:
    void sortByName();
    void sortByDate();

signals:
    void sortModeChanged(AlbumModel::SortMode mode);
    void albumSelected(const QString &name, const QString &categoryId);

private slots:
    void albumSelected(const QModelIndex &index);

    void ribbonValueChanged(int value);
    void yearRangeChanged(int minimum, int maximum);

    void currentChanged(const QModelIndex &current);

private:
    AlbumModel *m_albumModel;
    AlbumRibbon *m_ribbon;

    QStackedWidget *m_stack;
    QSmoothIconView *m_iconView;
    QSmoothList *m_listView;
};

#endif
