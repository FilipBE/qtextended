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

#ifndef DIALUP_H
#define DIALUP_H

#include <qtopianetworkinterface.h>
#include <qtopianetwork.h>
#include <qvaluespace.h>
#include <scriptthread.h>

#ifdef QTOPIA_CELL
#include <qphonecallmanager.h>
#include <qnetworkregistration.h>
#include <qcommservicemanager.h>
#endif

class DialupImpl : public QtopiaNetworkInterface
{
    Q_OBJECT
public:
    DialupImpl( const QString& confFile );
    virtual ~DialupImpl();

    virtual Status status();

    virtual void initialize();
    virtual void cleanup();
    virtual bool start( const QVariant options = QVariant() );
    virtual bool stop();
    virtual QString device() const;
    virtual bool setDefaultGateway();

    virtual QtopiaNetwork::Type type() const;

    virtual QtopiaNetworkConfiguration * configuration();

    virtual void setProperties(
            const QtopiaNetworkProperties& properties);

protected:
    bool isAvailable() const;
    bool isActive() const;

    void timerEvent( QTimerEvent* e );
private:
    enum { Initialize, Connect, Monitoring, Disappearing } state;

private:
    void updateTrigger( QtopiaNetworkInterface::Error code = QtopiaNetworkInterface::NoError, const QString& desc = QString() );

private:
    QtopiaNetworkConfiguration *configIface;
    Status ifaceStatus;
    mutable QString deviceName;
    QString pppIface;

    int tidStateUpdate;
    int logIndex;
    int trigger;

private slots:
    void updateState();
#ifdef QTOPIA_CELL
    void connectNotification( const QPhoneCall&, QPhoneCall::Notification, const QString& );
    void registrationStateChanged();
    void phoneCallStateChanged( const QPhoneCall& );
private:
    QTelephony::RegistrationState regState;
    QCommServiceManager* commManager;
    QPhoneCallManager* callManager;
    QPhoneCall dataCall;
    QNetworkRegistration *netReg;
    bool pppdProcessBlocked;
#endif
private:
    QValueSpaceObject* netSpace;
    ScriptThread thread;
    bool delayedGatewayInstall;
};

#endif
