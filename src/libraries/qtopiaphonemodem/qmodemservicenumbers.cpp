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

#include <qmodemservicenumbers.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <QSettings>

/*!
    \class QModemServiceNumbers
    \inpublicgroup QtCellModule

    \brief The QModemServiceNumbers class provides access to GSM service numbers such as voice mail and SMS service center for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CSVM}, \c{AT+CSCA}, and \c{AT+CNUM}
    commands from 3GPP TS 27.005 and 3GPP TS 27.007.

    QModemServiceNumbers implements the QServiceNumbers telephony interface.  Client
    applications should use the QServiceNumbers class instead of this class to
    access the modem's service number settings.

    \sa QServiceNumbers
*/

/*!
    Create an AT-based service number object for \a service.
*/
QModemServiceNumbers::QModemServiceNumbers( QModemService *service )
    : QServiceNumbers( service->service(), service, QCommInterface::Server )
{
    this->service = service;
}

/*!
    Destroy this AT-based service number object.
*/
QModemServiceNumbers::~QModemServiceNumbers()
{
}

/*!
    \reimp
*/
void QModemServiceNumbers::requestServiceNumber( QServiceNumbers::NumberId id )
{
    if ( id == QServiceNumbers::VoiceMail ) {
        service->primaryAtChat()->chat
            ( "AT+CSVM?", this, SLOT(csvm(bool,QAtResult)) );

    } else if ( id == QServiceNumbers::SmsServiceCenter ) {
        service->secondaryAtChat()->chat
            ( "AT+CSCA?", this, SLOT(csca(bool,QAtResult)) );
    } else if ( id == QServiceNumbers::SubscriberNumber ) {
        service->secondaryAtChat()->chat
            ( "AT+CNUM", this, SLOT(cnum(bool,QAtResult)) );
    } else {
        // Just in case others identifiers are added in the future.
        emit serviceNumber( id, QString() );
    }
}

/*!
    \reimp
*/
void QModemServiceNumbers::setServiceNumber
            ( QServiceNumbers::NumberId id, const QString& number )
{
    if ( id == QServiceNumbers::VoiceMail ) {
        if ( number.isEmpty() ) {
            service->primaryAtChat()->chat
                ( "AT+CSVM=0", this, SLOT(csvmSet(bool,QAtResult)) );
        } else {
            service->primaryAtChat()->chat
                ( "AT+CSVM=1," + QAtUtils::encodeNumber( number ),
                  this, SLOT(csvmSet(bool,QAtResult)) );
        }
    } else if ( id == QServiceNumbers::SmsServiceCenter ) {
        service->secondaryAtChat()->chat
            ( "AT+CSCA=" + QAtUtils::encodeNumber( number ),
              this, SLOT(cscaSet(bool,QAtResult)) );
    } else {
        emit setServiceNumberResult( id, QTelephony::Error );
    }
}

/*!
    Request the service number \a id from a local configuration file.
    This is called by modem vendor plug-ins in their requestServiceNumber()
    override if they do not have an appropriate AT command for accessing
    the requested service number.

    \sa requestServiceNumber(), setServiceNumberInFile()
*/
void QModemServiceNumbers::requestServiceNumberFromFile
        ( QServiceNumbers::NumberId id )
{
    QString name;
    if ( id == QServiceNumbers::VoiceMail )
        name = "VoiceMail";
    else if ( id == QServiceNumbers::SmsServiceCenter )
        name = "SmsServiceCenter";
    else if ( id == QServiceNumbers::SubscriberNumber )
        name = "SubscriberNumber";
    else {
        emit serviceNumber( id, QString() );
        return;
    }
    QSettings config( "Trolltech", "Modem" );
    config.beginGroup( "ServiceNumbers" );
    emit serviceNumber( id, config.value( name, QString() ).toString() );
}

/*!
    Sets the service number \a id to \a number in a local configuration file.
    This is called by modem vendor plug-ins in their setServiceNumber()
    override if they do not have an appropriate AT command for setting
    the requested service number.

    \sa setServiceNumber(), requestServiceNumberFromFile()
*/
void QModemServiceNumbers::setServiceNumberInFile
        ( QServiceNumbers::NumberId id, const QString& number )
{
    QString name;
    if ( id == QServiceNumbers::VoiceMail )
        name = "VoiceMail";
    else if ( id == QServiceNumbers::SmsServiceCenter )
        name = "SmsServiceCenter";
    else if ( id == QServiceNumbers::SubscriberNumber )
        name = "SubscriberNumber";
    else {
        emit setServiceNumberResult( id, QTelephony::Error );
        return;
    }
    QSettings config( "Trolltech", "Modem" );
    config.beginGroup( "ServiceNumbers" );
    config.setValue( name, number );
    emit setServiceNumberResult( id, QTelephony::OK );
}

void QModemServiceNumbers::csvm( bool, const QAtResult& result )
{
    // Voice mail query: <mode>[,<number>[,<type>]]
    QAtResultParser cmd( result );
    cmd.next( "+CSVM:" );
    if ( cmd.readNumeric() != 0 ) {
        QString num = cmd.readString();
        num = QAtUtils::decodeNumber( num, cmd.readNumeric() );
        emit serviceNumber( QServiceNumbers::VoiceMail, num );
    } else {
        emit serviceNumber( QServiceNumbers::VoiceMail, QString() );
    }
}

void QModemServiceNumbers::csca( bool, const QAtResult& result )
{
    QAtResultParser cmd( result );
    if ( cmd.next( "+CSCA:" ) ) {
        QString num = cmd.readString();
        num = QAtUtils::decodeNumber( num, cmd.readNumeric() );
        emit serviceNumber( QServiceNumbers::SmsServiceCenter, num );
    } else {
        emit serviceNumber( QServiceNumbers::SmsServiceCenter, QString() );
    }
}

void QModemServiceNumbers::cnum( bool, const QAtResult& result )
{
    // This only returns the first number in the list.  It is possible
    // that there may be more than one number for different services.
    // Modem vendor plugins can provide additional service number types
    // if it makes sense to do so.
    QAtResultParser cmd( result );
    if ( cmd.next( "+CNUM:" ) ) {
        cmd.readString();       // Ignore alpha id.
        QString num = cmd.readString();
        num = QAtUtils::decodeNumber( num, cmd.readNumeric() );
        emit serviceNumber( QServiceNumbers::SubscriberNumber, num );
    } else {
        emit serviceNumber( QServiceNumbers::SubscriberNumber, QString() );
    }
}

void QModemServiceNumbers::csvmSet( bool, const QAtResult& result )
{
    emit setServiceNumberResult( QServiceNumbers::VoiceMail,
                                 (QTelephony::Result)result.resultCode() );
}

void QModemServiceNumbers::cscaSet( bool, const QAtResult& result )
{
    emit setServiceNumberResult( QServiceNumbers::SmsServiceCenter,
                                 (QTelephony::Result)result.resultCode() );
}
