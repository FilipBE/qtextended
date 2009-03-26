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

#include <qstringlist.h>
#include <qpluginmanager.h>

#include <qmediaenginefactory.h>
#include <qmediaengine.h>

#include "engineloader.h"


namespace mediaserver
{

class EngineLoaderPrivate
{
public:
    QPluginManager*     pluginManager;
    QMediaEngineList    engines;
};

/*!
    \class mediaserver::EngineLoader
    \internal
*/

// {{{ EngineLoader
EngineLoader::EngineLoader():
    d(new EngineLoaderPrivate)
{
}

EngineLoader::~EngineLoader()
{
    delete d->pluginManager;
    delete d;
}

void EngineLoader::load()
{
    d->pluginManager = new QPluginManager("mediaengines");

    // Find
    foreach (QString const& pluginName, d->pluginManager->list())
    {
        QMediaEngineFactory*    factory;
        QObject*                instance = d->pluginManager->instance(pluginName);

        if ((factory = qobject_cast<QMediaEngineFactory*>(instance)) != 0)
        {
            QMediaEngine*   mediaEngine = factory->create();

            if (mediaEngine != 0)
            {
                d->engines.push_back(mediaEngine);
            }
        }
        else
            delete instance;
    }
}

void EngineLoader::unload()
{
    foreach (QMediaEngine* engine, d->engines)
    {
        delete engine;
    }

    d->engines.clear();
}


QMediaEngineList const& EngineLoader::engines()
{
    return d->engines;
}
// }}}

}   // ns mediaserver


