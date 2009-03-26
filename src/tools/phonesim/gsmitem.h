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

#ifndef GSMITEM_H
#define GSMITEM_H

#include <QString>
#include <QStringList>

class GSMItem
{

public:
    GSMItem(const QString&, const QString&, const QStringList&, const QStringList&, const QString&);
    GSMItem();
    QString getDescription();
    QString getProfile();
    QStringList getParameterFormat();
    QStringList getResponseFormat();

private:
    QString command;
    QString profile;
    QStringList parameterFormat;
    QStringList responses;
    QString description;

};

#endif
