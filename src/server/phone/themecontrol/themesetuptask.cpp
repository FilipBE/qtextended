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

#include "themesetuptask.h"
#include "qtopiaserverapplication.h"

/*!
    \class ThemeSetupTask
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer::Task
    \brief The ThemeSetupTask class ensures a theme preselection for Qtopia.

    The selection process happens in the following order:
    \list 1
        \o Use the theme specified in \c $HOME/Settings/Trolltech/qpe.conf using the \c Appearance/Theme key (user selected theme).
        \o Use the theme specified in Qtopia::qtopiaDir()/etc/default/Trolltech/qpe.conf using the \c Appearance/Theme key (Qt Extended default theme).
        \o Pick an arbitrary theme from Qtopia::qtopiaDir()/etc/themes/
        \o Don't use any theme.
    \endlist

    The ThemeSetupTask class provides the \c {ThemeSetup} task.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

static bool themeFileExists(const QString &file)
{
    bool themeExists = false;
    QStringList instPaths = Qtopia::installPaths();
    foreach (QString path, instPaths) {
        QString themeDataPath(path + QLatin1String("etc/themes/") + file);
        if (QFile::exists(themeDataPath)) {
            themeExists = true;
            break;
        }
    }

    return themeExists;
}

/*! \internal */
void ThemeSetupTask::validateTheme()
{
    QSettings config(QLatin1String("Trolltech"),QLatin1String("qpe"));
    config.beginGroup( QLatin1String("Appearance") );
    bool setTheme = false;

    // Start by asking QSettings normally
    QString newTheme = config.value("Theme", QString()).toString();
    newTheme = newTheme.replace(QRegExp("\\.desktop"), ".conf");  // backwards compat

    if ( !newTheme.isEmpty() && !themeFileExists(newTheme)) {
        qWarning() << "Selected theme" << newTheme << "does not exist.";
        // Get the default theme (could be the same as the theme we got before)
        QSettings defSettings( Qtopia::qtopiaDir() + "etc/default/Trolltech/qpe.conf", QSettings::IniFormat );
        defSettings.beginGroup("Appearance");
        newTheme = defSettings.value("Theme", QString()).toString();
        setTheme = true;
    }
    if ( newTheme.isEmpty() ) {
        qWarning("No default theme specified in qpe.conf");
    }

    if ( !newTheme.isEmpty() && !themeFileExists(newTheme) ) {
        qWarning() << "Default theme" << newTheme << "does not exist.";
        newTheme = QString();
    }

    if ( newTheme.isEmpty() ) {
        // Don't bail just yet though because tere might be a
        // .conf file we can use on the system.
        QStringList confFiles = QDir(Qtopia::qtopiaDir() + "etc/themes/").entryList(QStringList() << "*.conf");
        if ( confFiles.count() == 0 ) {
            //Qtopia Platform does not have/require any themes
            qWarning("No theme files found!");
            return;
        }
        // Arbitrary choice here, pick the first entry (if there's more than one).
        newTheme = confFiles[0];
        setTheme = true;
        qWarning() << "Found theme" << newTheme;
    }

    if ( setTheme ) {
        // Since QSettings can't be used to pull out a valid theme, we need to set the theme now.
        // This lets the rest of the code that uses the theme avoid doing all the checks above.
        config.setValue("Theme", newTheme);
    }
}



QTOPIA_STATIC_TASK(ThemeSetup, ThemeSetupTask::validateTheme());
