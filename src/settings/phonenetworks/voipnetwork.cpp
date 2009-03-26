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

#include "voipnetwork.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsoftmenubar.h>
#include <QtopiaItemDelegate>
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>

#include <QtopiaServiceRequest>
#include <QVBoxLayout>
#include <QDebug>

VoipNetworkRegister::VoipNetworkRegister( QWidget *parent )
    : QListWidget( parent )
{
    init();

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(operationSelected(QListWidgetItem*)) );
    connect( m_client, SIGNAL(registrationStateChanged()),
            this, SLOT(registrationStateChanged()) );
}

VoipNetworkRegister::~VoipNetworkRegister()
{
}

void VoipNetworkRegister::init()
{
    setItemDelegate( new QtopiaItemDelegate );
    setFrameStyle( QFrame::NoFrame );

    m_client = new QNetworkRegistration( "voip", this );
    m_config = new QTelephonyConfiguration( "voip", this );

    setObjectName( "voip" );

    m_configItem = new QListWidgetItem( tr( "Configure" ), this );

    QString menuText = registered() ? tr( "Unregister" ) : tr( "Register" );
    m_regItem = new QListWidgetItem( menuText, this );

    menuText = tr( "Set Presence Status" );
    m_presenceItem = new QListWidgetItem( menuText, this );

    setCurrentRow( 0 );
}

bool VoipNetworkRegister::registered() const
{
    return m_client->registrationState() == QTelephony::RegistrationHome;
}

void VoipNetworkRegister::showEvent( QShowEvent *e )
{
    m_presenceItem->setHidden( !registered() );
    QListWidget::showEvent( e );
}

void VoipNetworkRegister::operationSelected( QListWidgetItem * item )
{
    if ( item == m_regItem )
        updateRegistrationState();
    else if ( item == m_presenceItem )
        updatePresenceState();
    else if ( item == m_configItem )
        configureVoIP();
}

void VoipNetworkRegister::updateRegistrationState()
{
    if ( QMessageBox::question( this, tr( "VoIP" ),
                registered() ? tr( "<qt>Unregister from VoIP network?</qt>" ) : tr( "<qt>Register to VoIP network?</qt>" ),
                QMessageBox::Yes, QMessageBox::No ) == QMessageBox::No )
        return;

    m_client->setCurrentOperator( registered() ?
        QTelephony::OperatorModeDeregister : QTelephony::OperatorModeAutomatic);
}

void VoipNetworkRegister::updatePresenceState()
{
    QtopiaServiceRequest e( "Presence", "editPresence()" );
    e.send();
}

void VoipNetworkRegister::registrationStateChanged()
{
    m_regItem->setText( registered() ? tr( "Unregister" ) : tr( "Register" ) );
    m_presenceItem->setHidden( !registered() );
}

void VoipNetworkRegister::configureVoIP()
{
    QtopiaIpcEnvelope e( "QPE/Application/sipsettings", "VoIP::configure()" );
}
