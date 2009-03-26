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
#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <qdglobal.h>

#include <qsettings.h>
#include <qstring.h>

class QD_EXPORT DesktopSettings : public QSettings
{
public:
    DesktopSettings( const QString &section = QString() );
    virtual ~DesktopSettings();

    static void setDebugMode( bool debugMode );
    static bool debugMode();

    static void setPromptOnQuit( bool promptOnQuit );
    static bool promptOnQuit();

    static void setCurrentPlugin( const QString &plugin );
    static QString currentPlugin();

    static void setDefaultPlugin( const QString &plugin );
    static QString defaultPlugin();

    static void setInstalledDir( const QString &dir );
    static QString installedDir();

    static QString homePath();
    static QString systemLanguage();
    static QStringList languages();

    static void loadTranslations( const QString &file, QObject *parent );

    static QString deviceId();

private:
    static QStringList mLanguages;
    static QString mInstalledDir;
    static QString mCurrentPlugin;
};

#endif
