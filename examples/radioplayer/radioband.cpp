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

#include "radioband.h"

// Standard radio band types recognized by standardBand().
enum StandardBand
{
    Unknown,
    AM,
    FM,
    ShortWave,
    LongWave
};

/*
    Determine if the frequency range between \a low and \a high
    corresponds to a standard radio broadcast band, and determine
    which band.  The \a low and \a high parameters will normally
    be obtained from a radio device driver, indicating its
    available tuning range.  The tuning range may not necessarily be
    identical to the international standards for the band frequencies.
*/
static StandardBand standardBand
    ( RadioBand::Frequency low, RadioBand::Frequency high )
{
    // Get the mid-point of the range, to determine what band it falls within.
    RadioBand::Frequency freq = ( low + high ) / 2;

    // Check for FM.  This is usually 87.5 to 108.0 MHz, but according
    // to Wikipedia, some countries use frequencies as low as 65.9 MHz.
    // We check for 50 to 120 MHz, just to be on the safe side.
    if ( freq >= 50000000 && freq <= 120000000 )
        return FM;

    // Check for AM, which is typically between 520 and 1710 kHz.
    // We check for 500 to 1800 kHz, just to be on the safe side.
    if ( freq >= 500000 && freq <= 1800000 )
        return AM;

    // Check for short wave, which is typically between 2300 and 26100 kHz.
    // We check for 2000 to 30000 kHz, just to be on the safe side.
    if ( freq >= 2000000 && freq <= 30000000 )
        return ShortWave;

    // Check for long wave, which is typically between 153 and 279 kHz.
    // We check for 100 to 300 kHz, just to be on the safe side.
    if ( freq >= 100000 && freq <= 300000 )
        return LongWave;

    // We don't know what band this frequency falls within.
    return Unknown;
}

class RadioBandPrivate
{
public:
    RadioBandPrivate( const QString& _name )
    {
        name = _name;
        lowFrequency = 0;
        highFrequency = 0;
        scanStep = 0;
        scanOffStationStep = 0;
        scanWaitTime = 0;
        frequencyIsChannelNumber = false;
    }

    QString name;
    RadioBand::Frequency lowFrequency;
    RadioBand::Frequency highFrequency;
    RadioBand::Frequency scanStep;
    RadioBand::Frequency scanOffStationStep;
    int scanWaitTime;
    bool frequencyIsChannelNumber;
};

/*#
    Construct a new radio band object for a band called \a name,
    and attach it to \a parent.  Subclass constructors should
    call setLowFrequency(), setHighFrequency(), setScanStep(),
    setScanOffStationStep(), setScanWaitTime(), and
    setFrequencyIsChannelNumber() to set the properties of the band.
*/
RadioBand::RadioBand( const QString& name, QObject *parent )
    : QObject( parent )
{
    d = new RadioBandPrivate( name );
}

/*#
    Construct a new radio band which ranges between \a low and \a high,
    and attach it to \a parent.  If the range between \a low and \a high
    is recognized as a standard band, then the band will be given a
    meaningful name, such as \c AM or \c FM.  Otherwise, the band
    will be called \a name.

    Default values will be set for the properties lowFrequency(),
    highFrequency(), scanStep(), scanOffStationStep(), and scanWaitTime().
*/
RadioBand::RadioBand( const QString& name, RadioBand::Frequency low,
                      RadioBand::Frequency high, QObject *parent )
    : QObject( parent )
{
    QString n;

    // Determine if this is one of the standard bands.
    StandardBand band = standardBand( low, high );

    // Alter the name to something meaningful if it is a standard band.
    switch ( band ) {
        case AM:         n = tr("AM"); break;
        case FM:         n = tr("FM"); break;
        case ShortWave:  n = tr("SW"); break;
        case LongWave:   n = tr("LW"); break;
        default:         n = name; break;
    }
    d = new RadioBandPrivate( n );

    // Set up the band properties.
    setLowFrequency( low );
    setHighFrequency( high );
    if ( band == AM ) {
        setScanStep( 1000 );               // 1 kHz
        setScanOffStationStep( 5000 );     // 5 kHz
        setScanWaitTime( 50 );             // 50 ms
    } else if ( band == FM ) {
        setScanStep( 50000 );              // 0.05 MHz
        setScanOffStationStep( 500000 );   // 0.5 MHz
        setScanWaitTime( 200 );            // 200 ms
    } else {
        // Short wave or long wave - set up for 200 individual steps.
        setScanStep( ( high - low ) / 200 );
        setScanOffStationStep( scanStep() * 5 );
        setScanWaitTime( 200 );            // 200 ms
    }
}

/*#
    Destroy this radio band object.
*/
RadioBand::~RadioBand()
{
    delete d;
}

/*#
    Get the standard name for the band that \a freq falls within.
    Returns something like \c{FM}, \c{AM}, etc.  Returns an empty
    string if the frequency does not fall within any standard band.
*/
QString RadioBand::standardBandForFrequency( RadioBand::Frequency freq )
{
    switch ( standardBand( freq, freq ) ) {
        case AM:         return tr("AM");
        case FM:         return tr("FM");
        case ShortWave:  return tr("SW");
        case LongWave:   return tr("LW");
        default:         break;
    }
    return QString();
}

/*#
    \fn bool RadioBand::active() const

    Determine if this band is currently the active band being listened
    to by the user.  An inactive band should be muted, but an active band
    can be either muted or unmuted.
*/

/*#
    \fn bool RadioBand::muted() const

    Determine if this band would be muted if it was the active one.
*/

/*#
    \fn RadioBand::Frequency RadioBand::frequency() const

    Get the current frequency that this band is tuned to.  If the band
    is not currently active, this will be the frequency that it will be
    tuned to the next time it becomes active.

    If frequencyIsChannelNumber() is true, then this is the current
    channel number that the radio band is tuned to.
*/

/*#
    \fn bool RadioBand::signal() const

    Determine if there is currently signal on this radio band
    that indicates that we are on a valid station.  This is used when
    scanning bands to determine when we have found a station.
*/

/*#
    \fn bool RadioBand::stereo() const

    Determine if the current station is broadcasting in stereo.
*/

/*#
    \fn bool RadioBand::rds() const

    Determine if the current station is broadcasting Radio Data System (RDS)
    digital packets.
*/

/*#
    \fn bool RadioBand::signalDetectable() const

    Determine if the radio device driver can detect signal() changes.
    Some devices will report a signal() of false for every position on
    the dial, even if a station is playing.
*/

/*#
    \fn int RadioBand::volume() const

    Get the current volume level for this band (0-100).
*/

/*#
    Returns true if the audio is being routed to the hands-free speaker.

    \sa setSpeakerActive(), speakerPresent()
*/
bool RadioBand::speakerActive() const
{
    return false;
}

/*#
    Returns true if the radio band supports a hands-free speaker.
    The default return value is false.

    \sa speakerActive(), setSpeakerActive()
*/
bool RadioBand::speakerPresent() const
{
    return false;
}

/*#
    Get the name of this radio band.  Usually this will be something
    like \c{AM} or \c{FM}, and can be displayed to the user.
*/
QString RadioBand::name() const
{
    return d->name;
}

/*#
    Get the lowest frequency in this radio band, measured in Hz.
*/
RadioBand::Frequency RadioBand::lowFrequency() const
{
    return d->lowFrequency;
}

/*#
    Set the lowest frequency in this radio band to \a value,
    measured in Hz.  If frequencyIsChannelNumber() is true,
    then this is the lowest channel number that can be tuned.
*/
void RadioBand::setLowFrequency( RadioBand::Frequency value )
{
    d->lowFrequency = value;
}

/*#
    Get the highest frequency in this radio band, measured in Hz.
    If frequencyIsChannelNumber() is true, then this is the highest
    channel number that can be tuned.
*/
RadioBand::Frequency RadioBand::highFrequency() const
{
    return d->highFrequency;
}

/*#
    Set the highest frequency in this radio band to \a value,
    measured in Hz.
*/
void RadioBand::setHighFrequency( RadioBand::Frequency value )
{
    d->highFrequency = value;
}

/*#
    Get the frequency step to use when scanning forward or back
    through this band for new stations.
*/
RadioBand::Frequency RadioBand::scanStep() const
{
    return d->scanStep;
}

/*#
    Set the frequency step to use when scanning forward or back
    through this band for new stations to \a value.
*/
void RadioBand::setScanStep( RadioBand::Frequency value )
{
    d->scanStep = value;
}

/*#
    Get the frequency step to use when scanning forward or back
    through this band for new stations after we have just found a
    station.  This may be larger than scanStep() to avoid picking up
    the station again due to signal bleed-over around the station's
    actual frequency.
*/
RadioBand::Frequency RadioBand::scanOffStationStep() const
{
    return d->scanOffStationStep;
}

/*#
    Set the frequency step to use when scanning forward or back
    through this band for new stations after we have just found
    a station to \a value.  This may be larger than scanStep() to avoid
    picking up the station again due to signal bleed-over around
    the station's frequency.
*/
void RadioBand::setScanOffStationStep( RadioBand::Frequency value )
{
    d->scanOffStationStep = value;
}

/*#
    Get the time in milliseconds to wait on a frequency between moving
    on to the next one, to give the radio time to detect signal.
*/
int RadioBand::scanWaitTime() const
{
    return d->scanWaitTime;
}

/*#
    Set the time in milliseconds to wait on a frequency between moving
    on to the next one to \a value, to give the radio time to detect signal.
*/
void RadioBand::setScanWaitTime( int value )
{
    d->scanWaitTime = value;
}

/*#
    Determine if frequency values are actually channel numbers on a
    satellite-based radio network.  The default value is false.
*/
bool RadioBand::frequencyIsChannelNumber() const
{
    return d->frequencyIsChannelNumber;
}

/*#
    Set a flag that indicates if frequency values are actually channel
    numbers on a satellite-based radio network.
*/
void RadioBand::setFrequencyIsChannelNumber( bool value )
{
    d->frequencyIsChannelNumber = value;
}

/*#
    \fn void RadioBand::setActive( bool value )

    Set this band to be active or inactive according to \a value.
    The default implementation simply records the value for active().
    It will normally need to be overriden by subclasses.

    When switching between bands, \c{setActive(false)} should be
    called on the old band, and then \c{setActive(true)} should be
    called on the new band.  Only one band should be active at
    any given time.
*/

/*#
    \fn void RadioBand::setMuted( bool value )

    Mute or unmute the radio's audio according to \a value.
    Inactive bands are implicitly muted, even if this value is true.
*/

/*#
    \fn void RadioBand::setFrequency( RadioBand::Frequency value )

    Set the band frequency to \a value.  This may be called even if
    this band is not currently active.  The frequency change will take
    effect the next time the band becomes active.

    If frequencyIsChannelNumber() is true, then \a value is the
    channel number to tune the radio band to.
*/

/*#
    \fn void RadioBand::adjustVolume( int diff )

    Adjust the volume on this band by \a diff.  This may affect the volume
    on other bands if they are provided by the same underlying radio device.
*/

/*#
    Sets the active speaker mode to \a value.

    \sa speakerActive(), speakerPresent()
*/
void RadioBand::setSpeakerActive( bool )
{
    // Nothing to do here.
}
