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

#ifndef JABBERMANAGER_H
#define JABBERMANAGER_H

#include <QObject>
#include <qnetworkregistration.h>
#include <qcommservicemanager.h>
#include "qabstractcallpolicymanager.h"

class JabberManagerPrivate;

class JabberManager : public QAbstractCallPolicyManager
{
    Q_OBJECT
public:
    JabberManager(QObject *parent=0);

    QString callType() const;
    QString trCallType() const;
    QString callTypeIcon() const;
    QAbstractCallPolicyManager::CallHandling handling(const QString& number);
    QString registrationMessage() const;
    QString registrationIcon() const;

    QTelephony::RegistrationState registrationState() const;
    bool isAvailable(const QString &uri);
    bool doDnd();
    bool supportsPresence() const;
    void updateOnThePhonePresence(bool isOnThePhone);
    void setCellLocation(const QString &);

private slots:
    void registrationStateChanged();
    void localPresenceChanged();
    void servicesChanged();
    void hideMessageTimeout();
    void presenceProviderAvailable();
    void presenceProviderDisconnected();

private:
    JabberManagerPrivate *d;

    void serviceStarted();
    void serviceStopped();
    bool isOnDnd();
};

#endif
