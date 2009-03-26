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

#include "dtmfaudio.h"

#include <QCommServiceManager>


class DtmfAudioPrivate
{
public:
    DtmfAudioPrivate(DtmfAudio *parent)
        : q(parent), commManager(0), tones(0), exclusive(true), duration(200)
    {
    }


    void _q_servicesChanged()
    {
        if (commManager && commManager->supports<QTelephonyTones>().count() > 0) {
            if (!tones) {
                tones = new QTelephonyTones( QString(), q );
                QObject::connect( tones, SIGNAL(toneStopped(QString)),
                        q, SLOT(_q_toneStopped(QString)) );
                commManager->deleteLater();
                commManager = 0;
            }
        }
    }

    void _q_toneStopped(const QString& toneId)
    {
        if (toneIds.contains(toneId)) {
            emit q->toneStopped(toneId);
            toneIds.remove(toneId);
        }
    }

    void stopAllTones()
    {
        if (!tones) return;

        foreach(QString toneId, toneIds) {
            tones->stopTone(toneId);
        }

        toneIds.clear();
    }

    DtmfAudio *q;
    QCommServiceManager *commManager;
    QTelephonyTones *tones;
    QSet<QString> toneIds;
    bool exclusive;
    int duration;
};

/*!
    \class DtmfAudio
    \inpublicgroup QtTelephonyModule
    \brief The DtmfAudio class provides a simple interface for playing DTMF tones.
    \ingroup QtopiaServer::Telephony

    DTMF tones can be played concurrently or mutually exclusive. If the exclusive behavior
    is chosen any currently playing tone is stopped and the new tone played. The exclusivePlay() property
    should be used to determine the behaviour. The duration() determines for how long
    a DTMF is played.

    playDtmfTone() can be used to play a tone indefinitely by using -1 as duration parameter.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QTelephonyTones
*/


/*!
    \fn void DtmfAudio::toneStopped(const QString& toneId)

    Emitted when the DTMF tone identified by \a toneId stopped playing. This can happen due to a a call to stopAllTones(),
    stopDtmfTone() or when the playing stopped due to a given timeout.
*/

/*!
    Creates a new DtmfAudio instance with the given \a parent.
*/
DtmfAudio::DtmfAudio(QObject *parent)
    : QObject(parent)
{
    d = new DtmfAudioPrivate(this);
    d->commManager = new QCommServiceManager(this);
    connect(d->commManager, SIGNAL(servicesChanged()), this, SLOT(_q_servicesChanged()));
    d->_q_servicesChanged();
}

/*!
    Destroys the DtmfAudio object.
*/
DtmfAudio::~DtmfAudio()
{
    delete d;
}

/*!
    \property DtmfAudio::exclusivePlay
    \brief whether a request to play a DTMF tone should stop already playing dtmf tones.

    Setting this property to true announces to the system that any new request to play a
    DTMF tone should stop already playing tones.

    By default this property is set to true.
*/
bool DtmfAudio::exclusivePlay() const
{
    return d->exclusive;
}

void DtmfAudio::setExclusivePlay(bool exclusive)
{
    d->exclusive = exclusive;
}

/*!
    \property DtmfAudio::duration
    \brief how long the DTMF tone should be played.

    This property indicates the number of milliseconds to play a tone before automatically
    stopping. The property value must be greater then 0.

    By default this property is set to 200.
*/
int DtmfAudio::duration() const
{
    return d->duration;
}

void DtmfAudio::setDuration( int duration )
{
    if (duration > 0)
        d->duration = duration;
}

/*!
    This function determines the DTMF tone for the given Qt \a keycode and
    plays the tone for the given \l duration().
*/
void DtmfAudio::playDtmfKeyTone( int keycode )
{
    if (!d->tones)
        return;

    if (d->exclusive)
        d->stopAllTones();

    QTelephonyTones::Tone tone = (QTelephonyTones::Tone)-1;
    switch ( keycode ) {
        case Qt::Key_0: tone = QTelephonyTones::Dtmf0; break;
        case Qt::Key_1: tone = QTelephonyTones::Dtmf1; break;
        case Qt::Key_2: tone = QTelephonyTones::Dtmf2; break;
        case Qt::Key_3: tone = QTelephonyTones::Dtmf3; break;
        case Qt::Key_4: tone = QTelephonyTones::Dtmf4; break;
        case Qt::Key_5: tone = QTelephonyTones::Dtmf5; break;
        case Qt::Key_6: tone = QTelephonyTones::Dtmf6; break;
        case Qt::Key_7: tone = QTelephonyTones::Dtmf7; break;
        case Qt::Key_8: tone = QTelephonyTones::Dtmf8; break;
        case Qt::Key_9: tone = QTelephonyTones::Dtmf9; break;
        case Qt::Key_Asterisk: tone = QTelephonyTones::DtmfStar; break;
        case Qt::Key_NumberSign: tone = QTelephonyTones::DtmfHash; break;
        default: break;
    }
    if ( !tone )
        qWarning() << "Cannot recognize the key as a number key.";
    QString toneId = d->tones->startTone( tone, d->duration );
    d->toneIds.insert(toneId);
}

/*!
    This function plays the given DTMF \a tone for the given \a duration.
    If \a duration is -1 the tone will be played indefinitely. Note that the duration()
    property is ignored when calling this function.

    The returned value identifies the tone and casn be used to stop it via stopDtmfTone().

    /sa stopDtmfTone()
*/
QString DtmfAudio::playDtmfTone(QTelephonyTones::Tone tone, int duration)
{
    if (!d->tones)
        return QString();

    if (d->exclusive)
        d->stopAllTones();

    QString toneId = d->tones->startTone( tone, duration );
    d->toneIds.insert(toneId);

    return toneId;
}

/*!
    This function stops the tone identified by \a toneId.

    \sa playDtmfTone()
*/
void DtmfAudio::stopDtmfTone(const QString& toneId)
{
    if (d->tones)
        d->tones->stopTone(toneId);
}

/*!
    Stops all currently playing DTMF tones started by this object.
*/
void DtmfAudio::stopAllTones()
{
    d->stopAllTones();
}

QTOPIA_TASK_PROVIDES(DtmfAudio,DtmfAudio);
QTOPIA_TASK(DtmfAudio,DtmfAudio);

#include "moc_dtmfaudio.cpp"
