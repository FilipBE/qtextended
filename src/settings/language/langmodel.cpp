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

#include "langmodel.h"

#include <qtopianamespace.h>

QVariant LanguageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    static QVariant pixmapVariant(QPixmap(":image/selectedDict"));

    int pos = index.row();
    if (role == Qt::DisplayRole) {
        //we need to add LRM/RLM at the end of the string because
        //some names end with an ')'. In an RTL env this would move the ')'
        //to the beginning of the line
        if ( list[pos].direction == Qt::LeftToRight )
            return list[pos].langName+QChar(0x200E); //add LRM
        else
            return list[pos].langName+QChar(0x200F); //add RLM
    } else if (role == Qtopia::AdditionalDecorationRole) {
        if (list[pos].hasDict)
            return pixmapVariant;
        return QPixmap();
    } else if (role == Qt::FontRole) {
        if (list[pos].isCurrentLang) {
            QFont f = list[pos].fnt;
            f.setBold(true);
            return QVariant(f);
        }
        return QVariant(list[pos].fnt);
    } else if (role == Qt::UserRole) {
        return list[pos].hasDict;
    } else if (role == Qt::UserRole+1) {
        return list[pos].direction;
    }

    return QVariant();
}

int LanguageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return list.count();
}


bool LanguageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::UserRole)
    {
        FontedItem replacement = list[index.row()];
        replacement.hasDict = value.toBool();
        list.replace(index.row(), replacement);
        emit dataChanged(QAbstractListModel::index(0), QAbstractListModel::index(list.count()-1));
        return true;
    }
    return false;
}
