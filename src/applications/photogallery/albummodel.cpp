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
#include "albummodel.h"
#include "photogallery.h"
#include "thumbcache.h"

#include <QTimer>
#include <QContentSet>

AlbumModel::AlbumModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_creationDateKey(QLatin1String("CreationDate"))
    , m_categories(QLatin1String("PhotoAlbums"))
    , m_sortMode(SortByDate)
    , m_minimumYear(0)
    , m_maximumYear(0)
{
    connect(&m_categories, SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()));
    connect(PhotoGallery::thumbCache(), SIGNAL(changed(QContent)), this, SLOT(changed(QContent)));

    QTimer::singleShot(0, this, SLOT(categoriesChanged()));
}

AlbumModel::~AlbumModel()
{
    qDeleteAll(m_albums);
}

AlbumModel::SortMode AlbumModel::sortMode() const
{
    return m_sortMode;
}

void AlbumModel::setSortMode(SortMode mode)
{
    if (m_sortMode != mode) {
        m_sortMode = mode;

        if (m_sortMode == SortByDate) {
            qSort(m_albums.begin(), m_albums.end(), albumDateLessThan);
        } else {
            qSort(m_albums.begin(), m_albums.end(), albumNameLessThan);
        }

        emit reset();
    }
}

int AlbumModel::minimumYear() const
{
    return m_minimumYear;
}

int AlbumModel::maximumYear() const
{
    return m_maximumYear;
}

QModelIndex AlbumModel::indexForYear(int year)
{
    if (m_sortMode == SortByName || m_albums.count() == 0)
        return QModelIndex();

    for (int i = 0; i < m_albums.count() - 1; ++i)
        if (year <= m_albums.at(i)->date.date().year())
            return createIndex(i, 0);

    return createIndex(m_albums.count() - 1, 0);
}

QModelIndex AlbumModel::indexForCharacter(QChar character)
{
    if (m_sortMode == SortByDate || m_albums.count() == 0)
        return QModelIndex();

    for (int i = 0; i < m_albums.count() - 1; ++i)
        if (m_albums.at(i)->name.length() > 0 && character <= m_albums.at(i)->name.at(0))
            return createIndex(i, 0);

    return createIndex(m_albums.count() - 1, 0);
}

QVariant AlbumModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return m_albums.at(index.row())->name;
    case Qt::DecorationRole:
        {
            Album *album = m_albums.at(index.row());

            QImage thumbnail;

            if (QImage *t = PhotoGallery::thumbCache()->image(album->firstImage))
                thumbnail = *t;
            else
                PhotoGallery::thumbCache()->queueImage(album->firstImage);

            return thumbnail;
        }
    case CategoryIdRole:
        return m_albums.at(index.row())->categoryId;
    case YearRole:
        return m_albums.at(index.row())->date.date().year();
    case CountRole:
        return m_albums.at(index.row())->count;
    default:
        return QVariant();
    }
}

int AlbumModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid()
        ? m_albums.count()
        : 0;
}

void AlbumModel::categoriesChanged()
{
    QStringList categoryIds = m_categories.categoryIds();

    categoryIds.append(QLatin1String("Unfiled"));

    for (int i = 0; i < m_albums.count(); ++i)
        if (!categoryIds.contains(m_albums.at(i)->categoryId))
            delete m_albums.takeAt(i--);

    foreach (QString categoryId, categoryIds) {
        if (m_categories.isGlobal(categoryId))
            continue;

        bool exists = false;

        foreach (Album *album, m_albums) {
            if (album->categoryId == categoryId) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            Album *album = new Album;

            album->categoryId = categoryId;

            album->name = m_categories.label(categoryId);

            if (album->name.isNull())
                album->name = tr("Unfiled");

            QContentSet set;
            set.setCriteria(QContentFilter::mimeType(QLatin1String("image/*"))
                    & QContentFilter::category(categoryId));
            set.setSortCriteria(QContentSortCriteria(
                    QContentSortCriteria::Property, QLatin1String("none/CreationDate")));

            album->count = set.count();

            if (set.count() > 0) {
                album->firstImage = set.content(0);

                for (int i = 0; i < set.count(); ++i) {
                    QString dateString = set.content(i).property(m_creationDateKey);
                    if (!dateString.isNull()){
                        album->date = QDateTime::fromString(dateString, Qt::ISODate);
                        break;
                    }
                }
            }
            m_albums.append(album);
        }
    }

    if (m_sortMode == SortByDate) {
        qSort(m_albums.begin(), m_albums.end(), albumDateLessThan);

        int minimum = m_minimumYear;

        foreach (Album *album, m_albums) {
            if (album->date.isValid()) {
                minimum = album->date.date().year();
                break;
            }
        }

        int maximum = m_albums.count() > 0 && m_albums.last()->date.isValid()
                ? m_albums.last()->date.date().year()
                : minimum;

        if (minimum != m_minimumYear || maximum != m_maximumYear)
            emit yearRangeChanged(m_minimumYear = minimum, m_maximumYear = maximum);
    } else {
        qSort(m_albums.begin(), m_albums.end(), albumNameLessThan);
    }

    emit reset();
}

bool AlbumModel::albumDateLessThan(Album *left, Album *right)
{
    return left->date < right->date;
}

bool AlbumModel::albumNameLessThan(Album *left, Album *right)
{
    return QString::localeAwareCompare(left->name, right->name) < 0;
}

void AlbumModel::changed(const QContent &content)
{
    for (int i = 0; i < m_albums.count(); ++i) {
        if (m_albums.at(i)->firstImage == content) {
            QModelIndex index = createIndex(i, 0);

            emit dataChanged(index, index);
        }
    }
}
