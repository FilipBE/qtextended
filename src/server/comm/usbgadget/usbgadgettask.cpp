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

#include "usbgadgettask.h"

#include <QSettings>
#include <QTimer>

#include <QUsbManager>
#include <QUsbEthernetGadget>
#include <QUsbStorageGadget>
#include <QUsbSerialGadget>

#include <QtopiaIpcEnvelope>
#include <QFileSystemFilter>
#include <QtopiaService>
#include <QtopiaNetwork>
#include <QNetworkState>

#include <QDSServices>
#include <QDSAction>

#include "qtopiaserverapplication.h"
#include "taskmanagerentry.h"

#include "uifactory.h"
#include "applicationlauncher.h"

#define UNMOUNT_TRIES 2

/*!
    \class UsbGadgetTask
    \inpublicgroup QtConnectivityModule
    \ingroup QtopiaServer::Task
    \brief The UsbGadgetTask class loads the default USB gadget providers.

    The set of USB gadget providers that is loaded by the UsbGadgetTask is defined in the
    \c {Trolltech/Usb.conf} configuration file.  The USB gadget specified by the
    \c {PeripheralController/DefaultGadget]} setting will be automatically activated.

    \code
        [PeripheralController]
        Path=/sys/devices/platform/dummy_udc
        SupportedGadgets="Ethernet,Storage,Serial"
        DefaultGadget=Ethernet

        [Ethernet]
        ...

        [Storage]
        ...

        [Serial]
        ...
    \endcode

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QUsbManager, QUsbGadget
*/

/*!
  \internal
*/
UsbGadgetTask::UsbGadgetTask(QObject *parent)
    : QObject(parent),
      m_dialog(0),
      m_tries(UNMOUNT_TRIES),
      m_action(0)
{
    loadProviders();

    m_manager = new QUsbManager(this);
    connect(m_manager, SIGNAL(cableConnectedChanged(bool)),
            this, SLOT(cableConnectedChanged(bool)));

    connect(QtopiaServerApplication::instance(), SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)));
}

void UsbGadgetTask::loadProviders()
{
    QSettings settings("Trolltech", "Usb");
    settings.beginGroup("PeripheralController");

    QList<QByteArray> supportedGadgets = settings.value("SupportedGadgets").toByteArray().split(',');
    QByteArray defaultGadget = settings.value("DefaultGadget").toByteArray();

    foreach (QByteArray gadget, supportedGadgets) {
        if (gadget == "Ethernet") {
            QUsbEthernetGadgetProvider *gp = new QUsbEthernetGadgetProvider("Standard");
            if (gadget == defaultGadget)
                gp->activate();
        } else if (gadget == "Storage") {
            QUsbStorageGadgetProvider *gp = new QUsbStorageGadgetProvider("Standard");
            if (gadget == defaultGadget)
                gp->activate();
        } else if (gadget == "Serial") {
            QUsbSerialGadgetProvider *gp = new QUsbSerialGadgetProvider("Standard");
            if (gadget == defaultGadget)
                gp->activate();
        }
    }
}

void UsbGadgetTask::activateEthernet()
{
    QUsbEthernetGadget *ethernetGadget = new QUsbEthernetGadget;
    if (ethernetGadget && ethernetGadget->available()) {
        connect(ethernetGadget, SIGNAL(activated()), this, SLOT(ethernetActivated()));
        connect(ethernetGadget, SIGNAL(activateFailed()), this, SLOT(ethernetDeactivated()));
        connect(ethernetGadget, SIGNAL(deactivated()), this, SLOT(ethernetDeactivated()));

        ethernetGadget->activate();

        m_gadget = ethernetGadget;
    }
}

class ExportableFileSystemFilter : public QFileSystemFilter
{
public:
    bool filter(QFileSystem *fs)
    {
        if (fs->disk() == "HOME" ||
            fs->disk() == "PREFIX")
            return false;

        return true;
    }
};

class BackingStoreFileSystemFilter : public QFileSystemFilter
{
public:
    BackingStoreFileSystemFilter(const QStringList &disks)
        : m_disks(disks)
    {
    }

    bool filter(QFileSystem *fs)
    {
        return m_disks.contains(fs->disk());
    }
private:
    QStringList m_disks;
};

void UsbGadgetTask::activateStorage()
{
    QUsbStorageGadget *storageGadget = new QUsbStorageGadget;
    if (storageGadget && storageGadget->available()) {
        if (storageGadget->backingStore().isEmpty()) {
            qLog(USB) << "No backing store set, exporting all storage locations";

            ExportableFileSystemFilter filter;
            QList<QFileSystem *> exportable =
                QStorageMetaInfo::instance()->fileSystems(&filter, false);

            if (exportable.isEmpty()) {
                qLog(USB) << "No exportable storage locations";
                return;
            }

            foreach (QFileSystem *fileSystem, exportable) {
                storageGadget->addBackingStore(fileSystem->disk());
            }
            QCoreApplication::processEvents();
        }

        // disconnect all backing stores
        BackingStoreFileSystemFilter filter(storageGadget->backingStore());
        m_fileSystems = QStorageMetaInfo::instance()->fileSystems(&filter, false);
        connect(QStorageMetaInfo::instance(), SIGNAL(disksChanged()),
                this, SLOT(activateStorageGadget()));

        bool disconnected = false;
        foreach (QFileSystem *fileSystem, m_fileSystems) {
            if (fileSystem->isConnected()) {
                qLog(USB) << "disconnecting" << fileSystem->disk() << fileSystem->path();
                fileSystem->disconnect();
                disconnected = true;
            }
        }

        if (!disconnected)
            QTimer::singleShot(0, this, SLOT(activateStorageGadget()));

        connect(storageGadget, SIGNAL(deactivated()),
                this, SLOT(storageDeactivated()));
        connect(storageGadget, SIGNAL(activateFailed()),
                this, SLOT(storageDeactivated()));

        m_gadget = storageGadget;
    }
}

void UsbGadgetTask::activateStorageGadget()
{
    foreach (QFileSystem *fileSystem, m_fileSystems) {
        // file system still conntected abort
        if (fileSystem->isConnected()) {
            return;
        }
    }

    // all file systems disconnected
    disconnect(QStorageMetaInfo::instance(), SIGNAL(disksChanged()),
                this, SLOT(activateStorageGadget()));

    // unmount all backing stores
    qLog(USB) << "unmounting all filesystems";
    foreach (QFileSystem *fileSystem, m_fileSystems) {
        if (fileSystem->isMounted()) {
            qLog(USB) << "unmounting" << fileSystem->disk() << fileSystem->path();
            if (!fileSystem->unmount()) {
                if (--m_tries > 0) {
                    qLog(USB) << "unmount of" << fileSystem->disk()
                              << "failed, delaying umount."
                              << m_tries << "tries left.";

                    QTimer::singleShot(1000, this, SLOT(activateStorageGadget()));
                } else {
                    qLog(USB) << "unmount of" << fileSystem->disk() << "failed, aborting.";
                    QTimer::singleShot(0, this, SLOT(storageDeactivated()));
                }

                return;
            } else {
                m_tries = UNMOUNT_TRIES;
            }
        }
    }

    // all file systems unmounted
    m_gadget->activate();

    // Message box must be WindowModal or the context bar won't accept mouse events
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, tr("USB Mass Storage"), tr("Eject before disconnecting"), QMessageBox::Ok);
    msgBox->setWindowModality(Qt::WindowModal);
    QtopiaApplication::execDialog(msgBox);
}

void UsbGadgetTask::showDialog()
{
    if (!m_dialog) {
        m_dialog = UIFactory::createDialog("UsbGadgetSelector");
        if (!m_dialog) {
            qWarning("UsbGadgetSelector dialog not available");
            return;
        }

        connect(m_dialog, SIGNAL(accepted()), this, SLOT(activateGadget()));
        connect(m_dialog, SIGNAL(finished(int)), this, SLOT(deleteDialog()));

        m_taskEntry =
            new TaskManagerEntry(tr("USB Connection Mode"), "qpe/UsbSettings", m_dialog);
        connect(m_taskEntry, SIGNAL(activated()), this, SLOT(showDialog()));
        m_taskEntry->show();
    }

    QtopiaApplication::showDialog(m_dialog);
}

void UsbGadgetTask::deleteDialog()
{
    m_taskEntry->hide();
    delete m_taskEntry;
    m_taskEntry = 0;
    m_dialog->deleteLater();
    m_dialog = 0;
}

void UsbGadgetTask::cableConnectedChanged(bool connected)
{
    if (connected) {
        QSettings settings("Trolltech", "Usb");
        settings.beginGroup("ConnectionMode");

        // ask the user for desired connection mode
        if (settings.value("RememberChoice", false).toBool() &&
            !settings.value("Service", QString()).toString().isEmpty()) {
            activateGadget();
        } else {
            qLog(USB) << "No saved connection mode, displaying dialog";
            showDialog();
        }
    } else {
        foreach (QUsbGadget *gadget, m_manager->activeGadgets()) {
            QString service;
            if (gadget->interfaceName() == "QUsbEthernetGadget")
                service = "UsbGadget/Ethernet";
            else if (gadget->interfaceName() == "QUsbSerialGadget")
                service = "UsbGadget/Serial";
            else if (gadget->interfaceName() == "QUsbStorageGadget")
                service = "UsbGadget/Storage";

            delete gadget;

            if (!service.isEmpty()) {
                foreach (QString app, QtopiaService::apps(service)) {
                    QString channel = QtopiaService::channel(service, app);

                    QtopiaIpcEnvelope e(channel, "stopUsbService(QString)");
                    e << service;
                }
            }
        }

        // Force deactivation of gadgets. This will fail if a service needs to
        // deactivate the gadget, if for example it is holding open a device
        // which prevents the module from unloading.
        m_manager->deactivateGadgets();
    }
}

void UsbGadgetTask::activateGadget()
{
    if (!m_manager->cableConnected())
        return;

    QSettings settings("Trolltech", "Usb");
    settings.beginGroup("ConnectionMode");

    QString service = settings.value("Service", QString()).toString();
    if (!service.isEmpty()) {
        qLog(USB) << "activating USB service" << service;

        QSettings config(QtopiaService::config(service), QSettings::IniFormat);
        config.beginGroup("Service");

        if (config.value("Multiple", 0).toInt()) {
            foreach (const QString &app, QtopiaService::apps(service)) {
                QString channel = QtopiaService::channel(service, app);

                QtopiaIpcEnvelope e(channel, "startUsbService(QString)");
                e << service;
            }
        } else {
            QString app = settings.value("Application", QString()).toString();
            QString channel = QtopiaService::channel(service, app);

            QtopiaIpcEnvelope e(channel, "startUsbService(QString)");
            e << service;
        }
    }
}

void UsbGadgetTask::appMessage(const QString &msg, const QByteArray &data)
{
    QDataStream stream(data);
    if (msg == "startUsbService(QString)") {
        QString service;
        stream >> service;

        if (service == "UsbGadget/Ethernet") {
            activateEthernet();
        } else if (service == "UsbGadget/Storage") {
            activateStorage();
        }
    } else if (msg == "usbConnectionMode()") {
        showDialog();
    }
}

void UsbGadgetTask::ethernetActivated()
{
    QtopiaIpcEnvelope e("QPE/NetworkState", "updateNetwork()");

    QUsbEthernetGadget *ethernetGadget = qobject_cast<QUsbEthernetGadget *>(m_gadget);
    if (!ethernetGadget)
        return;

    QtopiaNetwork::Type deviceType = QNetworkState::deviceType(ethernetGadget->interface());
    if (QtopiaNetwork::availableNetworkConfigs(deviceType).isEmpty()) {
        QDSServices service("text/x-netsetup-settings");

        if (service.count() == 0)
            return;

        m_action = new QDSAction(service.first());
        connect(m_action, SIGNAL(response(QUniqueId)), this, SLOT(qdsResponse(QUniqueId)));
        connect(m_action, SIGNAL(error(QUniqueId,QString)),
                this, SLOT(qdsError(QUniqueId,QString)));

        foreach (QString path, Qtopia::installPaths()) {
            QFile data(path + "etc/network/lan.conf");
            if (data.exists()) {
                m_action->invoke(
                    QDSData(data, QMimeType("text/x-netsetup-settings")));

                break;
            }
        }
    }
}

void UsbGadgetTask::ethernetDeactivated()
{
    QtopiaIpcEnvelope e("QPE/NetworkState", "updateNetwork()");
    delete m_gadget;
    m_gadget = 0;
}

void UsbGadgetTask::storageDeactivated()
{
    delete m_gadget;
    m_gadget = 0;

    foreach (QFileSystem *fileSystem, m_fileSystems) {
        if (!fileSystem->isMounted())
            fileSystem->mount();

        if (!fileSystem->isConnected())
            fileSystem->connect();
    }
}

void UsbGadgetTask::qdsResponse(const QUniqueId &)
{
    m_action->deleteLater();
    m_action = 0;
}

void UsbGadgetTask::qdsError(const QUniqueId &, const QString &message)
{
    qLog(USB) << __PRETTY_FUNCTION__ << message;
    m_action->deleteLater();
    m_action = 0;
}

QTOPIA_TASK(UsbGadget, UsbGadgetTask);

static QWidget *connectionMode()
{
    QtopiaIpcEnvelope e("QPE/Application/qpe", "usbConnectionMode()");
    return 0;
}

QTOPIA_SIMPLE_BUILTIN(usbconnectionmode, connectionMode);

