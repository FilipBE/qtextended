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

#ifndef PHONEDECORATION_P_H
#define PHONEDECORATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaglobal.h>
#include <qwindowdecorationinterface.h>
#include <qimage.h>

class PhoneDecorationPrivate;
class DecorationBorderData;

class PhoneDecoration : public QWindowDecorationInterface
{
public:
    PhoneDecoration();
    virtual ~PhoneDecoration();

    virtual int metric( Metric m, const WindowData * ) const;
    virtual void drawArea( Area a, QPainter *, const WindowData * ) const;
    virtual void drawButton( Button b, QPainter *, const WindowData *, int x, int y, int w, int h, QDecoration::DecorationState ) const;
    virtual QRegion mask( const WindowData * ) const;
    virtual QString name() const;
    virtual QPixmap icon() const;

private:
    void drawStretch(QPainter *p, const QRect &r, const DecorationBorderData *data, const QBrush &b, Qt::Orientation o) const;
    QRegion maskStretch(const QRect &r, const DecorationBorderData *data, Qt::Orientation) const;

private:
    PhoneDecorationPrivate *d;
};

#endif
