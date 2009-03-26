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

#include <QTimer>
#include <QStringList>
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>
#include <QString>

#include <QBluetoothLocalDevice>
#include <QBluetoothSdpQuery>
#include <QBluetoothSdpRecord>
#include <QBluetoothAddress>
#include <QBluetoothSdpUuid>
#include <QBluetoothRemoteDeviceDialog>

#include <QtopiaApplication>
#include <QWaitWidget>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>

#include "query.h"

Query::Query(QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f)
{
    btDevice = new QBluetoothLocalDevice;

    setWindowTitle(tr("Bluetooth Query"));

    if (!btDevice->isValid()) {
        QLabel *label = new QLabel(tr("Bluetooth not available."), this);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        label->setWordWrap(true);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setCentralWidget(label);
        return;
    }

    QTimer::singleShot(0, this, SLOT(startQuery()));

    sdap = new QBluetoothSdpQuery(this);
    connect(sdap, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
        this, SLOT(searchComplete(QBluetoothSdpQueryResult)));

    waiter = new QWaitWidget(this);
    connect(waiter, SIGNAL(cancelled()), this, SLOT(cancelQuery()));

    startQueryAction = new QAction(tr("Query device..."), this);
    connect(startQueryAction, SIGNAL(triggered()), this, SLOT(startQuery()));
    QSoftMenuBar::menuFor(this)->addAction(startQueryAction);

    serviceList = new QListWidget(this);
    setCentralWidget(serviceList);
}

Query::~Query()
{
    
}

void Query::startQuery()
{
    canceled = false;
    waiter->setText(tr("Querying SDP..."));
    waiter->setCancelEnabled(true);

    startQueryAction->setVisible(false);
    serviceList->clear();
    QBluetoothAddress addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(this);

    if (!addr.isValid()) {
        QMessageBox::warning(this, tr("Query Error"),
                QString(tr("<P>No device selected")));
        startQueryAction->setVisible(true);
        return;
    }

    // Search using the L2CAP UUID to even find services
    // not in public browse group
    quint16 id = 0x0100;
    sdap->searchServices(addr, *btDevice, QBluetoothSdpUuid(id));

    waiter->show();
}

void Query::searchComplete(const QBluetoothSdpQueryResult &result)
{
    waiter->hide();

    startQueryAction->setVisible(true);

    if (canceled) {
        return;
    }

    if (!result.isValid()) {
        QMessageBox::warning(this, tr("Query Error"),
                QString(tr("<P>Error has occurred:"))
                        + result.error());
        return;
    }


    foreach (QBluetoothSdpRecord record, result.services()) {
        serviceList->addItem(record.serviceName());
    }
}

void Query::searchCancelled()
{
    waiter->hide();
    startQueryAction->setVisible(true);
}

void Query::cancelQuery()
{
    waiter->setText(tr("Aborting Query..."));
    waiter->setCancelEnabled(false);
    canceled = true;
}
