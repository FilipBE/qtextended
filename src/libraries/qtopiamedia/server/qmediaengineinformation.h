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

#ifndef QMEDIAENGINEINFORMATION_H
#define QMEDIAENGINEINFORMATION_H

#include <QString>
#include <QList>
#include <QMediaSessionBuilder>

#include <qtopiaglobal.h>


class QTOPIAMEDIA_EXPORT QMediaEngineInformation
{
public:
    virtual ~QMediaEngineInformation();

    virtual QString name() const = 0;
    virtual QString version() const = 0;

    virtual int idleTime() const = 0;

    virtual bool hasExclusiveDeviceAccess() const = 0;

    virtual QMediaSessionBuilderList sessionBuilders() const = 0;
};


#endif
