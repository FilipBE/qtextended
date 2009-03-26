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
#include <desktopsettings.h>
#include <trace.h>

#include <QLocale>
#include <QDir>
#include <QDesktopWidget>
#include <QTranslator>
#include <QApplication>
#include <QUuid>

static QString checkLanguage( const QString &language )
{
    TRACE(I18N) << "checkLanguage" << "language" << language;
    QString lang( language );
    QString i18dir = DesktopSettings::installedDir() + "i18n/";

    if ( QDir(i18dir + lang).exists() )
	return lang;

    int i  = lang.indexOf(".");
    if ( i > 0 ) {
	lang = lang.left( i );
	LOG() << "without a dot" << lang;
	if ( QDir(i18dir + lang).exists() )
	    return lang;
    }

    i = lang.indexOf( "_" );
    if ( i > 0 ) {
	lang = lang.left(i);
	LOG() << "without an underscore" << lang;
	if ( QDir(i18dir + lang).exists() )
	    return lang;
    }

    LOG() << "falling back to en";
    return "en_US";
}

// ====================================================================

/*!
  \class DesktopSettings
  \brief The DesktopSettings class maintains settings.
  \mainclass

  The DesktopSettings class has a number of global options. It is also used as a replacement
  for QSettings, providing various helper functions.

  DesktopSettings will cause settings to be located in Trolltech/Qt Extended Sync Agent/...
  On Windows, this can be found in the registry: HKCU\\Software\\Trolltech\\Qt Extended Sync Agent\\...
  On Mac OS X, this can be found in $HOME/Library/Preferences/Qt Extended Sync Agent.plist
  On Unix, this can be found in $HOME/.config/Trolltech/Qt Extended Sync Agent/...
*/

/*!
  Create a DesktopSettings instance. If the \a section is specified, settings will be stored there.
*/
DesktopSettings::DesktopSettings( const QString &section )
    : QSettings("Trolltech", "Qtopia Sync Agent")
{
    if ( !section.isEmpty() ) {
        beginGroup( section );
    }
}

/*!
  Destructor.
*/
DesktopSettings::~DesktopSettings()
{
}

/*!
  \internal
*/
void DesktopSettings::setDebugMode( bool debugMode )
{
    DesktopSettings settings("settings");
    settings.setValue( "debugmode", debugMode );
}

/*!
  Return the debugMode global option.
*/
bool DesktopSettings::debugMode()
{
    DesktopSettings settings("settings");
    return settings.value( "debugmode", false ).toBool();
}

/*!
  \internal
*/
void DesktopSettings::setPromptOnQuit( bool promptOnQuit )
{
    DesktopSettings settings("settings");
    settings.setValue( "promptonquit", promptOnQuit );
}

/*!
  Return the promptOnQuit global option.
*/
bool DesktopSettings::promptOnQuit()
{
    DesktopSettings settings("settings");
    return settings.value( "promptonquit", true ).toBool();
}

QString DesktopSettings::mCurrentPlugin;

/*!
  \internal
*/
void DesktopSettings::setCurrentPlugin( const QString &plugin )
{
    mCurrentPlugin = plugin;
}

/*!
  Return the current plugin. Note that this may not be accurate when using multi-window mode.
*/
QString DesktopSettings::currentPlugin()
{
    return mCurrentPlugin;
}

/*!
  \internal
*/
void DesktopSettings::setDefaultPlugin( const QString &plugin )
{
    DesktopSettings settings("settings");
    settings.setValue( "defaultplugin", plugin );
}

/*!
  Return the default plugin.
*/
QString DesktopSettings::defaultPlugin()
{
    DesktopSettings settings("settings");
    return settings.value( "defaultplugin", "" ).toString();
}

QString DesktopSettings::mInstalledDir;

/*!
  \internal
*/
void DesktopSettings::setInstalledDir( const QString &dir )
{
    mInstalledDir = dir;

    if ( mInstalledDir[(int)mInstalledDir.length() - 1] != QDir::separator() ) {
        mInstalledDir.append( QDir::separator() );
    }
}

/*!
  Return the location that Qt Extended Sync Agent has been installed to.
*/
QString DesktopSettings::installedDir()
{
    return mInstalledDir;
}

/*!
  Return the system language (or a derivitave that exists, falling back to english)
*/
QString DesktopSettings::systemLanguage()
{
    return checkLanguage( QLocale::system().name() );
}

QStringList DesktopSettings::mLanguages;

/*!
  Return a bunch of related languages. For example: en_US, en.
*/
QStringList DesktopSettings::languages()
{
    static bool firstTime = true;
    if ( firstTime ) {
	firstTime = false;

        TRACE(I18N) << "DesktopSettings::languages";
	DesktopSettings settings("settings");
	QString i18dir = mInstalledDir + "i18n/";
	QString lang = settings.value( "language" ).toString();
	LOG() << "from the settings" << lang;
	if (lang.isEmpty()) {
	    lang = systemLanguage();
	    LOG() << "from the system" << lang;
	}

	int i  = lang.indexOf(".");
	if ( i > 0 ) {
	    lang = lang.left( i );
	    LOG() << "without a dot" << lang;
	}
	mLanguages << lang;

	i = lang.indexOf( "_" );
	if ( i > 0 ) {
	    lang = lang.left(i);
	    LOG() << "without an underscore" << lang;
	    mLanguages << lang;
	}

	if ( mLanguages.count() == 0 ) {
	    LOG() << "falling back to en";
	    mLanguages << "en_US" << "en";
	}
    }

    return mLanguages;
}

/*!
  Return the location that Qt Extended Sync Agent can write files to.
  This function exists because the location varies for different platforms.
*/
QString DesktopSettings::homePath()
{
    QString r = QDir::homePath();
#if defined(Q_WS_MACX)
    r += "/Library/Application Support/Qtopia Sync Agent/";
#elif defined(Q_OS_WIN32)
    r += "/Application Data/Qtopia Sync Agent/";
#else
    r += "/.qtopiadesktop/";
#endif
    return r;
}

/*!
  Load translations from \a file. The translations will be unloaded when \a parent is destroyed.
*/
void DesktopSettings::loadTranslations( const QString &file, QObject *parent )
{
    TRACE(I18N) << "DesktopSettings::loadTranslations" << "file" << file;
    foreach ( const QString &lang, DesktopSettings::languages() ) {
        QString tfn = QString("%1i18n/%2/%3.qm").arg(DesktopSettings::installedDir()).arg(lang).arg(file);
        if ( !QFile::exists(tfn) ) {
            LOG() << "Translation file" << QString("%1.qm").arg(file) << "for language" << lang << "is missing." << QDir::toNativeSeparators(QDir::cleanPath(tfn));
            continue;
        }
        QTranslator *trans = new QTranslator( parent );
        if ( trans->load( tfn ) ) {
            LOG() << "Loaded translation for file" << file << "lang" << lang;
            qApp->installTranslator( trans );
        } else {
            WARNING() << "Could not load translation for file" << file << "lang" << lang;
            delete trans;
        }
    }
}

/*!
  Returns a unique string for this device.
*/
QString DesktopSettings::deviceId()
{
    static QString id;
    if ( id.isEmpty() ) {
        DesktopSettings settings("settings");
        id = settings.value("deviceId").toString();
        if ( id.isEmpty() ) {
            id = QUuid::createUuid();
            settings.setValue("deviceId", id);
        }
    }
    return id;
}

