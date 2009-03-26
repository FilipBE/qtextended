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

#include <qmodempinmanager.h>
#include "qmodempinmanager_p.h"
#include <qmodemservice.h>
#include <qatchat.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qtopialog.h>
#include <QTimer>

/*!
    \class QModemPinManager
    \inpublicgroup QtCellModule

    \brief The QModemPinManager class provides a method for the modem to query the user interface for pin values.
    \ingroup telephony::modem

    This class uses the \c{AT+CPIN}, \c{AT+CPWD}, and \c{AT+CLCK} commands
    from 3GPP TS 27.007.

    Whenever an AT command fails in a way that requires a pin, the QModemPinManager
    class will emit the pinStatus() signal to request the desired pin.  Once the correct
    pin has been entered, normal operations continue.

    The QModemPinManager class implements the QPinManager telephony interface.
    Client applications should use QPinManager instead of this class to
    access the modem's pin functionality.

    This modem interface handles the following posted events,
    via QModemService::post():

    \list
        \o \c{pinquery} - query the modem for the current pin.  This is
           sent in response to an error from the modem indicating that a
           pin is required.  This uses the \c{AT+CPIN?} command to determine
           the pin to ask the user for.
        \o \c{simready} - sufficient pin's have been supplied to return
           the SIM to the \c{READY} state.  The pending operation can
           now continue.
    \endlist

    \sa QPinManager
*/

class QModemPinManagerPrivate
{
public:
    QModemPinManagerPrivate( QModemService *service )
    {
        this->service = service;
        this->querying = false;
        this->simMissing = false;
    }

    QModemService *service;
    QString expectedPin;
    QString expectedAskPin;
    QString changingPin;
    QString lastSimPin;
    QString currentPin;
    bool querying;
    QList<QModemPendingPin *> pending;
    QString pendingPin;
    QTimer *lastPinTimer;
    bool simMissing;

    QModemPendingPin *findPending( const QString& type );
};

QModemPendingPin *QModemPinManagerPrivate::findPending( const QString& type )
{
    QList<QModemPendingPin *>::ConstIterator it;
    for ( it = pending.begin(); it != pending.end(); ++it ) {
        if ( (*it)->type() == type )
            return *it;
    }
    return 0;
}

/*!
    Create a modem pin manager for \a service.
*/
QModemPinManager::QModemPinManager( QModemService *service )
    : QPinManager( service->service(), service, QCommInterface::Server )
{
    d = new QModemPinManagerPrivate( service );
    d->lastPinTimer = new QTimer( this );
    d->lastPinTimer->setSingleShot( true );
    connect( d->lastPinTimer, SIGNAL(timeout()), this, SLOT(lastPinTimeout()) );

    // Hook onto the "pinquery" event to allow other providers
    // to ask us to query for the pin the modem wants.  This may
    // be sent during initialization, functionality changes,
    // or when the modem reports a "pin/puk required" error.
    service->connectToPost( "pinquery", this, SLOT(sendQuery()) );
    service->connectToPost( "simnotinserted", this, SLOT(simMissing()) );
}

/*!
    Destroy this modem pin manager.
*/
QModemPinManager::~QModemPinManager()
{
    delete d;
}

/*!
    Ask for a specific \a type of pin and deliver it to \a pinSlot on
    \a target.  The slot takes a single QString argument for the pin.
    If the user cancels the request, then invoke \a cancelSlot
    on \a target.

    This method is typically used with \c{AT+CPWD}, \c{AT+CLCK}, and commands
    that require \c{SIM PIN2}.

    If \a type indicates a puk (e.g. \c{SIM PUK2}), then this method
    will ask for both a puk and a new pin and then deliver the new
    pin to \a pinSlot once the puk has been verified.  If repeated
    attempts to enter a puk fail, then \a cancelSlot will be notified.
*/
void QModemPinManager::needPin( const QString& type, QObject *target,
                                const char *pinSlot, const char *cancelSlot )
{
    QModemPendingPin *pending = new QModemPendingPin( this );
    pending->setType( type );
    connect( pending, SIGNAL(havePin(QString)), target, pinSlot );
    connect( pending, SIGNAL(cancelPin()), target, cancelSlot );
    d->pending.append( pending );

    QPinOptions options;
    options.setMaxLength( pinMaximum() );
    options.setCanCancel( true );

    if ( type.contains("PUK") ) {
        d->expectedPin = type;
        emit pinStatus( type, QPinManager::NeedPuk, options );
    } else {
        emit pinStatus( type, QPinManager::NeedPin, options );
    }
}

/*!
    \reimp
*/
void QModemPinManager::querySimPinStatus()
{
    sendQuery();
}

/*!
    \reimp
*/
void QModemPinManager::enterPin( const QString& type, const QString& pin )
{
    // Pending pins should be sent to the object that asked for them.
    QModemPendingPin *pending;
    bool hadPending = false;
    while ( ( pending = d->findPending( type ) ) != 0 ) {
        d->pending.removeAll( pending );
        pending->emitHavePin( pin );
        delete pending;
        hadPending = true;
    }
    if ( hadPending )
        return;

    // If this wasn't a pending pin, then we were probably processing
    // the answer to a sendQuery() request.
    if ( type != d->expectedPin ) {
        qLog( Modem ) << "Pin" << type
                      << "entered, expecting" << d->expectedPin;
        return;
    }
    d->currentPin = pin;
    d->service->chat( "AT+CPIN=\"" + QAtUtils::quote( pin ) + "\"",
                      this, SLOT(cpinResponse(bool,QAtResult)) );
}

// Encode a PUK or PIN according to the rules in 3GPP TS 11.11, section 9.3.
static QByteArray encodePukOrPin( const QString& value )
{
    static char const padding[8] =
        {(char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF,
         (char)0xFF, (char)0xFF, (char)0xFF, (char)0xFF};
    QByteArray data = value.toUtf8();
    if ( data.size() >= 8 )
        return data.left(8);
    else
        return data + QByteArray( padding, 8 - data.size() );
}

/*!
    \reimp
*/
void QModemPinManager::enterPuk
        ( const QString& type, const QString& puk, const QString& newPin )
{
    if ( ( type != "SIM PUK" && type != "SIM PUK2" ) || !d->expectedPin.isEmpty() ) {
        if ( type != d->expectedPin ) {
            qLog( Modem ) << "Puk" << type
                          << "entered, expecting" << d->expectedPin;
            return;
        }
    }
    if ( type.contains("PUK") && d->findPending( type ) != 0 ) {
        // We will need newPin later if the puk is verified.
        d->pendingPin = newPin;
        d->service->chat( "AT+CPIN=\"" + QAtUtils::quote( puk ) + "\",\"" +
                          QAtUtils::quote( newPin ) + "\"",
                          this, SLOT(cpukResponse(bool)) );
    } else if ( d->expectedPin.isEmpty() ) {
        // We were not expecting a PUK in answer to a AT+CPIN query,
        // so we cannot use AT+CPIN to send the PUK.  We therefore
        // send the PUK and new PIN using a low-level "UNBLOCK CHV"
        // command, as described in 3GPP TS 11.11.
        QByteArray data;
        data += (char)0xA0;
        data += (char)0x2C;
        data += (char)0x00;
        if ( type.contains( QChar('2') ) )
            data += (char)0x02;
        else
            data += (char)0x00;
        data += (char)0x10;
        data += encodePukOrPin( puk );
        data += encodePukOrPin( newPin );
        d->currentPin = newPin;
        d->expectedPin = type;
        d->service->chat( "AT+CSIM=" + QString::number( data.size() * 2 ) + "," +
                          QAtUtils::toHex( data ),
                          this, SLOT(csimResponse(bool,QAtResult)) );
    } else {
        d->currentPin = newPin;
        d->expectedPin = type;
        d->service->chat( "AT+CPIN=\"" + QAtUtils::quote( puk ) + "\",\"" +
                          QAtUtils::quote( newPin ) + "\"",
                          this, SLOT(cpinResponse(bool,QAtResult)) );
    }
}

/*!
    \reimp
*/
void QModemPinManager::cancelPin( const QString& type )
{
    QModemPendingPin *pending;
    while ( ( pending = d->findPending( type ) ) != 0 ) {
        d->pending.removeAll( pending );
        pending->emitCancelPin();
        delete pending;
    }
    if ( d->expectedPin == type ) {
        d->expectedPin = QString();
    }
}

/*!
    \reimp
*/
void QModemPinManager::changePin
        ( const QString& type, const QString& oldPin, const QString& newPin )
{
    QString code = pinTypeToCode( type );
    if ( code.isEmpty() ) {
        // Invalid type for pin changes, so fail the request.
        emit changePinResult( type, false );
        return;
    }
    d->changingPin = type;
    d->service->chat( "AT+CPWD=\"" + code + "\",\"" +
                      QAtUtils::quote( oldPin ) + "\",\"" +
                      QAtUtils::quote( newPin ) + "\"",
                      this, SLOT(cpwdResponse(bool)) );
}

class QClckUserData : public QAtResult::UserData
{
public:
    QClckUserData( const QString& type )
    { this->type = type; }

    QString type;
};

/*!
    \reimp
*/
void QModemPinManager::requestLockStatus( const QString& type )
{
    QString code = pinTypeToCode( type );
    if ( code.isEmpty() ) {
        emit lockStatus( type, false );
        return;
    }
    d->service->chat( "AT+CLCK=\"" + code + "\",2",
                      this, SLOT(clckQuery(bool,QAtResult)),
                      new QClckUserData( type ) );
}

/*!
    \reimp
*/
void QModemPinManager::setLockStatus
        ( const QString& type, const QString& password, bool enabled )
{
    QString code = pinTypeToCode( type );
    if ( code.isEmpty() ) {
        emit setLockStatusResult( type, false );
        return;
    }
    if ( enabled ) {
        d->service->chat( "AT+CLCK=\"" + code + "\",1,\"" +
                          QAtUtils::quote( password ) + "\"",
                          this, SLOT(clckSet(bool,QAtResult)),
                          new QClckUserData( type ) );
    } else {
        d->service->chat( "AT+CLCK=\"" + code + "\",0,\"" +
                          QAtUtils::quote( password ) + "\"",
                          this, SLOT(clckSet(bool,QAtResult)),
                          new QClckUserData( type ) );
    }
}

void QModemPinManager::sendQuery()
{
    if ( !d->querying ) {
        d->querying = true;
        if ( d->service->multiplexer()->channel( "primary" )->isValid() ) {
            d->service->chat
                ( "AT+CPIN?", this, SLOT(cpinQuery(bool,QAtResult)) );
        } else {
            // The underlying serial device could not be opened,
            // so we are not talking to a modem.  Fake sim ready.
            QAtResult result;
            result.setResultCode( QAtResult::OK );
            result.setContent( "+CPIN: READY" );
            cpinQuery( true, result );
        }
    }
}

void QModemPinManager::sendQueryAgain()
{
    d->service->chat
        ( "AT+CPIN?", this, SLOT(cpinQuery(bool,QAtResult)) );
}

// Process the response to a AT+CPIN? command.
void QModemPinManager::cpinQuery( bool ok, const QAtResult& result )
{
    if ( !ok ) {

        // The AT+CPIN? request failed.  The SIM may not be ready.
        // Wait three seconds and resend.
        if ( !d->simMissing )
            QTimer::singleShot( 3000, this, SLOT(sendQueryAgain()) );

    } else {

        // No longer querying for the pin.
        d->querying = false;

        // Extract the required pin from the returned result.
        QAtResultParser parser( result );
        parser.next( "+CPIN:" );
        QString pin = parser.line().trimmed();
        if ( pin == "READY" || ( pin.isEmpty() && emptyPinIsReady() ) ) {

            // No more PIN's are required, so the sim is ready.
            d->expectedPin = QString();
            d->service->post( "simready" );

            // Notify the application level that the sim is ready.
            emit pinStatus( "READY", QPinManager::Valid, QPinOptions() );

        } else if ( pin == "SIM PIN" && !d->lastSimPin.isEmpty() ) {

            // We have a cached version of the primary "SIM PIN"
            // that we know is valid.  Re-send it immediately.
            // Some modems ask for the SIM PIN multiple times.
            // 3GPP TS 27.007 requires this behaviour.
            d->expectedPin = pin;
            d->currentPin = d->lastSimPin;
            d->service->chat( "AT+CPIN=\"" + QAtUtils::quote( d->lastSimPin ) +
                              "\"", this, SLOT(cpinResponse(bool,QAtResult)) );

        } else {

            // Ask that the pin be supplied by the user.
            d->expectedPin = pin;
            QPinManager::Status status;
            if ( pin.contains( "PUK" ) )
                status = QPinManager::NeedPuk;
            else {
                status = QPinManager::NeedPin;
                d->service->post( "simpinrequired" );
            }
            QPinOptions options;
            options.setMaxLength( pinMaximum() );
            emit pinStatus( d->expectedPin, status, options );

        }
    }
}

// Process the response to a AT+CPIN=[puk,]pin command.
void QModemPinManager::cpinResponse( bool ok, const QAtResult& result )
{
    d->lastSimPin = QString();
    if ( ok ) {

        // The pin was entered correctly.
        QPinOptions options;
        options.setMaxLength( pinMaximum() );
        emit pinStatus( d->expectedPin, QPinManager::Valid, options );
        if ( d->expectedPin.contains( "PUK" ) ) {
            // If we just successfully sent the PUK, then the PIN
            // version is also successfull.
            QString pin = d->expectedPin.replace( "PUK", "PIN" );
            emit pinStatus( pin, QPinManager::Valid, options );
        }
        if ( d->expectedPin == "SIM PIN" ) {
            // Cache the last valid SIM PIN value because some modems will ask
            // for it again immediately even when they accept it first time.
            // 3GPP TS 27.007 requires this behaviour.
            d->lastSimPin = d->currentPin;
            d->lastPinTimer->start( 5000 ); // Clear it after 5 seconds.
        }
        d->expectedPin = QString();
        d->service->post( "simpinentered" );
    }
    d->currentPin = QString();

    // If the result includes a "+CPIN" line, then the modem may
    // be asking for a new pin/puk already.  Otherwise send AT+CPIN?
    // to ask whether the modem is "READY" or needs another pin/puk.
    QAtResultParser parser( result );
    if ( parser.next( "+CPIN:" ) ) {
        d->querying = true;
        cpinQuery( true, result );
    } else {
        sendQuery();
    }
}

// Process the response to an "UNBLOCK CHV" command sent via AT+CSIM.
void QModemPinManager::csimResponse( bool ok, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( !ok ) {
        cpinResponse( ok, result );
    } else if ( parser.next( "+CSIM:" ) ) {
        QString line = parser.line();
        uint posn = 0;
        QAtUtils::parseNumber( line, posn );    // Skip length.
        QString data;
        if ( ((int)posn) < line.length() && line[posn] == ',' )
            data = line.mid( posn + 1 );
        else
            data = line.mid( posn );
        if ( data.contains( QChar('"') ) )
            data = data.remove( QChar('"') );
        if ( data.length() <= 4 ) {
            // Need at least sw1 and sw2 on the end of the command.
            cpinResponse( false, result );
            return;
        }
        QByteArray sw = QAtUtils::fromHex( data.right(4) );
        cpinResponse( ( sw.size() > 0 && sw[0] != (char)0x98 ), result );
    } else {
        cpinResponse( false, result );
    }
}

// Process the response to a AT+CPIN=[puk,]pin command when we are
// asking for a puk due to a needPin() call.
void QModemPinManager::cpukResponse( bool ok )
{
    if ( ok ) {

        // The puk was entered correctly.
        QPinOptions options;
        options.setMaxLength( pinMaximum() );
        emit pinStatus( d->expectedPin, QPinManager::Valid, options );

        // Notify interested parties of the new pin.
        QModemPendingPin *pending;
        while ( ( pending = d->findPending( d->expectedPin ) ) != 0 ) {
            d->pending.removeAll( pending );
            pending->emitHavePin( d->pendingPin );
            delete pending;
        }
        d->expectedPin = QString();
        d->pendingPin = QString();

    } else {

        // The supplied puk is invalid.  Ask again.  If the puk
        // eventually locks out the SIM, this could be a problem.
        // But since this code will only be used for a puk entry
        // dialog that can be cancelled, the user can still abort.
        d->pendingPin = QString();
        QPinOptions options;
        options.setMaxLength( pinMaximum() );
        options.setCanCancel( true );
        emit pinStatus( d->expectedPin, QPinManager::NeedPuk, options );

    }
}

// Process the response to a AT+CPWD=fac,oldpin,newpin command.
void QModemPinManager::cpwdResponse( bool ok )
{
    emit changePinResult( d->changingPin, ok );
    d->changingPin = QString();
}

// Process the response to a AT+CLCK=fac,2 command.
void QModemPinManager::clckQuery( bool, const QAtResult& result )
{
    QString type = ((QClckUserData *)result.userData())->type;
    QAtResultParser parser( result );
    if ( parser.next( "+CLCK:" ) ) {
        emit lockStatus( type, parser.readNumeric() != 0 );
    } else {
        emit lockStatus( type, false );
    }
}

// Process the response to a AT+CLCK=fac,(0|1),pw command.
void QModemPinManager::clckSet( bool ok, const QAtResult& result )
{
    QString type = ((QClckUserData *)result.userData())->type;
    emit setLockStatusResult( type, ok );
}

// Discard cached "SIM PIN" value after a short period.
void QModemPinManager::lastPinTimeout()
{
    d->lastSimPin = QString();
}

/*!
    Returns true if an empty string in response to \c{AT+CPIN?} should
    be treated the same as \c{READY}; false otherwise.  This is provided to work around
    bugs on some modems.  Such modems should inherit QModemPinManager
    and override this method to return true.  The default return value
    is false.
*/
bool QModemPinManager::emptyPinIsReady() const
{
    return false;
}

/*!
    Convert a pin \a type such as \c{SIM PIN}, \c{PH-SIM PIN}, etc,
    into the two-letter code to be used with \c{AT+CPWD}.  Returns
    an empty string if \a type is not recognized.  This method
    can be overridden by modems that have more pin types than
    those defined by 3GPP TS 27.007.
*/
QString QModemPinManager::pinTypeToCode( const QString& type ) const
{
    if ( type == QLatin1String("SIM PIN") )
        return "SC";
    else if ( type == QLatin1String("PH-SIM PIN") )
        return "PS";
    else if ( type == QLatin1String("PH-FSIM PIN") )
        return "PF";
    else if ( type == QLatin1String("SIM PIN2") )
        return "P2";
    else if ( type == QLatin1String("PH-NET PIN") )
        return "PN";
    else if ( type == QLatin1String("PH-NETSUB PIN") )
        return "PU";
    else if ( type == QLatin1String("PH-SP PIN") )
        return "PP";
    else if ( type == QLatin1String("PH-CORP PIN") )
        return "PC";
    else if ( type == QLatin1String("CNTRL PIN") )
        return "CS";
    else
        return QString();
}

/*!
    Returns the maximum size of a pin.  The default implementation
    returns 32.
*/
int QModemPinManager::pinMaximum() const
{
    return 32;
}

/*!
    \internal
    Sets the current state as sim missing.
*/
void QModemPinManager::simMissing()
{
    d->simMissing = true;
}
