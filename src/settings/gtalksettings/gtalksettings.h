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

#ifndef GTALKSETTINGS_H
#define GTALKSETTINGS_H

#include <qdialog.h>
#include "ui_gtalksettingsbase.h"
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>

class QAction;

class GTalkSettings : public QDialog
{
    Q_OBJECT
public:
    GTalkSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~GTalkSettings();

protected:
    void accept();
    void reject();

private slots:
    void actionRegister();
    void updateRegister();
    void registrationStateChanged();

private:
    Ui::GTalkSettingsBase *settings;
    QAction *registerAction;
    bool registered;
    QNetworkRegistration *netReg;
    QTelephonyConfiguration *config;

    bool m_autoRegister;

    QString m_account;
    QString m_password;
    QString m_server;
    QString m_port;
    bool m_requireEncryption;
    bool m_ignoreSslErrors;
    bool m_oldSsl;

    void copyToWidgets();
    void copyFromWidgets();
    bool isRegistrationChanged() const;

    void updateRegistrationConfig();
    void registerToServer();
    void deregisterFromServer();
};

#endif
