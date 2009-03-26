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

#include "rdsprograminfo.h"
#include "rdsgroup.h"

/*#
    \class RdsProgramInfo
    \brief The RdsProgramInfo class provides information from an RDS data stream.

    The RdsProgramInfo class provides information from an RDS data stream
    in a form that is usable by client applications.

    The implementation is based on the United States RBDS Standard,
    April 9, 1998.  It currently implements the RDS group types
    0A, 0B, 2A, 2B, and 4A.
*/

class RdsProgramInfoPrivate
{
public:
    int piCode;
    int ptyCode;
    bool trafficProgram;
    bool trafficAnnouncement;
    bool isMusic;
    bool lastABState;
    char programName[8];
    char radioTextBuf[64];
    QString radioText;
    QList<RadioBand::Frequency> alternateFrequencies;
};

RdsProgramInfo::RdsProgramInfo( QObject *parent )
    : QObject( parent )
{
    d = new RdsProgramInfoPrivate();
    clear();
}

RdsProgramInfo::~RdsProgramInfo()
{
    delete d;
}

/*#
    Clear all program information.  This is usually called upon a
    change in frequency.
*/
void RdsProgramInfo::clear()
{
    d->piCode = -1;
    d->ptyCode = -1;
    d->trafficProgram = false;
    d->trafficAnnouncement = false;
    d->isMusic = true;
    d->lastABState = false;
    memset( d->programName, 0, sizeof(d->programName) );
    memset( d->radioTextBuf, 0, sizeof(d->radioTextBuf) );
    d->radioText = QString();
}

void RdsProgramInfo::addGroup( const RdsGroup& group )
{
    // Extract information that is common to every group.
    d->piCode = group.word(0);
    d->ptyCode = ((group.msb(1) & 0x03) << 3) | ((group.lsb(1) & 0xE0) >> 5);
    d->trafficProgram = ( (group.msb(1) & 0x04) != 0 );

    // Determine how to interpret the rest of the group.
    int flags = (group.lsb(1) & 0x1F);
    switch ( group.groupType() ) {

        case 0:
        {
            // Basic tuning and switching information.
            d->trafficAnnouncement = ( ( flags & 0x10 ) != 0 );
            d->isMusic = ( ( flags & 0x08 ) != 0 );
            int posn = ( ( flags & 0x03 ) << 1 );
            d->programName[posn] = (char)( group.msb(3) );
            d->programName[posn + 1] = (char)( group.lsb(3) );
        }
        break;

        case 2:
        {
            // Radio text information.
            bool ab = ( ( flags & 0x10 ) != 0 );
            if ( ab != d->lastABState ) {
                // The A/B flag has flipped, so clear the buffer and restart.
                memset( d->radioTextBuf, 0, sizeof(d->radioTextBuf) );
                d->lastABState = ab;
            }
            int index = ( flags & 0x0F );
            if ( group.isTypeAGroup() ) {
                // Four characters per message.
                index *= 4;
                d->radioTextBuf[index] = (char)( group.msb(2) );
                d->radioTextBuf[index + 1] = (char)( group.lsb(2) );
                d->radioTextBuf[index + 2] = (char)( group.msb(3) );
                d->radioTextBuf[index + 3] = (char)( group.lsb(3) );
                index += 4;
            } else {
                // Two characters per message.
                index *= 2;
                d->radioTextBuf[index] = (char)( group.msb(3) );
                d->radioTextBuf[index + 1] = (char)( group.lsb(3) );
                index += 2;
            }
            char *cr = (char *)memchr( d->radioTextBuf, 0x0D,
                                       sizeof(d->radioTextBuf) );
            if ( index >= (int)sizeof(d->radioTextBuf) || cr != 0 ) {
                // We have received a complete radio text message,
                // or an in-place update to a previously complete message.
                // Convert it into a string to display to the user.
                if ( cr != 0 )
                    index = cr - d->radioTextBuf;
                QByteArray text( d->radioTextBuf, index );
                d->radioText = QString::fromLatin1( text.trimmed() );
            }
        }
        break;

        case 4:
        {
            // Not relevant for type 4B groups, only 4A.
            if ( group.isTypeBGroup() )
                break;

            // Extract the date and time information.
            int julianDay = (group.word(2) >> 1) | ((flags & 0x03) << 15);
            int hour = ((group.lsb(2) & 0x01) << 4) |
                       ((group.msb(3) & 0xF0) >> 4);
            int minute = ((group.msb(3) & 0x0F) << 2) |
                         ((group.lsb(3) & 0xC0) >> 6);
            int zone = (group.lsb(3) & 0x1F) * 30;
            if ( ( group.lsb(3) & 0x20 ) != 0 )
                zone = -zone;
            QDate date = QDate::fromJulianDay( julianDay );
            QTime time( hour, minute );
            emit timeNotification( QDateTime( date, time ), zone );
        }
        break;
    }
}

/*#
    Get the program identification code for this station, or -1 if
    the code is unknown.
*/
int RdsProgramInfo::piCode() const
{
    return d->piCode;
}

/*#
    Get the program type code for the current station, or -1 if
    the type is unknown.
*/
int RdsProgramInfo::ptyCode() const
{
    return d->ptyCode;
}

/*#
    Determine if the current station carries traffic reports, even if it
    isn't necessarily broadcasting a traffic report at present.
*/
bool RdsProgramInfo::trafficProgram() const
{
    return d->trafficProgram;
}

/*#
    Determine if the current station is currently broadcasting a
    traffic report announcement.
*/
bool RdsProgramInfo::trafficAnnouncement() const
{
    return d->trafficAnnouncement;
}

/*#
    Determine if the station is currently broadcasting music.
    This is the default if the music/speech state is unknown.
*/
bool RdsProgramInfo::isMusic() const
{
    return d->isMusic;
}

/*#
    Determine if the station is currently broadcasting speech.
*/
bool RdsProgramInfo::isSpeech() const
{
    return !d->isMusic;
}

/*#
    Get the program name for the station.
*/
QString RdsProgramInfo::programName() const
{
    QByteArray buf( d->programName, sizeof(d->programName) );
    buf.replace( (char)0x00, (char)0x20 );
    return QString::fromLatin1( buf.trimmed() );    // XXX - which charset?
}

/*#
    Convert the program type code from ptyCode() into a human-readable string.
*/
QString RdsProgramInfo::programType() const
{
    return QString();
}

/*#
    Get the most recent radio text message on the current station.
*/
QString RdsProgramInfo::radioText() const
{
    return d->radioText;
}

/*#
    Get the list of alternate frequencies upon which this station can
    also be found on the radio spectrum.
*/
QList<RadioBand::Frequency> RdsProgramInfo::alternateFrequencies() const
{
    return d->alternateFrequencies;
}

/*#
    \fn void RdsProgramInfo::timeNotification( const QDateTime& time, int zone )

    Signal that is emitted when a time notification is received on
    the current radio station.  The \a time parameter is the current
    time in UTC.  The \a zone parameter indicates the number of minutes
    offset from UTC (positive for west of UTC, negative for east of UTC).
*/
