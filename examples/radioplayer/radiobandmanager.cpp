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

#include "radiobandmanager.h"
#ifdef QTOPIA_DUMMY_RADIO
#include "radiobanddummy.h"
#endif
#ifdef QTOPIA_CUSTOM_RADIO
#include "radiobandcustom.h"
#else
#include "radiobandv4l.h"
#endif
#include <QTimer>
#include <QSettings>

class RadioBandManagerPrivate
{
public:
    RadioBandManagerPrivate( RadioBandManager *obj )
    {
        scanning = false;
        scanForward = false;
        scanAll = false;
        startBand = -1;
        startFrequency = -1;
        currentBand = 0;
        muted = false;
        prevSignal = false;
        prevStereo = false;
        firstAfterFreqChange = false;

        // Open the best radio band driver we can find.
    #ifdef QTOPIA_DUMMY_RADIO
        QSettings settings( "Trolltech", "Radio" );
        settings.beginGroup( "Device" );
        if ( settings.value( "Dummy" ).toBool() )
            RadioBandDummy::createBands( bands, obj );
        else
    #endif
    #ifdef QTOPIA_CUSTOM_RADIO
            RadioBandCustom::createBands( bands, obj );
    #else
            RadioBandV4L::createBands( bands, obj );
    #endif
        if ( bands.size() == 0 ) {
            currentBand = -1;
        }
    }

    QList<RadioBand *> bands;
    bool scanning;
    bool scanForward;
    bool scanAll;
    QTimer *scanTimer;
    int startBand;
    RadioBand::Frequency startFrequency;
    int currentBand;
    bool muted;
    bool prevSignal;
    bool prevStereo;
    bool firstAfterFreqChange;
    QTimer *signalTimer;
    QTimer *signalBackoffTimer;
};

/*#
    Create a new radio device and attach it to \a parent.
*/
RadioBandManager::RadioBandManager( QObject *parent )
    : QObject( parent )
{
    d = new RadioBandManagerPrivate( this );
    d->scanTimer = new QTimer( this );
    connect( d->scanTimer, SIGNAL(timeout()), this, SLOT(scanTimeout()) );

    d->signalTimer = new QTimer( this );
    connect( d->signalTimer, SIGNAL(timeout()), this, SLOT(signalTimeout()) );

    d->signalBackoffTimer = new QTimer( this );
    d->signalBackoffTimer->setSingleShot( true );
    connect( d->signalBackoffTimer, SIGNAL(timeout()),
             this, SLOT(backoffSignalCheck()) );

    // Unmute and activate the first band.
    if ( d->currentBand >= 0 ) {
        RadioBand *band = currentBand();
        band->setMuted( false );
        band->setActive( true );
        startSignalCheck();
    }
}

/*#
    Destroy this radio device.
*/
RadioBandManager::~RadioBandManager()
{
    delete d;
}

/*#
    Get the list of all radio bands that can be tuned by this radio device.
    Returns an empty list if there is no radio device available.
*/
QList<RadioBand *> RadioBandManager::bands() const
{
    return d->bands;
}

/*#
    Get the band that is currently tuned in by this radio device.
    The return value will be an index into the bands() list, or -1
    if there are no bands.  The frequency() function specifies which
    frequency is being tuned on this band.
*/
int RadioBandManager::band() const
{
    return d->currentBand;
}

/*#
    Get the frequency that is currently tuned in by this radio device,
    or -1 if no band has been selected.  The band() function specifies
    which band is being tuned at this frequency.
*/
RadioBand::Frequency RadioBandManager::frequency() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->frequency();
    else
        return -1;
}

/*#
    Get the name of the currently tuned band.
*/
QString RadioBandManager::bandName() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->name();
    else
        return QString();
}

/*#
    Determine if the radio is muted.
*/
bool RadioBandManager::muted() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->muted();
    else
        return false;
}

/*#
    Determine if the station that is currently tuned in
    is broadcasting in stereo.
*/
bool RadioBandManager::stereo() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->stereo();
    else
        return false;
}

/*#
    Determine if the frequency that is currently turned in
    is showing signal from a station.
*/
bool RadioBandManager::signal() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->signal();
    else
        return false;
}

/*#
    Determine if the station that is currently turned in is
    broadcasting Radio Data System digital information.
*/
bool RadioBandManager::rds() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->rds();
    else
        return false;
}

/*#
    Determine if the current band can detect signal changes.
*/
bool RadioBandManager::signalDetectable() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->signalDetectable();
    else
        return false;
}

/*#
    Get the volume of the radio on the current band.
*/
int RadioBandManager::volume() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->volume();
    else
        return 0;
}

/*#
    Returns true if the audio is being routed to the hands-free speaker.

    \sa setSpeakerActive(), speakerPresent()
*/
bool RadioBandManager::speakerActive() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->speakerActive();
    else
        return false;
}

/*#
    Returns true if the radio band supports a hands-free speaker.
    The default return value is false.

    \sa speakerActive(), setSpeakerActive()
*/
bool RadioBandManager::speakerPresent() const
{
    RadioBand *band = currentBand();
    if ( band )
        return band->speakerPresent();
    else
        return false;
}

/*#
    Determine if this radio device is currently scanning for new stations.
*/
bool RadioBandManager::scanning() const
{
    return d->scanning;
}

/*#
    Get the radio band object for the current band,
    or null if there is no current band.
*/
RadioBand *RadioBandManager::currentBand() const
{
    int band = d->currentBand;
    if ( band < 0 || band >= d->bands.size() )
        return 0;
    else
        return d->bands[band];
}

/*#
    Get the radio band object for the band called \a name,
    or null if there is no such band.
*/
RadioBand *RadioBandManager::bandFromName( const QString& name ) const
{
    foreach ( RadioBand *band, d->bands ) {
        if ( band->name() == name )
            return band;
    }
    return 0;
}

/*#
    Format \a frequency as a user-viewable string.
    The output will be something like \c{97.35 MHz}.
*/
QString RadioBandManager::formatFrequency( RadioBand::Frequency frequency )
{
    QString formatted;

    // Format the frequency value as something semi-readable.
    if ( frequencyIsChannelNumber( frequency ) ) {
        // The frequency is probably a channel number on a
        // satellite-based radio network.
        formatted = tr("Channel %1").arg((int)frequency);
    } else if ( frequency >= 1000000000 ) {
        int tenthghz = (int)(frequency / 100000000);
        formatted = tr("%1.%2 GHz").arg(tenthghz / 10).arg(tenthghz % 10);
    } else if ( frequency > 50000000 ) {
        int hundredthmhz = (int)(frequency / 10000);
        formatted = tr("%1.%2%3 MHz").arg(hundredthmhz / 100)
                        .arg((hundredthmhz % 100) / 10)
                        .arg(hundredthmhz % 10);
    } else {
        formatted = tr("%1 kHz").arg(frequency / 1000);
    }

    return formatted;
}

/*#
    Determine if \a frequency appears to be a channel number on a
    sattelite-based radio network rather than a regular radio frequency.
*/
bool RadioBandManager::frequencyIsChannelNumber
        ( RadioBand::Frequency frequency )
{
    return ( frequency < 50000 );
}

/*#
    Set the current radio band to \a value.  It should be an index
    into the bands() list.  This will also change the frequency to the
    last selected frequency on the band.
*/
void RadioBandManager::setBand( int value )
{
    if ( value < 0 || value >= d->bands.size() )
        return;
    if ( d->currentBand != value ) {
        RadioBand *band = currentBand();
        band->setActive( false );
        d->currentBand = value;
        band = currentBand();
        band->setActive( true );
    }
}

/*#
    Set the frequency to be tuned to \a value.  If the frequency is on a
    different band than the current one, the band will be changed.
*/
bool RadioBandManager::setFrequency( RadioBand::Frequency value )
{
    int band = bandFromFrequency( value );
    if ( band >= 0 ) {
        RadioBand *bandinfo = d->bands[band];
        if ( band != d->currentBand ) {
            bandinfo->setActive( false );
            d->currentBand = value;
            bandinfo->setActive( true );
        }
        bandinfo->setFrequency( value );
        startSignalCheck();
        return true;
    } else {
        return false;
    }
}

/*#
    Set the frequency to be tuned to \a value on the specified \a band.
    If the frequency is on a different band than the current one,
    the band will be changed.
*/
bool RadioBandManager::setFrequency
        ( const QString& band, RadioBand::Frequency value )
{
    RadioBand *bandinfo = bandFromName( band );
    int bandNumber;
    if ( !bandinfo ) {
        // Try just the frequency, in case the band name is wrong.
        bandNumber = bandFromFrequency( value );
        if ( bandNumber >= 0 )
            bandinfo = d->bands[bandNumber];
    }
    if ( bandinfo ) {
        bandNumber = d->bands.indexOf( bandinfo );
        if ( bandNumber != d->currentBand ) {
            bandinfo->setActive( false );
            d->currentBand = bandNumber;
            bandinfo->setActive( true );
        }
        bandinfo->setFrequency( value );
        startSignalCheck();
        return true;
    } else {
        return false;
    }
}

/*#
    Initiate a scanning process to scan forward from the current
    frequency looking for a station.  The process will stop,
    and scanStopped() will be emitted, when a station is found
    or all bands have been completely scanned without finding a station.

    The scanFoundStation() signal will be emitted when a station is found.
    During the scan, the scanProgress() signal will be emitted to
    indicate the current frequency and band so that the user interface
    can give the user some feedback as to the progress.
*/
void RadioBandManager::scanForward()
{
    startScan( true, false );
}

/*#
    Initiate a scanning process to scan backward from the current
    frequency looking for a station.  The process will stop,
    and scanStopped() will be emitted, when a station is found
    or all bands have been completely scanned without finding a station.

    The scanFoundStation() signal will be emitted when a station is found.
    During the scan, the scanProgress() signal will be emitted to
    indicate the current frequency and band so that the user interface
    can give the user some feedback as to the progress.
*/
void RadioBandManager::scanBackward()
{
    startScan( false, false );
}

/*#
    Initiate a scanning process to scan all bands and frequencies looking
    for stations.  Scanning will start at the current frequency on the
    current band.

    As each station is found, scanFoundStation() will be emitted.  The
    scanStopped() signal will be called once all bands and frequencies have
    been scanned.  The radio will be left tuned into the last station that
    was found when the scan stops.

    During the scan, the scanProgress() signal will be emitted to
    indicate the current frequency and band so that the user interface
    can give the user some feedback as to the progress.
*/
void RadioBandManager::scanAll()
{
    // Stop the current scan, if any.
    stopScan();

    // If we have signal already, then report our first station
    // just before we move on to the next frequency in range.
    if ( signal() ) {
        RadioBand *band = currentBand();
        emit scanProgress( band->frequency(), d->currentBand );
        emit scanFoundStation( band->frequency(), d->currentBand );
    }

    // Start up a new scan.
    startScan( true, true );
}

/*#
    Stop the current scanning process and emit scanStopped().
*/
void RadioBandManager::stopScan()
{
    if ( d->scanning ) {
        d->scanning = false;
        d->scanTimer->stop();
        emit scanStopped();
    }
}

/*#
    Set the frequency on the current band to one step higher than the
    current value.
*/
void RadioBandManager::stepForward()
{
    RadioBand *band = currentBand();
    if ( !band )
        return;

    RadioBand::Frequency freq = band->frequency();
    RadioBand::Frequency step = band->scanStep();

    // Normalize to the nearest step position.
    if ( ( freq % step ) != 0 )
        freq -= freq % step;

    // Move up one step and wrap around if necessary.
    freq += step;
    if ( freq > band->highFrequency() )
        freq = band->lowFrequency();

    // Set the new frequency.
    setFrequency( freq );
}

/*#
    Set the frequency on the current band to one step lower than the
    current value.
*/
void RadioBandManager::stepBackward()
{
    RadioBand *band = currentBand();
    if ( !band )
        return;

    RadioBand::Frequency freq = band->frequency();
    RadioBand::Frequency step = band->scanStep();

    // Normalize to the nearest step position.
    if ( ( freq % step ) != 0 )
        freq -= freq % step;

    // Move down one step and wrap around if necessary.
    freq -= step;
    if ( freq < band->lowFrequency() )
        freq = band->highFrequency();

    // Set the new frequency.
    setFrequency( freq );
}

/*#
    Set the mute state on the current band to \a value.
*/
void RadioBandManager::setMuted( bool value )
{
    RadioBand *band = currentBand();
    if ( band )
        band->setMuted( value );
}

/*#
    Adjust the volume on the current band by \a diff.
*/
void RadioBandManager::adjustVolume( int diff )
{
    RadioBand *band = currentBand();
    if ( band )
        band->adjustVolume( diff );
}

/*#
    Sets the active speaker mode to \a value.

    \sa speakerActive(), speakerPresent()
*/
void RadioBandManager::setSpeakerActive( bool value )
{
    RadioBand *band = currentBand();
    if ( band )
        band->setSpeakerActive( value );
}

/*#
    \fn void RadioBandManager::scanProgress( RadioBand::Frequency frequency, int band )

    Signal that is emitted that indicates that the radio is looking
    at \a frequency on \a band for a station during a scan.  If a station
    is detected at \a frequency on \a band, then this signal will be
    followed by scanFoundStation().
*/

/*#
    \fn void RadioBandManager::scanFoundStation( RadioBand::Frequency frequency, int band )

    Signal that is emitted when a station is found at \a frequency on \a band.
    If scanForward() or scanBackward() were used, then scanStopped() will be
    emitted after this signal.  If scanAll() was used, then the scan will
    continue looking for more stations until all bands have been scanned,
    and then scanStopped() will be emitted.
*/

/*#
    \fn void RadioBandManager::scanStarted()

    Signal that is emitted when a scan starts because of a call to
    scanForward(), scanBackward(), or scanAll().
*/

/*#
    \fn void RadioBandManager::scanStopped()

    Signal that is emitted when the scan stops, either because stopScan()
    was called, a station was found and scanAll() was not used, or because
    the device has finished scanning all bands for scanAll().
*/

/*#
    \fn void RadioBandManager::signalChanged()

    Signal that is emitted when the signal characterisics signal() and
    stereo() change.
*/

void RadioBandManager::startScan( bool forward, bool all )
{
    // Ignore the request if there are no bands.
    RadioBand *band = currentBand();
    if ( !band )
        return;

    // Stop a current scan if there is one.
    stopScan();

    // Record the starting point, so we know when we wrap around.
    d->startBand = d->currentBand;
    d->startFrequency = band->frequency();

    // Set up for the new scan.
    d->scanForward = forward;
    d->scanAll = all;
    d->scanning = true;
    emit scanStarted();

    // Move on to the next frequency from our current position.
    nextScan();

    // Start the scan wait timer.
    d->scanTimer->start( band->scanWaitTime() );
}

void RadioBandManager::nextScan()
{
    RadioBand *bandinfo = currentBand();
    RadioBand::Frequency frequency = bandinfo->frequency();

    // Adjust the frequency to move on to the next possible station location.
    RadioBand::Frequency step;
    if ( bandinfo->signal() )
        step = bandinfo->scanOffStationStep();
    else
        step = bandinfo->scanStep();
    if ( d->scanForward ) {
        frequency += step;
        if ( frequency > bandinfo->highFrequency() )
            frequency = bandinfo->lowFrequency();
    } else {
        frequency -= step;
        if ( frequency < bandinfo->lowFrequency() )
            frequency = bandinfo->highFrequency();
    }
    bandinfo->setFrequency( frequency );
    startSignalCheck();

    // Report the current scan progress.
    emit scanProgress( frequency, d->currentBand );
}

void RadioBandManager::scanTimeout()
{
    // Did we see signal on the current frequency?
    if ( signal() ) {

        // Report that we found a station on this frequency.
        RadioBand *band = currentBand();
        emit scanFoundStation( band->frequency(), d->currentBand );

        // If we aren't scanning for all, then stop the scan process.
        if ( !d->scanAll ) {
            d->scanning = false;
            d->scanTimer->stop();
            emit scanStopped();
            return;
        }
    }

    // Move on to the next frequency.
    nextScan();
}

void RadioBandManager::signalTimeout()
{
    bool currentSignal = signal();
    bool currentStereo = stereo();
    if ( d->firstAfterFreqChange ||
         currentSignal != d->prevSignal ||
         currentStereo != d->prevStereo ) {
        // Advertise the change in signal to listening parties.
        d->prevSignal = currentSignal;
        d->prevStereo = currentStereo;
        emit signalChanged();

        // Speed up signal checks until the signal stablises.
        startSignalCheck( false );
    }
}

void RadioBandManager::startSignalCheck( bool firstAfterFreqChange )
{
    d->firstAfterFreqChange = firstAfterFreqChange;
    d->signalTimer->start( 300 );
    d->signalBackoffTimer->start( 5000 );
}

void RadioBandManager::backoffSignalCheck()
{
    // We have had a consistent signal for some time, so back off the checks.
    // If it changes, then speed up the checks again until it stablises.
    d->signalTimer->start( 5000 );
}

int RadioBandManager::bandFromFrequency( RadioBand::Frequency frequency ) const
{
    int index = 0;
    foreach ( RadioBand *band, d->bands ) {
        if ( frequency >= band->lowFrequency() &&
             frequency <= band->highFrequency() )
            return index;
        ++index;
    }
    return -1;
}
