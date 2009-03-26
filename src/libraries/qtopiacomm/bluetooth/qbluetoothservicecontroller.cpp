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

#include "qbluetoothservicecontroller.h"

#include <qtopiaipcadaptor.h>
#include <QValueSpaceItem>

/*
    This receives IPC messages from the BluetoothServiceManager when a service
    is registered, modified, etc. It's enabled the "other end" of the messages
    to and from ServiceUserMessenger in src/server/bluetoothservicemanager.cpp.
 */

class QBluetoothServiceControllerPrivate : public QtopiaIpcAdaptor
{
    friend class QBluetoothServiceController;
    Q_OBJECT

public:
    QBluetoothServiceControllerPrivate(QBluetoothServiceController *parent);
    ~QBluetoothServiceControllerPrivate();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name);

    QString displayName(const QString &name);
    QStringList services();

public slots:
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void serviceStopped(const QString &name);

signals:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setServiceSecurity(const QString &name, QBluetooth::SecurityOptions);

private:
    QVariant serviceValue(const QString &name, const QString &attr) const;

    static const QString VALUE_SPACE_PATH;
    QBluetoothServiceController *m_parent;
};



const QString QBluetoothServiceControllerPrivate::VALUE_SPACE_PATH = "Communications/Bluetooth/Services";

QBluetoothServiceControllerPrivate::QBluetoothServiceControllerPrivate(QBluetoothServiceController *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceListeners", parent),
      m_parent(parent)
{
    publishAll(SignalsAndSlots);
}

QBluetoothServiceControllerPrivate::~QBluetoothServiceControllerPrivate()
{
}

void QBluetoothServiceControllerPrivate::start(const QString &name)
{
    emit startService(name);
}

void QBluetoothServiceControllerPrivate::stop(const QString &name)
{
    emit stopService(name);
}

QBluetoothServiceController::ServiceState QBluetoothServiceControllerPrivate::state(const QString &name) const
{
    return QBluetoothServiceController::ServiceState(
                    serviceValue(name, "State").toInt());
}

void QBluetoothServiceControllerPrivate::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    emit setServiceSecurity(name, options);
}

QBluetooth::SecurityOptions QBluetoothServiceControllerPrivate::securityOptions(const QString &name)
{
    QVariant options = serviceValue(name, "Security");
    if (options.isValid())
        return static_cast<QBluetooth::SecurityOptions>(options.toInt());

    // no valid security options
    return 0;
}

QString QBluetoothServiceControllerPrivate::displayName(const QString &name)
{
    return serviceValue(name, "DisplayName").toString();
}

QStringList QBluetoothServiceControllerPrivate::services()
{
    return QValueSpaceItem(VALUE_SPACE_PATH).subPaths();
}

void QBluetoothServiceControllerPrivate::serviceStarted(const QString &name, bool error, const QString &desc)
{
    emit m_parent->started(name, error, desc);
}

void QBluetoothServiceControllerPrivate::serviceStopped(const QString &name)
{
    emit m_parent->stopped(name);
}

QVariant QBluetoothServiceControllerPrivate::serviceValue(const QString &name, const QString &attr) const
{
    return QValueSpaceItem(VALUE_SPACE_PATH + '/' + name).value(attr);
}


/*!
    \class QBluetoothServiceController
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothServiceController class provides a means to control and access information for Qt Extended Bluetooth services.

    This class allows the programmer to start and stop Qt Extended Bluetooth
    services, set their security options, and view a service's attributes,
    such as its state, name and security options.

    Qt Extended has a number of built-in Bluetooth services, as found in the
    Bluetooth settings application. Any of these services can be controlled
    through an instance of QBluetoothServiceController. You can also create
    your own Qt Extended Bluetooth services that will be controllable through
    this interface; simply subclass QBluetoothAbstractService. (See the
    QBluetoothAbstractService class documentation for more details.)

    \ingroup qtopiabluetooth
 */

/*!
    \enum QBluetoothServiceController::ServiceState

    Defines the possible states for a service which may be returned from state().

    \value NotRunning The service is not running.
    \value Starting The service is starting.
    \value Running The service is running.
*/

/*!
    Constructs a QBluetoothServiceController with parent object \a parent.
 */
QBluetoothServiceController::QBluetoothServiceController(QObject *parent)
    : QObject(parent),
      m_data(new QBluetoothServiceControllerPrivate(this))
{
}

/*!
    Destroys a QBluetoothServiceController.
 */
QBluetoothServiceController::~QBluetoothServiceController()
{
}

/*!
    Starts the service named \a name.

    The started() signal will be emitted when the service has started.

    \sa stop()
 */
void QBluetoothServiceController::start(const QString &name)
{
    m_data->start(name);
}

/*!
    Stops the service named \a name.

    The stopped() signal will be emitted when the service has stopped.

    \sa start()
 */
void QBluetoothServiceController::stop(const QString &name)
{
    m_data->stop(name);
}

/*!
    Returns the state of the service named \a name.

    \sa stopped(), started()
 */
QBluetoothServiceController::ServiceState QBluetoothServiceController::state(const QString &name) const
{
    return m_data->state(name);
}

/*!
    Sets the security options for the service with name \a name to the given
    \a options.

    \sa securityOptions()
 */
void QBluetoothServiceController::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    m_data->setSecurityOptions(name, options);
}

/*!
    Returns the security options for the service named \a name.

    \sa setSecurityOptions()
 */
QBluetooth::SecurityOptions QBluetoothServiceController::securityOptions(const QString &name) const
{
    return m_data->securityOptions(name);
}

/*!
    Returns the user-friendly, internationalized display name for the service
    named \a name.
 */
QString QBluetoothServiceController::displayName(const QString &name) const
{
    return m_data->displayName(name);
}

/*!
    Returns a list of the names of all known Bluetooth services within Qtopia.
 */
QStringList QBluetoothServiceController::services() const
{
    return m_data->services();
}


/*!
    \fn void QBluetoothServiceController::started(const QString &name, bool error, const QString &description)

    This signal is emitted when the service named \a name has started or
    failed while attempting to start. If there was a failure, \a error is \c true
    and \a description provides the human-readable error description.

    \sa start(), stopped()
 */

/*!
    \fn void QBluetoothServiceController::stopped(const QString &name)

    This signal is emitted when the service named \a name has stopped.

    \sa stop(), started()
*/


#include "qbluetoothservicecontroller.moc"
