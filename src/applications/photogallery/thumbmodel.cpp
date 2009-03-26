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
#include "thumbmodel.h"
#include "photogallery.h"
#include "thumbcache.h"

#include <QImageReader>

ThumbModel::ThumbModel(QContentSet *set, QObject *parent)
    : QContentSetModel(set, parent)
{
    connect(PhotoGallery::thumbCache(), SIGNAL(changed(QContent)), this, SLOT(changed(QContent)));
}

ThumbModel::~ThumbModel()
{
}

QVariant ThumbModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        {
            QContent c = content(index);

            QImage thumb;

            if (QImage *t = PhotoGallery::thumbCache()->image(c)) {
                thumb = *t;
            } else {
                m_queuedThumbs[c.id()] = index;

                PhotoGallery::thumbCache()->queueImage(c);
            }

            return thumb.isNull()
                    ? QContentSetModel::data(index, role)
                    : thumb;
        }
    case RotationRole:
    case Qt::SizeHintRole:
        {
            QContent c = content(index);

            int rotation = PhotoGallery::rotation(c);

            if (role == RotationRole)
                return rotation;

            QSize size;

            if (QImage *t = PhotoGallery::thumbCache()->image(c))
                size = t->size();

            return size;
        }
    default:
        return QContentSetModel::data(index, role);
    }
}

bool ThumbModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == RotationRole) {
        QContent image = content(index);

        PhotoGallery::setRotation(&image, value.toInt());

        return true;
    } else {
        return QContentSetModel::setData(index, value, role);
    }
}

void ThumbModel::changed(const QContent &content)
{
    QPersistentModelIndex index = m_queuedThumbs.take(content.id());

    if (index.isValid())
        emit dataChanged(index, index);
}
