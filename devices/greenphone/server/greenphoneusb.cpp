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

#include <QSettings>

#include "qtopiaserverapplication.h"

#include "greenphoneethernetgadget.h"
#include "greenphoneserialgadget.h"

//! [1]
class GreenphoneUsbGadgetTask
{
public:
    static void loadProviders();
};
//! [1]

//! [2]
void GreenphoneUsbGadgetTask::loadProviders()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup("PeripheralController");

    QList<QByteArray> supportedGadgets = settings.value("SupportedGadgets").toByteArray().split(',');
    QByteArray defaultGadget = settings.value("DefaultGadget").toByteArray();

    foreach (QByteArray gadget, supportedGadgets) {
        if (gadget == "GreenphoneEthernet") {
            GreenphoneEthernetGadgetProvider *gp = new GreenphoneEthernetGadgetProvider("Greenphone");
            if (gadget == defaultGadget)
                gp->activate();
/*        } else if (gadget == "GreenphoneStorage") {
            GreenphoneStorageGadgetProvider *gp = new GreenphoneStorageGadgetProvider("Greenphone");
            if (gadget == defaultGadget)
                gp->activate(); */
        } else if (gadget == "GreenphoneSerial") {
            GreenphoneSerialGadgetProvider *gp = new GreenphoneSerialGadgetProvider("Greenphone");
            if (gadget == defaultGadget)
                gp->activate();
        }
    }
}
//! [2]

//! [3]
QTOPIA_STATIC_TASK(GreenphoneUsbGadget, GreenphoneUsbGadgetTask::loadProviders());
//! [3]

