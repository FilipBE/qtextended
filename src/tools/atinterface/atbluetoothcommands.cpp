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

#include <stdio.h>

#include "atbluetoothcommands.h"
#include "atcommands.h"
#include "atparseutils.h"

#include <QtopiaIpcAdaptor>
#include <QAtUtils>
#include <QValueSpaceItem>

// This class was copied from src/server/bluetooth/hf/qbluetoothhfservice.cpp
class AtInterface_HandsfreeIpcAdaptor : public QtopiaIpcAdaptor
{
    Q_OBJECT
    friend class AtBluetoothCommands;

public:
    AtInterface_HandsfreeIpcAdaptor(AtBluetoothCommands *parent);

public slots:
    void setSpeakerVolume(int vol);
    void setMicrophoneVolume(int vol);

signals:
    void speakerVolumeChanged(int volume);
    void microphoneVolumeChanged(int volume);

private:
    AtBluetoothCommands *m_parent;
};

AtInterface_HandsfreeIpcAdaptor::AtInterface_HandsfreeIpcAdaptor(AtBluetoothCommands *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothHandsfree", parent), m_parent(parent)
{
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

void AtInterface_HandsfreeIpcAdaptor::setSpeakerVolume(int vol)
{
    m_parent->setSpeakerVolume(vol);
}

void AtInterface_HandsfreeIpcAdaptor::setMicrophoneVolume(int vol)
{
    m_parent->setMicrophoneVolume(vol);
}


AtBluetoothCommands::AtBluetoothCommands( AtCommands * parent ) : QObject( parent )
{
    atc = parent;
    m_adaptor = new AtInterface_HandsfreeIpcAdaptor(this);

    // Add commands that are specific to Bluetooth hands-free.
    if ( atc->options()->hasStartupOption( "handsfree" ) ) {
        atc->add( "+BLDN", this, SLOT(atbldn()) );
        atc->add( "+BRSF", this, SLOT(atbrsf(QString)) );
        atc->add( "+VGM",  this, SLOT(atvgm(QString)) );
        atc->add( "+VGS",  this, SLOT(atvgs(QString)) );
        atc->add( "+NREC", this, SLOT(atnrec(QString)) );
        atc->add( "+BVRA", this, SLOT(atbvra(QString)) );
        atc->add( "+BINP", this, SLOT(atbinp(QString)) );
        atc->add( "+BTRH", this, SLOT(atbtrh(QString)) );
    }
}

AtBluetoothCommands::~AtBluetoothCommands()
{
    if (m_adaptor)
        delete m_adaptor;
}


/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+BLDN Bluetooth Last Dialed Number}
    \compat

    The \c{AT+BLDN} command is used by Bluetooth hands-free (HF) devices
    to request that the last number dialed be re-dialed.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BLDN} \o \c{OK}
    \row \o \c{AT+BLDN}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atbldn()
{
    QValueSpaceItem item( "/Communications/Calls" );
    QString lastNumber = item.value("LastDialedCall").toString();

    QAtResult::ResultCode result = atc->manager()->callManager()->dial( lastNumber + ";" );
    if ( result != AtCallManager::Defer )
        atc->done( result );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BRSF Bluetooth Retrieve Supported Features}
    \compat

    The \c{AT+BRSF} command is used by Bluetooth hands-free (HF) devices
    to report the features that are supported by the hands-free unit,
    and to request the phone's audio gateway (AG) feature set.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BRSF=<HF supported features bitmap>}
         \o \c{+BRSF: <AG supported features bitmap>}
    \row \o \c{AT+BRSF=<HF supported features bitmap>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \endtable

    Set command is used by the hands-free device to send a bitmap
    of its supported features to the phone's audio gateway.  The audio
    gateway in turn responds with a separate bitmap of its supported
    features.

    \table
    \row \o \c{<HF supported features bitmap>}
         \o Decimal integer containing the following bits:
            \list
                \o 0 - EC / NR function
                \o 1 - Call waiting and 3-way calling
                \o 2 - CLI presentation capability.
                \o 3 - Voice recognition activation.
                \o 4 - Remote volume control
                \o 5 - Enhanced call status
                \o 6 - Enhanced call control
                \o 7-31 - Reserved
            \endlist
    \row \o \c{<AG supported features bitmap>}
         \o Decimal integer containing the following bits:
            \list
                \o 0 - Three way calling
                \o 1 - EC / NR function
                \o 2 - Voice recognition
                \o 3 - In-band ring
                \o 4 - Attach a number to voice tag
                \o 5 - Ability to reject a call
                \o 6 - Enhanced call status
                \o 7 - Enhanced call control
                \o 8 - Extended error result codes
                \o 9-31 - Reserved
            \endlist
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atbrsf(const QString &params)
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint bitmap = 0;
            if ( posn < (uint)params.length() ) {
                bitmap = QAtUtils::parseNumber( params, posn );
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)params.length() ) {
                // too many params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            Q_UNUSED(bitmap);   // TODO - save this somewhere for later.
            atc->send( "+BRSF: 225" );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+VGM Gain of Microphone}
    \compat

    The \c{AT+VGM} command is used by a Bluetooth hands-free (HF) device
    to report its current microphone gain level setting to the phone's
    audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VGM=<gain>} \o \c{OK}
    \row \o \c{AT+VGM=<gain>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \row \o \c{AT+VGM?} \o \c{+VGM: <gain>}
    \row \o \c{AT+VGM=?} \o \c{+VGM: (0-15)}
    \endtable

    \table
    \row \o \c{<gain>}
         \o Decimal integer indicating the microphone gain between 0 and
            15, where 15 is the maximum gain.
    \endtable

    Set command reports the HF's current gain level setting.  Get command
    reports the previous setting.  Test command returns the valid gain levels.

    The audio gateway may also send unsolicited \c{+VGM: <gain>} messages
    whenever the gain changes on the AG.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atvgm(const QString &params)
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint volume = 0;
            if ( posn < (uint)params.length() ) {
                volume = QAtUtils::parseNumber( params, posn );
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)params.length() ) {
                // too many params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            emit m_adaptor->microphoneVolumeChanged(volume);
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            // TODO: retrieve the current value and report it.
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+VGM: (0-15)" );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+VGS Gain of Speaker}
    \compat

    The \c{AT+VGS} command is used by a Bluetooth hands-free (HF) device
    to report its current speaker gain level setting to the phone's
    audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VGS=<gain>} \o \c{OK}
    \row \o \c{AT+VGS=<gain>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \row \o \c{AT+VGS?} \o \c{+VGS: <gain>}
    \row \o \c{AT+VGS=?} \o \c{+VGS: (0-15)}
    \endtable

    \table
    \row \o \c{<gain>}
         \o Decimal integer indicating the speaker gain between 0 and
            15, where 15 is the maximum gain.
    \endtable

    Set command reports the HF's current gain level setting.  Get command
    reports the previous setting.  Test command returns the valid gain levels.

    The audio gateway may also send unsolicited \c{+VGS: <gain>} messages
    whenever the gain changes on the AG.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atvgs(const QString &params)
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint volume = 0;
            if ( posn < (uint)params.length() ) {
                volume = QAtUtils::parseNumber( params, posn );
            } else {
                // not enough params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            if ( posn < (uint)params.length() ) {
                // too many params.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }

            emit m_adaptor->speakerVolumeChanged(volume);
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            // Should never happen
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+VGS: (0-15)" );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+NREC Control Noise Reduction and Echo Cancellation}
    \compat

    The \c{AT+NREC} command is used by a Bluetooth hands-free (HF) device
    to turn off the Noise Reduction and Echo Cancellation capabilities of
    the phone's audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+NREC=0} \o \c{OK}
    \row \o \c{AT+NREC=0}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atnrec(const QString &params)
{
    Q_UNUSED( params );
    atc->done( QAtResult::Error );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BVRA Control Bluetooth Voice Recognition}
    \compat

    The \c{AT+BVRA} command is used by a Bluetooth hands-free (HF) device
    to turn on and off the Voice Recognition capabilities of the phone's
    audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BVRA=<num>} \o \c{OK}
    \row \o \c{AT+BVRA=<num>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    The AG can also initiate Bluetooth Voice Recognition events by sending the
    {+BVRA=<num>} unsolicited result code.

    \table
    \row \o \c{<num>}
         \o Decimal integer. 0 indicates voice recognition functionality should be ended.  1 indicates
            voice recognition functionality should be started.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atbvra(const QString &params)
{
    Q_UNUSED( params );
    atc->done( QAtResult::Error );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BINP Bluetooth: Attach Phone Number to a Voice Tag}
    \compat

    The \c{AT+BINP} command can be used by the Bluetooth hands-free (HF) device
    to request Phone Number from the AG.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BINP=1} \o +BINP:<PhoneNumber>, \c{OK}
    \row \o \c{AT+BINP=1}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    Each time this command is sent, the Audio Gateway shall respond with a different Phone Number.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atbinp(const QString &params)
{
    Q_UNUSED( params );
    atc->done( QAtResult::Error );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BTRH Bluetooth Respond and Hold}
    \compat

    The \c{AT+BTRH} command can be used by the Bluetooth hands-free (HF) device
    to perform the Respond and Hold feature.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BTRH?} \o [+BTRH:<number>], \c{OK}
    \row \o \c{AT+BTRH = <num>} \o +BTRH:<num>, \c{OK}
    \row \o \c{AT+BTRH = <num>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    \table
    \header \o Num Value \o Description
    \row \o \c{0} \o Put the incoming call on hold.
    \row \o \c{1} \o Accept the call which was held.
    \row \o \c{2} \o Reject the call which was held.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtBluetoothCommands::atbtrh(const QString &params)
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            atc->done(QAtResult::Error);
        }
        break;

        case AtParseUtils::Get:
        {
            // Should never happen
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}
/*!
    This function is called by the Bluetooth Hands-Free
    IpcAdaptor when its setSpeakerVolume slot
    is called.  This function sends the unsolicited
    "+VGS: <new-volume>" notification.
*/
void AtBluetoothCommands::setSpeakerVolume(int volume)
{
    char buf[64];
    sprintf(buf, "+VGS: %d", volume);
    atc->send( buf );
}

/*!
    This function is called by the Bluetooth Hands-Free
    IpcAdaptor when its setMicrophoneVolume slot
    is called.  This function sends the unsolicited
    "+VGM: <new-volume>" notification.
*/
void AtBluetoothCommands::setMicrophoneVolume(int volume)
{
    char buf[64];
    sprintf(buf, "+VGM: %d", volume);
    atc->send( buf );
}

// we defined a class in this .cpp, so to avoid the vtable/undef errors:
#include "atbluetoothcommands.moc"



