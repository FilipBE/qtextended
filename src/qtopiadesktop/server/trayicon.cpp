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
#include "trayicon.h"

#include <qdplugin.h>
#include <desktopsettings.h>
#include <qtopiadesktoplog.h>
#include <qcopchannel_qd.h>

#include <QApplication>
#include <QMenu>
#include <QPoint>
#include <qdebug.h>

#ifdef Q_OS_MACX
Q_GUI_EXPORT void qt_mac_set_dock_menu(QMenu *menu);
#endif

#include <QSystemTrayIcon>

class TrayIconPrivate : public QObject
{
    Q_OBJECT
public:
    TrayIconPrivate( TrayIcon *_q ) : QObject( _q ), q( _q ) {}
    ~TrayIconPrivate() {}

public slots:
    void iconActivated( QSystemTrayIcon::ActivationReason reason )
    {
        if ( reason == QSystemTrayIcon::Trigger )
            q->iconClicked();
    }

public:
    TrayIcon *q;
    QSystemTrayIcon *qSysTray;
};

#ifdef Q_OS_WIN
#include <qt_windows.h>
// The icon size is set by the system (it's not constant!)
static QSize iconSize;
#define TRAYICON_SIZE iconSize
#else
#define TRAYICON_SIZE QSize(24,24)
#endif

/*!
  \class TrayIcon
  \brief The TrayIcon class creates an icon in the system tray.
  
  The system tray (also known as the notification area) is a useful location for long-running
  but minimally-interacting applications to display status information.

  This class uses QSystemTrayIcon or a home-grown solution ported from Qt Extended Desktop 2.

  This class also handles the Dock menu on Mac OS X but not clicks on the Dock icon. Those are
  handled by the QtopiaDesktopApplication::macEventFilter() function.
*/

/*!
  Construct a TrayIcon with \a parent as the owning QObject.
  Use the installed() function to determine if it is attached to a system tray.
*/
TrayIcon::TrayIcon( QObject *parent )
    : QObject( parent )
{
#ifdef Q_OS_WIN
    int x = GetSystemMetrics(SM_CXSMICON);
    int y = GetSystemMetrics(SM_CYSMICON);
    iconSize = QSize(x,y);
#endif

    popupMenu = new QMenu();
    openAction = popupMenu->addAction( tr("&Open Qtopia Sync Agent") );
    connect( openAction, SIGNAL(triggered(bool)), this, SIGNAL(clicked()) );
    syncAction = popupMenu->addAction( tr("&Synchronize") );
    connect( syncAction, SIGNAL(triggered(bool)), this, SIGNAL(sync()) );
    QAction *quitAction = popupMenu->addAction( tr("&Quit") );
    connect( quitAction, SIGNAL(triggered(bool)), this, SIGNAL(quit()) );

#ifdef Q_OS_MACX
    qt_mac_set_dock_menu( popupMenu );
#endif

    connect( qApp, SIGNAL(lastWindowClosed()), this, SLOT(lastWindowClosed()) );

    d = new TrayIconPrivate( this );
    d->qSysTray = new QSystemTrayIcon( this );
    d->qSysTray->setContextMenu( popupMenu );
    connect( d->qSysTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), d, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );

    setState( QDConPlugin::Disconnected );
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(setState(int)) );

    QCopChannel *con = new QCopChannel( "QD/Connection", this );
    connect( con, SIGNAL(received(QString,QByteArray)),
             this, SLOT(connectionMessage(QString,QByteArray)) );

    show();
}

/*!
  Destructor.
*/
TrayIcon::~TrayIcon()
{
}

/*!
  Attempt to show the system tray icon.
*/
void TrayIcon::show()
{
    d->qSysTray->show();
}

/*!
  Return true if there was a system tray that could be attached to.
*/
bool TrayIcon::installed()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

/*!
  Hide the system tray icon.
*/
void TrayIcon::hide()
{
    d->qSysTray->hide();
}

/*/!
  Set the icon based on \a state.
*/
void TrayIcon::setState( int state )
{
    mState = state;
    switch ( mState ) {
        case QDConPlugin::Connected:
            setIcon( QPixmap(":image/tray_connected") );
            break;
        case QDConPlugin::Disconnected:
            setIcon( QPixmap(":image/tray_disconnected") );
            break;
        case QDConPlugin::Connecting:
            setIcon( QPixmap(":image/tray_connecting") );
            break;
    }
    syncAction->setEnabled( mState == QDConPlugin::Connected );
}

/*!
  \internal
*/
void TrayIcon::popup( const QPoint &p )
{
    qLog(UI) << "popup at" << p;
    popupMenu->popup( p );
    popupMenu->activateWindow();
}

/*!
  \internal
*/
void TrayIcon::lastWindowClosed()
{
    openAction->setEnabled( true );
    if ( DesktopSettings::debugMode() || !installed() )
        emit quit();
}

/*!
  \internal
*/
void TrayIcon::iconClicked()
{
    openAction->setEnabled( false );
    emit clicked();
}

/*!
  \internal
*/
void TrayIcon::connectionMessage( const QString &message, const QByteArray &data )
{
    QDataStream ds( data );
    if ( message == "setBusy()" ) {
        Q_ASSERT( mState == QDConPlugin::Connected );
        setIcon( QPixmap(":image/tray_busy") );
    } else if ( message == "clearBusy()" ) {
        setState( mState );
    }
}

/*!
  Set the tray icon to \a pm (but resize it appropriately first).
*/
void TrayIcon::setIcon( const QPixmap &pm )
{
    d->qSysTray->setIcon( QIcon( pm ).pixmap( TRAYICON_SIZE ) );
}

/*!
  \fn TrayIcon::clicked()
  This signal is emitted when the tray icon is clicked.
*/

/*!
  \fn TrayIcon::quit()
  This signal is emitted when the quit entry is selected from the tray menu.
*/

/*!
  \fn TrayIcon::sync()
  This signal is emitted when the sync entry is selected from the tray menu.
*/

#include "trayicon.moc"
