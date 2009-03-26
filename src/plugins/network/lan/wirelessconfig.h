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

#ifndef WIRELESSCONFIG_H
#define WIRELESSCONFIG_H

#include <custom.h>

#ifndef NO_WIRELESS_LAN

#include <QWidget>
#include <qtopianetworkinterface.h>

#include "ui_wirelessbase.h"

class WirelessPage : public QWidget
{
    Q_OBJECT
public:
    WirelessPage( const QtopiaNetworkProperties& cfg, QWidget* parent = 0, Qt::WFlags flags = 0 );
    ~WirelessPage();

    QtopiaNetworkProperties properties();
    void setProperties( const QtopiaNetworkProperties& cfg );

private slots:
    void changeChannelMode( int index );
    void newNetSelected( int index );
    void setNewNetworkTitle( const QString& );

    void removeWLAN();
    void addWLAN();

private:
    void init();
    void initNetSelector( const QtopiaNetworkProperties& prop );
    void readConfig( );
    void saveConfig( );

private:
    Ui::WirelessBase ui;
    QtopiaNetworkProperties changedSettings;
    int lastIndex;

};
#endif //NO_WIRELESS_LAN

#endif
