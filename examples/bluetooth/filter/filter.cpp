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

#include <QLabel>
#include <QVBoxLayout>
#include <QVariant>
#include <QComboBox>

#include <QBluetoothAddress>
#include <QBluetoothRemoteDeviceDialog>
#include <QBluetoothRemoteDevice>

#include <QtopiaApplication>
#include <QAction>
#include <QMenu>
#include <QSoftMenuBar>

#include "filter.h"

class GreenphoneFilter : public QBluetoothRemoteDeviceDialogFilter
{
public:
    bool filterAcceptsDevice(const QBluetoothRemoteDevice &device);
};

bool GreenphoneFilter::filterAcceptsDevice(const QBluetoothRemoteDevice &device)
{
    // Greenphones have address range of:
    // 00:19:65:00:08:01-00:19:65:00:08:64
    // 00:19:65:04:00:00-00:19:65:04:04:B0

    QString addrString = device.address().toString();
    if (addrString.startsWith("00:19:65:00:08:")) {
        int offset = addrString.mid(15, 2).toInt(0, 16);
        if ( (offset >= 0x1) && (offset <= 0x64) )
            return true;
        return false;
    }

    if (addrString.startsWith("00:19:65:04:")) {
        int offset = (addrString.mid(12, 2).toInt(0, 16) << 8) +
            addrString.mid(15, 2).toInt(0, 16);
        if ( (offset >= 0x0000) && (offset <= 0x04B0) )
            return true;
    }

    return false;
}

Filter::Filter(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
    , greenphoneFilter(0)
{
    QAction *startAction = new QAction(tr("Start discovery..."), this);
    connect(startAction, SIGNAL(triggered()), this, SLOT(showDialog()));
    QSoftMenuBar::menuFor(this)->addAction(startAction);

    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *helpLabel = new QLabel(tr("<P>Select the device type to show and profile to validate below"
                                      " then use the context menu to start discovery & validation"),
                                   this);
    helpLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    helpLabel->setWordWrap(true);
    helpLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(helpLabel);

    QLabel *filterLabel = new QLabel(tr("Select filter:"), this);
    filterChoice = new QComboBox(this);
    filterChoice->addItem(tr("Phone"), static_cast<uint>(QBluetooth::Phone));
    filterChoice->addItem(tr("Computer"), static_cast<uint>(QBluetooth::Computer));
    filterChoice->addItem(tr("Audio/Video"), static_cast<uint>(QBluetooth::AudioVideo));
    filterChoice->addItem(tr("Imaging"), static_cast<uint>(QBluetooth::Imaging));
    filterChoice->addItem(tr("Peripheral"), static_cast<uint>(QBluetooth::Peripheral));
    filterChoice->addItem(tr("LAN Access"), static_cast<uint>(QBluetooth::LANAccess));
    filterChoice->addItem(tr("Greenphone"));

    greenphoneFilter = new GreenphoneFilter;

    layout->addWidget(filterLabel);
    layout->addWidget(filterChoice);

    QLabel *serviceLabel = new QLabel(tr("Ensure service exists:"), this);
    serviceChoice = new QComboBox(this);
    serviceChoice->addItem(tr("PUSH"), static_cast<uint>(QBluetooth::ObjectPushProfile));
    serviceChoice->addItem(tr("Headset"), static_cast<uint>(QBluetooth::HeadsetProfile));
    serviceChoice->addItem(tr("Handsfree"), static_cast<uint>(QBluetooth::HandsFreeProfile));
    serviceChoice->addItem(tr("Dial Up"), static_cast<uint>(QBluetooth::DialupNetworkingProfile));
    serviceChoice->addItem(tr("FTP"), static_cast<uint>(QBluetooth::FileTransferProfile));

    layout->addWidget(serviceLabel);
    layout->addWidget(serviceChoice);

    setLayout(layout);
    setWindowTitle(tr("Bluetooth Filter"));
}

Filter::~Filter()
{
    delete greenphoneFilter;
}

void Filter::showDialog()
{
    QBluetooth::SDPProfile profile =
        static_cast<QBluetooth::SDPProfile>(serviceChoice->itemData(serviceChoice->currentIndex()).toUInt());
    QSet<QBluetooth::SDPProfile> profiles;
    profiles.insert(profile);

    QBluetoothAddress addr;

    if (filterChoice->currentText() == QString(tr("Greenphone"))) {
        addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(0, profiles, greenphoneFilter);
    } else {
        QBluetoothRemoteDeviceDialogFilter filter;
        QBluetooth::DeviceMajor major =
            static_cast<QBluetooth::DeviceMajor>(filterChoice->itemData(filterChoice->currentIndex()).toUInt());
        QSet<QBluetooth::DeviceMajor> majors;
        majors.insert(major);
        filter.setAcceptedDeviceMajors(majors);
        addr = QBluetoothRemoteDeviceDialog::getRemoteDevice(0, profiles, &filter);
    }

    if (!addr.isValid()) {
        QMessageBox::warning(this, tr("Selection Error"),
                QString(tr("<P>No device selected")));
    }
}
