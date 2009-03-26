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

#include <qmodemcallsettings.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>

/*!
    \class QModemCallSettings
    \inpublicgroup QtCellModule

    \brief The QModemCallSettings class implements access to auxiliary call settings for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CCWA}, \c{AT+CLIR}, \c{AT+CGSMS}, \c{AT+CLIP},
    \c{AT+COLP} commands from 3GPP TS 27.007 and 3GPP TS 27.005.

    QModemCallSettings implements the QCallSettings telephony interface.  Client
    applications should use QCallSettings instead of this class to access the
    modem's call settings.

    \sa QCallSettings
*/

/*!
    Construct a new modem call settings handler for \a service.
*/
QModemCallSettings::QModemCallSettings( QModemService *service )
    : QCallSettings( service->service(), service, QCommInterface::Server )
{
    this->service = service;
}

/*!
    Destroy this modem call settings handler.
*/
QModemCallSettings::~QModemCallSettings()
{
}

/*!
    \reimp
*/
void QModemCallSettings::requestCallWaiting()
{
    service->primaryAtChat()->chat
        ( "AT+CCWA=1,2", this, SLOT(ccwaRequest(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::setCallWaiting
        ( bool enable, QTelephony::CallClass cls )
{
    QString command = "AT+CCWA=1,";
    if ( enable )
        command += "1,";
    else
        command += "0,";
    command += QString::number( (int)cls );
    service->primaryAtChat()->chat
        ( command, this, SLOT(ccwaSet(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::requestCallerIdRestriction()
{
    service->primaryAtChat()->chat
        ( "AT+CLIR?", this, SLOT(clirRequest(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::setCallerIdRestriction
        ( QCallSettings::CallerIdRestriction clir )
{
    service->primaryAtChat()->chat
        ( "AT+CLIR=" + QString::number( (int)clir ),
          this, SLOT(clirSet(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::requestSmsTransport()
{
    service->primaryAtChat()->chat
        ( "AT+CGSMS?", this, SLOT(cgsmsRequest(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::setSmsTransport
        ( QCallSettings::SmsTransport transport )
{
    service->primaryAtChat()->chat
        ( "AT+CGSMS=" + QString::number( (int)transport ),
          this, SLOT(cgsmsSet(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::requestCallerIdPresentation()
{
    service->primaryAtChat()->chat
        ( "AT+CLIP?", this, SLOT(clipRequest(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallSettings::requestConnectedIdPresentation()
{
    service->primaryAtChat()->chat
        ( "AT+COLP?", this, SLOT(colpRequest(bool,QAtResult)) );
}

void QModemCallSettings::ccwaRequest( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    QTelephony::CallClass cls = QTelephony::CallClassNone;
    while ( parser.next( "+CCWA:" ) ) {
        uint status = parser.readNumeric();
        uint classx = parser.readNumeric();
        if ( status != 0 )
            cls = (QTelephony::CallClass)( cls | classx );
    }
    emit callWaiting( cls );
}

void QModemCallSettings::ccwaSet( bool, const QAtResult& result )
{
    emit setCallWaitingResult( (QTelephony::Result)result.resultCode() );
}

void QModemCallSettings::clirRequest( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CLIR:" ) ) {
        uint n = parser.readNumeric();
        uint m = parser.readNumeric();
        emit callerIdRestriction
                ( (QCallSettings::CallerIdRestriction)n,
                  (QCallSettings::CallerIdRestrictionStatus)m );
    } else {
        // The command failed, so report the CLIR state as unknown.
        emit callerIdRestriction( Subscription, Unknown );
    }
}

void QModemCallSettings::clirSet( bool, const QAtResult& result )
{
    emit setCallerIdRestrictionResult
            ( (QTelephony::Result)result.resultCode() );
}

void QModemCallSettings::cgsmsRequest( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CGSMS:" ) ) {
        uint service = parser.readNumeric();
        emit smsTransport( (QCallSettings::SmsTransport)service );
    } else {
        emit smsTransport( QCallSettings::SmsTransportUnavailable );
    }
}

void QModemCallSettings::cgsmsSet( bool, const QAtResult& result )
{
    emit setSmsTransportResult
            ( (QTelephony::Result)result.resultCode() );
}

void QModemCallSettings::clipRequest( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CLIP:" ) ) {
        parser.readNumeric();   // Skip <n> value.
        uint prov = parser.readNumeric();
        emit callerIdPresentation( (QCallSettings::PresentationStatus)prov );
    } else {
        emit callerIdPresentation( QCallSettings::PresentationUnknown );
    }
}

void QModemCallSettings::colpRequest( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+COLP:" ) ) {
        parser.readNumeric();   // Skip <n> value.
        uint prov = parser.readNumeric();
        emit connectedIdPresentation( (QCallSettings::PresentationStatus)prov );
    } else {
        emit connectedIdPresentation( QCallSettings::PresentationUnknown );
    }
}
