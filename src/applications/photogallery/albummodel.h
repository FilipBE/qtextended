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
#ifndef ALBUMMODEL_H
#define ALBUMMODEL_H

#include <QAbstractListModel>
#include <QContent>
#include <QCategoryManager>
#include <QImage>

class AlbumModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum
    {
        CategoryIdRole = Qt::UserRole,
        YearRole,
        CountRole
    };

    enum SortMode
    {
        SortByDate,
        SortByName
    };

    AlbumModel(QObject *parent = 0);
    ~AlbumModel();

    SortMode sortMode() const;
    void setSortMode(SortMode mode);

    int minimumYear() const;
    int maximumYear() const;

    QModelIndex indexForYear(int year);
    QModelIndex indexForCharacter(QChar character);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

signals:
    void yearRangeChanged(int minimum, int maximum);

private slots:
    void categoriesChanged();
    void changed(const QContent &content);

private:
    struct Album
    {
        QString categoryId;
        QString name;
        QDateTime date;
        QContent firstImage;
        int count;
    };

    static bool albumDateLessThan(Album *left, Album *right);
    static bool albumNameLessThan(Album *left, Album *right);

    void queueThumbnail(Album *album) const;

    const QString m_creationDateKey;
    QCategoryManager m_categories;
    QList<Album *> m_albums;
    SortMode m_sortMode;
    int m_minimumYear;
    int m_maximumYear;
};

#endif
