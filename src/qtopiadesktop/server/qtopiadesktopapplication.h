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
#ifndef QTOPIADESKTOPAPPLICATION_H
#define QTOPIADESKTOPAPPLICATION_H

#include <qdplugindefs.h>

#include "qtsingleapplication.h"

class MainWindow;
class SettingsDialog;
class TrayIcon;
class ConnectionManager;
class SyncManager;

class QStringList;
class QSplashScreen;
class QStackedWidget;
class QMessageBox;
class QLabel;

class QtopiaDesktopApplication : public QtSingleApplication
{
    friend class DesktopWrapper;
    friend class PluginManager;

    Q_OBJECT
public:
    QtopiaDesktopApplication( int &argc, char **argv );
    ~QtopiaDesktopApplication();

    void initialize();
    void initializePlugin( QDPlugin *p );

signals:
    void setConnectionState( int state );
    void pluginsChanged();
    void showingPlugin( QDAppPlugin *plugin );

private slots:
    void appMessage( const QString &message );
    void reinit();
    void showPlugin( QDAppPlugin *plugin );
    void setSingle( bool singleWindow );
    void windowClosing( QDAppPlugin *plugin );
    void trayIconClicked();
    void filequit();
    void syncall();
    void syncallslow();
    void serverMessage( const QString &message, const QByteArray &data );

private:
    void processArgs( const QStringList &args );
    void init();
    void setupPlugins();
    void findPluginToShow( QWidget *&toplevel, QDAppPlugin *&currentPlugin );
#ifdef Q_OS_MACX
    bool macEventFilter( EventHandlerCallRef, EventRef );
#endif

    enum debug_t { DEBUG_NOT_SET, DEBUG_ON, DEBUG_OFF };
    debug_t debugMode;
    bool syncOnly;
    bool trayMode;
    bool showSplash;
    bool forkProcess;
    bool waitForDebugger;
    bool safeMode;
    QSplashScreen *splash;
    QStackedWidget *single;
    SettingsDialog *settingsDialog;
    TrayIcon *trayIcon;
    ConnectionManager *connectionManager;
    SyncManager *syncManager;
};

#endif
