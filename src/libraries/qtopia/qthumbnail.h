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

#ifndef QTHUMBNAIL_H
#define QTHUMBNAIL_H

#include <qtopiaglobal.h>

#include <QString>
#include <QSize>
#include <QPixmap>

class QThumbnailPrivate;

class QTOPIA_EXPORT QThumbnail
{
public:
    explicit QThumbnail( const QString& fileName );
    explicit QThumbnail( QIODevice *device );

    ~QThumbnail();

    QSize actualSize( const QSize& size = QSize(), Qt::AspectRatioMode mode = Qt::KeepAspectRatio );

    QPixmap pixmap( const QSize& size, Qt::AspectRatioMode mode = Qt::KeepAspectRatio, Qt::TransformationMode transformationMode = Qt::FastTransformation );

private:
    QThumbnailPrivate *d;
};

#endif
