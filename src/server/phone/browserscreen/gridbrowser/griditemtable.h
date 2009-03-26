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

#ifndef GRIDITEMTABLE_H
#define GRIDITEMTABLE_H

#include <QMap>
#include <QPair>

class GridItem;


class GridItemTable
{
public:

    typedef QPair<int,int> Key;

    GridItemTable();

    bool add(GridItem *);

    GridItem *item(int row,int col) const;

    GridItem *item(const Key &) const;

    int topRow() const { return mTopRow; }

    int topColumn() const { return mTopColumn; }


private:

    // Note: operator> and operator< are defined for QPair. The comparison is done on
    // the first value (row). If that ties, a comparison is made using the second value (column).
    QMap<Key,GridItem *> map;

    int mTopRow;
    int mTopColumn;
};

#endif
