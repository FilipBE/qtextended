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

#ifndef BUILDERMANAGER_H
#define BUILDERMANAGER_H

#include <QString>

#include <QMediaSessionBuilder>

class QMediaServerSession;

namespace mediaserver
{

class BuilderManagerPrivate;

class BuilderManager
{
public:
    BuilderManager();
    ~BuilderManager();

    void addBuilders(QString const& engineName,
                     QMediaSessionBuilderList const& builderList);

    void removeBuilders(QString const& engineName,
                    QMediaSessionBuilderList const& builderList);

    QMediaServerSession* createSession(QMediaSessionRequest const& sessionRequest, QString *engineName);
    void destroySession(QMediaServerSession* mediaSession);

private:
    BuilderManagerPrivate*  d;
};

}   // ns mediaserver

#endif
