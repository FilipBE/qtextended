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

#include <qmodemsimgenericaccess.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemSimGenericAccess
    \inpublicgroup QtCellModule

    \brief The QModemSimGenericAccess class implements SIM generic access for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CSIM} command from 3GPP TS 27.007
    to access the SIM.

    QModemSimGenericAccess implements the QSimGenericAccess telephony interface.
    Client applications should use QSimGenericAccess instead of this class to
    send generic SIM commands.

    \sa QSimGenericAccess
*/

/*!
    Construct a new modem-based SIM generic access object for \a service.
*/
QModemSimGenericAccess::QModemSimGenericAccess( QModemService *service )
    : QSimGenericAccess( service->service(), service, Server )
{
    this->service = service;
}

/*!
    Destroy this modem-based SIM generic access object.
*/
QModemSimGenericAccess::~QModemSimGenericAccess()
{
}

class QCSimUserData : public QAtResult::UserData
{
public:
    QCSimUserData( const QString& reqid )
    { this->reqid = reqid; }

    QString reqid;
};

/*!
    \reimp
*/
void QModemSimGenericAccess::command
        ( const QString& reqid, const QByteArray& data )
{
    QString cmd = "AT+CSIM=" + QString::number( data.size() * 2 ) + "," +
                  QAtUtils::toHex( data );
    service->chat( cmd, this, SLOT(csim(bool,QAtResult)),
                   new QCSimUserData( reqid ) );
}

void QModemSimGenericAccess::csim( bool ok, const QAtResult& result )
{
    QByteArray data;
    QString reqid = ((QCSimUserData *)result.userData())->reqid;
    if ( ok ) {
        QAtResultParser parser( result );
        if ( parser.next( "+CSIM:" ) ) {
            uint posn = 0;
            QString line = parser.line();
            QAtUtils::parseNumber( line, posn );    // Skip length.
            if ( ((int)posn) < line.length() && line[posn] == ',' )
                ++posn;
            data = QAtUtils::fromHex( line.mid( (int)posn ) );
        }
    }
    emit response( reqid, (QTelephony::Result)result.resultCode(), data );
}
