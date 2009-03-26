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

#ifndef URINEGOTIATOR_H
#define URINEGOTIATOR_H

#include "buildernegotiator.h"

namespace mediaserver
{

class UriNegotiatorPrivate;

class UriNegotiator : public BuilderNegotiator
{
public:
    UriNegotiator();
    ~UriNegotiator();

    QString type() const;
    Attributes const& attributes() const;

    void addBuilder(QString const& tag, int priority, QMediaSessionBuilder* sessionBuilder);
    void removeBuilder(QString const& tag, QMediaSessionBuilder* sessionBuilder);

    virtual QMediaSessionBuilder* sessionBuilder( QMediaServerSession *session );

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* mediaSession);

private:
    UriNegotiatorPrivate*   d;
};

}   // ns mediaserver

#endif
