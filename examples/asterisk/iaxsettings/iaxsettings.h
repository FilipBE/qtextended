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

#ifndef IAXSETTINGS_H
#define IAXSETTINGS_H

#include <qdialog.h>
#include "ui_iaxsettingsbase.h"
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>

class QAction;

class IaxSettings : public QDialog
{
    Q_OBJECT
public:
    IaxSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~IaxSettings();

protected:
    void accept();
    void reject();

private slots:
    void actionRegister();
    void updateRegister();
    void registrationStateChanged();

private:
    Ui::IaxSettingsBase *settings;
    QAction *registerAction;
    bool registered;
    QNetworkRegistration *netReg;
    QTelephonyConfiguration *config;

    QString savedUserId;
    QString savedPassword;
    QString savedHost;
    bool savedAutoRegister;
    QString savedCallerIdNumber;
    QString savedCallerIdName;

    void copyToWidgets();
    void copyFromWidgets();
    bool isRegistrationChanged() const;
    bool isCallerIdChanged() const;

    void updateRegistrationConfig();
    void updateCallerIdConfig();
    void registerToProxy();
    void deregisterFromProxy();
};

#endif
