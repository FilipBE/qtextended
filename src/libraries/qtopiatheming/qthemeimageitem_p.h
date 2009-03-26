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

#ifndef QTHEMEIMAGEITEM_P_H
#define QTHEMEIMAGEITEM_P_H

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

#include <QThemeItemAttribute>
#include <QMap>
#include <QString>
#include <QPixmap>
#include <QSizeF>

class QThemeImageItemPrivate
{

public:
    QThemeImageItemPrivate();
    ~QThemeImageItemPrivate();
    QMap<QString, QPixmap> buffer;

    QThemeItemAttribute source;
    QThemeItemAttribute tile;
    QThemeItemAttribute scale;
    QThemeItemAttribute alpha;
    QThemeItemAttribute color;
    QThemeItemAttribute replaceColor;
    QSizeF imagesSize;
    bool forceReload;
};

#endif
