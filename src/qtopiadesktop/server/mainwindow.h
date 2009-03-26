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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qdplugindefs.h>

#include <QMainWindow>
#include <QMap>

#define QD_DEFAULT_WITDH 600
#define QD_DEFAULT_HEIGHT 500

class QtopiaDesktopApplication;
class ConnectionStatusWidget;
class StatusBar;
class PluginChooser;

class QStackedWidget;
class QProgressBar;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow( QDAppPlugin *plugin, QWidget *appWidget, QWidget *parent = 0 );
    virtual ~MainWindow();

    void loadGeometry();
    QIcon icon() const;

public slots:
    void slotSetSingle( bool single );

private slots:
    void statusMessage( const QString &message, const QByteArray &data );

signals:
    void setSingle( bool single );
    void closing( QDAppPlugin *plugin );

    void import();
    void sync();
    void syncall();
    void syncallslow();
    void backuprestore();
    void settings();
    void manual();
    void about();
    void quit();

private:
    void closeEvent( QCloseEvent *e );
    void stealMenuAndToolbars( QMainWindow *mw );
    void setupMenuAndToolbars();
    void resizeEvent( QResizeEvent *e );

    QDAppPlugin *plugin;
    ConnectionStatusWidget *mConnectionStatusIcon;
    PluginChooser *pluginChooser;
    StatusBar *statusBar;
    QMenu *windowMenu;
    QAction *singleAction;
};

#endif
