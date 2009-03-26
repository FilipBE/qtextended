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

#include "qphonestatus.h"
#include <QSignalSource>
#include <qvaluespace.h>
#include <QVariant>
#include <QDebug>
#include <QStringList>

class QPhoneStatusPrivate
{
public:
    QValueSpaceItem *teleValueSpace;
    QValueSpaceItem *clockValueSpace;
    QValueSpaceItem *profileValueSpace;
    QValueSpaceItem *batteryValueSpace;
    bool haveIncoming;
    QSignalSource* signalSource; 
};

struct teleValueNameMapStruct {
    QPhoneStatus::StatusItem id;
    const char *name;
} teleValueNameMap[] = {
    { QPhoneStatus::OperatorName, "OperatorName" },
    { QPhoneStatus::MissedCalls, "MissedCalls" },
    { QPhoneStatus::NewMessages, "NewMessages" },
    { QPhoneStatus::ActiveCalls, "ActiveCalls" },
    { QPhoneStatus::Roaming, "Roaming" },
    { QPhoneStatus::Locked, "Locked" },
    { QPhoneStatus::CallDivert, "CallDivert" },
    { QPhoneStatus::NetworkRegistered, "NetworkRegistered" },
    { QPhoneStatus::VoIPRegistered, "VoIP/Registered" },
    { QPhoneStatus::Presence, "VoIP/Presence" },
    { QPhoneStatus::StatusItem(-1), 0 }
};

//---------------------------------------------------------------------------

/*!
  \class QPhoneStatus
    \inpublicgroup QtTelephonyModule

  \brief The QPhoneStatus class provides basic status information about
  the phone.

  This class provides the most commonly needed information for
  presentation on a secondary display.  The Qt Extended phone library can be
  used to retrieve complete telephony status information.

  A value can be queried using the QPhoneStatus::value() function.
  Whenever a status item changes the QPhoneStatus::statusChanged() signal
  is emitted.

  See \l{Tutorial: Dual Screen Display} for more information on programming
  for secondary displays.

  \ingroup telephony
*/

/*!
  \enum QPhoneStatus::StatusItem
  \value BatteryLevel battery charge level (0-100)
  \value SignalLevel phone signal level (0-100)
  \value MissedCalls the number of calls missed
  \value NewMessages the number of new messages
  \value ActiveCalls the number of active calls
  \value OperatorName the name of the operator
  \value Profile the profile currently in use
  \value Roaming true if currently roaming
  \value Locked true if the phone is locked
  \value Alarm true if the daily alarm is active
  \value CallDivert true if call diversion is enabled
  \value NetworkRegistered true if a network is registered
  \value VoIPRegistered true if a VoIP network is registered
  \value Presence true if presence is available
*/

/*! \fn void QPhoneStatus::incomingCall(const QString &number, const QString &name)
  This signal is emitted when an incoming call arrives.  The \a number
  and \a name parameters indicate the incoming call's known properties.
  They may be empty if the property is unknown (e.g. caller-id information
  is not available).
*/

/*! \fn void QPhoneStatus::statusChanged()
  This signal is emitted whenever any of the status information changes.

  \sa value()
*/

/*!
  Constructs a QPhoneStatus object with parent \a parent.
*/
QPhoneStatus::QPhoneStatus(QObject *parent)
    : QObject(parent)
{
    d = new QPhoneStatusPrivate;
    d->teleValueSpace = new QValueSpaceItem("/Telephony/Status");
    connect(d->teleValueSpace, SIGNAL(contentsChanged()),
            this, SIGNAL(statusChanged()));
    connect(d->teleValueSpace, SIGNAL(contentsChanged()),
            this, SLOT(phoneStatusChanged()));
    d->clockValueSpace = new QValueSpaceItem("/Clock");
    connect(d->clockValueSpace, SIGNAL(contentsChanged()),
            this, SIGNAL(statusChanged()));
    d->profileValueSpace = new QValueSpaceItem("/PhoneProfile");
    connect(d->profileValueSpace, SIGNAL(contentsChanged()),
            this, SIGNAL(statusChanged()));
    d->batteryValueSpace = new QValueSpaceItem("/Hardware/Accessories/QPowerSource/DefaultBattery");
    connect(d->batteryValueSpace, SIGNAL(contentsChanged()),
            this, SIGNAL(statusChanged()) );
    d->signalSource = new QSignalSource( "modem" );
    connect( d->signalSource, SIGNAL(signalStrengthChanged(int)), 
            this, SIGNAL(statusChanged()) );
    d->haveIncoming = false;
}

/*!
  Destroys the QPhoneStatus object.
*/
QPhoneStatus::~QPhoneStatus()
{
    delete d->teleValueSpace;
    delete d->clockValueSpace;
    delete d->profileValueSpace;
    delete d;
}

/*!
  Returns the value of status item \a item.  If no status is available
  for the requested item an invalid value is returned.

  \sa statusChanged()
*/
QVariant QPhoneStatus::value(StatusItem item)
{
    if (item == Alarm) {
        return d->clockValueSpace->value("Daily Alarm/Enabled").toString() == QLatin1String("true");
    } else if (item == Profile) {
        int current = d->profileValueSpace->value("Profiles/Selected").toInt();
        QString prof = QString("Profile %1/Name").arg(current, 2, 10, QChar('0'));
        QByteArray profStr = prof.toLatin1();
        return d->profileValueSpace->value(profStr);
    } else if ( item == SignalLevel ) {
        int value = d->signalSource->signalStrength();
        if ( value < 0 || d->signalSource->availability() != QSignalSource::Available )
            return QVariant();
        else
            return QVariant( value );
    } else if ( item == BatteryLevel ) {
        return d->batteryValueSpace->value( QLatin1String("Charge") , QVariant() );
    } else {
        int i = 0;
        while (teleValueNameMap[i].name) {
            if (teleValueNameMap[i].id == item) {
                QString name = teleValueNameMap[i].name;
                return d->teleValueSpace->value(name);
            }
            i++;
        }
    }
    return QVariant();
}

void QPhoneStatus::phoneStatusChanged()
{
    QVariant value = d->teleValueSpace->value("IncomingCall/Number");
    if (value.isValid() && !d->haveIncoming) {
        d->haveIncoming = true;
        QString number = value.toString();
        QString name = d->teleValueSpace->value("IncomingCall/Name").toString();
        emit incomingCall(number, name);
    } else if (!value.isValid() && d->haveIncoming) {
        d->haveIncoming = false;
        emit incomingCall(QString(), QString());
    }
}
