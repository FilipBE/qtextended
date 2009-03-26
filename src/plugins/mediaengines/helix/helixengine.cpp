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

#include <QUrl>
#include <QString>
#include <qcoreevent.h>

#include <qmediaengineinformation.h>
#include <qmediasessionrequest.h>

#include <qtopialog.h>
#include <config.h>

#include <stdlib.h>

#include "helixengine.h"
#include "helixsession.h"
#include "qmediahelixsettingsserver.h"
#include "reporterror.h"
#include "helixutil.h"

#include <hxcom.h>
#include <hxcore.h>
#include <ihxmedpltfm.h>


/*!
   \namespace qtopia_helix
   \internal
*/
namespace qtopia_helix
{

// typedef
typedef HX_RESULT (HXEXPORT_PTR FPRMSETDLLACCESSPATH) (const char*);

// {{{ Helix Library Functions
struct HelixLibrary
{
    FPRMCREATEENGINE CreateEngine;
    FPRMCLOSEENGINE CloseEngine;
    FPRMSETDLLACCESSPATH SetDLLAccessPath;
};
// }}}

// {{{ HelixInformation
class HelixInformation : public QMediaEngineInformation
{
public:
    HelixInformation(QMediaSessionBuilder* helixBuilder):
        m_helixBuilder(helixBuilder)
    {
    }

    QString name() const
    {
        return "Helix";
    }

    QString version() const
    {
        return "1.0";
    }

    int idleTime() const
    {
        return 20 * 1000;  // seconds
    }

    bool hasExclusiveDeviceAccess() const
    {
        return true;
    }

    QMediaSessionBuilderList sessionBuilders() const
    {
        QMediaSessionBuilderList builders;

        builders << m_helixBuilder;

        return builders;
    }

private:
    QMediaSessionBuilder*   m_helixBuilder;
};
// }}}

// {{{ HelixEnginePrivate
class HelixEnginePrivate
{
public:
    int                     timerId;
    HelixLibrary            symbols;
    IHXClientEngine*        engine;
    HelixInformation*       info;
    QMediaSessionBuilder::Attributes    builderAttributes;
};
// }}}

// {{{ HelixEngine

/*!
    \class qtopia_helix::HelixEngine
    \internal
*/

HelixEngine::HelixEngine():
    d(new HelixEnginePrivate)
{
    // init
    d->timerId = -1;
    d->engine = 0;
    d->info = new HelixInformation(this);

    // Helix does not support dynamically discovering the mime types and
    // uri schemes.  Grab them from QSettings
    QSettings   settings("Trolltech", "helix");
    settings.beginGroup("Simple");

    // Supported URI Schemes
    d->builderAttributes.insert("uriSchemes", settings.value("UriSchemes").toStringList());

    // Supported mime types
    d->builderAttributes.insert("mimeTypes", settings.value("MimeTypes").toStringList());
}

HelixEngine::~HelixEngine()
{
    delete d;
}

void HelixEngine::initialize()
{
    // Get helix lib
    QLibrary library(helix_library_path() + QLatin1String("/hxmedpltfm.so")) ;

    d->symbols.CreateEngine = (FPRMCREATEENGINE) library.resolve( "CreateEngine" );
    d->symbols.CloseEngine = (FPRMCLOSEENGINE) library.resolve( "CloseEngine" );
    d->symbols.SetDLLAccessPath = (FPRMSETDLLACCESSPATH) library.resolve( "SetDLLAccessPath" );

    if (d->symbols.CreateEngine != 0 &&
        d->symbols.CloseEngine != 0 &&
        d->symbols.SetDLLAccessPath != 0)
    {
        // Set library path
        QByteArray env = getenv( "HELIX_PLUGIN_PATH" );
        QByteArray path = env.isNull() ? helix_library_path().toLatin1() : env;

        QByteArray dllAccessPath = QByteArray() + "DT_Common=" + path + '\0' +
                                    "DT_Plugins=" + path + '\0' +
                                    "DT_Codecs=" + path + "\0\0";

        d->symbols.SetDLLAccessPath(dllAccessPath);

        if (d->symbols.CreateEngine(&d->engine) == HXR_OK)
        {
            // Initialize engine and preload plugins
            IHXClientEngineSetup*   enginesetup = 0;

            if (queryInterface(d->engine, IID_IHXClientEngineSetup, enginesetup) == HXR_OK)
            {
                GenericContext context;

                enginesetup->Setup(&context);
            }

            // Create global settings control
            new QMediaHelixSettingsServer(d->engine);
        }
    }
    else
    {
        REPORT_ERROR(ERR_HELIX);
    }
}

void HelixEngine::start()
{
    // already started
}

void HelixEngine::stop()
{
    d->symbols.CloseEngine(d->engine);

    d->engine = 0;
}

void HelixEngine::suspend()
{
    if (d->timerId != -1) {
        killTimer(d->timerId);  // Shut down timer straight away
        d->timerId = -1;
        foreach( HelixSession *s, sessions ) {
            if ( !s->isSuspended() ) {
                s->suspend();
                suspendedSessions.append(s);
            }
        }
    }
}

void HelixEngine::resume()
{
    // Start helix event timer
    if ( d->timerId == -1 && !sessions.isEmpty() ) {
        foreach( HelixSession *s, suspendedSessions )
            s->resume();
        suspendedSessions.clear();

        d->timerId = startTimer(300);
    }
}

QMediaEngineInformation const* HelixEngine::engineInformation()
{
    return d->info;
}

// private:
void HelixEngine::timerEvent(QTimerEvent* timerEvent)
{
    if (timerEvent->timerId() == d->timerId)
    {
        // Pump helix event loop
        if(d && d->engine) {
            d->engine->EventOccurred((_HXxEvent*)0);
        }
    }
}
// }}}

// {{{ QMediaSessionBuilder
QString HelixEngine::type() const
{
    return "com.trolltech.qtopia.uri";
}

QMediaSessionBuilder::Attributes const& HelixEngine::attributes() const
{
    return d->builderAttributes;
}

QMediaServerSession* HelixEngine::createSession(QMediaSessionRequest sessionRequest)
{
    QMediaServerSession*    mediaSession = 0;
    QUrl                    url;

    sessionRequest >> url;

    if (url.isValid()) {
        mediaSession = new HelixSession(d->engine, sessionRequest.id(), url.toString());

        if (sessions.isEmpty())   // Startup event loop
            d->timerId = startTimer(300);

        HelixSession *s = static_cast<HelixSession*>(mediaSession);
        sessions.append(s);
    }

    return mediaSession;
}

void HelixEngine::destroySession(QMediaServerSession* serverSession)
{
    HelixSession *s = static_cast<HelixSession*>(serverSession);
    sessions.removeAll(s);
    delete serverSession;

    if (sessions.isEmpty()) {
        killTimer(d->timerId);  // Shut down timer straight away
        d->timerId = -1;
    }
}
// }}}

}   // ns qtopia_helix






