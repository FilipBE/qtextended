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

#include <qmodemcallvolume.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qtopiaipcadaptor.h>
#include <QtopiaIpcEnvelope>

#include <QSettings>
#include <QTimer>

#include <QDebug>

/*!
    \class QModemCallVolume
    \inpublicgroup QtCellModule

    \preliminary
    \brief The QModemCallVolume class implements call volume functionality for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+VGR} and \c{AT+VGT} commands.

    QModemCallVolume implements the QCallVolume telephony interface.  Client applications
    should use QCallVolume instead of this class to access the modem's call volume
    functionality.

    \sa QCallVolume
*/

class QModemVolumeService : public QtopiaIpcAdaptor
{
    Q_OBJECT
    enum AdjustType { Relative, Absolute };

public:
    explicit QModemVolumeService( QModemCallVolume *callVolume );
    ~QModemVolumeService();

public slots:
    void setVolume(int volume);
    void increaseVolume(int increment);
    void decreaseVolume(int decrement);
    void setMute(bool mute);

private slots:
    void registerService();

private:
    QModemCallVolume *callVolume;
};

QModemVolumeService::QModemVolumeService( QModemCallVolume *callVolume )
    : QtopiaIpcAdaptor("QPE/AudioVolumeManager/QModemVolumeService", callVolume)
{
    this->callVolume = callVolume;

    publishAll(Slots);

    QTimer::singleShot(0, this, SLOT(registerService()));
}

QModemVolumeService::~QModemVolumeService()
{
}

void QModemVolumeService::setVolume(int volume)
{
    callVolume->setSpeakerVolume(volume);
}

void QModemVolumeService::increaseVolume(int increment)
{
    callVolume->setSpeakerVolume(callVolume->speakerVolume() + increment);
}

void QModemVolumeService::decreaseVolume(int decrement)
{
    callVolume->setSpeakerVolume(callVolume->speakerVolume() - decrement);
}

void QModemVolumeService::setMute(bool mute)
{
    Q_UNUSED(mute);
}

void QModemVolumeService::registerService()
{
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Phone") << QString("QPE/AudioVolumeManager/QModemVolumeService");
}


class QModemCallVolumePrivate
{
public:
    QModemService *service;
    QModemVolumeService *mvs;

    void notifyAudioVolumeManager(int val)
    {
        QString volume;
        volume.setNum(val);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
    }

};


/*!
    Create an AT-based call volume handler for \a service.
*/
QModemCallVolume::QModemCallVolume( QModemService *service )
    : QCallVolume( service->service(), service, QCommInterface::Server )
{
    d = new QModemCallVolumePrivate;
    d->service = service;
    d->mvs = new QModemVolumeService(this);

    QTimer::singleShot(0, this, SLOT(initialize()));
}

/*!
    Destroy this AT-based call volume handler.
*/
QModemCallVolume::~QModemCallVolume()
{
    delete d;
}

/*!
    Returns true if the modem requires delayed initialization of call volume
    settings; false otherwise.  The default implementation returns false.

    Subclasses that reimplement this function to return true, must call
    callVolumesReady() when the modem is ready to accept call volume related
    commands.

    \sa callVolumesReady()
*/
bool QModemCallVolume::hasDelayedInit() const
{
    return false;
}

/*!
    \reimp
*/
void QModemCallVolume::setSpeakerVolume( int volume )
{
    int min = value("MinimumSpeakerVolume").toInt();
    int max = value("MaximumSpeakerVolume").toInt();
    int boundedVolume = qBound(min, volume, max);
    int temp = (max-min)*boundedVolume;

    d->service->chat( QString("AT+VGR=%1").arg(boundedVolume) );
    setValue( "SpeakerVolume", boundedVolume );
    d->notifyAudioVolumeManager( temp == 0 ? 0 : (100/(max-min)*boundedVolume) );
    emit speakerVolumeChanged(boundedVolume);

}

/*!
    \reimp
*/
void QModemCallVolume::setMicrophoneVolume( int volume )
{
    int boundedVolume = qBound(value("MinimumMicrophoneVolume").toInt(), volume,
                              value("MaximumMicrophoneVolume").toInt());

    d->service->chat( QString("AT+VGT=%1").arg(boundedVolume) );
    setValue( "MicrophoneVolume", boundedVolume );
    emit microphoneVolumeChanged(boundedVolume);
}

/*!
    Indicate that the modem is now ready to accept call volume related
    commands.  See hasDelayedInit() for more information.

    \sa hasDelayedInit()
*/
void QModemCallVolume::callVolumesReady()
{
    d->service->chat( "AT+VGR=?", this, SLOT(vgrRangeQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGR?", this, SLOT(vgrQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGT=?", this, SLOT(vgtRangeQuery(bool,QAtResult)) );
    d->service->chat( "AT+VGT?", this, SLOT(vgtQuery(bool,QAtResult)) );
}

void QModemCallVolume::initialize()
{
    if (!hasDelayedInit())
        callVolumesReady();
}

void QModemCallVolume::vgrRangeQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    int minimum = 0;
    int maximum = 0;

    QAtResultParser parser( result );
    while ( parser.next( "+VGR:" ) ) {
        QList<QAtResultParser::Node> nodes = parser.readList();
        if (!nodes.isEmpty()) {
            if (nodes.at(0).isRange()) {
                minimum = nodes.at(0).asFirst();
                maximum = nodes.at(0).asLast();
            }
        }
    }

    setValue( "MinimumSpeakerVolume", minimum );
    setValue( "MaximumSpeakerVolume", maximum );
}

void QModemCallVolume::vgrQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    QAtResultParser parser( result );
    if ( parser.next( "+VGR:" ) ) {
        int modemDefault = parser.readNumeric();

        QSettings cfg( "Trolltech", "Phone" );
        cfg.beginGroup( "CallVolume" );

        int defaultVolume = cfg.value( "SpeakerVolume", modemDefault ).toInt();

        if (defaultVolume != modemDefault)
            d->service->chat( QString("AT+VGR=%1").arg(defaultVolume) );

        setValue( "SpeakerVolume", defaultVolume );
    }
}

void QModemCallVolume::vgtRangeQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    int minimum = 0;
    int maximum = 0;

    QAtResultParser parser( result );
    if ( parser.next( "+VGT:" ) ) {
        QList<QAtResultParser::Node> nodes = parser.readList();
        if (!nodes.isEmpty()) {
            if (nodes.at(0).isRange()) {
                minimum = nodes.at(0).asFirst();
                maximum = nodes.at(0).asLast();
            }
        }
    }

    setValue( "MinimumMicrophoneVolume", minimum );
    setValue( "MaximumMicrophoneVolume", maximum );
}

void QModemCallVolume::vgtQuery(bool ok, const QAtResult &result)
{
    if (!ok)
        return;

    QAtResultParser parser( result );
    if ( parser.next( "+VGT:" ) ) {
        int modemDefault = parser.readNumeric();

        QSettings cfg( "Trolltech", "Phone" );
        cfg.beginGroup( "CallVolume" );

        int defaultVolume = cfg.value( "MicrophoneVolume", modemDefault ).toInt();

        if (defaultVolume != modemDefault)
            d->service->chat( QString("AT+VGT=%1").arg(defaultVolume) );

        setValue( "MicrophoneVolume", defaultVolume );
    }
}

#include <qmodemcallvolume.moc>
