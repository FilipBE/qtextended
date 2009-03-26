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

#include "radiobanddummy.h"

// This dummy device pretends to be an AM/FM/XM radio with a number of
// pre-allocated stations so that we can test the radio program
// without an actual radio device being present.  The AM/FM station
// frequencies below are popular radio stations in Brisbane, Australia.

int RadioBandDummy::_volume = 50;   // Simulate a shared volume setting.

RadioBandDummy::RadioBandDummy
        ( const QString& name, RadioBand::Frequency low,
          RadioBand::Frequency high, bool isXm,
          QList<RadioBand::Frequency>& stations, QObject *parent )
    : RadioBand( name, low, high, parent )
{
    this->stations = stations;
    this->_active = false;
    this->_muted = false;
    this->_frequency = low;
    if ( isXm ) {
        // AM/FM settings are created in the base class.  Set up XM here.
        setScanStep( 1 );
        setScanOffStationStep( 1 );
        setScanWaitTime( 200 );
        setFrequencyIsChannelNumber( true );
    }
}

RadioBandDummy::~RadioBandDummy()
{
}

void RadioBandDummy::createBands( QList<RadioBand *>& bands, QObject *parent )
{
    QList<RadioBand::Frequency> amStations;
    amStations.append(  612000 );
    amStations.append(  693000 );
    amStations.append( 1116000 );
    bands.append( new RadioBandDummy
        ( QString(), 520000, 1650000, false, amStations, parent ) );

    QList<RadioBand::Frequency> fmStations;
    fmStations.append(  94900000 );
    fmStations.append(  97300000 );
    fmStations.append( 104000000 );
    fmStations.append( 105300000 );
    fmStations.append( 106900000 );
    fmStations.append( 107700000 );
    bands.append( new RadioBandDummy
        ( QString(), 87500000, 108000000, false, fmStations, parent ) );

    QList<RadioBand::Frequency> xmStations;
    for ( int station = 10; station <= 200; station += 10 )
        xmStations.append( (RadioBand::Frequency)station );
    bands.append( new RadioBandDummy
        ( "XM", 1, 200, true, xmStations, parent ) ); // No tr
}

bool RadioBandDummy::active() const
{
    return _active;
}

bool RadioBandDummy::muted() const
{
    return _muted;
}

RadioBand::Frequency RadioBandDummy::frequency() const
{
    return _frequency;
}

bool RadioBandDummy::signal() const
{
    // We have signal if we are on a station.
    return stations.contains( _frequency );
}

bool RadioBandDummy::stereo() const
{
    // All of our dummy stations are stereo.
    return signal();
}

bool RadioBandDummy::rds() const
{
    // We don't support RDS in the dummy implementation.
    return false;
}

bool RadioBandDummy::signalDetectable() const
{
    // We can detect signal on this band.
    return true;
}

int RadioBandDummy::volume() const
{
    return _volume;
}

void RadioBandDummy::setActive( bool value )
{
    _active = value;
}

void RadioBandDummy::setMuted( bool value )
{
    _muted = value;
}

void RadioBandDummy::setFrequency( RadioBand::Frequency value )
{
    _frequency = value;
}

void RadioBandDummy::adjustVolume( int diff )
{
    _volume += diff;
    if ( _volume < 0 )
        _volume = 0;
    else if ( _volume >= 100 )
        _volume = 100;
}
