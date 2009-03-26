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

#include "btpinhelper.h"
#include "qtopiaserverapplication.h"

#include <QtopiaApplication>
#include <QString>
#include <QByteArray>
#include <QTextStream>
#include <QSettings>
#include <QTimer>
#include <QValueSpaceItem>

#include <qtopiaservices.h>
#include <qphoneprofile.h>
#include <qpassworddialog.h>
#include <qtopiaipcenvelope.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothpasskeyrequest.h>
#include <qbluetoothremotedevice.h>
#include <qtopialog.h>
#include <qvibrateaccessory.h>

/*!
    \class BluetoothPasskeyAgentTask
    \inpublicgroup QtBluetoothModule
    \ingroup QtopiaServer::Task::Bluetooth
    \brief The BluetoothPasskeyAgentTask class provides a default passkey agent for performing Bluetooth passkey authentications.

    The BluetoothPasskeyAgentTask class implements a Qt Extended global passkey agent.  The
    internal implementation uses the QPasswordDialog to ask the user for
    the passkey.

    This class is part of the Qt Extended server and cannot be used by other QtopiaApplications.
    \sa QBluetoothPasskeyAgent
 */

/*!
    Constructs a new BTPinHelper class.  The QObject parent is given
    by \a parent.
 */
BTPinHelper::BTPinHelper(QObject *parent)
    : QBluetoothPasskeyAgent("DefaultPasskeyAgent", parent),
      m_passDialog(new QPasswordDialog)
{
    m_passDialog->setInputMode(QPasswordDialog::Plain);

    registerDefault();
}

/*!
    Destructor.
 */
BTPinHelper::~BTPinHelper()
{
    delete m_passDialog;
}

/*!
    \reimp
 */
void BTPinHelper::requestPasskey(QBluetoothPasskeyRequest &req)
{
    QBluetoothLocalDevice device(req.localDevice());
    QBluetoothRemoteDevice remote(req.remoteDevice());
    device.updateRemoteDevice(remote);

    if (m_passDialog->isVisible()) {
        qLog(Bluetooth) << "Got new pairing request for"
            << remote.address().toString()
            << "but another pairing already in progress, rejecting new pair request";
        req.setRejected();
        return;
    }

    QString msg = "<P>";
    msg += tr("You are trying to pair with %1 [%2]. Please enter a PIN. You will need to enter the same PIN on the other device.", "%1=name, %2=address")
            .arg(remote.name()).arg(remote.address().toString());

    // brighten the screen
    QtopiaServiceRequest e1("QtopiaPowerManager", "setBacklight(int)");
    e1 << -1; // read brightness setting from config
    e1.send();

    // vibrate the phone if profile allows
    QPhoneProfileManager manager;
    QPhoneProfile profile = manager.activeProfile();
    if (profile.vibrate()) {
        QVibrateAccessory acc;
        acc.setVibrateNow(true);
        QTimer::singleShot(1000, this, SLOT(stopVibration()));
    }

    m_passDialog->reset();
    m_passDialog->setPrompt(msg);
    m_passDialog->setWindowModality(Qt::WindowModal);
    QtopiaApplication::execDialog(m_passDialog);

    QString pairPin = m_passDialog->password();
    if (!pairPin.isEmpty())
        req.setPasskey(pairPin);
    else
        req.setRejected();
}

/*!
    \reimp
 */
void BTPinHelper::cancelRequest(const QString & /*localDevice*/, const QBluetoothAddress & /*remoteAddr*/)
{
    qLog(Bluetooth) << "BTPinHelper::cancelRequest()";
    m_passDialog->reject();
}

/*!
    \reimp
 */
void BTPinHelper::release()
{

}

/*!
    \internal
*/
void BTPinHelper::stopVibration()
{
    QVibrateAccessory acc;
    acc.setVibrateNow(false);
}

/*!
    Constructs a new BluetoothPasskeyAgentTask class.  The QObject parent is given
    by \a parent.
 */
BluetoothPasskeyAgentTask::BluetoothPasskeyAgentTask(QObject* parent)
    : QObject( parent )
{
    //we start this once the GUI is up and running
    serverWidgetVsi = new QValueSpaceItem("/System/ServerWidgets/Initialized", this);
    connect( serverWidgetVsi, SIGNAL(contentsChanged()), this, SLOT(delayedAgentStart()) );
    delayedAgentStart(); //in case its visible already
}

/*!
  \internal
  */
void BluetoothPasskeyAgentTask::delayedAgentStart()
{
    if ( serverWidgetVsi && serverWidgetVsi->value( QByteArray(), false ).toBool() ) {
        serverWidgetVsi->disconnect();
        serverWidgetVsi->deleteLater();
        serverWidgetVsi = 0;
        QTimer::singleShot( 5000, this, SLOT(activateAgent()) );
    }
}

/*!
  \internal
  */
void BluetoothPasskeyAgentTask::activateAgent()
{
    new BTPinHelper( this );
}

QTOPIA_TASK(DefaultBluetoothPassKeyAgent, BluetoothPasskeyAgentTask);
