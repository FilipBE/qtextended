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
#include "pluginmanager.h"

#include <qdplugin.h>
#include <center.h>
#include <desktopsettings.h>
#include <qcopenvelope_qd.h>

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QApplication>
#include <QVBoxLayout>
#include <qdebug.h>
#include <QResizeEvent>

#ifndef VENDOR
#define VENDOR "[VENDOR]"
#endif

#define ID_ROLE (Qt::UserRole + 1)

class InfoPageApp;

class InfoPage : public QWidget
{
    Q_OBJECT
public:
    InfoPage( InfoPageApp *plugin, QWidget *parent = 0 );
    ~InfoPage();

private slots:
    void refreshDeviceInfo();

private:
    InfoPageApp *plugin;
    QLabel *deviceIcon;
    QLabel *deviceInfo;
};

// ====================================================================

class PluginEnabler : public QWidget
{
    Q_OBJECT
public:
    PluginEnabler( InfoPageApp *plugin, QWidget *parent = 0 );
    ~PluginEnabler();

public slots:
    //void recheckSettings();
    void applySettings();
    void revertSettings();

private:
    bool eventFilter( QObject *watched, QEvent *event );

    InfoPageApp *plugin;
    QTabWidget *tabs;
    QListWidget *pluginFileList;
    QTableWidget *pluginIdList;
};

// ====================================================================

class InfoPageApp : public QDAppPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(InfoPageApp,QDAppPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.app.infopage"; }
    QString displayName() { return tr("Info Page"); }

    // QDAppPlugin
    QIcon icon() { return QPixmap(":image/appicon"); }
    QWidget *initApp() { return new InfoPage( this ); }
    QWidget *initSettings() { return new PluginEnabler( this ); }
};

QD_REGISTER_PLUGIN(InfoPageApp)

// ====================================================================

InfoPage::InfoPage( InfoPageApp *_plugin, QWidget *parent )
    : QWidget( parent ),
    plugin( _plugin )
{
    QGridLayout *gbox = new QGridLayout( this );

    QLabel *icon = new QLabel;
    icon->setPixmap( QPixmap(":image/appicon") );
    icon->setAlignment( Qt::AlignTop );

    QLabel *info = new QLabel;
    info->setAlignment( Qt::AlignTop );

    QString message = "<qt>";
    message += tr("%1 Qt Extended Sync Agent<br>", "1=vendor").arg(VENDOR);
    message += tr("Version %1<br>", "1=version number").arg(VERSION);
    message += tr("Built on %1<br>", "1=date").arg(__DATE__);
    message += tr("Built by %1<br>", "1=user@host").arg(BUILDER);
    message += tr("Copyright (C) %1 %2").arg("2009").arg("Trolltech ASA");
    info->setText( message );

    deviceIcon = new QLabel;
    deviceIcon->setAlignment( Qt::AlignTop );

    deviceInfo = new QLabel;
    deviceInfo->setAlignment( Qt::AlignTop );

    gbox->addWidget( icon, 0, 0 );
    gbox->addWidget( info, 0, 1 );
    gbox->addItem( new QSpacerItem(0, QFontMetrics(QFont()).height()), 1, 0 );
    gbox->addWidget( deviceIcon, 2, 0 );
    gbox->addWidget( deviceInfo, 2, 1 );
    gbox->setColumnStretch( 1, 1 );
    gbox->setRowStretch( 3, 1 );

    refreshDeviceInfo();
    connect( qApp, SIGNAL(setConnectionState(int)), this, SLOT(refreshDeviceInfo()) );
}

InfoPage::~InfoPage()
{
}

void InfoPage::refreshDeviceInfo()
{
    QString message;
    QPixmap icon;
    QDDevPlugin *iface = plugin->centerInterface()->currentDevice();
    if ( iface ) {
        icon = iface->icon();
        message +=
            tr("Connected to %1 running %2 %3 using the %4 driver", "1=model, 2=system, 3=version, 4=driver")
            .arg(iface->model())
            .arg(iface->system())
            .arg(iface->versionString())
            .arg(iface->displayName());
    } else {
        message +=
            tr("No device is connected");
    }
    deviceIcon->setPixmap( icon );
    deviceInfo->setText( message );
    deviceInfo->setWordWrap( true );
}

// ====================================================================

PluginEnabler::PluginEnabler( InfoPageApp *_plugin, QWidget *parent )
    : QWidget( parent ),
    plugin( _plugin )
{
    setWindowTitle( tr("Plugin Enabler") );
    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );
    tabs = new QTabWidget;
    vbox->addWidget( tabs );

    // Plugin Files
    pluginFileList = new QListWidget;
    tabs->addTab( pluginFileList, tr("Files") );

    foreach ( const QString &id, qdPluginManager()->detectedPluginFiles() ) {
#if defined(Q_OS_WIN)
        QString name = QString("%1.dll").arg(id);
#elif defined(Q_WS_MAC)
        QString name = QString("%1.dylib").arg(id);
#elif defined(Q_OS_UNIX)
        QString name = QString("%1.so").arg(id);
#else
        QString name = id;
#endif
        QListWidgetItem *item = new QListWidgetItem( name );
        item->setData( ID_ROLE, id );
        item->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsTristate|Qt::ItemIsEnabled );
        pluginFileList->addItem( item );
    }

    // Plugin Ids
    pluginIdList = new QTableWidget;
    pluginIdList->verticalHeader()->hide();
    pluginIdList->horizontalHeader()->hide();
    pluginIdList->setColumnCount(2);
    pluginIdList->installEventFilter( this );
    pluginIdList->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    tabs->addTab( pluginIdList, tr("Plugins") );

    QMap<QString,QString> detectedPluginIds = qdPluginManager()->detectedPluginIds();
    int row = 0;
    for ( QMap<QString,QString>::const_iterator it = detectedPluginIds.constBegin(); it != detectedPluginIds.constEnd(); ++it ) {
        QString id = it.key();
        // Don't show the plugin enabler!
        if ( id == plugin->id() ) continue;

        pluginIdList->setRowCount( row+1 );

        QString name = it.value();
        QTableWidgetItem *item = new QTableWidgetItem( name );
        item->setData( ID_ROLE, id );
        item->setFlags( Qt::ItemIsUserCheckable|Qt::ItemIsTristate|Qt::ItemIsEnabled );
        pluginIdList->setItem( row, 0, item );

        name = id;
        item = new QTableWidgetItem( name );
        pluginIdList->setItem( row, 1, item );

        ++row;
    }

    pluginIdList->resizeColumnToContents( 1 );

    revertSettings();
}

PluginEnabler::~PluginEnabler()
{
}

void PluginEnabler::applySettings()
{
    // Plugin Files
    QStringList disabledPluginFiles;
    for ( int i = 0; i < pluginFileList->count(); ++i ) {
        QListWidgetItem *item = pluginFileList->item( i );
        QString id = item->data( ID_ROLE ).toString();
        if ( item->checkState() == Qt::Unchecked )
            disabledPluginFiles << id;
    }

    // Plugin Ids
    QStringList disabledPluginIds;
    for ( int i = 0; i < pluginIdList->rowCount(); ++i ) {
        QTableWidgetItem *item = pluginIdList->item( i, 0 );
        QString id = item->data( ID_ROLE ).toString();
        if ( item->checkState() == Qt::Unchecked )
            disabledPluginIds << id;
    }

    DesktopSettings settings("settings");
    settings.setValue("DisabledPluginFiles", disabledPluginFiles);
    settings.setValue("DisabledPluginIds", disabledPluginIds);

    //qdPluginManager()->setupPlugins();
}

void PluginEnabler::revertSettings()
{
    DesktopSettings settings("settings");
    QStringList disabledPluginFiles = settings.value("DisabledPluginFiles").toStringList();
    QStringList disabledPluginIds = settings.value("DisabledPluginIds").toStringList();
    QStringList loadedPluginFiles = settings.value("loadedPluginFiles").toStringList();
    QStringList loadedPlugins = settings.value("loadedPlugins").toStringList();

    // Plugin Files
    for ( int i = 0; i < pluginFileList->count(); ++i ) {
        QListWidgetItem *item = pluginFileList->item( i );
        QString id = item->data( ID_ROLE ).toString();
        if ( disabledPluginFiles.contains(id) ) {
            item->setCheckState( Qt::Unchecked );
        } else {
            if ( !loadedPluginFiles.contains(id) ) {
                item->setCheckState( Qt::PartiallyChecked );
            } else {
                item->setCheckState( Qt::Checked );
            }
        }
    }

    // Plugin Ids
    for ( int i = 0; i < pluginIdList->rowCount(); ++i ) {
        QTableWidgetItem *item = pluginIdList->item( i, 0 );
        QString id = item->data( ID_ROLE ).toString();
        if ( disabledPluginIds.contains(id) ) {
            item->setCheckState( Qt::Unchecked );
        } else {
            if ( !loadedPlugins.contains(id) ) {
                item->setCheckState( Qt::PartiallyChecked );
            } else {
                item->setCheckState( Qt::Checked );
            }
        }
    }
}

bool PluginEnabler::eventFilter( QObject * /*watched*/, QEvent *event )
{
    if ( event->type() == QEvent::Resize ) {
        QResizeEvent *e = (QResizeEvent*)event;
        // Where do the 4 extra pixels come from?
        // I don't know so I've disabled the scroll bar altogether.
        int width = e->size().width() - pluginIdList->columnWidth( 1 ) - 4;
        pluginIdList->setColumnWidth( 0, width );
    }
    return false;
}

#include "infopage.moc"
