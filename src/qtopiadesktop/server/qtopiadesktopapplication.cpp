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
#include "qtopiadesktopapplication.h"
#include "mainwindow.h"
#include "desktopwrapper.h"
#include "settingsdialog.h"
#include "trayicon.h"
#include "pluginmanager.h"
#include "qtopiaresource_p.h"
#include "connectionmanager.h"
#include "syncmanager.h"

#include <qdplugin.h>
#include <private/qdplugin_p.h>

#include <desktopsettings.h>
#include <qtopiadesktoplog.h>
#include <trace.h>
#include <QCopEnvelope>
#include <qcopchannel_qd.h>

#include <QtGui>

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif

extern "C" {
#include <stdlib.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif
}

class SingleWindow : public QStackedWidget
{
    Q_OBJECT
public:
    SingleWindow() : QStackedWidget()
    {
        loadGeometry();
        connect( this, SIGNAL(currentChanged(int)), this, SLOT(refreshWindow(int)) );
    }

    void loadGeometry()
    {
        // Set the geometry (can be overridden from the commandline)
        DesktopSettings settings( "singlewindow" );
        QRect desk = qApp->desktop()->availableGeometry();
        QRect r = settings.value( "geometry" ).toRect();
        if ( !r.isValid() ) {
            int w = qMin(QD_DEFAULT_WITDH,desk.width());
            int h = qMin(QD_DEFAULT_HEIGHT,desk.height());
            int x = r.x()+(desk.width()-w)/2;
            int y = r.y()+(desk.height()-h)/2;
            r = QRect( x, y, w, h );
        }
        setGeometry( r );
        QPoint p = settings.value( "pos", r.topLeft() ).toPoint();
        move( p );
    }

private slots:
    void refreshWindow( int index )
    {
        if (index == -1)
            return;
        TRACE(QDA) << "SingleWindow::refreshWindow";
        MainWindow *w = (MainWindow*)widget(index);
        setWindowTitle( w->windowTitle() );
#ifndef Q_OS_MACX
        setWindowIcon( w->icon() );
#endif
    }

    void turnOnUpdates()
    {
        TRACE(QDA) << "SingleWindow::turnOnUpdates";
        setUpdatesEnabled( true );
    }

private:
    void closeEvent( QCloseEvent *e )
    {
        TRACE(QDA) << "SingleWindow::closeEvent";
        DesktopSettings settings( "singlewindow" );
        settings.setValue( "geometry", geometry() );
        settings.setValue( "pos", pos() );
        QStackedWidget::closeEvent( e );
    }
};

// ==========================================================================

QtopiaDesktopApplication::QtopiaDesktopApplication( int &argc, char **argv )
    : QtSingleApplication( "09F911029D74E35BD84156C5635688C0", argc, argv ),
    splash( 0 ),
    single( 0 ),
    settingsDialog( 0 ),
    connectionManager( 0 ),
    syncManager( 0 )
{
    // Allow Qtopia resources to be located
    setApplicationName("qtopiadesktop");
    static QFileResourceFileEngineHandler *fileengine = 0;
    if ( !fileengine ) {
        fileengine = new QFileResourceFileEngineHandler();
    }

    // Setup the installedDir based on the platform and location of the executable.
    // This can be overridden by a config value.
    QString qpedir = qApp->applicationDirPath();
#if defined(Q_WS_MACX)
    qpedir = QDir(qpedir + "/../Resources").absolutePath();
#elif !defined(Q_OS_WIN32)
    qpedir = QDir(qpedir + "/../qtopiadesktop").absolutePath();
#endif
    qpedir = QDir::cleanPath(qpedir);
    //qLog(UI) << "installedDir is" << qpedir;
    DesktopSettings::setInstalledDir( qpedir );

    // Ensure our data dir exists...
    QString homeDir = DesktopSettings::homePath();
    QDir().mkpath( homeDir );
}

QtopiaDesktopApplication::~QtopiaDesktopApplication()
{
    TRACE(QDA) << "QtopiaDesktopApplication::~QtopiaDesktopApplication";
    delete connectionManager;
    delete qdPluginManager();
    if ( single )
        delete single;
    delete settingsDialog;
}

void QtopiaDesktopApplication::initialize()
{
    // The uninstaller calls qtopiadesktop.exe --quit to quit the running instance.
    // If there was no running instance we get here. Nothing has happened yet so we
    // can just bail out.
    if ( arguments().contains("--quit") )
        ::exit( 0 );

    QtSingleApplication::initialize( false );
    connect( this, SIGNAL(messageReceived(QString)), this, SLOT(appMessage(QString)) );

    debugMode = DEBUG_NOT_SET;
#if defined(Q_WS_MACX) || defined(DEBUG)
    forkProcess = false;
#else
    forkProcess = true;
#endif
    showSplash = true;
    syncOnly = false;
    trayMode = false;
    waitForDebugger = false;
    safeMode = false;

    processArgs( arguments() );

    if ( waitForDebugger )
        QMessageBox::information(0, "Waiting for debugger", "Attach the debugger now.", QMessageBox::Ok);

    init();
}

void QtopiaDesktopApplication::processArgs( const QStringList &args )
{
    foreach ( const QString &arg, args ) {
        if ( arg == "--debug" )
            debugMode = DEBUG_ON;
        else if ( arg == "--nodebug" )
            debugMode = DEBUG_OFF;
        else if ( arg == "--fork" )
            forkProcess = true;
        else if ( arg == "--nofork" )
            forkProcess = false;
        else if ( arg == "--splash" )
            showSplash = true;
        else if ( arg == "--nosplash" )
            showSplash = false;
        else if ( arg == "--dosync" )
            syncOnly = true;
        else if ( arg == "--tray" )
            trayMode = true;
        else if ( arg == "--wait" )
            waitForDebugger = true;
        else if ( arg == "--safe" )
            safeMode = true;
        else if ( arg == "--help" ) {
            // This should really be translated but very few people are going to see it so I don't care.
            QString usage = "Usage:  "QTOPIA_TARGET" [options]\n\n"
                "--help: display this help.\n"
                "--debug: force debug mode on.\n"
                "--nodebug: force debug mode off.\n"
                "--fork: force forking on.\n"
                "--nodebug: force forking off.\n"
                "--splash: force splash screen on.\n"
                "--nosplash: force splash screen off.\n"
                "--dosync: synchronize with a device.\n"
                "--tray: don't show the main window.\n"
                "--wait: Delay at startup so that a debugger can attach.\n"
                "--safe: Force safe mode (No plugins are loaded).\n"
                "\n";
            qWarning() << usage.toLocal8Bit().constData();
            ::exit( 2 );
        }
    }
}

void QtopiaDesktopApplication::init()
{
    TRACE(QDA) << "QtopiaDesktopApplication::init";
#ifndef Q_OS_WIN32
    if ( forkProcess ) {
        switch( ::fork() ) {
            case 0:
                // we are the child
                break;
            case -1:
                // an error occurred so the child couldn't be created
                // continue without forking
                WARNING() << "Could not fork! Continuing anyway.";
                break;
            default:
                // the child was created
                ::exit(0);
        }
    }
#endif

    if ( debugMode != DEBUG_NOT_SET )
	DesktopSettings::setDebugMode( debugMode == DEBUG_ON );

    // Check the installed dir
    DesktopSettings settings;
    QString key = "/qtopiadir";
    if ( DesktopSettings::debugMode() )
        key += "-debug"; // No tr
    key += QString("-%1").arg(VERSION);
    QString dir = settings.value( key ).toString();
    if ( dir.isNull() )
        dir = DesktopSettings::installedDir();
    DesktopSettings::setInstalledDir( dir );

    if ( showSplash ) {
        QPixmap pm = QPixmap(":image/splashscreen");
        QPainter p( &pm );
        p.setPen( Qt::white );
        QFont f;
        f.setPixelSize( 24 );
        p.setFont( f );
        p.drawText( QRect(20, 197, 160, 59), Qt::AlignHCenter|Qt::AlignTop, tr("Sync Agent") );
	splash = new QSplashScreen( pm );
        splash->setAttribute( Qt::WA_DeleteOnClose, true );
	splash->show();
    }

    // do some "extra" translations first...
    DesktopSettings::loadTranslations("qt", this);
    DesktopSettings::loadTranslations("server", this);

    QTimer::singleShot( 0, this, SLOT(reinit()) );
}

void QtopiaDesktopApplication::reinit()
{
#ifndef Q_OS_MACX
    qApp->setWindowIcon( QPixmap(":image/appicon") );
#endif
    trayIcon = new TrayIcon;
    DesktopSettings settings( "settings" );
    setQuitOnLastWindowClosed( false );
    connect( trayIcon, SIGNAL(clicked()), this, SLOT(trayIconClicked()) );
    connect( trayIcon, SIGNAL(sync()), this, SLOT(syncall()) );
    connect( trayIcon, SIGNAL(quit()), this, SLOT(filequit()) );

    setSingle( settings.value( "singlewindow", true ).toBool() );

    settingsDialog = new SettingsDialog;

    //splash->showMessage( tr("Setup Plugins") );
    setupPlugins();

    QWidget *toplevel;
    QDAppPlugin *currentPlugin;

    findPluginToShow( toplevel, currentPlugin );

    if ( !syncOnly && !trayMode )
	toplevel->show();

    showPlugin( currentPlugin );

#if 0
    if ( syncOnly ) {
        appMessage("");
    }
#endif

    if ( showSplash )
        splash->finish( toplevel );

    connectionManager = new ConnectionManager;
    connect( connectionManager, SIGNAL(setConnectionState(int)), this, SIGNAL(setConnectionState(int)) );

    if ( safeMode ) {
        QCopEnvelope e( "QD/Connection", "setHint(QString,QString)" );
        e << tr("Qtopia Sync Agent was started in safe mode. "
                "You should choose which plugins to load from the settings dialog "
                "and restart.")
          << QString("safe mode");
        QMessageBox::information( 0, tr("Safe Mode"), QString("<qt>%1").arg(
                tr("Qtopia Sync Agent was started in safe mode. "
                   "You should choose which plugins to load from the settings dialog "
                   "and restart.")) );
    }

    QCopChannel *chan = new QCopChannel( "QD/Server", this );
    connect( chan, SIGNAL(received(QString,QByteArray)), this, SLOT(serverMessage(QString,QByteArray)) );
}

static void raiseWindow( QWidget *window )
{
    // make sure we've got the toplevel window
    QWidget *parent = window;
    while ( parent->parentWidget() )
        parent = parent->parentWidget();

#ifdef Q_WS_X11
    bool fixGeometry = false;
    if ( parent->isVisible() ) {
        // By hiding and then showing the window, we bring it to the current virtual desktop
        // Unfortunatley this causes flicker if the window is already on the current desktop
        // but I'm not sure how to avoid that.
        // It looks like I should be able to compare the X11 root window's _NET_CURRENT_DESKTOP
        // property with the window's _NET_WM_DESKTOP property...
        QPoint pos = parent->pos();
        parent->hide();
        parent->move( pos );
    } else {
        // If the window is not visible, it's going to be hit by a bug in Qt/X11
        // that causes windows to move when you call close();show();
        // Luckily we save the geometry of the important windows when they close
        // so we can just restore it to work around the bug.
        fixGeometry = true;
    }
#endif

    // unminimize
    if ( parent->isMinimized() )
        parent->showNormal();
    else
        parent->show();

#ifdef Q_WS_X11
    if ( fixGeometry ) {
        SingleWindow *sw = qobject_cast<SingleWindow*>(parent);
        if ( sw )
            sw->loadGeometry();
        else {
            MainWindow *mw = qobject_cast<MainWindow*>(parent);
            if ( mw )
                mw->loadGeometry();
        }
    }
#endif

    // activate and raise it
    parent->activateWindow();
}

void QtopiaDesktopApplication::appMessage( const QString &message )
{
    QStringList args = message.split( "!!!" );
    // The uninstaller calls qtopiadesktop.exe --quit to quit the running instance.
    // Just do a normal quit action.
    if ( args.contains("--quit") ) {
        filequit();
    } else {
        processArgs( args );
        trayIconClicked();
    }
}

void QtopiaDesktopApplication::showPlugin( QDAppPlugin *plugin )
{
    TRACE(QDA) << "QtopiaDesktopApplication::showPlugin" << plugin;
    if ( single )
        single->setUpdatesEnabled( false );
    emit showingPlugin( plugin );
    MainWindow *mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
    if ( single ) {
        if ( single->currentIndex() != single->indexOf( mainWindow ) ) {
            single->setCurrentIndex( single->indexOf( mainWindow ) );
            DesktopSettings::setDefaultPlugin( plugin->id() );
        }
    } else {
        Q_ASSERT( mainWindow );
        mainWindow->show();
        mainWindow->raise();
    }
    QTimer::singleShot( 0, single, SLOT(turnOnUpdates()) );
}

void QtopiaDesktopApplication::setupPlugins()
{
    TRACE(QDA) << "QtopiaDesktopApplication::setupPlugins";
    DesktopSettings settings("settings");
    // Check for a previous startup crash
    if ( !safeMode ) safeMode = settings.value("SafeMode", false).toBool();
    // If setupPlugins() crashes, startup in safe mode
    settings.setValue("SafeMode", true);
    settings.sync();
    qdPluginManager()->setupPlugins( safeMode );
    foreach ( QDPlugin *plugin, qdPluginManager()->plugins() ) {
        TRACE(QDA) << "QDPlugin::init" << "for plugin" << plugin->displayName();
        initializePlugin( plugin );
    }
    foreach ( QDAppPlugin *p, qdPluginManager()->appPlugins() ) {
        QDAppPluginData *dta = qdPluginManager()->pluginData(p);
        QWidget *appWidget = p->initApp();
        if ( appWidget ) {
            dta->appWidget = appWidget;
            MainWindow *mainWindow = new MainWindow( p, appWidget );
            dta->mainWindow = mainWindow;
            if ( single )
                single->addWidget( mainWindow );
            mainWindow->slotSetSingle( single );
            connect( mainWindow, SIGNAL(setSingle(bool)), this, SLOT(setSingle(bool)) );
            connect( mainWindow, SIGNAL(closing(QDAppPlugin*)), this, SLOT(windowClosing(QDAppPlugin*)) );
            connect( mainWindow, SIGNAL(settings()), settingsDialog, SLOT(show()) );
            connect( mainWindow, SIGNAL(quit()), this, SLOT(filequit()) );
            connect( mainWindow, SIGNAL(syncall()), this, SLOT(syncall()) );
            connect( mainWindow, SIGNAL(syncallslow()), this, SLOT(syncallslow()) );
#if 0
            connect( mainWindow, SIGNAL(import()), this, SLOT() );
            connect( mainWindow, SIGNAL(sync()), this, SLOT() );
            connect( mainWindow, SIGNAL(backuprestore()), this, SLOT() );
            connect( mainWindow, SIGNAL(manual()), this, SLOT() );
            connect( mainWindow, SIGNAL(about()), this, SLOT() );
#endif
        }
        dta->settingsWidget = p->initSettings();
    }
    // It looks like we didn't crash. Turn off safe mode.
    settings.setValue("SafeMode", false);
    settings.sync();
    emit pluginsChanged();
}

void QtopiaDesktopApplication::setSingle( bool singleWindow )
{
#ifdef Q_OS_MACX
    // FIXME this breaks the menu bar so force it off
    singleWindow = false;
#endif

    DesktopSettings settings( "settings" );
    settings.setValue( "singlewindow", singleWindow );

    if ( singleWindow && !single ) {
        MainWindow *mw = qobject_cast<MainWindow*>(QApplication::activeWindow());
        QDAppPlugin *cp = 0;
        single = new SingleWindow;
        foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
            MainWindow *mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
            if ( !mainWindow )
                continue;
            if ( mainWindow == mw ) {
                cp = plugin;
                QPoint pos = mainWindow->pos();
                mainWindow->hide();
                single->setGeometry( mainWindow->geometry() );
                single->move( pos );
                single->show();
            }
            mainWindow->slotSetSingle( singleWindow );
            single->addWidget( mainWindow );
        }
        if ( cp )
            showPlugin( cp );
    } else if ( single ) {
        QPoint pos = single->pos();
        single->hide();
        QString defaultPlugin = DesktopSettings::defaultPlugin();
        foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
            MainWindow *mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
            if ( !mainWindow )
                continue;
            mainWindow->slotSetSingle( singleWindow );
            mainWindow->hide();
            mainWindow->setParent( 0 );
#ifndef Q_OS_MACX
            // the icon doesn't stick properly so we need to set it now
            // that the window is a toplevel window
            mainWindow->setWindowIcon( plugin->icon() );
#endif
            if ( plugin->id() == defaultPlugin ) {
                mainWindow->setGeometry( single->geometry() );
                mainWindow->move( pos );
                mainWindow->show();
            } else {
                mainWindow->hide();
                mainWindow->loadGeometry();
            }
        }
        delete single;
        single = 0;
    }
}

void QtopiaDesktopApplication::windowClosing( QDAppPlugin *plugin )
{
    if ( single )
        return;
    bool last = true;
    foreach ( QDAppPlugin *p, qdPluginManager()->appPlugins() ) {
        MainWindow *mainWindow = qdPluginManager()->pluginData(p)->mainWindow;
        if ( !mainWindow )
            continue;
        if ( p != plugin && mainWindow->isVisible() ) {
            last = false;
            break;
        }
    }
    if ( last )
        DesktopSettings::setDefaultPlugin( plugin->id() );
}

void QtopiaDesktopApplication::trayIconClicked()
{
    TRACE(QDA) << "QtopiaDesktopApplication::trayIconClicked";
    if ( single && single->isVisible() ) {
        raiseWindow( single );
    } else {
        bool found = false;
        foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
            MainWindow *mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
            if ( !mainWindow )
                continue;
            if ( mainWindow->isVisible() ) {
                raiseWindow( mainWindow );
                found = true;
            }
        }
        if ( !found ) {
            QWidget *toplevel;
            QDAppPlugin *currentPlugin;
            findPluginToShow( toplevel, currentPlugin );
            raiseWindow( toplevel );
            showPlugin( currentPlugin );
        }
    }
    if ( settingsDialog && settingsDialog->isVisible() ) {
        raiseWindow(settingsDialog);
    }
}

void QtopiaDesktopApplication::findPluginToShow( QWidget *&toplevel, QDAppPlugin *&currentPlugin )
{
    TRACE(QDA) << "QtopiaDesktopApplication::findPluginToShow";
    MainWindow *mainWindow = 0;
    currentPlugin = 0;
    QString defaultPlugin = DesktopSettings::defaultPlugin();
    foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
        if ( plugin->id() == defaultPlugin ) {
            currentPlugin = plugin;
            mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
            break;
        }
    }
    if ( !mainWindow ) {
        foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
            if ( qdPluginManager()->pluginData(plugin)->mainWindow ) {
                currentPlugin = plugin;
                mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
                break;
            }
        }
    }
    Q_ASSERT(mainWindow);
    Q_ASSERT(currentPlugin);

    toplevel = mainWindow;
    if ( single )
        toplevel = single;
}

void QtopiaDesktopApplication::filequit()
{
    TRACE(QDA) << "QtopiaDesktopApplication::filequit";
    // If a sync is in progress, abort it.
    if ( syncManager && syncManager->isVisible() ) {
        syncManager->abort();
    }
    // If the settings dialog is up, cancel it.
    if ( settingsDialog && settingsDialog->isVisible() ) {
        settingsDialog->reject();
    }
    // Kill the connection.
    connectionManager->stop();
    if ( single )
        single->setUpdatesEnabled( false );
    // Close each of the windows (so that positions can be saved)
    foreach ( QDAppPlugin *plugin, qdPluginManager()->appPlugins() ) {
        MainWindow *mainWindow = qdPluginManager()->pluginData(plugin)->mainWindow;
        if ( mainWindow )
            mainWindow->close();
    }
    // Close the single window.
    if ( single )
        single->close();
    // Remove the tray icon.
    trayIcon->hide();
    qApp->quit();
}

void QtopiaDesktopApplication::syncall()
{
    TRACE(QDA) << "QtopiaDesktopApplication::syncall";
    if ( syncManager ) {
        LOG() << "Can't call syncall() while a sync is in progress!";
        return;
    }
    { QCopEnvelope e("QPE/QDSync", "syncStart()"); }
    syncManager = new SyncManager;
    syncManager->init();
    bool ok = syncManager->exec();
    LOG() << "Sync" << (ok?"completed":"aborted") << "with" << syncManager->errors() << (syncManager->errors()==1?"error":"errors");
    delete syncManager;
    syncManager = 0;
    { QCopEnvelope e("QPE/QDSync", "syncEnd()"); }
}

void QtopiaDesktopApplication::syncallslow()
{
    TRACE(QDA) << "QtopiaDesktopApplication::syncallslow";
    if ( syncManager ) {
        LOG() << "Can't call syncall() while a sync is in progress!";
        return;
    }
    DesktopSettings settings("settings");
    settings.setValue("ForceSlowSync", true);
    settings.sync();
    syncall();
    settings.setValue("ForceSlowSync", false);
}

void QtopiaDesktopApplication::serverMessage( const QString &message, const QByteArray &data )
{
    TRACE(QDA) << "QtopiaDesktopApplication::serverMessage" << "message" << message << "data" << data;
    if ( connectionManager->state() != QDConPlugin::Connected ) {
        LOG() << "Ignoring QD/Server message" << message << "because connection state is" << connectionManager->state();
        return;
    }
    if ( message == "startSync()" ) {
        syncall();
    } else if ( message == "cancelSync()" ) {
        if ( syncManager )
            syncManager->abort();
    }
}

#ifdef Q_OS_MACX
bool QtopiaDesktopApplication::macEventFilter( EventHandlerCallRef, EventRef event )
{
    // This should be done with Carbon Event constants but my headers
    // don't seem to match what I'm getting from the system.
    // This detects clicks on the dock icon and shows the main window.
    if ( GetEventClass( event ) == 1701867619 &&
            GetEventKind( event ) == 1 ) {
        trayIconClicked();
    }
    return false;
}
#endif

void QtopiaDesktopApplication::initializePlugin( QDPlugin *plugin )
{
    qdPluginManager()->pluginData(plugin)->center = new DesktopWrapper( plugin );
    plugin->init();
    plugin->internal_init();
}

#include "qtopiadesktopapplication.moc"
