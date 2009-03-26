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

#include "oscillator_p.h"

#include <math.h>

Oscillator::Oscillator(qreal _yMin,qreal _yMax,qreal xMax,qreal _factor, qreal _origin)
        : yMax(_yMax)
        , yMin(_yMin)
        , m1(-(yMax-yMin)/2/xMax)
        , m2((yMax-yMin)/2/xMax)
        , factor(_factor)
        , origin(_origin)
{
}

qreal Oscillator::operator() (qreal x) {
    qreal max = m1 * x + yMax;
    qreal min = m2 * x + yMin;

    // If we need more oscillations or less, we add a factor and go like
    // this
    x *= factor;
    // for example, factor = 2 will give us twice as many peaks

    qreal hr = (max - min)/2;
    qreal rc = (hr * sin(x)) + origin;
    return rc;
}
