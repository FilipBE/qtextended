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

#ifndef IAXNETWORKREGISTRATION_H
#define IAXNETWORKREGISTRATION_H

#include <qnetworkregistration.h>

class IaxTelephonyService;

class IaxNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit IaxNetworkRegistration( IaxTelephonyService *service );
    ~IaxNetworkRegistration();

    QString callUri() const;

public slots:
    void setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology );
    void requestAvailableOperators();
    void registrationEvent( int eventType );
    void autoRegisterToServer();
    void registerToServer();
    void deregisterFromServer();
    void updateRegistrationConfig();

private:
    IaxTelephonyService *service;
    bool pendingSetCurrentOperator;
    int registrationId;
    QString callUriValue;
};

#endif
