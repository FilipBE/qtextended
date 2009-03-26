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

#ifndef BTHEADSETTASK_H
#define BTHEADSETTASK_H

#include <QObject>

class QBluetoothHeadsetService;
class BtHeadsetServiceTask : public QObject
{
    Q_OBJECT

public:
    BtHeadsetServiceTask( QObject* parent = 0 );
    ~BtHeadsetServiceTask();

private:
    QBluetoothHeadsetService *m_hsService;
};

#endif
