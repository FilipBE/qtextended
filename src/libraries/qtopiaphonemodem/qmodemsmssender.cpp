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

#include <qmodemsmssender.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <QTimer>

/*!
    \class QModemSMSSender
    \inpublicgroup QtCellModule

    \brief The QModemSMSSender class provides SMS sending facilities for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CMGS} and \c{AT+CMMS} commands from
    3GPP TS 27.005.

    QModemSMSSender implements the QSMSSender telephony interface.  Client
    applications should use QSMSSender instead of this class to
    access the modem's SMS sending facilities.

    \sa QSMSSender
*/

class QModemSMSSenderPending
{
public:
    QString id;
    QList<QSMSMessage> msgs;
    int posn;
    QModemSMSSenderPending *next;
    QTelephony::Result result;
    int retries;
};

class QModemSMSSenderPrivate
{
public:
    QModemSMSSenderPrivate( QModemService *service )
    {
        this->service = service;
        this->first = 0;
        this->last = 0;
        this->sending = false;
    }

    QModemService *service;
    QModemSMSSenderPending *first;
    QModemSMSSenderPending *last;
    QTimer *needTimeout;
    bool sending;
};

/*!
    Create a new AT-based SMS sending object for for \a service.
*/
QModemSMSSender::QModemSMSSender( QModemService *service )
    : QSMSSender( service->service(), service, QCommInterface::Server )
{
    d = new QModemSMSSenderPrivate( service );
    service->connectToPost( "smsready", this, SLOT(smsReady()) );

    d->needTimeout = new QTimer( this );
    d->needTimeout->setSingleShot( true );
    connect( d->needTimeout, SIGNAL(timeout()), this, SLOT(smsReadyTimeout()) );
}

/*!
    Destroy this SMS sending object.
*/
QModemSMSSender::~QModemSMSSender()
{
    delete d;
}

/*!
    \reimp
*/
void QModemSMSSender::send( const QString& id, const QSMSMessage& msg )
{
    // Create the message block and add it to the pending list.
    QModemSMSSenderPending *pending = new QModemSMSSenderPending();
    pending->id = id;
    pending->posn = 0;
    pending->result = QTelephony::OK;
    pending->retries = 5;
    pending->msgs = msg.split();    // Split into GSM-sized message fragments.
    pending->next = 0;
    if ( d->last )
        d->last->next = pending;
    else
        d->first = pending;
    d->last = pending;

    // If this was the first message, then start the transmission process.
    if ( d->first == pending ) {
        d->needTimeout->start( 15 * 1000 );
        d->service->post( "needsms" );
    }
}

void QModemSMSSender::smsReady()
{
    // Stop the "need SMS" timeout if it was active.
    d->needTimeout->stop();

    // Bail out if no messages to send or we are already sending.
    // QModemSMSReader may have caused the "smsReady()" signal to
    // be emitted, so this is a false positive.
    if ( !d->first || d->sending )
        return;

    // Transmit the next message in the queue.
    sendNext();
}

void QModemSMSSender::smsReadyTimeout()
{
    // Fail all of the pending requests as the SMS system is not available.
    QModemSMSSenderPending *current = d->first;
    QModemSMSSenderPending *next;
    while ( current != 0 ) {
        next = current->next;
        emit finished( current->id, QTelephony::SMSOperationNotAllowed );
        delete current;
        current = next;
    }
    d->first = 0;
    d->last = 0;
}

void QModemSMSSender::sendNext()
{
    // We are currently sending.
    d->sending = true;

    // Emit a finished signal if the current message is done.
    QModemSMSSenderPending *current = d->first;
    if ( !d->first ) {
        d->sending = false;
        return;
    }
    if ( current->posn >= current->msgs.size() ) {
        emit finished( current->id, current->result );
        d->first = current->next;
        if ( !d->first )
            d->last = 0;
        delete current;
        if ( !d->first ) {
            d->sending = false;
            return;
        }
        current = d->first;
    }

    // If this is the first message in a multi-part, send AT+CMMS=1.
    if ( current->posn == 0 && current->msgs.size() > 1 ) {
        d->service->chat( "AT+CMMS=1" );
    }

    // Transmit the next message in the queue.
    QSMSMessage m = current->msgs[ (current->posn)++ ];

    // Convert the message into a hexadecimal PDU string.
    QByteArray pdu = m.toPdu();

    // Get the length of the data portion of the PDU,
    // excluding the service centre address.
    uint pdulen = pdu.length() - QSMSMessage::pduAddressLength( pdu );

    // Build and deliver the send command.  Note: "\032" == CTRL-Z.
    QString cmd = "AT+CMGS=" + QString::number( pdulen );
    d->service->primaryAtChat()->chatPDU
        ( cmd, pdu, this, SLOT(transmitDone(bool,QAtResult)) );
}

void QModemSMSSender::transmitDone( bool ok, const QAtResult& result )
{
    // If the send was successful, then move on to the next message to be sent.
    if ( ok ) {
        sendNext();
        return;
    }

    // If this is the first or only message in a multi-part,
    // then retry after a second.  The system may not be ready.
    QModemSMSSenderPending *current = d->first;
    if ( current->posn == 1 ) {
        if ( --(current->retries) > 0 ) {
            --(current->posn);
            QTimer::singleShot( 1000, this, SLOT(sendNext()) );
            return;
        }
    }

    // Record the error and then move on to the next message.
    current->result = (QTelephony::Result)( result.resultCode() );
    sendNext();
}
