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
#ifndef THUMBMODEL_H
#define THUMBMODEL_H

#include <QContentSetModel>

class ThumbModel : public QContentSetModel
{
    Q_OBJECT
public:
    enum { RotationRole = ThumbnailRole + 1 };

    ThumbModel(QContentSet *set, QObject *parent = 0);
    ~ThumbModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private slots:
    void changed(const QContent &content);

private:
    mutable QMap<QContentId, QPersistentModelIndex> m_queuedThumbs;
};


#endif
