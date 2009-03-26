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

#include "gsmitem.h"

GSMItem::GSMItem(const QString& comm, const QString& prof, const QStringList& farForm, const QStringList& respFormat, const QString& desc)
{
    command = comm;
    profile = prof;
    description = desc;
    parameterFormat = farForm;
    responses = respFormat;
}

GSMItem::GSMItem()
{
}

QString GSMItem::getProfile()
{
    return profile;
}


QStringList GSMItem::getParameterFormat()
{
    return parameterFormat;
}


QStringList GSMItem::getResponseFormat()
{
    return responses;
}

QString GSMItem::getDescription()
{
    return description;
}
