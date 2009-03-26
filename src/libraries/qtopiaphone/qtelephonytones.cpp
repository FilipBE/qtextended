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

#include <qtelephonytones.h>
#include <quuid.h>

/*!
    \class QTelephonyTones
    \inpublicgroup QtTelephonyModule

    \brief The QTelephonyTones class provides access to a tone generator for DTMF tones and other special telephony sounds.

    This class is intended for use by dialer user interfaces to generate
    DTMF tones for the user when digits are typed in prior to call setup.
    It can also be used to generate supervisory tones such as dial tones,
    busy signals, etc.

    When on a call, DTMF tones should be generated with QPhoneCall::tone()
    so that the DTMF information can be properly transmitted to the
    remote party.  The QTelephonyTones class should only be used for
    DTMF tones that occur during call setup.

    Some tones, such as dial tone and busy signals, may be country-specific.
    The server implementation of QTelephonyTones is responsible for generating
    the correct country-specific tone if it can determine the country.
    This allows dialer user interfaces to request a specific type of
    supervisory tone without being aware of the country-specific details.

    \sa QCommInterface, QPhoneCall::tone()
    \ingroup telephony
*/

/*!
    \enum QTelephonyTones::Tone
    This enum defines a telephony-related tone to be played using QTelephonyTones::startTone().

    \value Dtmf0 Tone for DTMF digit 0
    \value Dtmf1 Tone for DTMF digit 1
    \value Dtmf2 Tone for DTMF digit 2
    \value Dtmf3 Tone for DTMF digit 3
    \value Dtmf4 Tone for DTMF digit 4
    \value Dtmf5 Tone for DTMF digit 5
    \value Dtmf6 Tone for DTMF digit 6
    \value Dtmf7 Tone for DTMF digit 7
    \value Dtmf8 Tone for DTMF digit 8
    \value Dtmf9 Tone for DTMF digit 9
    \value DtmfStar Tone for DTMF digit *
    \value DtmfHash Tone for DTMF digit #
    \value DtmfA Tone for DTMF digit A
    \value DtmfB Tone for DTMF digit B
    \value DtmfC Tone for DTMF digit C
    \value DtmfD Tone for DTMF digit D
    \value Busy Busy signal when the other party cannot be reached
    \value Dial Dial tone
    \value Dial2 Secondary dial tone, for outside lines
    \value Alerting Tone indicating that the called party is alerting (ringing).
    \value CallWaiting Tone indicating that there is another call waiting.
    \value MessageWaiting Tone indicating that this is a message waiting.
    \value NoService Tone indicating that there is no network service.  This
           may be played instead of Dial to audibly indicate to the user
           that the current call attempt will probably not succeed.
           The tone provider could choose to play no tone at all for this case.
    \value User First user-defined tone.
*/

/*!
    Construct a new tone generation object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports tone generation.  If there is more than one
    service that supports tone generation, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QTelephonyTones objects for each.

    \sa QCommServiceManager::supports()
*/
QTelephonyTones::QTelephonyTones
	( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QTelephonyTones", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys this tone generation object.
*/
QTelephonyTones::~QTelephonyTones()
{
}

/*!
    Starts playing \a tone and returns an identifier for it.
    If \a duration is -1, the tone should play indefinitely
    until stopTone() is called.  If \a duration is not -1,
    it indicates the number of milliseconds to play the tone
    before automatically stopping.

    DTMF tones during a call should be played with QPhoneCall::tone()
    rather than this function so that the DTMF information can be
    properly transmitted to the remote party.  DTMF tones should
    only be played with this function when they occur outside a
    call during the dialing step.

    \sa stopTone(), toneStopped(), QPhoneCall::tone()
*/
QString QTelephonyTones::startTone( QTelephonyTones::Tone tone, int duration )
{
    QString id = QUuid::createUuid().toString();
    startTone( id, tone, duration );
    return id;
}

/*!
    Starts playing \a tone and associates it with \a id.
    If \a duration is -1, the tone should play indefinitely
    until stopTone() is called.  If \a duration is not -1,
    it indicates the number of milliseconds to play the tone
    before automatically stopping.

    DTMF tones during a call should be played with QPhoneCall::tone()
    rather than this function so that the DTMF information can be
    properly transmitted to the remote party.  DTMF tones should
    only be played with this function when they occur outside a
    call during the dialing step.

    \sa stopTone(), toneStopped(), QPhoneCall::tone()
*/
void QTelephonyTones::startTone
	( const QString& id, QTelephonyTones::Tone tone, int duration )
{
    invoke( SLOT(startTone(QString,QTelephonyTones::Tone,int)),
	    id, qVariantFromValue(tone), duration );
}

/*!
    Stops playing the tone associated with \a id.

    \sa startTone(), toneStopped()
*/
void QTelephonyTones::stopTone( const QString& id )
{
    invoke( SLOT(stopTone(QString)), id );
}

/*!
    \fn void QTelephonyTones::toneStopped( const QString& id )

    Signal that is emitted when the tone associated with \a id
    stops playing.

    \sa startTone(), stopTone()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QTelephonyTones::Tone)
