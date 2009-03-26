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
#include "mainwindow.h"
#include "connectionstatuswidget.h"
#include "statusbar.h"
#include "logwindow.h"
#include "pluginchooser.h"

#include <qdplugin.h>

#include <qcopchannel_qd.h>
#include <desktopsettings.h>

#include <qdebug.h>
#include <QProgressBar>
#include <QDesktopWidget>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QLabel>
#include <QToolTip>
#include <QMenuBar>
#include <QToolBar>
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow( QDAppPlugin *_plugin, QWidget *appWidget, QWidget *parent )
    : QMainWindow( parent ),
    plugin( _plugin ),
    pluginChooser( 0 )
{
    loadGeometry();
#ifndef Q_OS_MACX
    if ( !parent )
        setWindowIcon( plugin->icon() );
#endif
    if ( DesktopSettings::debugMode() ) {
        setWindowTitle( QString("%1 [DEBUG MODE]").arg(tr("%1 - Qtopia Sync Agent", "1=plugin name").arg(plugin->displayName())) );
    } else {
        setWindowTitle( tr("%1 - Qtopia Sync Agent", "1=plugin name").arg(plugin->displayName()) );
    }

    QWidget *central = new QWidget;
    setCentralWidget( central );
    QVBoxLayout *vbox = new QVBoxLayout( central );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );

    Q_ASSERT(appWidget);
    vbox->addWidget( appWidget, 1 );
    // If the appWidget is a QMainWindow, steal it's menu bar and toolbars
    if ( QMainWindow *mw = qobject_cast<QMainWindow*>(appWidget) ) {
        stealMenuAndToolbars( mw );
    }
    setupMenuAndToolbars();

    statusBar = new StatusBar;
    vbox->addWidget( statusBar );

    pluginChooser = new PluginChooser;
    pluginChooser->setWindowMenu( windowMenu );

    // A progress bar (is it used?)
    //mProgressBar = new QProgressBar;
    //mProgressBar->setMaximumWidth( 150 );

    // The connection status icon
    mConnectionStatusIcon = new ConnectionStatusWidget;

    statusBar->addWidget( pluginChooser, 0, true );
    //statusBar->addWidget( mProgressBar, 0, true );
    statusBar->addWidget( mConnectionStatusIcon, 0, true );

    // The log window keeps track of messages that the user has seen
    connect( statusBar, SIGNAL(clicked()), LogWindow::getInstance(), SLOT(show()) );

    QCopChannel *status = new QCopChannel( "QD/Status", this );
    connect( status, SIGNAL(received(QString,QByteArray)), this, SLOT(statusMessage(QString,QByteArray)) );
}

MainWindow::~MainWindow()
{
    //qDebug() << "~MainWindow" << plugin;
}

void MainWindow::statusMessage( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "message(QString)" ) {
        QString msg;
        stream >> msg;
        statusBar->showMessage( msg, 5000 );
    }
}

void MainWindow::stealMenuAndToolbars( QMainWindow *mw )
{
    QMenuBar *mb = mw->menuBar();
    mb->setParent( this );
    setMenuBar( mb );
    foreach( QObject *object, mw->children() ) {
        if ( QToolBar *toolbar = qobject_cast<QToolBar*>(object) ) {
            mw->removeToolBar( toolbar );
            toolbar->setParent( this );
            addToolBar( toolbar );
            toolbar->setMovable( false );
        }
    }
}

void MainWindow::setupMenuAndToolbars()
{
    QString pluginName = plugin->displayName();

    // Make sure certain items exist in the menu
    QMenuBar *menubar = menuBar();
    QMenu *firstMenu = 0;
    QMenu *fileMenu = 0;
    QMenu *toolsMenu = 0;
    windowMenu = 0;
    QMenu *helpMenu = 0;
    //QAction *action = 0;

    // If our menus exist, just add our items to them
    foreach ( QObject *object, menubar->children() ) {
        if ( QMenu *menu = qobject_cast<QMenu*>(object) ) {
            if ( !firstMenu )
                firstMenu = menu;
            if ( !fileMenu && menu->title().startsWith( tr("File"), Qt::CaseInsensitive ) ) {
                fileMenu = menu;
            }
            if ( !toolsMenu && menu->title().startsWith( tr("Tools"), Qt::CaseInsensitive ) ) {
                toolsMenu = menu;
            }
            if ( !windowMenu && menu->title().startsWith( tr("Window"), Qt::CaseInsensitive ) ) {
                windowMenu = menu;
            }
            if ( !helpMenu && menu->title().startsWith( tr("Help"), Qt::CaseInsensitive ) ) {
                helpMenu = menu;
            }
        }
    }
    if ( !fileMenu ) {
        fileMenu = menubar->addMenu( tr("&File") );
        if ( firstMenu )
            menubar->insertMenu( firstMenu->menuAction(), fileMenu );
    } else {
        fileMenu->addSeparator();
        fileMenu->setTitle( tr("&File") );
    }
    fileMenu->addAction( tr("&Close Window"), this, SLOT(close()),
            QKeySequence(tr("Ctrl+W", "close window shortcut")) );
    fileMenu->addAction( tr("&Quit Qtopia Sync Agent"), this, SIGNAL(quit()),
            QKeySequence(tr("Ctrl+Q", "quit Qtopia Sync Agent shortcut")) );

    if ( !toolsMenu ) {
        toolsMenu = menubar->addMenu( tr("&Tools") );
    } else {
        toolsMenu->addSeparator();
        toolsMenu->setTitle( tr("&Tools") );
    }
    // FIXME only do this if there's an import plugin for this app
    toolsMenu->addAction( tr("&Import"), this, SIGNAL(import()) );
    // FIXME only do this is there's a sync plugin for this app
    QAction *syncAction = 0;
    //if ( CenterInterface::syncPluginForApp("myappname") ) {
        syncAction = toolsMenu->addAction( QIcon(":image/sync"),
                tr("&Sync %1", "1=plugin name").arg(pluginName), this, SIGNAL(sync()) );
    //}
    QAction *syncAllAction = toolsMenu->addAction( QIcon(":image/syncall"),
            tr("Sync &All"), this, SIGNAL(syncall()) );
    QAction *slowSyncAction = toolsMenu->addAction( QIcon(":image/syncall"),
            tr("Sync &All (slow)"), this, SIGNAL(syncallslow()) );
    toolsMenu->addAction( tr("&Backup/Restore"), this, SIGNAL(backuprestore()) );
    toolsMenu->addSeparator();
    toolsMenu->addAction( tr("&Settings"), this, SIGNAL(settings()),
            QKeySequence(tr("Ctrl+;", "settings shortcut")) );

    if ( !windowMenu ) {
        windowMenu = menubar->addMenu( tr("&Window") );
    } else {
        windowMenu->addSeparator();
        windowMenu->setTitle( tr("&Window") );
    }
    singleAction = windowMenu->addAction( tr("Single Window Interface") );
    singleAction->setCheckable( true );
    connect( singleAction, SIGNAL(triggered(bool)), this, SIGNAL(setSingle(bool)) );
    windowMenu->addSeparator();

    if ( !helpMenu ) {
        helpMenu = menubar->addMenu( tr("&Help") );
    } else {
        helpMenu->addSeparator();
        helpMenu->setTitle( tr("&Help") );
    }
    helpMenu->addAction( tr("Qtopia Sync Agent &Manual"), this, SIGNAL(manual()) );
    helpMenu->addAction( tr("&About Qtopia Sync Agent"), this, SIGNAL(about()) );
    helpMenu->addAction( tr("About &Qt"), qApp, SLOT(aboutQt()) );

    QToolBar *syncBar = addToolBar( tr("Sync") );
    syncBar->setMovable( false );
    if ( syncAction )
        syncBar->addAction( syncAction );
    syncBar->addAction( syncAllAction );
}

void MainWindow::slotSetSingle( bool single )
{
    singleAction->blockSignals( true );
    singleAction->setChecked( single );
    singleAction->blockSignals( false );
}

void MainWindow::loadGeometry()
{
    // Set the geometry (can be overridden from the commandline)
    DesktopSettings settings( plugin->id() );
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

void MainWindow::closeEvent( QCloseEvent *e )
{
    TRACE(QDA) << "MainWindow::closeEvent";
    if ( singleAction->isChecked() ) {
        if ( parentWidget() ) {
            parentWidget()->close();
            e->ignore();
            return;
        }
    } else {
        DesktopSettings settings( plugin->id() );
        settings.setValue( "geometry", geometry() );
        settings.setValue( "pos", pos() );
        if ( isVisible() )
            emit closing( plugin );
    }
    QMainWindow::closeEvent( e );
}

QIcon MainWindow::icon() const
{
    return plugin->icon();
}

void MainWindow::resizeEvent( QResizeEvent * /*e*/ )
{
    TRACE(QDA) << "MainWindow::resizeEvent";
}

