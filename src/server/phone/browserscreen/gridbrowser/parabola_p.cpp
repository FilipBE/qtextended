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

#include "parabola_p.h"

Parabola::Parabola(qreal maxX,qreal _peak)
    : peak(_peak)
{
    // Half the width of the parabola when x = 0
    halfX = maxX/2;
    a = -peak/(halfX * halfX);
}

qreal Parabola::operator()(qreal x)
{
    x -= halfX;
    qreal rc = a * x * x + peak;
    return rc;
}
