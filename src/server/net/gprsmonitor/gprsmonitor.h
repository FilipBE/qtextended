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

#ifndef GPRSMONITOR_H
#define GPRSMONITOR_H

#include <QObject>
class QFileSystemWatcher;
class QNetworkDevice;
class QNetworkRegistration;
class QValueSpaceObject;
class QCommServiceManager;
class GPRSMonitor : public QObject
{
    Q_OBJECT
public:
    GPRSMonitor( QObject* parent = 0 );
    ~GPRSMonitor();
private slots:
    void dataAccountsChanged();
    void gprsStateChanged();
    void currentOperatorChanged();
    void servicesAdded();

private:
    QFileSystemWatcher* watcher;
    QList<QNetworkDevice*> knownGPRSDevices;
    QValueSpaceObject* vso;
    QNetworkRegistration* netReg;
    QCommServiceManager* commManager;
    bool umts;
};

#endif
