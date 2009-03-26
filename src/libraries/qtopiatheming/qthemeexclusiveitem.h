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

#ifndef QTHEMEEXCLUSIVEITEM_H
#define QTHEMEEXCLUSIVEITEM_H

#include <qtopiaglobal.h>
#include <QThemeItem>

class QThemeExclusiveItemPrivate;

class QTOPIATHEMING_EXPORT QThemeExclusiveItem : public QThemeItem
{

public:
    QThemeExclusiveItem(QThemeItem *parent = 0);
    ~QThemeExclusiveItem();

    enum { Type = ThemeItemType + 4 };
    int type() const;

protected:
    void layout();

private:
    QThemeExclusiveItemPrivate *d;
};

#endif
