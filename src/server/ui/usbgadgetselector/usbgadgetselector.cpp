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

#include "usbgadgetselector.h"

#include "uifactory.h"

#include <QUsbManager>
#include <QUsbEthernetGadget>
#include <QUsbSerialGadget>
#include <QUsbStorageGadget>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QAction>
#include <QSoftMenuBar>
#include <QMenu>

#include <QTranslatableSettings>

#include <QtopiaService>
#include <QtopiaIpcEnvelope>

#include <QDebug>
#include <qtopialog.h>

/*!
    \class UsbGadgetSelector
    \inpublicgroup QtConnectivityModule
    \ingroup QtopiaServer::GeneralUI
    \brief The UsbGadgetSelector class allows the user to choose how the device communicates via the USB interface.

    This dialog presents a list of services that operate over the USB interface and allows
    the user to choose the desired service.  Once selected the chosen service is started.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
    Creates a UsbGadgetSelector instance with the given \a parent and \a flags.
*/
UsbGadgetSelector::UsbGadgetSelector(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_selectedItem(0)
{
    setObjectName(QLatin1String("usbgadgetselector"));
    setWindowTitle(tr("USB Connection Mode"));

    QSettings settings("Trolltech", "Usb");
    settings.beginGroup("ConnectionMode");

    m_service = settings.value("Service", QString()).toString();
    m_application = settings.value("Application", QString()).toString();

    m_manager = new QUsbManager(this);
    connect(m_manager, SIGNAL(cableConnectedChanged(bool)),
            this, SLOT(cableConnectedChanged(bool)));

    QVBoxLayout *layout = new QVBoxLayout;

    if (m_manager->cableConnected()) {
        m_label = new QLabel(tr("USB cable connected.  Choose connection mode."));
    } else {
        m_label = new QLabel(tr("USB cable not connected."));
    }
    m_label->setWordWrap(true);
    layout->addWidget(m_label);

    m_services = new QListWidget;
    m_services->setWordWrap(true);
    m_services->setSortingEnabled(true);
    m_services->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_services);
    connect(m_services, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(itemActivated(QListWidgetItem*)));

    m_rememberChoice = new QAction(tr("Don't ask again"), this);
    m_rememberChoice->setCheckable(true);
    if (settings.value("RememberChoice", false).toBool())
        m_rememberChoice->setChecked(true);

    QMenu *menu = QSoftMenuBar::menuFor(this);
    menu->addAction(m_rememberChoice);

    loadServices();

    setLayout(layout);
}

/*!
    \internal
*/
void UsbGadgetSelector::accept()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup("ConnectionMode");

    settings.setValue("RememberChoice", m_rememberChoice->isChecked());

    if (m_selectedItem) {
        settings.setValue("Service", m_selectedItem->data(Qt::UserRole));
        settings.setValue("Application", m_selectedItem->data(Qt::UserRole + 1));
    } else {
        settings.remove("Service");
        settings.remove("Application");
    }

    QDialog::accept();
}

/*!
    \internal
*/
void UsbGadgetSelector::cableConnectedChanged(bool connected)
{
    if (connected)
        m_label->setText(tr("USB cable connected.  Choose connection mode."));
    else
        reject();
}

void UsbGadgetSelector::itemActivated(QListWidgetItem *item)
{
    if (m_selectedItem == item)
        return;

    QFont font;
    if (m_selectedItem) {
        m_selectedItem->font();
        font.setBold(false);
        m_selectedItem->setFont(font);
    }

    m_selectedItem = item;
    font = m_selectedItem->font();
    font.setBold(true);
    m_selectedItem->setFont(font);
}

/*!
    \internal
*/
void UsbGadgetSelector::loadServices()
{
    foreach (const QString &service, QtopiaService::list()) {
        if (!service.startsWith("UsbGadget/"))
            continue;

        QSettings config(QtopiaService::config(service), QSettings::IniFormat);
        config.beginGroup("Service");
        const QString gadgetName = config.value("Gadget").toString();
        if (gadgetName.isEmpty())
            continue;

        QUsbGadget *gadget = new QUsbGadget(gadgetName);
        if (!gadget->available()) {
            delete gadget;
            continue;
        }
        delete gadget;

        foreach (const QString &app, QtopiaService::apps(service)) {
            QString appConfig = QtopiaService::appConfig(service, app);

            QTranslatableSettings config(appConfig, QSettings::IniFormat);

            config.beginGroup("Standard");

            QListWidgetItem *item = new QListWidgetItem(config.value("Name", app).toString());

            QString icon = config.value("Icon").toString();
            if (!icon.isEmpty())
                item->setIcon(QIcon(":icon/" + icon));

            item->setData(Qt::UserRole, service);
            item->setData(Qt::UserRole + 1, app);

            m_services->addItem(item);
            if (m_service == service &&
                m_application == app) {
                itemActivated(item);
                m_services->setCurrentItem(item);
            }
        }
    }
}

UIFACTORY_REGISTER_WIDGET(UsbGadgetSelector);

