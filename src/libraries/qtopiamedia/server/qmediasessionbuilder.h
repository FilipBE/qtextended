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

#ifndef QMEDIASESSIONBUILDER_H
#define QMEDIASESSIONBUILDER_H

#include <QVariant>
#include <QList>
#include <QMap>

#include <qtopiaglobal.h>

class QMediaServerSession;
class QMediaSessionRequest;

class QTOPIAMEDIA_EXPORT QMediaSessionBuilder
{
public:
    typedef QMap<QString, QVariant> Attributes;

    virtual ~QMediaSessionBuilder();

    virtual QString type() const = 0;
    virtual Attributes const& attributes() const = 0;

    virtual QMediaServerSession* createSession(QMediaSessionRequest sessionRequest) = 0;
    virtual void destroySession(QMediaServerSession* serverSession) = 0;
};

typedef QList<QMediaSessionBuilder*> QMediaSessionBuilderList;

#endif
