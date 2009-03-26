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

#ifndef BUILDERNEGOTIATOR_H
#define BUILDERNEGOTIATOR_H

#include <QString>
#include <QMediaSessionBuilder>

class QMediaSessionBuilder;
class QMediaServerSession;

namespace mediaserver
{

class BuilderNegotiator : public QMediaSessionBuilder
{
public:
    virtual ~BuilderNegotiator();

    virtual void addBuilder(QString const& tag, int priority, QMediaSessionBuilder* sessionBuilder) = 0;
    virtual void removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder) = 0;

    virtual QMediaSessionBuilder* sessionBuilder( QMediaServerSession *session ) = 0;
};

}   // ns mediaserver

#endif
