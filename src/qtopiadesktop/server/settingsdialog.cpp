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
#include "settingsdialog.h"
#include "pluginchooser.h"
#include "pluginmanager.h"

#include <qdplugin.h>
#include <private/qdplugin_p.h>

#include <desktopsettings.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QFrame>
#include <qdebug.h>

SettingsDialog::SettingsDialog( QWidget *parent )
    : QDialog( parent )
{
    loadGeometry();
    setWindowTitle( tr("Qtopia Sync Agent Settings") );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );

    QWidget *top = new QWidget;
    vbox->addWidget( top );

    QHBoxLayout *hbox = new QHBoxLayout( top );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );

    pluginChooser = new PluginChooser;
    pluginChooser->setOrientation( PluginChooser::Vertical );
    pluginChooser->setWidgetType( PluginChooser::Settings );
    pluginChooser->setFrameShadow( QFrame::Raised );
    pluginChooser->setFrameShape( QFrame::StyledPanel );
    connect( pluginChooser, SIGNAL(showPlugin(QDAppPlugin*)), this, SLOT(showPlugin(QDAppPlugin*)) );

    stack = new QStackedWidget;

    hbox->addWidget( pluginChooser );
    hbox->addWidget( stack );

    QWidget *buttonBox = new QWidget;
    vbox->addWidget(buttonBox);
    hbox = new QHBoxLayout( buttonBox );
    hbox->setMargin( 6 );
    hbox->setSpacing( 6 );
    QPushButton *ok = new QPushButton( tr("Ok"), buttonBox );
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    QPushButton *apply = new QPushButton( tr("Apply"), buttonBox );
    connect( apply, SIGNAL(clicked()), this, SLOT(apply()) );
    QPushButton *cancel = new QPushButton( tr("Cancel"), buttonBox );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    hbox->addStretch( 1 );
    hbox->addWidget( ok );
    hbox->addWidget( apply );
    hbox->addWidget( cancel );
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::loadGeometry()
{
    // Set the geometry (can be overridden from the commandline)
    DesktopSettings settings( "settingsdialog" );
    QRect desk = qApp->desktop()->availableGeometry();
    QRect r = settings.value( "geometry" ).toRect();
    if ( !r.isValid() ) {
        int w = qMin(600,desk.width());
        int h = qMin(500,desk.height());
        int x = r.x()+(desk.width()-w)/2;
        int y = r.y()+(desk.height()-h)/2;
        r = QRect( x, y, w, h );
    }
    setGeometry( r );
    QPoint p = settings.value( "pos", r.topLeft() ).toPoint();
    move( p );
}

void SettingsDialog::showEvent( QShowEvent *e )
{
    DesktopSettings settings("settingsdialog");
    QString id = settings.value("defaultPlugin").toString();
    QDAppPlugin *plugintoshow = 0;
    QDAppPluginFilter filter;
    filter.settingsWidget = QDPluginFilter::Set;
    foreach( QDAppPlugin *plugin, qdPluginManager()->appPlugins( &filter ) ) {
        if ( !plugintoshow )
            plugintoshow = plugin;
        if ( plugin->id() == id ) {
            plugintoshow = plugin;
            break;
        }
    }
    if ( plugintoshow ) {
        showPlugin( plugintoshow );
        pluginChooser->highlightPlugin( plugintoshow );
    }
    emit recheckSettings();
    QDialog::showEvent( e );
}

void SettingsDialog::closeEvent( QCloseEvent *e )
{
    DesktopSettings settings( "settingsdialog" );
    settings.setValue( "geometry", geometry() );
    settings.setValue( "pos", pos() );
    QDialog::closeEvent( e );
}

void SettingsDialog::showPlugin( QDAppPlugin *plugin )
{
    //qDebug() << "SettingsDialog::showPlugin" << plugin->id();
    QWidget *w = qdPluginManager()->pluginData(plugin)->settingsWidget;
    if ( stack->indexOf( w ) == -1 ) {
        //qDebug() << "loading settings for" << plugin->id();
        const QMetaObject *mo = w->metaObject();
        if ( mo->indexOfSlot("applySettings()") != -1 ) {
            connect( this, SIGNAL(applySettings()), w, SLOT(applySettings()) );
            //qDebug() << plugin->id() << "can do applySettings()";;
        }
        if ( mo->indexOfSlot("revertSettings()") != -1 ) {
            //qDebug() << plugin->id() << "can do revertSettings()";;
            connect( this, SIGNAL(revertSettings()), w, SLOT(revertSettings()) );
        }
        if ( mo->indexOfSlot("recheckSettings()") != -1 ) {
            //qDebug() << plugin->id() << "can do recheckSettings()";;
            connect( this, SIGNAL(recheckSettings()), w, SLOT(recheckSettings()) );
        }
        stack->addWidget( w );
    } else {
    }
    //qDebug() << "showing settings for" << plugin->id();
    stack->setCurrentIndex( stack->indexOf( w ) );
    DesktopSettings settings("settingsdialog");
    settings.setValue("defaultPlugin", plugin->id());
    QString title = w->windowTitle();
    if ( title.isEmpty() )
        title = plugin->displayName();
    setWindowTitle( tr("%1 - Qtopia Sync Agent Settings", "1=plugin").arg(title) );
}

void SettingsDialog::accept()
{
    emit applySettings();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    emit revertSettings();
    QDialog::reject();
}

void SettingsDialog::apply()
{
    emit applySettings();
    emit recheckSettings();
}

