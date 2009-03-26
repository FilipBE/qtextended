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

#include "gtalksettings.h"
#include <qsettings.h>
#include <qvalidator.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

GTalkSettings::GTalkSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal(true);
    setWindowTitle( tr("Google Talk") );

    registered = false;

    netReg = new QNetworkRegistration( "jabber", this );
    connect( netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(registrationStateChanged()) );

    config = new QTelephonyConfiguration( "jabber", this );

    settings = new Ui::GTalkSettingsBase();
    settings->setupUi( this );
    settings->GTalkSettingsTab->setCurrentWidget( settings->registration );

    // Make the QLineEdit fields act right according to the input methods.
    QtopiaApplication::setInputMethodHint
        ( settings->usernameEntry, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->passwordEntry, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->serverEntry, QtopiaApplication::Text );
    QtopiaApplication::setInputMethodHint
        ( settings->portEntry, QtopiaApplication::Text );

    registerAction = new QAction( tr("Register"), this );
    connect( registerAction, SIGNAL(triggered()), this, SLOT(actionRegister()) );

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(registerAction);

    copyToWidgets();

    // Get the current registration state and update the UI.
    registrationStateChanged();
}

GTalkSettings::~GTalkSettings()
{
    delete settings;
}

void GTalkSettings::accept()
{
    bool regChanged = isRegistrationChanged();
    copyFromWidgets();

    if ( regChanged )
        updateRegistrationConfig();

    if ( regChanged && !registered && !m_autoRegister ) {
        int result = QMessageBox::warning
                ( this, tr("GTalk"),
                  tr("<qt>Would you like to register to the network now?</qt>"),
                  QMessageBox::Yes, QMessageBox::No);
        if ( result == QMessageBox::Yes ) {
            registerToServer();
        }
    }

    QDialog::accept();
    close();
}

void GTalkSettings::reject()
{
    QDialog::reject();
    close();
}

void GTalkSettings::copyToWidgets()
{
    QSettings config("Trolltech", "GTalk");

    config.beginGroup("Registration");
    m_autoRegister = config.value("AutoRegister", false).toBool();
    if ( m_autoRegister )
        settings->autoRegister->setCheckState(Qt::Checked);
    else
        settings->autoRegister->setCheckState(Qt::Unchecked);
    config.endGroup();

    config.beginGroup("Parameters");

    m_account = config.value("account", QString()).toString();
    settings->usernameEntry->setText(m_account);

    m_password = config.value("password", QString( "" )).toString();
    if ( m_password.startsWith( ":" ) ) {
        // The password has been base64-encoded.
        m_password = QString::fromUtf8(QByteArray::fromBase64(m_password.mid(1).toLatin1()));
    }
    settings->passwordEntry->setText(m_password);

    m_server = config.value("server", QString("talk.google.com")).toString();
    settings->serverEntry->setText(m_server);

    m_port = config.value("port", QString("5223")).toString();
    settings->portEntry->setText(m_port);

    m_requireEncryption = config.value("require-encryption", true).toBool();
    settings->useEncryption->setChecked(m_requireEncryption);

    m_ignoreSslErrors = config.value("ignore-ssl-errors", true).toBool();
    settings->ignoreSslErrors->setChecked(m_ignoreSslErrors);
    settings->ignoreSslErrors->setEnabled(m_ignoreSslErrors);

    m_oldSsl = config.value("old-ssl", true).toBool();
    settings->oldSsl->setChecked(m_ignoreSslErrors);
    settings->oldSsl->setEnabled(m_ignoreSslErrors);

    config.endGroup();
}

void GTalkSettings::copyFromWidgets()
{
    QSettings config("Trolltech", "GTalk");
    config.beginGroup("Registration");
    config.setValue("AutoRegister", settings->autoRegister->isChecked());
    config.endGroup();

    config.beginGroup("Parameters");
    m_account = settings->usernameEntry->text();
    config.setValue("account", m_account);

    m_password = settings->passwordEntry->text().toUtf8();
    config.setValue("password", ":" + QString::fromLatin1(m_password.toLatin1().toBase64()));

    m_server = settings->serverEntry->text();
    config.setValue("server", m_server);

    m_port = settings->portEntry->text();
    config.setValue("port", m_port);

    m_requireEncryption = settings->useEncryption->isChecked();
    config.setValue("require-encryption", m_requireEncryption);

    m_ignoreSslErrors = settings->ignoreSslErrors->isChecked();
    config.setValue("ignore-ssl-errors", m_ignoreSslErrors);

    m_oldSsl = settings->oldSsl->isChecked();
    config.setValue("old-ssl", m_oldSsl);

    config.endGroup();
}

bool GTalkSettings::isRegistrationChanged() const
{
    if ( settings->autoRegister->checkState() == Qt::Checked ) {
        if ( !m_autoRegister )
            return true;
    } else if ( m_autoRegister ) {
        return true;
    }
    if (settings->usernameEntry->text() != m_account)
        return true;
    if (settings->passwordEntry->text() != m_password)
        return true;
    if (settings->serverEntry->text() != m_server)
        return true;
    if (settings->portEntry->text() != m_port)
        return true;
    if (settings->useEncryption->isChecked() != m_requireEncryption)
        return true;
    if (settings->ignoreSslErrors->isChecked() != m_ignoreSslErrors)
        return true;
    if (settings->oldSsl->isChecked() != m_oldSsl)
        return true;

    return false;
}

void GTalkSettings::actionRegister()
{
    if ( isRegistrationChanged() ) {
        copyFromWidgets();
        updateRegistrationConfig();
    }
    if ( registered )
        deregisterFromServer();
    else
        registerToServer();
}

void GTalkSettings::updateRegister()
{
    registerAction->setEnabled( true );
    registerAction->setVisible( true );
    if ( registered )
        registerAction->setText( tr("Unregister") );
    else
        registerAction->setText( tr("Register") );
}

void GTalkSettings::updateRegistrationConfig()
{
    config->update( "registration", QString() ); // No tr
}

void GTalkSettings::registerToServer()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeAutomatic );
}

void GTalkSettings::deregisterFromServer()
{
    netReg->setCurrentOperator( QTelephony::OperatorModeDeregister );
}

void GTalkSettings::registrationStateChanged()
{
    registered =
        ( netReg->registrationState() == QTelephony::RegistrationHome );
    updateRegister();
}
