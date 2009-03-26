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

#ifndef ENCRYPTIONCONFIG_H
#define ENCRYPTIONCONFIG_H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QContent>
#include <QWidget>
#include <QHash>
#include <qtopianetworkinterface.h>

#include "ui_wirelessencryptbase.h"

class WirelessEncryptionPage : public QWidget
{
    Q_OBJECT
public:
    WirelessEncryptionPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~WirelessEncryptionPage();
    QtopiaNetworkProperties properties();

    void setProperties( const QtopiaNetworkProperties& cfg );

private:
    void init( const QtopiaNetworkProperties& cfg );
    void readConfig();
    void saveConfig();

private slots:
    void newNetSelected(int idx);
    void selectEncryptAlgorithm( int index );
    void selectEncryptType( int index );
    void checkPassword();
    void wpaEnterpriseChanged(int index);
    void fileSelected();

private:
    Ui::WirelessEncryptionBase ui;
    QtopiaNetworkProperties props;
    int lastIndex;
    QHash<QToolButton*,QContent> documents;
};

#endif // NO_WIRELESS_LAN
#endif
