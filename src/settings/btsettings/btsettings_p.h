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

#ifndef BTSETTINGS_P_H
#define BTSETTINGS_P_H

#include <qbluetoothnamespace.h>

class QBluetoothAddress;

namespace BtSettings
{
    void setAudioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile, int channel);
    int audioProfileChannel(const QBluetoothAddress &addr, QBluetooth::SDPProfile profile);
};

#endif
