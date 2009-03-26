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

#include <qmodemcellbroadcast.h>
#include <qmodemservice.h>
#include <qnetworkregistration.h>
#include <qsmsmessage.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <QTimer>
#include <QTextCodec>

/*!
    \class QModemCellBroadcast
    \inpublicgroup QtCellModule

    \brief The QModemCellBroadcast class implements cell broadcast functionality for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CSCB} command and the \c{+CBM}
    unsolicited result from 3GPP TS 27.005.

    QModemCellBroadcast implements the QCellBroadcast telephony interface.  Client
    applications should use QCellBroadcast instead of this class to access the
    modem's cell broadcast functionality.

    \sa QCellBroadcast
*/

class QModemCellBroadcastPrivate
{
public:
    QModemService *service;
    QList<int> channels;
    QCBSMessage previousCBM;
    QNetworkRegistration *netReg;
    QList<int> prevChannels;
    bool sawPrevChannels;
    bool resultEmitPending;
};

/*!
    Create an AT-based cell broadcast handler for \a service.
*/
QModemCellBroadcast::QModemCellBroadcast( QModemService *service )
    : QCellBroadcast( service->service(), service, QCommInterface::Server )
{
    d = new QModemCellBroadcastPrivate();
    d->service = service;
    d->channels.append( 50 );
    d->netReg = 0;
    d->sawPrevChannels = false;
    d->resultEmitPending = false;
    setValue( "channels", qVariantFromValue( d->channels ) );

    QObject::connect( service->primaryAtChat(),
                      SIGNAL(pduNotification(QString,QByteArray)),
                      this,
                      SLOT(pduNotification(QString,QByteArray)) );

    service->connectToPost( "smsready", this, SLOT(smsReady()) );
}

/*!
    Destroy this AT-based cell broadcast handler.
*/
QModemCellBroadcast::~QModemCellBroadcast()
{
    delete d;
}

/*!
    \reimp
*/
void QModemCellBroadcast::setChannels( const QList<int>& list )
{
    d->channels = list;
    d->resultEmitPending = true;
    sendChange();
    setValue( "channels", qVariantFromValue( d->channels ) );
}

void QModemCellBroadcast::sendChange()
{
    // If we can make the change immediately, then do so.
    if ( !removeBeforeChange() ) {
        sendAdd();
        return;
    }

    // Get the current channel list if we don't know it.
    if ( !d->sawPrevChannels ) {
        d->service->secondaryAtChat()->chat
            ( "AT+CSCB?", this, SLOT(cscbQuery(bool,QAtResult)) );
    } else {
        sendRemove();
    }
}

void QModemCellBroadcast::sendAdd()
{
    d->service->secondaryAtChat()->chat
        ( command( 0, d->channels ), this, SLOT(cscb(bool,QAtResult)) );
}

void QModemCellBroadcast::sendRemove()
{
    QList<int> toBeRemoved = d->prevChannels;
    foreach ( int chan, d->channels ) {
        // Don't need to remove channels we are about to add back again.
        toBeRemoved.removeAll( chan );
    }
    if ( !toBeRemoved.isEmpty() ) {
        if ( removeOneByOne() ) {
            // We need to remove the channels one by one.
            foreach ( int chan, toBeRemoved ) {
                d->service->secondaryAtChat()->chat
                    ( "AT+CSCB=1,\"" + QString::number( chan ) + "\"" );
            }
        } else {
            // We can remove the channels as a block.
            d->service->secondaryAtChat()->chat( command( 1, toBeRemoved ) );
        }
    }
    sendAdd();
}

void QModemCellBroadcast::cscb( bool ok, const QAtResult& result )
{
    if ( d->resultEmitPending ) {
        d->resultEmitPending = false;
        emit setChannelsResult( (QTelephony::Result)result.resultCode() );
    }
    if ( ok ) {
        d->prevChannels = d->channels;
        d->sawPrevChannels = true;
    } else {
        // Try again in 5 seconds if we are then registered to the network.
        QTimer::singleShot( 5000, this, SLOT(registrationStateChanged()) );
    }
}

void QModemCellBroadcast::cscbQuery( bool ok, const QAtResult& result )
{
    // If the command failed, then the SIM is probably not ready.
    // Wait until the next registration change or SMS ready notification.
    if ( !ok ) {
        if ( d->resultEmitPending ) {
            d->resultEmitPending = false;
            emit setChannelsResult( (QTelephony::Result)result.resultCode() );
        }
        return;
    }

    // Parse out the list of channels that are currently being listened to.
    QList<int> list;
    QAtResultParser parser( result );
    if ( parser.next( "+CSCB:" ) ) {
        parser.readNumeric();   // Skip mode;
        QString chans = parser.readString();
        if ( !chans.isEmpty() ) {
            int posn = 0;
            int first = -1;
            int value = 0;
            while ( posn < chans.length() ) {
                uint ch = chans[posn++].unicode();
                if ( ch == ',' ) {
                    if ( first >= 0 ) {
                        // Add all values between first and value.
                        while ( first <= value ) {
                            list += first;
                            ++first;
                        }
                    } else {
                        list += value;
                    }
                    first = -1;
                    value = 0;
                } else if ( ch == '-' ) {
                    first = value;
                    value = 0;
                } else if ( ch >= '0' && ch <= '9' ) {
                    value = value * 10 + (int)(ch - '0');
                }
            }

            // Deal with the last range or value in the list.
            if ( first >= 0 ) {
                while ( first <= value ) {
                    list += first;
                    ++first;
                }
            } else {
                list += value;
            }
        }
    }
    d->prevChannels = list;
    d->sawPrevChannels = true;

    // Now remove the ones we don't want.
    sendRemove();
}

void QModemCellBroadcast::pduNotification
            ( const QString& type, const QByteArray& pdu )
{
    if ( type.startsWith( "+CBM:" ) ) {

        // Directly delivered cell broadcast message.
        QCBSMessage msg;
        if ( type.contains( QChar(',') ) ) {
            // Cell broadcast message sent to us in text mode,
            // even though we wanted PDU mode.  Can sometimes
            // happen if the message arrives before AT+CMGF=0
            // is accepted by the modem.
            uint posn = 5;
            msg.setMessageCode( QAtUtils::parseNumber( type, posn ) );
            msg.setChannel( QAtUtils::parseNumber( type, posn ) );
            uint dcs = QAtUtils::parseNumber( type, posn );
            msg.setPage( QAtUtils::parseNumber( type, posn ) );
            msg.setNumPages( QAtUtils::parseNumber( type, posn ) );
            QString codec;
            switch ( (QSMSDataCodingScheme)dcs ) {

                case QSMS_DefaultAlphabet:
                {
                    // Decode using the "gsm" 7-bit codec.
                    QTextCodec *codec = QAtUtils::codec( "gsm" );
                    msg.setText( codec->toUnicode( pdu ) );
                }
                break;

                case QSMS_UCS2Alphabet:
                {
                    // Decode using the "ucs2-hex" codec.
                    QTextCodec *codec = QAtUtils::codec( "ucs2-hex" );
                    msg.setText( codec->toUnicode( pdu ) );
                }
                break;

                default:
                {
                    // Assume 8-bit Latin-1 alphabet for everything else.
                    msg.setText( QString::fromLatin1( pdu ) );
                }
                break;
            }

            // We don't know the language in text mode, so assume English.
            // Hopefully the modem will switch the PDU mode soon and this
            // hack will no longer be necessary.
            msg.setLanguage( QCBSMessage::English );
        } else {
            // Cell broadcast message sent to us in PDU mode.
            msg = QCBSMessage::fromPdu( pdu );
        }
        if ( msg != d->previousCBM ) {
            d->previousCBM = msg;
            emit broadcast( msg );
        }

    }
}

void QModemCellBroadcast::registrationStateChanged()
{
    QTelephony::RegistrationState state = d->netReg->registrationState();
    switch ( state ) {

        case QTelephony::RegistrationHome:
        case QTelephony::RegistrationUnknown:
        case QTelephony::RegistrationRoaming:
        {
            // Update the cell broadcast parameters once we are registered.
            sendChange();
        }
        break;

        default: break;
    }
}

void QModemCellBroadcast::smsReady()
{
    // Force an update of the cell broadcast parameters when the SMS
    // system becomes ready, because previous attempts may have failed.
    QTimer::singleShot( 500, this, SLOT(registrationStateChanged()) );
}

/*!
    \reimp
*/
void QModemCellBroadcast::groupInitialized( QAbstractIpcInterfaceGroup *group )
{
    // Now that all interfaces are initialized, find network registration.
    d->netReg = group->interface<QNetworkRegistration>();
    if ( d->netReg ) {
        connect( d->netReg, SIGNAL(registrationStateChanged()),
                 this, SLOT(registrationStateChanged()) );
        registrationStateChanged();     // Force an update immediately.
    }
}

/*!
    Determine if existing channels need to be removed by issuing
    the \c{AT+CSCB=1} command before the list of channels can be
    changed with \c{AT+CSCB=0}.  This is needed on modems that
    add to the existing list of current channels with \c{AT+CSCB=0},
    rather than replacing it with the new list.  The default implementation
    returns false.

    \sa removeOneByOne()
*/
bool QModemCellBroadcast::removeBeforeChange() const
{
    return false;
}

/*!
    Determine if channels need to be removed one by one because
    the \c{AT+CSCB=1} command does not understand the multi-channel form.
    The default implementation returns false.

    \sa removeBeforeChange()
*/
bool QModemCellBroadcast::removeOneByOne() const
{
    return false;
}

QString QModemCellBroadcast::command
    ( int mode, const QList<int>& channels ) const
{
    QString cmd = "AT+CSCB=" + QString::number( mode ) + ",\"";
    bool first = true;
    foreach ( int channel, channels ) {
        if ( !first )
            cmd += ",";
        else
            first = false;
        cmd += QString::number( channel );
    }
    cmd += "\"";
    return cmd;
}
