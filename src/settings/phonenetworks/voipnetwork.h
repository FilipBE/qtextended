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

#ifndef VOIPNETWORK_H
#define VOIPNETWORK_H

#include <QListWidget>

class QModemNetworkRegistration;
class QWaitWidget;
class QListWidgetItem;
class QCollectivePresence;
class QTelephonyConfiguration;
class QNetworkRegistration;

class VoipNetworkRegister : public QListWidget
{
    Q_OBJECT
public:
    VoipNetworkRegister( QWidget *parent = 0 );
    ~VoipNetworkRegister();

protected:
    void showEvent( QShowEvent * );

private slots:
    void operationSelected( QListWidgetItem * );
    void registrationStateChanged();

private:
    QNetworkRegistration *m_client;
    QCollectivePresence *m_presenceProvider;
    QTelephonyConfiguration *m_config;
    QListWidgetItem *m_regItem, *m_presenceItem, *m_configItem;

    void init();
    bool registered() const;
    bool visible() const;
    void updateRegistrationState();
    void updatePresenceState();
    void configureVoIP();
};

#endif
