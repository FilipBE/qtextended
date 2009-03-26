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

#include "iaxsettings.h"
#include <qsettings.h>
#include <qvalidator.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

IaxSettings::IaxSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setWindowTitle( tr("Asterisk") );

    registered = false;

    netReg = new QNetworkRegistration( "asterisk", this );
    connect( netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(registrationStateChanged()) );

    config = new QTelephonyConfiguration( "asterisk", this );

    settings = new Ui::IaxSettingsBase();
    settings->setupUi( this );
    settings->tabWidget->setCurrentWidget( settings->registration );

    // Make the QLineEdit fields act right according to the input methods.
    QtopiaApplication::setInputMethodHint
        ( settings->userId, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->password, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->host, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->callerIdNumber, QtopiaApplication::PhoneNumber );
    QtopiaApplication::setInputMethodHint
        ( settings->callerIdName, QtopiaApplication::Text );

    registerAction = new QAction( tr("Register"), this );
    connect( registerAction, SIGNAL(triggered()), this, SLOT(actionRegister()) );

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(registerAction);

    copyToWidgets();

    // Get the current registration state and update the UI.
    registrationStateChanged();
}

IaxSettings::~IaxSettings()
{
    delete settings;
}

void IaxSettings::accept()
{
    bool regChanged = isRegistrationChanged();
    bool callerIdChanged = isCallerIdChanged();
    copyFromWidgets();

    // Update the caller id information.
    if ( callerIdChanged )
        updateCallerIdConfig();

    // Force iaxagent to change to the new registration details.
    if ( regChanged ) {
        if ( registered )
            deregisterFromProxy();
        updateRegistrationConfig();
        if ( registered )
            registerToProxy();
    }

    // If we have useful values, ask if user wants to register now.
    // If the user wants auto-registration, the iaxagent daemon
    // will do the registration automatically for us so no need to ask.
    if ( regChanged && !registered && !savedAutoRegister ) {
        int result = QMessageBox::warning
                ( this, tr("VoIP"),
                  tr("<qt>Would you like to register to the network now?</qt>"),
                  QMessageBox::Yes, QMessageBox::No);
        if ( result == QMessageBox::Yes ) {
            registerToProxy();
        }
    }

    QDialog::accept();
    close();
}

void IaxSettings::reject()
{
    QDialog::reject();
    close();
}

void IaxSettings::copyToWidgets()
{
    QSettings config( "Trolltech", "Asterisk" );
    config.beginGroup( "Registration" );

    // Get the user and server identities.
    QString userId = config.value( "UserId", QString() ).toString();
    savedUserId = userId;
    settings->userId->setText( userId );
    QString host = config.value( "Server", QString() ).toString();
    savedHost = host;
    settings->host->setText( host );
    QString password =
        config.value( "Password", QString( "" ) ).toString();
    if ( password.startsWith( ":" ) ) {
        // The password has been base64-encoded.
        password = QString::fromUtf8
            ( QByteArray::fromBase64( password.mid(1).toLatin1() ) );
    }
    settings->password->setText( password );
    savedPassword = password;
    bool autoRegister = config.value( "AutoRegister", false ).toBool();
    if ( autoRegister )
        settings->autoRegister->setCheckState( Qt::Checked );
    else
        settings->autoRegister->setCheckState( Qt::Unchecked );
    savedAutoRegister = autoRegister;

    config.endGroup();      // Registration

    // Get the callerid information.
    config.beginGroup( "CallerId" );
    QString callerIdNumber = config.value( "Number" ).toString();
    QString callerIdName = config.value( "Name" ).toString();
    savedCallerIdNumber = callerIdNumber;
    savedCallerIdName = callerIdName;
    settings->callerIdNumber->setText( callerIdNumber );
    settings->callerIdName->setText( callerIdName );
    config.endGroup();
}

void IaxSettings::copyFromWidgets()
{
    QSettings config( "Trolltech", "Asterisk" );
    config.beginGroup( "Registration" );

    config.setValue( "UserId", settings->userId->text() );
    QByteArray password = settings->password->text().toUtf8();
    config.setValue( "Password",
                     ":" + QString::fromLatin1( password.toBase64() ) );
    config.setValue( "Server", settings->host->text() );
    config.setValue( "AutoRegister",
                     ( settings->autoRegister->checkState() == Qt::Checked ) );

    config.endGroup();      // Registration
    config.beginGroup( "CallerId" );

    config.setValue( "Number", settings->callerIdNumber->text() );
    config.setValue( "Name", settings->callerIdName->text() );

    config.endGroup();      // CallerId

    savedUserId = settings->userId->text();
    savedPassword = settings->password->text();
    savedHost = settings->host->text();
    savedAutoRegister = ( settings->autoRegister->checkState() == Qt::Checked );
    savedCallerIdNumber = settings->callerIdNumber->text();
    savedCallerIdName = settings->callerIdName->text();
}

bool IaxSettings::isRegistrationChanged() const
{
    if ( settings->autoRegister->checkState() == Qt::Checked ) {
        if ( !savedAutoRegister )
            return true;
    } else if ( savedAutoRegister ) {
        return true;
    }
    if ( settings->userId->text() != savedUserId )
        return true;
    if ( settings->password->text() != savedPassword )
        return true;
    if ( settings->host->text() != savedHost )
        return true;
    return false;
}

bool IaxSettings::isCallerIdChanged() const
{
    if ( settings->callerIdNumber->text() != savedCallerIdNumber )
        return true;
    if ( settings->callerIdName->text() != savedCallerIdName )
        return true;
    return false;
}

void IaxSettings::actionRegister()
{
    if ( isCallerIdChanged() ) {
        updateCallerIdConfig();
    }
    if ( isRegistrationChanged() ) {
        copyFromWidgets();
        updateRegistrationConfig();
    }
    if ( registered )
        deregisterFromProxy();
    else
        registerToProxy();
}

void IaxSettings::updateRegister()
{
    registerAction->setEnabled( true );
    registerAction->setVisible( true );
    if ( registered )
        registerAction->setText( tr("Unregister") );
    else
        registerAction->setText( tr("Register") );
}

void IaxSettings::updateRegistrationConfig()
{
    config->update( "registration", QString() ); // No tr
}

void IaxSettings::updateCallerIdConfig()
{
    config->update( "callerid", QString() );     // No tr
}

void IaxSettings::registerToProxy()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeAutomatic );
}

void IaxSettings::deregisterFromProxy()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeDeregister );
}

void IaxSettings::registrationStateChanged()
{
    registered =
        ( netReg->registrationState() == QTelephony::RegistrationHome );
    updateRegister();
}
