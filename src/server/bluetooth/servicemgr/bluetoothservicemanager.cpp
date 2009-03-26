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

#include "bluetoothservicemanager.h"

#include "qtopiaserverapplication.h"
#include <QBluetoothServiceController>
#include <QValueSpaceObject>
#include <QValueSpaceItem>
#include <qtopiaipcadaptor.h>
#include <qtopialog.h>

#include <QSettings>
#include <QHash>
#include <QSet>


// The default security for a service.
static int DEFAULT_SERVICE_SECURITY =
        int(QBluetooth::Authenticated | QBluetooth::Encrypted);

// The path at which Bluetooth service settings are stored in the value space.
static const QString VALUE_SPACE_PATH = "Communications/Bluetooth/Services";

// The name of the QSettings file in which Bluetooth service settings are
// stored persistently.
static const QString SERVICE_SETTINGS = "BluetoothServices";


class BluetoothServiceSettings
{
public:
    BluetoothServiceSettings();
    ~BluetoothServiceSettings();

    QHash<QString, QVariant> loadSettings(const QString &name);

    void setValue(const QString &name, const QString &key, const QVariant &value);
    void setAllValues(const QString &name, const QHash<QString, QVariant> &settings);

    QVariant value(const QString &name, const QString &key);

    QSettings m_settingsFile;
    QValueSpaceObject *m_settingsValueSpace;
    QSet<QString> m_persistentSettings;
};

BluetoothServiceSettings::BluetoothServiceSettings()
    : m_settingsFile("Trolltech", SERVICE_SETTINGS),
      m_settingsValueSpace(new QValueSpaceObject(VALUE_SPACE_PATH))
{
    // These are the settings that need to be stored across reboots (i.e. in
    // the QSettings object and not just the Value Space)
    m_persistentSettings << "Autostart" << "Security";
}

BluetoothServiceSettings::~BluetoothServiceSettings()
{
    delete m_settingsValueSpace;
}

/*
    Returns the saved settings for service \a name. If there are no saved
    settings, returns the default settings.
 */
QHash<QString, QVariant> BluetoothServiceSettings::loadSettings(const QString &name)
{
    // Temporary settings that are only written to the value space
    QHash<QString, QVariant> settings;
    settings["DisplayName"] = QString();    // translatable name
    settings["State"] = (int)QBluetoothServiceController::NotRunning;

    // Persistent settings (these are also in value space, but get saved
    // permanently too)
    // Use defaults if no settings saved for this service
    QSettings saved("Trolltech", SERVICE_SETTINGS);
    saved.beginGroup(name);
    settings["Autostart"] = saved.value("Autostart", true).toBool();
    settings["Security"] = saved.value("Security", DEFAULT_SERVICE_SECURITY).toInt();

    return settings;
}


void BluetoothServiceSettings::setValue(const QString &name, const QString &key, const QVariant &value)
{
    m_settingsValueSpace->setAttribute(name + '/' + key, value);

    // some settings need to be saved into config for persistence
    if (m_persistentSettings.contains(key))
        m_settingsFile.setValue(name + '/' + key, value);
}

void BluetoothServiceSettings::setAllValues(const QString &name, const QHash<QString, QVariant> &settings)
{
    QList<QString> keys = settings.keys();
    for (int i=0; i<keys.size(); i++)
        setValue(name, keys[i], settings.value(keys[i]));
}

QVariant BluetoothServiceSettings::value(const QString &name, const QString &key)
{
    return QValueSpaceItem(VALUE_SPACE_PATH + '/' + name).value(key);
}



//=============================================================


/*
    This class passes messages over IPC between the BluetoothServiceManager and
    a QBluetoothAbstractService.
 */
class ServiceMessenger : public QtopiaIpcAdaptor
{
    friend class BluetoothServiceManager;
    Q_OBJECT
public:
    ServiceMessenger(BluetoothServiceManager *manager);

    BluetoothServiceManager *m_manager;

public slots:
    void registerService(const QString &name, const QString &displayName);
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

signals:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options);
};

ServiceMessenger::ServiceMessenger(BluetoothServiceManager *manager)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceProviders", manager),
      m_manager(manager)
{
    publishAll(SignalsAndSlots);
}

void ServiceMessenger::registerService(const QString &name, const QString &displayName)
{
    m_manager->registerService(name, displayName);
}

void ServiceMessenger::serviceStarted(const QString &name, bool error, const QString &desc)
{
    m_manager->serviceStarted(name, error, desc);
}

void ServiceMessenger::serviceStopped(const QString &name)
{
    m_manager->serviceStopped(name);
}


//===============================================================


/*
    This class passes messages over IPC between the BluetoothServiceManager
    and those who want to receive messages about services, or send messages
    to change service settings (e.g. QBluetoothServiceController).
 */
class ServiceUserMessenger : public QtopiaIpcAdaptor
{
    friend class BluetoothServiceManager;
    Q_OBJECT

public:
    ServiceUserMessenger(BluetoothServiceManager *parent);
    ~ServiceUserMessenger();

public slots:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

signals:
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

private:
    BluetoothServiceManager *m_manager;
};

ServiceUserMessenger::ServiceUserMessenger(BluetoothServiceManager *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceListeners", parent),
      m_manager(parent)
{
    publishAll(SignalsAndSlots);
}

ServiceUserMessenger::~ServiceUserMessenger()
{
}

void ServiceUserMessenger::startService(const QString &name)
{
    m_manager->startService(name);
}

void ServiceUserMessenger::stopService(const QString &name)
{
    m_manager->stopService(name);
}

void ServiceUserMessenger::setServiceSecurity(const QString &name,
        QBluetooth::SecurityOptions options)
{
    m_manager->setServiceSecurity(name, options);
}


//===============================================================

/*!
    \class BluetoothServiceManager
    \inpublicgroup QtBluetoothModule
    \brief The BluetoothServiceManager class controls Bluetooth services.

    The Bluetooth Service Manager communicates over IPC with the
    Bluetooth services in order to control and track their states and
    attributes. It also communicates with other parties, such as
    the Bluetooth settings application, who are interested in
    controlling and tracking Bluetooth services.

    The presence of the Bluetooth Service Manager provides several benefits:
    \list
    \o It automatically starts a Bluetooth service when it is created if it
    was still running at the end of the last Qt Extended session (or whenever it
    was last terminated).
    \o It records generic Bluetooth service settings across Qt Extended sessions.
    When a service is created, if it has previously registered with the 
    Bluetooth Service Manager, the manager will set the service to use the
    the security options that were previously assigned to the service.
    \o External parties can control and access Qt Extended Bluetooth services
    through QBluetoothServiceController.
    \endlist

    Without the presence of the Bluetooth Service Manager, components such as
    the QBluetoothServiceController class and the Bluetooth settings
    application will not function properly, as they rely on communication with
    the Bluetooth Service Manager in order to access and control Bluetooth
    services.

    You can create custom Bluetooth services that will be automatically
    integrated into the Qt Extended Bluetooth framework, and which will be
    accessible by the Bluetooth Service Manager, by subclassing
    QBluetoothAbstractService. This is how the built-in Bluetooth
    services, such as QBluetoothHandsfreeService and QbluetoothHeadsetServices, are
    implemented.

    The manager stores persistent service settings in \c BluetoothServices.conf.
    A default configuration file is provided at
    \c etc/default/Trolltech/BluetoothServices.conf. This can be modified to
    provide default settings for services.

    See the \l {bluetooth-servicemanager.html} {Bluetooth Service Management Framework}
    page for more details on the Bluetooth Service Manager's communication
    architecture.
  
    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.

    \ingroup QtopiaServer::Task::Bluetooth
*/

/*!
    Constructs a BluetoothServiceManager with the given \a parent.
*/
BluetoothServiceManager::BluetoothServiceManager(QObject *parent)
    : QObject(parent),
      m_settings(new BluetoothServiceSettings),
      m_serviceIpc(new ServiceMessenger(this)),
      m_serviceUserIpc(new ServiceUserMessenger(this))
{
}

/*!
    Destructor.
*/
BluetoothServiceManager::~BluetoothServiceManager()
{
    delete m_settings;
}

/*!
    \internal
    Called when a service first registers itself with \a name and
    \a displayName.

    If the service is set to autostart (in the config settings), the manager
    will immediately send a start() message to the service.
 */
void BluetoothServiceManager::registerService(const QString &name, const QString &displayName)
{
    // request from a service provider to register a service.

    qLog(Bluetooth) << "BluetoothServiceManager: registering" << name;

    // load saved/default settings for this service
    QHash<QString, QVariant> settings = m_settings->loadSettings(name);
    settings["DisplayName"] = displayName;

    // write settings to value space / conf files so everyone
    // (i.e. QBluetoothServiceController) can see them
    m_settings->setAllValues(name, settings);

    if (settings["Autostart"].toBool())
        startService(name);
}

/*!
    \internal
    Starts the service \a name.
 */
void BluetoothServiceManager::startService(const QString &name)
{
    qLog(Bluetooth) << "BluetoothServiceManager: start service" << name;

    m_settings->setValue(name, "State", (int)QBluetoothServiceController::Starting);

    // set the security options before starting the service, in case the
    // service has trouble setting its security if it has already started
    int options = m_settings->value(name, "Security").toInt();
    emit m_serviceIpc->setSecurityOptions(name, QBluetooth::SecurityOptions(options));

    // tell service to start
    emit m_serviceIpc->startService(name);
}

/*!
    \internal
    Called when service \a name emits its started() signal with \a error
    and \a desc.
 */
void BluetoothServiceManager::serviceStarted(const QString &name, bool error, const QString &desc)
{
    qLog(Bluetooth) << "BluetoothServiceManager::serviceStarted()" << name << error << desc;

    if (error) {
        m_settings->setValue(name, "State", (int)QBluetoothServiceController::NotRunning);

    } else {
        m_settings->setValue(name, "State", (int)QBluetoothServiceController::Running);

        // Since the service has been started, set to autostart it next time.
        // This allows for autostarts across reboots
        m_settings->setValue(name, "Autostart", true);
    }

    // pass on message to e.g. QBluetoothServiceController
    emit m_serviceUserIpc->serviceStarted(name, error, desc);
}

/*!
    \internal
    Stops the service \a name.
 */
void BluetoothServiceManager::stopService(const QString &name)
{
    qLog(Bluetooth) << "BluetoothServiceManager: stop service" << name;

    //m_settings->setValue(name, "ChangingState", true);

    // Don't autostart it next time
    // This allows for controlling auto-starting of services across reboots
    m_settings->setValue(name, "Autostart", false);

    // tell service to stop
    emit m_serviceIpc->stopService(name);
}

/*!
    \internal
    Called when service \a name emits its stopped() signal.
 */
void BluetoothServiceManager::serviceStopped(const QString &name)
{
    qLog(Bluetooth) << "BluetoothServiceManager::serviceStopped()" << name;

    m_settings->setValue(name, "State", (int)QBluetoothServiceController::NotRunning);
    m_settings->setValue(name, "Autostart", false);

    // pass on message to e.g. QBluetoothServiceController
    emit m_serviceUserIpc->serviceStopped(name);
}

/*!
    \internal
    Sets the security options for the service \a name to \a options.
 */
void BluetoothServiceManager::setServiceSecurity(const QString &name, QBluetooth::SecurityOptions options)
{
    qLog(Bluetooth) << "BluetoothServiceManager::setServiceSecurity" << name;
    m_settings->setValue(name, "Security", (int)options);

    // tell service to set security options (we just have to assume it worked)
    emit m_serviceIpc->setSecurityOptions(name, options);
}


// Make this a QPE server task
QTOPIA_TASK(BluetoothServiceManager, BluetoothServiceManager);

#include "bluetoothservicemanager.moc"
