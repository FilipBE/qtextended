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

#ifndef LANGMODEL_H
#define LANGMODEL_H

#include <QAbstractListModel>
#include <QFont>
#include <QtopiaItemDelegate>

struct FontedItem
{
    FontedItem(const QString &lang, QFont &f, bool hasDictionary, bool isCurrentLanguage)
        : langName(lang), fnt(f), hasDict(hasDictionary), direction( Qt::LeftToRight ),
          isCurrentLang(isCurrentLanguage)
    {
    }

    FontedItem&operator=(const FontedItem &other)
    {
        langName = other.langName;
        fnt = other.fnt;
        hasDict = other.hasDict;
        isCurrentLang = other.isCurrentLang;
        return *this;
    }

    QString langName;
    QFont fnt;
    bool hasDict;
    Qt::LayoutDirection direction;
    bool isCurrentLang;
};

class LanguageModel : public QAbstractListModel
{
public:
    LanguageModel(QObject *parent, const QList<FontedItem> &l)
        :QAbstractListModel(parent), list(l) {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    QList<FontedItem> list;

};

class LanguageDelegate : public QtopiaItemDelegate {

public:
    LanguageDelegate( QObject* parent )
        :QtopiaItemDelegate( parent )
    {
    }

    void paint( QPainter * p, const QStyleOptionViewItem& opt, const QModelIndex& index ) const
    {
        QtopiaItemDelegate::paint( p, opt, index );
    }

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const
    {
        QSize sz = QtopiaItemDelegate::sizeHint(opt, index);
        if (Qtopia::mousePreferred())
            return QSize(sz.width(), sz.height()+20);
        else
            return sz;
    }
};

#endif
