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

#include "griditemtable.h"
#include "griditem.h"


/*!
  \internal
  \class GridItemTable
    \inpublicgroup QtBaseModule

  \brief   GridItemTable stores pointers to GridItem objects in a row * column grid format.

  The size of the table (rows or columns) grows as GridItem objects are added to it.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa GridItem
*/


/*!
  \internal
  \fn GridItemTable::GridItemTable()
  Constructs an empty table containing neither rows nor columns.
*/
GridItemTable::GridItemTable() : mTopRow(-1), mTopColumn(-1) {}

/*!
  \internal
  \fn bool GridItemTable::add(GridItem *item)
  Adds the given object and returns true if there was no previous item at item's row and column,
  or else returns false if an item has already been placed at item's row and column, in which
  case the item is not added. Note that this method may affect the value(s) returned by
  \l{function}{topRow()} and \l{function}{topColumn()}.
*/
bool GridItemTable::add(GridItem *item)
{
    Key key(item->row(),item->column());

    if ( map.contains(key) ) {
        return false;
    }

    map[key] = item;

    if ( item->row() > mTopRow ) {
        mTopRow = item->row();
    }
    if ( item->column() > mTopColumn ) {
        mTopColumn = item->column();
    }

    return true;
}

/*!
  \internal
  \fn GridItem *GridItemTable::item(const Key &key) const
  Returns the item at Key's row and column, or 0 if none exists.
*/
GridItem *GridItemTable::item(const Key &key) const
{
    if ( !(map.contains(key)) ) {
        return 0;
    }
    return map[key];
}

/*!
  \internal
  \fn GridItem *GridItemTable::item(int row,int col) const
  Returns the item at the given row and column, or 0 if none exists.
*/
GridItem *GridItemTable::item(int row,int col) const
{
    Key key(row,col);
    return item(key);
}


