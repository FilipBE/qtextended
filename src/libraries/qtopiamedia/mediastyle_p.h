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

#ifndef MEDIASTYLE_P_H
#define MEDIASTYLE_P_H

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

#include <qtopiabase/qtopiaglobal.h>

#include <QtGui>

class QTOPIAMEDIA_EXPORT MediaStyle : public QCommonStyle
{
public:
    void drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const;

    int pixelMetric( PixelMetric pm, const QStyleOption *opt, const QWidget *widget ) const;

private:
    mutable QPixmap m_groovebuffer;
    mutable QPixmap m_barbuffer;
    mutable QPixmap m_silhouettebuffer;
};

#endif
