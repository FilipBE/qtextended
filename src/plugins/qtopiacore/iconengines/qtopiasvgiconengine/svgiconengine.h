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

#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

class QtopiaSvgIconEnginePrivate;

class QtopiaSvgIconEngine : public QIconEngineV2
{
public:
    QtopiaSvgIconEngine();
    QtopiaSvgIconEngine(const QtopiaSvgIconEngine &other);
    virtual ~QtopiaSvgIconEngine();
    virtual void paint(QPainter *painter, const QRect &rect,
                       QIcon::Mode mode, QIcon::State state);
    virtual QSize actualSize(const QSize &size, QIcon::Mode mode,
                             QIcon::State state);
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                           QIcon::State state);

    virtual void addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                           QIcon::State state);
    virtual void addFile(const QString &fileName, const QSize &size,
                         QIcon::Mode mode, QIcon::State state);

    QString key() const;
    QIconEngineV2 *clone() const;
    bool read(QDataStream &in);
    bool write(QDataStream &out) const;

private:
    QSharedDataPointer<QtopiaSvgIconEnginePrivate> d;
};

#endif
