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

#include <QStringList>
#include <QValueSpaceObject>
#include <QDebug>
#include <QMediaSessionRequest>


#include "urinegotiator.h"

#include "buildermanager.h"


namespace mediaserver
{

typedef QMap<QString, BuilderNegotiator*>       NegotiatorMap;
typedef QMap<QString, QMediaSessionBuilderList> BuilderMap;
typedef QMap<QMediaSessionBuilder*, QString>    BuilderEngineMap;
typedef QMap<QMediaServerSession*, QMediaSessionBuilder*>  ActiveSessionMap;



class BuilderManagerPrivate
{
public:
    QStringList         orderedEngines;
    QValueSpaceObject*  info;
    NegotiatorMap       negotiators;
    BuilderMap          builders;
    BuilderEngineMap    builderEngines;
    ActiveSessionMap    activeSessionMap;
};


BuilderManager::BuilderManager():
    d(new BuilderManagerPrivate)
{
    d->info = new QValueSpaceObject("/Media/Engines");

    // Hard coded
    d->negotiators.insert("com.trolltech.qtopia.uri", new UriNegotiator);

    // Sort out priorities of media engines the system was configured with
    foreach (QString const& engine , QString(CONFIGURED_ENGINES).split(' '))
        d->orderedEngines.append(engine.toLower());
}

BuilderManager::~BuilderManager()
{
    delete d->info;
    delete d;
}

void BuilderManager::addBuilders
(
 QString const& engineName,
 QMediaSessionBuilderList const& builderList
)
{
    foreach (QMediaSessionBuilder* sessionBuilder, builderList)
    {
        d->builderEngines[sessionBuilder] = engineName;

        QString type = sessionBuilder->type();
        NegotiatorMap::iterator it = d->negotiators.find(type);

        if (it != d->negotiators.end())
        {
            int pri = d->orderedEngines.indexOf(engineName.toLower());

            if (pri == -1)
                pri = 1000000;  // yeah

            // Known builder type, let negotiator deal with it
            (*it)->addBuilder(engineName, pri, sessionBuilder);
        }
        else
        {
            // No negotiator for builder - just install it
            d->builders[type].append(sessionBuilder);
        }

        // Add builder info
        QString buildDir = engineName + "/Builders/" + type + "/";
        QMediaSessionBuilder::Attributes const& attributes = sessionBuilder->attributes();

        for (QMediaSessionBuilder::Attributes::const_iterator it = attributes.begin();
             it != attributes.end();
             ++it)
        {
            d->info->setAttribute(buildDir + it.key(), it.value());
        }
    }
}


QMediaServerSession* BuilderManager::createSession(QMediaSessionRequest const& sessionRequest, QString *engineName)
{
    QMediaServerSession*        mediaSession = 0;
    QMediaSessionBuilder*       sessionBuilder;

    NegotiatorMap::iterator it = d->negotiators.find(sessionRequest.type());

    if (it != d->negotiators.end())
    {
        mediaSession = (*it)->createSession(sessionRequest);
        sessionBuilder = *it;
        if (engineName)
            *engineName = d->builderEngines[ (*it)->sessionBuilder(mediaSession) ];
    }
    else
    {
        QMediaSessionBuilderList&   sessionBuilders = d->builders[sessionRequest.type()];
        int                         builderCount = sessionBuilders.size();

        for (int i = 0; i < builderCount; ++i)
        {
            sessionBuilder = sessionBuilders.takeFirst();

            mediaSession = sessionBuilder->createSession(sessionRequest);
            if (mediaSession != 0)
            {
                if (engineName)
                    *engineName = d->builderEngines[sessionBuilder];

                sessionBuilders.prepend(sessionBuilder);
                break;
            }

            sessionBuilders.append(sessionBuilder);
        }
    }

    if (mediaSession != 0)
    {
        d->activeSessionMap.insert(mediaSession, sessionBuilder);
    }

    return mediaSession;
}

void BuilderManager::destroySession(QMediaServerSession* mediaSession)
{
    ActiveSessionMap::iterator it = d->activeSessionMap.find(mediaSession);

    if (it != d->activeSessionMap.end())
    {
        // Remove status info
        it.value()->destroySession(mediaSession);

        d->activeSessionMap.remove(mediaSession);
    }
}

}   // ns mediaserver
