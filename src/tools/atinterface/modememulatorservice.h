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

#ifndef MODEMEMULATORSERVICE_H
#define MODEMEMULATORSERVICE_H

#include <qtopiaabstractservice.h>

class AtSessionManager;
class QValueSpaceObject;
class QUsbSerialGadget;

class ModemEmulatorService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    ModemEmulatorService( AtSessionManager *parent );
    ~ModemEmulatorService();

public slots:
    void addSerialPort( const QString& deviceName );
    void addSerialPort( const QString& deviceName, const QString& options );
    void addTcpPort( int tcpPort, bool localHostOnly );
    void addTcpPort( int tcpPort, bool localHostOnly, const QString& options );
    void removeSerialPort( const QString& deviceName );
    void removeTcpPort( int tcpPort );

private slots:
    void updateValueSpace();

    void appMessage(const QString &msg, const QByteArray &data);
    void serialActivated();
    void serialDeactivated();

private:
    AtSessionManager *sessions;
    QValueSpaceObject *info;
    int pendingTcpPort;
    QString pendingSerialPort;
    QUsbSerialGadget *gadget;
};

#endif
