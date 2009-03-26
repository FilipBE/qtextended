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

#include <qcopenvelope_qd.h>
#include <desktopsettings.h>

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QApplication>
#include <QPushButton>
#include <qdebug.h>

class TestPageApp;

class TestPage : public QWidget
{
    Q_OBJECT
public:
    TestPage( TestPageApp *plugin, QWidget *parent = 0 );
    ~TestPage();

public slots:
    void disconnect();
    void startFileTransfer();
    void stopFileTransfer();
    void sendAQCopMessage();

private:
    TestPageApp *plugin;
};

// ====================================================================

class TestPageApp : public QDAppPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(TestPageApp,QDAppPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.app.testpage"; }
    QString displayName() { return tr("Test Page"); }
    void init();

    // QDAppPlugin
    QIcon icon() { return QPixmap(":image/construction"); }
    QWidget *initApp() { return new TestPage( this ); }
};

QD_REGISTER_PLUGIN(TestPageApp)

void TestPageApp::init()
{
    DesktopSettings settings(id());

    // Clear and initialize the properties structure
    settings.remove( "properties" );
}

// ====================================================================


TestPage::TestPage( TestPageApp *_plugin, QWidget *parent )
    : QWidget( parent ),
    plugin( _plugin )
{
    QGridLayout *gbox = new QGridLayout( this );

    QLabel *icon = new QLabel;
    icon->setPixmap( QPixmap(":image/construction") );
    icon->setAlignment( Qt::AlignTop );

    QWidget *container = new QWidget;

    gbox->addWidget( icon, 0, 0 );
    gbox->addWidget( container, 0, 1 );
    gbox->setColumnStretch( 1, 1 );
    gbox->setRowStretch( 3, 1 );

    gbox = new QGridLayout( container );

    int row = 0;
    QLabel *label = 0;
    QPushButton *button = 0;

    label = new QLabel( "Make the connection disconnect" );
    button = new QPushButton( tr("Disconnect") );
    connect( button, SIGNAL(clicked()), this, SLOT(disconnect()) );

    gbox->addWidget( label, row, 0 );
    gbox->addWidget( button, row, 1 );
    row++;

    label = new QLabel( "Start a file transfer" );
    button = new QPushButton( tr("Start") );
    connect( button, SIGNAL(clicked()), this, SLOT(startFileTransfer()) );

    gbox->addWidget( label, row, 0 );
    gbox->addWidget( button, row, 1 );
    row++;

    label = new QLabel( "Stop a file transfer" );
    button = new QPushButton( tr("Stop") );
    connect( button, SIGNAL(clicked()), this, SLOT(stopFileTransfer()) );

    gbox->addWidget( label, row, 0 );
    gbox->addWidget( button, row, 1 );
    row++;

    label = new QLabel( "Send a QCop message" );
    button = new QPushButton( tr("QCop") );
    connect( button, SIGNAL(clicked()), this, SLOT(sendAQCopMessage()) );

    gbox->addWidget( label, row, 0 );
    gbox->addWidget( button, row, 1 );
    row++;
}

TestPage::~TestPage()
{
}

void TestPage::disconnect()
{
    qDebug() << "TestPage::disconnect";
    foreach ( QDConPlugin *plugin, qdPluginManager()->conPlugins() ) {
        if ( plugin->device() )
            plugin->stop();
    }
}

void TestPage::startFileTransfer()
{
    qDebug() << "TestPage::startFileTransfer";
    QCopEnvelope e( "QD/Connection", "setBusy()" );
}

void TestPage::stopFileTransfer()
{
    qDebug() << "TestPage::stopFileTransfer";
    QCopEnvelope e( "QD/Connection", "clearBusy()" );
}

void TestPage::sendAQCopMessage()
{
    qDebug() << "TestPage::sendAQCopMessage";
    QCopEnvelope e( "QPE/QDSync", "displayText(QString)" );
    e << QString("Testing QCop message delivery...");
}

#include "testpage.moc"
