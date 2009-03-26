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

#include "deviceupdater.h"
#include "localsocketlistener.h"
#include "packagescanner.h"
#include "inetdadaptor.h"
#include "configure.h"
#include "configuredata.h"

#include "ui_deviceupdaterbase.h"

#include <QTimer>
#include <qdebug.h>

DeviceUpdaterWidget::DeviceUpdaterWidget( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    setupUi( this );
    connect( quitButton, SIGNAL(clicked()),
            this, SLOT(quitWidget()) );
    connect( sendButton, SIGNAL(clicked()),
            this, SLOT(sendButtonClicked()) );
    connect( serverOnRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(serverButtonToggled()) );
    connect( serverOffRadioButton, SIGNAL(toggled(bool)),
            this, SLOT(serverButtonToggled()) );
    connect( configurePushButton, SIGNAL(clicked()),
            this, SLOT(showConfigure()) );

    progressBar->reset();
    serverOffRadioButton->setChecked( true );
    sendButton->setEnabled( false );
}

DeviceUpdaterWidget::~DeviceUpdaterWidget()
{
}

void DeviceUpdaterWidget::quitWidget()
{
    emit done( QDialog::Accepted );
}

void DeviceUpdaterWidget::showConfigure()
{
    ConfigureData oldData = Configure::dialog()->data();
    if ( Configure::dialog()->exec() == QDialog::Accepted )
    {
        const ConfigureData &newData = Configure::dialog()->data();
        if ( oldData.docRoot() != newData.docRoot() )
        {
            connectScanner( new PackageScanner( newData.docRoot(), this ));
        }
        if ( oldData.server() != newData.server()
                || oldData.port() != newData.port() )
        {
            InetdAdaptor *inetd = InetdAdaptor::getInstance();
            inetd->stop();
            inetd->start();
        }
        if ( oldData.command() != newData.command() )
        {
            // TODO: implement running mkPackages command
        }
    }
}

void DeviceUpdaterWidget::connectLocalSocket( LocalSocketListener *socket ) const
{
    connect( socket, SIGNAL(commandReceived(QString)),
            this, SLOT(handleCommand(QString)) );

    InetdAdaptor *inetd = InetdAdaptor::getInstance();
    connect( inetd, SIGNAL(startedRunning()),
            this, SLOT(serverStarted()) );
    connect( inetd, SIGNAL(stoppedRunning()),
            this, SLOT(serverStopped()) );
    inetd->start();
}

void DeviceUpdaterWidget::connectScanner( PackageScanner *scanner ) const
{
    connect( this, SIGNAL(sendPackage(QModelIndex)),
            scanner, SLOT(sendPackage(QModelIndex)) );
    connect( updateListButton, SIGNAL(clicked()),
            scanner, SLOT(refresh()) );
    connect( scanner, SIGNAL(progressValue(int)),
            progressBar, SLOT(setValue(int)) );
    connect( scanner, SIGNAL(progressValue(int)),
            this, SLOT(progressWake()) );
    connect( scanner, SIGNAL(progressMessage(QString)),
            statusLabel, SLOT(setText(QString)) );
    connect( scanner, SIGNAL(updated()),
            this, SLOT(scannerUpdated()) );
    packageListView->setModel( scanner );
    packageListView->setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void DeviceUpdaterWidget::progressWake()
{
    if ( !progressBar->isVisible() )
        progressBar->show();
}

void DeviceUpdaterWidget::scannerUpdated()
{
    // 15 seconds after the update, hide the progress bar
    QTimer::singleShot( 15000, progressBar, SLOT(hide()) );
    PackageScanner *scanner = qobject_cast<PackageScanner*>( packageListView->model() );
    Q_ASSERT( scanner );
    if ( scanner->rowCount() > 0 )
    {
        QItemSelectionModel *sel = packageListView->selectionModel();
        if ( !sel->hasSelection() )
            sel->setCurrentIndex( scanner->index( 0 ), QItemSelectionModel::Select );
        sendButton->setEnabled( true );
    }
    else
    {
        sendButton->setEnabled( false );
    }
}


void DeviceUpdaterWidget::serverStopped()
{
    if ( !serverOffRadioButton->isChecked() )
        serverOffRadioButton->setChecked( true );
    if ( sendButton->isEnabled() )
        sendButton->setEnabled( false );
}

void DeviceUpdaterWidget::serverStarted()
{
    if ( !serverOnRadioButton->isChecked() )
        serverOnRadioButton->setChecked( true );
    if ( !sendButton->isEnabled() )
        sendButton->setEnabled( true );
}

void DeviceUpdaterWidget::serverButtonToggled()
{
    if ( serverOnRadioButton->isChecked() )
        InetdAdaptor::getInstance()->start();
    else
        InetdAdaptor::getInstance()->stop();
}

void DeviceUpdaterWidget::sendPackage( const QString &packageName )
{
    // TODO:
    Q_UNUSED( packageName );
}

void DeviceUpdaterWidget::sendButtonClicked()
{
    progressBar->reset();
    QModelIndex ix = packageListView->currentIndex();
    if ( !ix.isValid() )
        return;
    emit sendPackage( ix );
}

void DeviceUpdaterWidget::handleCommand( const QString &command )
{
    if ( command.startsWith( "SendPackage " ))
    {
        int namePos = ::strlen( "SendPackage " );
        sendPackage( command.mid( namePos ));
    }
    else if ( command.startsWith( "RaiseWindow" ))
    {
        showNormal();
    }
    else
    {
        statusLabel->setText( tr( "Received unknown command \"%1\"" )
                .arg( command ));
        qWarning() << "Received unknown command" << command;
    }
}
