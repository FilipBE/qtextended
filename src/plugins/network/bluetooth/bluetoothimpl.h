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

#ifndef BLUETOOTHIMPL_H
#define BLUETOOTHIMPL_H

#include <qtopianetworkinterface.h>
#include <scriptthread.h>

class QValueSpaceObject;
class BluetoothDialupDevice;
class QProcess;
class QTimerEvent;
class QCommDeviceSession;
class BluetoothImpl : public QtopiaNetworkInterface
{
    Q_OBJECT
public:
    BluetoothImpl( const QString& confFile );
    virtual ~BluetoothImpl();

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

    void timerEvent( QTimerEvent* ev);

private slots:
    void updateState();
    void serialPortConnected();

private:
    enum { Initialize, Connect, Monitoring, Disappearing } state;
    void updateTrigger( QtopiaNetworkInterface::Error e = QtopiaNetworkInterface::NoError, const QString& desc = QString() );

private:
    QtopiaNetworkConfiguration *configIface;
    Status ifaceStatus;
    mutable QString deviceName;
    QValueSpaceObject* netSpace;
    int trigger;

    BluetoothDialupDevice* dialupDev;
    QCommDeviceSession *session;
    ScriptThread thread;
    QByteArray pppIface;
    int tidStateUpdate;
    int logIndex;
};
#endif
