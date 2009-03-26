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

#include "colortint.h"
#include <QColor>

QColor ColorTint::darken(const QColor &color, int percent)
{
    QColor rv( (color.red() * percent) / 100,
               (color.green() * percent) / 100,
               (color.blue() * percent) / 100);

    return rv;
}

QColor ColorTint::lighten(const QColor &color, int percent)
{
    QColor rv( color.red() + ((255 - color.red()) * percent) / 100,
              color.green() + ((255 - color.green()) * percent) / 100,
              color.blue() + ((255 - color.blue()) * percent) / 100);

    return rv;
}

