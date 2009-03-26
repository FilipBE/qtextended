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

#ifndef BTPINHELPER_H
#define BTPINHELPER_H

#include <qbluetoothpasskeyagent.h>

class QPasswordDialog;
class QValueSpaceItem;

class BluetoothPasskeyAgentTask : public QObject 
{
    Q_OBJECT
public:
    BluetoothPasskeyAgentTask( QObject* parent = 0 );
protected slots:
    void activateAgent();
    void delayedAgentStart();
private:
    QValueSpaceItem* serverWidgetVsi;
};

class BTPinHelper : public QBluetoothPasskeyAgent {
    Q_OBJECT
public:
    BTPinHelper(QObject *parent = 0);
    ~BTPinHelper();

    virtual void requestPasskey(QBluetoothPasskeyRequest &req);
    virtual void cancelRequest(const QString &localDevice,
                               const QBluetoothAddress &remoteAddr);
    virtual void release();

private slots:
    void stopVibration();

private:
    QPasswordDialog *m_passDialog;
};

#endif
