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

#include "environmentsetuptask.h"

#include <QTextCodec>
#include <QSettings>
#include <QString>
#include <QPixmapCache>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#include <qscreendriverfactory_qws.h>
#endif
#include <QRegExp>
#include <stdlib.h>
#include "qtopiaserverapplication.h"
#include <qtopialog.h>
#include <time.h>
#include <QTimeZone>


extern int qws_display_id;

/*!
  \class EnvironmentSetupTask
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The EnvironmentSetupTask class initializes the basic system environment required by Qtopia.

  The EnvironmentSetupTask configures the basic environment variables required
  by Qtopia.  It should execute before QtopiaApplication or any other task in
  the system.

  The EnvironmentSetupTask class provides the \c {EnvironmentSetup} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
void EnvironmentSetupTask::initEnvironment()
{
    int argc = QtopiaServerApplication::argc();
    char **argv = QtopiaServerApplication::argv();

    QSettings config("Trolltech","locale");
    config.beginGroup( "Location" );
    QString tz = config.value( "Timezone", QTimeZone::current().id() ).toString().trimmed();

    setenv( "TZ", tz.toLatin1(), 1 );
    tzset();
    config.endGroup();

    config.beginGroup( "Language" );
    QString lang = config.value( "Language", getenv("LANG") ).toString().trimmed();
    if( lang.isNull() || lang.isEmpty())
        lang = "en_US";
    lang += ".UTF-8";

    setenv( "LANG", lang.toLatin1().constData(), 1 );

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /*
       Figure out what QWS_DISPLAY should be set to.
       The algorithm goes like this:

       1) -display <arg> overrides QWS_DISPLAY, which overrides defaultbuttons.conf::Environment::QWS_DISPLAY

       It's expected that a device would have a defaultbuttons.conf::Environment::QWS_DISPLAY like one of
       these:
       LinuxFb:mmWidth34:mmHeight44:0 -- single screen device
       Multi: LinuxFb:mmHeight57:0 LinxFb:offset=0,320:1 :0  -- multi-screen device
    */

#ifdef Q_WS_QWS
    // Start with QWS_DISPLAY
    QString qws_display = getenv("QWS_DISPLAY");
    // -display overrides QWS_DISPLAY
    for (int i = 1; i<argc; i++) {
        QString arg = argv[i];
        if (arg == "-display") {
            if (++i < argc) {
                qws_display = argv[i];
                break;
            }
        }
    }
    bool addTransformed = false;
    if ( qws_display.isEmpty() ) {
        // fall back to defaultbuttons.conf (doesn't work with QVFb skins but runqtopia figures it out)
        QSettings env(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
        env.beginGroup("Environment");
        qws_display = env.value("QWS_DISPLAY").toString();
        addTransformed = true;
    }

    // final fall back, :0
    if ( qws_display.isEmpty() ) {
        qws_display = ":0";
        addTransformed = true;
    }

    // Let the search for defaultbuttons.conf (below) work for QVFb's defaultbuttons.conf
    QRegExp display(":(\\d+)$");
    if ( display.indexIn( qws_display ) != -1 )
        qws_display_id = QVariant(display.cap(1)).toInt();

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    // Now we prepend Transformed:Rot0: if the transformed driver is available (but not already enabled)
    // This is to prevent the embarassing situation where the rotation app is present but doens't work.
    // Note that this code doens't fire if QWS_DISPLAY is set in the environment or if -display is used.
    if ( addTransformed && qws_display.indexOf("Transformed:") == -1 && QScreenDriverFactory::keys().contains("Transformed") ) {
        if ( qws_display.indexOf(':') != 0 )
            qws_display.prepend(':');
        qws_display.prepend("Transformed:Rot0");
    }
#endif

    qLog(QtopiaServer) << "QWS_DISPLAY" << qws_display;
    setenv( "QWS_DISPLAY", qws_display.toLocal8Bit().constData(), 1 );
#endif

    // We know we'll have lots of cached pixmaps due to App/DocLnks
    QPixmapCache::setCacheLimit(512);

#ifdef Q_WS_QWS
    //Turn off green screen frame buffer init
    QWSServer::setBackground(Qt::NoBrush);
#endif

    // Set other, miscellaneous environment
    QSettings env(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    env.beginGroup("Environment");
    QStringList envKeys = env.childKeys();
    for(int ii = 0; ii < envKeys.count(); ++ii) {
        const QString & key = envKeys.at(ii);
        // QWS_DISPLAY is handled above
        if ( key == "QWS_DISPLAY" ) continue;
        QString value = env.value(key).toString();
        if ( qgetenv(key.toAscii().constData()).count() == 0 )
            setenv(key.toAscii().constData(), value.toAscii().constData(), 1);
    }

    // The default timeout (5 seconds) is too short for Qtopia because the server
    // needs to launch processes while it is starting up (and blocking the event loop).
    // Use a 30 second timeout instead so that the processes do not die prematurely.
    QString qws_connection_timeout = getenv("QWS_CONNECTION_TIMEOUT");
    if ( qws_connection_timeout.isEmpty() )
        setenv("QWS_CONNECTION_TIMEOUT", "30", 1);
}

QTOPIA_STATIC_TASK(EnvironmentSetup, EnvironmentSetupTask::initEnvironment());

