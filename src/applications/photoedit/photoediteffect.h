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

#ifndef PHOTOEDITEFFECT_H
#define PHOTOEDITEFFECT_H

#include <QMap>
#include <qtopiaglobal.h>

class QString;
class QVariant;
class QImage;

class PhotoEditEffect
{
public:
    virtual ~PhotoEditEffect();
    virtual void applyEffect(const QString &effect, const QMap<QString, QVariant> &parameters, QImage *image) = 0;
};

Q_DECLARE_INTERFACE(PhotoEditEffect, "com.trolltech.Qtopia.PhotoEditEffect/1.0");

#endif
