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

#include <qmodemcallforwarding.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemCallForwarding
    \inpublicgroup QtCellModule

    \brief The QModemCallForwarding class implements the call forwarding settings for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CCFC} command from 3GPP TS 27.007.

    QModemCallForwarding implements the QCallForwarding telephony interface.  Client
    applications should use the QCallForwarding class instead of this class to
    access the modem's call forwarding settings.

    \sa QCallForwarding
*/

/*!
    Construct a new modem call forwarding handler for \a service.
*/
QModemCallForwarding::QModemCallForwarding( QModemService *service )
    : QCallForwarding( service->service(), service, QCommInterface::Server )
{
    this->service = service;
}

/*!
    Destroy this modem call forwarding handler.
*/
QModemCallForwarding::~QModemCallForwarding()
{
}

class QForwardingUserData : public QAtResult::UserData
{
public:
    QForwardingUserData( QCallForwarding::Reason reason )
    { this->reason = reason; }

    QCallForwarding::Reason reason;
};

/*!
    \reimp
*/
void QModemCallForwarding::requestForwardingStatus
        ( QCallForwarding::Reason reason )
{
    service->primaryAtChat()->chat
        ( "AT+CCFC=" + QString::number( (int)reason ) + ",2",
          this, SLOT(requestDone(bool,QAtResult)),
          new QForwardingUserData( reason ) );
}

/*!
    \reimp
*/
void QModemCallForwarding::setForwarding
        ( QCallForwarding::Reason reason,
          const QCallForwarding::Status& status, bool enable )
{
    QString command = "AT+CCFC=" + QString::number( (int)reason ) + ",";
    if ( enable ) {
        command += "3,";            // "Registration" from 3GPP TS 27.007.
        command += QAtUtils::encodeNumber( status.number, true );
        command += ",";
    } else {
        command += "4,,,";          // "Erasure" from 3GPP TS 27.007.
    }
    command += QString::number( (int)status.cls );
    if ( status.time != 0 && enable && reason == QCallForwarding::NoReply ) {
        command += ",,," + QString::number( status.time );
    }
    service->primaryAtChat()->chat
        ( command, this, SLOT(setDone(bool,QAtResult)),
          new QForwardingUserData( reason ) );
}

void QModemCallForwarding::requestDone( bool, const QAtResult& result )
{
    QCallForwarding::Reason reason
        = ((QForwardingUserData *)result.userData())->reason;
    QAtResultParser parser( result );
    QList<QCallForwarding::Status> list;
    while ( parser.next( "+CCFC:" ) ) {
        // Parse the fields from the response line.
        uint status = parser.readNumeric();
        if ( !status )
            continue;       // Not interesting if rule is not active.
        uint classx = parser.readNumeric();
        QString number = parser.readString();
        number = QAtUtils::decodeNumber( number, parser.readNumeric() );
        parser.readString();        // Skip subaddr
        parser.readNumeric();       // Skip satype
        uint time = parser.readNumeric();

        // Construct a status block and add it to the list.
        QCallForwarding::Status st;
        st.cls = (QTelephony::CallClass)classx;
        st.number = number;
        st.time = (int)time;
        list.append( st );
    }
    emit forwardingStatus( reason, list );
}

void QModemCallForwarding::setDone( bool, const QAtResult& result )
{
    QCallForwarding::Reason reason
        = ((QForwardingUserData *)result.userData())->reason;
    emit setForwardingResult( reason, (QTelephony::Result)result.resultCode() );
}
