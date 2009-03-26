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

#include <QMap>
#include <QStringList>

#include <QPluginManager>
#include <QMediaCodecPlugin>
#include <QMediaEngineInformation>

#include "cruxusurisessionbuilder.h"
#include "cruxussimplesession.h"
#include "cruxusoutputthread.h"
#include "cruxusoutputdevices.h"

#include "cruxusengine.h"

namespace cruxus
{

// {{{ EngineInformation
class EngineInformation : public QMediaEngineInformation
{
public:
    EngineInformation(QMediaSessionBuilderList const& sessionBuilders):
        m_sessionBuilders(sessionBuilders)
    {
    }

    ~EngineInformation()
    {
    }

    QString name() const
    {
        return "Cruxus";
    }

    QString version() const
    {
        return "1.0";
    }

    int idleTime() const
    {
        return 0;
    }

    bool hasExclusiveDeviceAccess() const
    {
#ifdef HAVE_OSS
        return true;
#else
        return false;
#endif
    }

    QMediaSessionBuilderList sessionBuilders() const
    {
        return m_sessionBuilders;
    }

private:
    QMediaSessionBuilderList const& m_sessionBuilders;
};
// }}}

// {{{ CruxusEnginePrivate
class CruxusEnginePrivate
{
public:
    QPluginManager*             pluginManager;
    EngineInformation*          engineInfo;
    OutputThread*               outputThread;
    QMediaSessionBuilderList    sessionBuilders;
    QList<SimpleSession*>       sessions;
};
// }}}

// {{{ Engine

/*!
    \class cruxus::Engine
    \internal
*/

Engine::Engine():
    d(new CruxusEnginePrivate)
{
    d->pluginManager = new QPluginManager("codecs", this);
    d->outputThread = static_cast<OutputThread*>( OutputDevices::createOutputDevice() );
}

Engine::~Engine()
{
    delete d->engineInfo;
    delete d;
}

void Engine::initialize()
{
    MediaCodecPluginMap mimeTypeMapping;

    // Find plugins
    foreach (QString const& pluginName, d->pluginManager->list()) {
        QMediaCodecPlugin*  plugin;
        QObject*            instance = d->pluginManager->instance(pluginName);

        if ((plugin = qobject_cast<QMediaCodecPlugin*>(instance)) != 0) {

            foreach (QString const& mimeType, plugin->mimeTypes())
                mimeTypeMapping.insert(mimeType, plugin);
        }
        else
            delete instance;    // unload unwanted
    }

    // Add the builder
    d->sessionBuilders.push_back(new UriSessionBuilder(this, mimeTypeMapping));

    // Create our info object
    d->engineInfo = new EngineInformation(d->sessionBuilders);
}


void Engine::start()
{
}

void Engine::stop()
{
}

void Engine::suspend()
{
    d->outputThread->suspend();
}

void Engine::resume()
{
    d->outputThread->resume();
}

QMediaEngineInformation const* Engine::engineInformation()
{
    return d->engineInfo;
}

void Engine::registerSession(QMediaServerSession* session)
{
    d->sessions.append(static_cast<SimpleSession*>(session));
}

void Engine::unregisterSession(QMediaServerSession* session)
{
    for (int i = 0; i < d->sessions.size(); ++i) {
        if (d->sessions.at(i) == static_cast<SimpleSession*>(session))
            d->sessions.removeAt(i);
    }
}
// }}}

}   // ns cruxus
