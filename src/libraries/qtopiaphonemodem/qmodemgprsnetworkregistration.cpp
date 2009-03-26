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

#include <qmodemgprsnetworkregistration.h>
#include <qmodemservice.h>
#include <qnetworkregistration.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemGprsNetworkRegistration
    \inpublicgroup QtCellModule

    \brief The QModemGprsNetworkRegistration class provides information about GPRS network registration on AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CGREG} command from 3GPP TS 27.007.

    QModemGprsNetworkRegistration implements the QGprsNetworkRegistration telephony
    interface.  Client applications should use QGprsNetworkRegistration instead
    of this class to access the modem's GPRS network registration state.

    \sa QGprsNetworkRegistration
*/

class QModemGprsNetworkRegistrationPrivate
{
public:
    QModemService *service;
};

/*!
    Create an AT-based GPRS network registration handler for \a service.
*/
QModemGprsNetworkRegistration::QModemGprsNetworkRegistration
        ( QModemService *service )
    : QGprsNetworkRegistrationServer( service->service(), service )
{
    d = new QModemGprsNetworkRegistrationPrivate();
    d->service = service;

    service->primaryAtChat()->registerNotificationType
        ( "+CGREG:", this, SLOT(cgregNotify(QString)), true );
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );
}

/*!
    Destroy this AT-based GPRS network registration handler.
*/
QModemGprsNetworkRegistration::~QModemGprsNetworkRegistration()
{
    delete d;
}

void QModemGprsNetworkRegistration::resetModem()
{
    // Turn on unsolicited AT+CGREG notifications.
    d->service->primaryAtChat()->chat( "AT+CGREG=2" );

    // Query the current state.
    d->service->primaryAtChat()->chat
        ( "AT+CGREG?", this, SLOT(cgregQuery(bool,QAtResult)) );
}

void QModemGprsNetworkRegistration::cgregQuery( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CGREG:" ) ) {
        parser.readNumeric();  // Skip unsolicited result flag which is unused.
        uint stat = parser.readNumeric();
        QString lac = parser.readString();
        QString ci = parser.readString();
        if ( !lac.isEmpty() && !ci.isEmpty() ) {
            // We have location information after the state value.
            updateRegistrationState( (QTelephony::RegistrationState)stat,
                                     lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
        } else {
            // We don't have any location information.
            updateRegistrationState( (QTelephony::RegistrationState)stat );
        }
    }
}

void QModemGprsNetworkRegistration::cgregNotify( const QString& msg )
{
    uint posn = 7;
    uint stat = QAtUtils::parseNumber( msg, posn );
    QString lac = QAtUtils::nextString( msg, posn );
    QString ci = QAtUtils::nextString( msg, posn );
    if ( !lac.isEmpty() && !ci.isEmpty() ) {
        // We have location information after the state value.
        updateRegistrationState( (QTelephony::RegistrationState)stat,
                                 lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
    } else {
        // We don't have any location information.
        updateRegistrationState( (QTelephony::RegistrationState)stat );
    }
}
