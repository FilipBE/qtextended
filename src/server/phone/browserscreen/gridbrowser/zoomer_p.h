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

#ifndef ZOOMER_P_H
#define ZOOMER_P_H

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

#include "animator_p.h"

#include <QString>

class Painter;
class SelectedItem;


class Zoomer : public Animator
{
public:

    //void animate(QPainter *,QSvgRenderer *,int imageSize,QGraphicsRectItem *,qreal percent);
    void animate(QPainter *,SelectedItem *,qreal percent);

    // Returns description of this class for configuration purposes.
    static const QString &description() { return mDescription; }

private:

    static const QString mDescription;

    // Percentag by which the image grows during the zoom.
    static const qreal growthFactor;

    static const qreal stop1;
    static const qreal stop2;
};

#endif
