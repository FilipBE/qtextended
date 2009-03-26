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

#include "server.h"
#include "phonesim.h"
#include "hardwaremanipulator.h"
#include <qdebug.h>

static int phonenumber = 555000;

PhoneSimServer::PhoneSimServer(const QString &f, quint16 port, QObject *parent)
    : QTcpServer(parent), fact(0), currentRules(0)
{
    listen( QHostAddress::Any, port );
    filename = f;
}

PhoneSimServer::~PhoneSimServer()
{
    setHardwareManipulator(0);
}

void PhoneSimServer::setHardwareManipulator(HardwareManipulatorFactory *f)
{
    delete fact;
    fact = f;
    if (f)
        f->setRuleFile(filename);
}

void PhoneSimServer::incomingConnection(int s)
{
  SimRules *sr = new SimRules(s, this, filename, fact);
    sr->setPhoneNumber(QString::number(phonenumber));
    phonenumber++;
    currentRules = sr;
}
