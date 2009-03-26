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

#ifndef ACCOUNTCONFIG_H
#define ACCOUNTCONFIG_H

#include <QWidget>
#include <qtopianetwork.h>
#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

class QComboBox;
class QLabel;
class QLineEdit;

class AccountPagePrivate;
class QTOPIACOMM_EXPORT AccountPage : public QWidget
{
    Q_OBJECT
public:

    AccountPage( QtopiaNetwork::Type type,
            const QtopiaNetworkProperties& cfg,
            QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~AccountPage();

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop);

private:
    QtopiaNetwork::Type accountType;
    QLineEdit* name;
    QLabel* startup_label;
    QComboBox* startup;

    QLabel* dialup_label;
    QLineEdit* dialup;

    QLabel* user_label;
    QLineEdit* user;
    QLabel* password_label;
    QLineEdit* password;

    AccountPagePrivate* d;
    Q_PRIVATE_SLOT( d, void _q_selectBluetoothDevice() );
    Q_PRIVATE_SLOT( d, void _q_BluetoothStateChanged() );
};
#endif
