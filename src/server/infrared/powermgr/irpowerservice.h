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

#ifndef IRPOWERSERVICE_H
#define IRPOWERSERVICE_H

#include "qabstractdevicemanager.h"

class IrPowerService_Private;
class IrPowerService : public QAbstractCommDeviceManager
{
    Q_OBJECT
public:
    IrPowerService(const QByteArray &serverPath,
                   const QByteArray &devId,
                   QObject *parent = 0);
    ~IrPowerService();

    virtual void bringUp();
    virtual void bringDown();
    virtual bool isUp() const;
    virtual bool shouldBringDown(QUnixSocket *) const;

private:
    IrPowerService_Private *m_data;
};

class QValueSpaceObject;

class IrPowerServiceTask : public QObject
{
Q_OBJECT
public:
    IrPowerServiceTask(QObject *parent = 0);
    ~IrPowerServiceTask();

private:
    IrPowerService *m_irPower;
    QValueSpaceObject *m_valueSpace;
};

#endif
