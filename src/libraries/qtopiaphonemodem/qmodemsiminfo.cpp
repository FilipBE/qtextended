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

#include <qmodemsiminfo.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <QTimer>

// Number of milliseconds between polling attempts on AT+CIMI command.
#ifndef CIMI_TIMEOUT
#define CIMI_TIMEOUT    2000
#endif

/*!
    \class QModemSimInfo
    \inpublicgroup QtCellModule

    \brief The QModemSimInfo class provides SIM identity information for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CIMI} command from 3GPP TS 27.007.

    QModemSimInfo implements the QSimInfo telephony interface.  Client
    applications should use QSimInfo instead of this class to
    access the modem's SIM identity information.

    \sa QSimInfo
*/

class QModemSimInfoPrivate
{
public:
    QModemService *service;
    QTimer *checkTimer;
    int count;
    bool simPinRequired;
};

/*!
    Construct an AT-based SIM information object for \a service.
*/
QModemSimInfo::QModemSimInfo( QModemService *service )
    : QSimInfo( service->service(), service, QCommInterface::Server )
{
    d = new QModemSimInfoPrivate();
    d->service = service;
    d->checkTimer = new QTimer( this );
    d->checkTimer->setSingleShot( true );
    connect( d->checkTimer, SIGNAL(timeout()), this, SLOT(requestIdentity()) );
    d->count = 0;
    d->simPinRequired = false;

    // Perform an initial AT+CIMI request to get the SIM identity.
    QTimer::singleShot( 0, this, SLOT(requestIdentity()) );

    // Hook onto the posted event of the service to determine
    // the current sim pin status
    connect( service, SIGNAL(posted(QString)), this, SLOT(serviceItemPosted(QString)) );
}

/*!
    Destroy this AT-based SIM information object.
*/
QModemSimInfo::~QModemSimInfo()
{
    delete d;
}

/*!
    Called by modem vendor plug-ins to indicate that an unsolicited
    SIM insert message has been detected.  The QModemSimInfo class
    will then initiate a request to obtain the SIM identity using
    the \c{AT+CIMI} command.  Once the identity has been determined,
    QModemSimInfo will set the identity() and emit inserted() to
    notify client applications.

    \sa simRemoved(), identity(), inserted()
*/
void QModemSimInfo::simInserted()
{
    if ( !d->checkTimer->isActive() )
        requestIdentity();
}

/*!
    Called by modem vendor plug-ins to indicate that an unsolicited
    SIM remove message has been detected.  The identity() will be
    set to an empty string, and the removed() signal will be emitted.

    \sa simInserted(), identity(), removed()
*/
void QModemSimInfo::simRemoved()
{
    setIdentity( QString() );
}

void QModemSimInfo::requestIdentity()
{
    d->service->primaryAtChat()->chat
        ( "AT+CIMI", this, SLOT(cimi(bool,QAtResult)) );
}

void QModemSimInfo::cimi( bool ok, const QAtResult& result )
{
    QString id = extractIdentity( result.content().trimmed() );
    if ( ok && !id.isEmpty() ) {
        // We have a valid SIM identity.
        setIdentity( id );
    } else {
        // No SIM identity, so poll again in a few seconds for the first two minutes.
        setIdentity( QString() );

        if ( d->count < 120000/CIMI_TIMEOUT ) {
            d->checkTimer->start( CIMI_TIMEOUT );
            d->count++;
        } else {
            d->count = 0;
            // If not waiting for SIM pin to be entered by the user
            if ( !d->simPinRequired ) {
                // post a message to modem service to stop SIM PIN polling
                d->service->post( "simnotinserted" );
                emit notInserted();
            }
        }
        // If we got a definite "not inserted" error, then emit notInserted().
        if ( result.resultCode() == QAtResult::SimNotInserted )
            emit notInserted();
    }
}

void QModemSimInfo::serviceItemPosted( const QString &item )
{
    if ( item == "simpinrequired" )
        d->simPinRequired = true;
    else if ( item == "simpinentered" )
        d->simPinRequired = false;
}

// Extract the identity information from the content of an AT+CIMI response.
// It is possible that we got multiple lines, including some unsolicited
// notifications from the modem that are not yet recognised.  Skip over
// such garbage and find the actual identity.
QString QModemSimInfo::extractIdentity( const QString& content )
{
    QStringList lines = content.split( QChar('\n') );
    foreach ( QString line, lines ) {
        if ( line.length() > 0 ) {
            uint ch = line[0].unicode();
            if ( ch >= '0' && ch <= '9' )
                return line;
        }
    }
    return QString();
}
