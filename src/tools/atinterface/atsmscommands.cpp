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

#include "atsmscommands.h"
#include "atcommands.h"
#include "atparseutils.h"

#include "atgsmnoncellcommands.h"

#include <QAtUtils>
#include <QSMSSender>
#include <QSMSReader>
#include <QSMSMessage>
#include <QMailStore>
#include <QMailMessage>
#include <QMailMessageKey>
#include <QMailAddress>
#include <QMailMessageId>
#include <QServiceNumbers>


AtSmsCommands::AtSmsCommands( AtCommands * parent ) : QObject( parent )
{

//debug - deliberate error to check make.

    // we need a reference to our parent, so we can call "send", "done" etc.
    atc = parent;

    sendingSms = false;
    smsSender = new QSMSSender( "modem" );
    smsMessageReference = 1;
    smsReader = new QSMSReader( "modem" );
    readingSms = false;
    readingSmsCount = -1;
    wantedSmsIndex = -1;
    writingSms = false;
    cmgw_address_type = 129;
    cmgw_status = "\"STO UNSENT\"";

    // ---------------------------------------

    connect( atc->frontEnd(), SIGNAL(extra(QString,bool)),
             this, SLOT(extraLine(QString,bool)) );

    connect( smsReader,
             SIGNAL(fetched(QString,QSMSMessage)),
             this,
             SLOT(smsFetched(QString,QSMSMessage)) );
    connect( smsSender,
             SIGNAL(finished(QString,QTelephony::Result)),
             this,
             SLOT(smsFinished(QString,QTelephony::Result)) );

    // ---------------------------------------

    atc->add( "+CMGD", this, SLOT(atcmgd(QString)) );
    atc->add( "+CMGF", this, SLOT(atcmgf(QString)) );
    atc->add( "+CMGL", this, SLOT(atcmgl(QString)) );
    atc->add( "+CMGR", this, SLOT(atcmgr(QString)) );
    atc->add( "+CMGS", this, SLOT(atcmgs(QString)) );
    atc->add( "+CMGW", this, SLOT(atcmgw(QString)) );
    atc->add( "+CMMS", this, SLOT(atcmms(QString)) );
    atc->add( "+CNMI", this, SLOT(atcnmi(QString)) );
    atc->add( "+CPMS", this, SLOT(atcpms(QString)) );

}


AtSmsCommands::~AtSmsCommands()
{
    delete smsSender;
    delete smsReader;
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGD Delete Message}
    \compat

    The \c{AT+CMGD} command allows deletion of messages
    from the message store.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGD=<index>[,<delflag>]} \o \c{+CMS ERROR: <err>}
    \row \o \c{AT+CMGD=?} \o \c{+CMGD: }(list of supported \c{<index>}s)\c{[,}
                                        (list of supported \c{<delflag>}s)\c{]}
    \endtable

    If the \c{<delflag>} parameter is present and non-zero,
    the \c{<index>} is ignored.  Otherwise, the message at the
    location \c{<index>} is deleted.

    Possible values for \c{<delflag>} are as follows:
    \table
    \row \o 0 (or omitted) \o Delete the message at specified \c{<index>}
    \row \o 1 \o Delete all "REC READ" messages from the store
    \row \o 2 \o Delete all "REC READ" and "STO SENT" messages from the store
    \row \o 3 \o Delete all "REC READ", "STO SENT" and "STO UNSENT" messages from the store
    \row \o 4 \o Delete all messages from the store
    \endtable

    Conforms with 3GPP TS 27.005.
*/
void AtSmsCommands::atcmgd( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint del_index = 0;
            if ( posn < (uint)params.length() ) {
                del_index = QAtUtils::parseNumber( params, posn );
            } else {
                atc->done( QAtResult::SMSOperationNotAllowed );
                break;
            }

            uint delflag = 0;
            if ( posn < (uint)params.length() ) {
                delflag = QAtUtils::parseNumber( params, posn );
            }

            QMailStore *qms = QMailStore::instance();
            QMailMessageKey showSmsKey( QMailMessageKey::Type, QMailMessage::Sms );
            QMailMessageKey showIncomingKey( QMailMessageKey::Status, QMailMessage::Incoming, QMailDataComparator::Includes );
            QMailMessageKey showOutgoingKey( QMailMessageKey::Status, QMailMessage::Outgoing, QMailDataComparator::Includes );
            QMailMessageKey showReadKey( QMailMessageKey::Status, QMailMessage::Read, QMailDataComparator::Includes );
            QMailMessageKey showSentKey( QMailMessageKey::Status, QMailMessage::Sent, QMailDataComparator::Includes );
            QList<QMailMessageId> qlm;
            QList<QMailMessageId>::ConstIterator it;

            switch ( delflag ) {
                case 0: // delete the index
                {
                    QMailMessage qmm = qms->message( QMailMessageId( del_index ) );
                    if ( !qmm.id().isValid() || ! ( qmm.messageType() & QMailMessage::Sms ) ) {
                        // this was either not valid, or not an sms.
                        atc->done( QAtResult::InvalidMemoryIndex );
                    } else {
                        // ok, delete it.
                        if ( qms->removeMessage( QMailMessageId( del_index ) ) ) {
                            atc->done();
                        } else {
                            atc->done( QAtResult::SMSMemoryFailure );
                        }
                    }
                    return;
                }
                break;

                case 1: // delete REC READ
                {
                    qlm = qms->queryMessages( showSmsKey & showIncomingKey & showReadKey );
                }
                break;

                case 2: // delete REC READ, STO SENT
                {
                    qlm = qms->queryMessages( showSmsKey & ( (showIncomingKey & showReadKey) | (showOutgoingKey & showSentKey) ) );
                }
                break;

                case 3: // delete REC READ, STO SENT, STO UNSENT
                {
                    qlm = qms->queryMessages( showSmsKey & ( (showIncomingKey & showReadKey) | showOutgoingKey ) );
                }
                break;

                case 4: // delete all messages
                {
                    qlm = qms->queryMessages( showSmsKey );
                }
                break;

                default: // unknown delflag.
                {
                    atc->done( QAtResult::SMSOperationNotAllowed );
                    return;
                }
                break;
            }

            bool success = true;
            for ( it = qlm.begin(); it != qlm.end(); ++it ) {
                if ( !qms->removeMessage( *it ) ) {
                    success = false;
                }
            }

            success ? atc->done() : atc->done( QAtResult::SMSMemoryFailure );
        }
        break;

        case AtParseUtils::Support:
        {
            QString status = "+CMGD: (";

            QMailStore *qms = QMailStore::instance();
            QMailMessageKey showSmsKey( QMailMessageKey::Type, QMailMessage::Sms );
            QList<QMailMessageId> qlm = qms->queryMessages( showSmsKey );
            QList<QMailMessageId>::ConstIterator it;
            for ( it = qlm.begin(); it != qlm.end(); ++it ) {
                status += "," + QString::number( (*it).toULongLong() );
            }
            status += "),(0-4)";

            atc->send( status );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::SMSOperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGF Message Format}
    \compat

    The \c{AT+CMGF} command can be used to set the SMS message format
    to PDU (0) or text (1).  The default is PDU.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGF=<mode>} \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT+CMGF?} \o \c{+CMGF: <mode>}
    \row \o \c{AT+CMGF=?} \o \c{+CMGF: (0,1)}
    \endtable

    Conforms with: 3GPP TS 27.005.
*/
void AtSmsCommands::atcmgf( const QString& params )
{
    // Ignore the command if the SMS subsystem is not yet ready.
    if ( smsReader->ready() )
        atc->atgnc()->flagCommand( "+CMGF: ", atc->options()->messageFormat, params );
    else
        atc->done( QAtResult::USimBusy );
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGL List Messages}
    \compat

    The \c{AT+CMGL} command returns messages with status \c{<stat>}
    from the message store to the TE.  If the status of a message
    listed is "REC UNREAD", status of that message should change
    to "REC READ".

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGL[=<stat>]}
         \o \list
              \o \bold{if text mode (+CMGF=1): }
                 \c{+CMGL: <index>,<stat>,<address>,[<alpha>],[<scts>]
                 [,<address_type>,<body_length>]<CR><LF><data>
                 [<CR><LF>+CMGL: <index>,<stat>,<address>,[<alpha>],[<scts>]
                 [,<address_type>,<body_length>]<CR><LF><data>
                 [...]]}
              \o \bold{otherwise: }
                 \c{+CMS ERROR: <err>}
            \endlist
    \row \o \c{AT+CMGL=?} \o \c{+CMGL: }(list of supported \c{<stat>}s)
    \endtable

    Note that \c{<scts>} is only shown if the message is incoming,
    and \c{<alpha>} is shown if the \c{<address>} has a corresponding
    entry in the PhoneBook.  \c{<address_type>} and \c{<body_length>}
    are shown if \c{AT+CSDH=1}.

    Does not conform entirely with 3GPP TS 27.005, since only
    text mode (AT+CMGF=1) command is supported.
*/
void AtSmsCommands::atcmgl( const QString& params )
{
    switch ( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::CommandOnly: // flow on
        case AtParseUtils::Set:
        {
            if ( !atc->options()->messageFormat ) {
                // PDU mode selected - currently unsupported.
                atc->done( QAtResult::SMSOperationNotSupported );
                break;
            }

            QMailStore *qms = QMailStore::instance();
            QMailMessageKey showSmsKey( QMailMessageKey::Type, QMailMessage::Sms, QMailDataComparator::Includes );
            QMailMessageKey showIncomingKey( QMailMessageKey::Status, QMailMessage::Incoming, QMailDataComparator::Includes );
            QMailMessageKey showOutgoingKey( QMailMessageKey::Status, QMailMessage::Outgoing, QMailDataComparator::Includes );
            QMailMessageKey showReadKey( QMailMessageKey::Status, QMailMessage::Read, QMailDataComparator::Includes );
            QMailMessageKey showSentKey( QMailMessageKey::Status, QMailMessage::Sent, QMailDataComparator::Includes );
            QList<QMailMessageId> qml;

            // identify which messages we are required to list.
            uint posn = 1;
            QString stat = "";
            uint nstat = 4;
            if ( posn < (uint)params.length() ) {
                if ( atc->options()->messageFormat ) {
                    stat = QAtUtils::nextString( params, posn );
                    if ( stat == "REC READ" ) {
                        qml = qms->queryMessages( showSmsKey & ( showIncomingKey & showReadKey ) );
                        nstat = 0;
                    } else if ( stat == "REC UNREAD" ) {
                        qml = qms->queryMessages( showSmsKey & ( showIncomingKey & ~showReadKey ) );
                        nstat = 1;
                    } else if ( stat == "STO SENT" ) {
                        qml = qms->queryMessages( showSmsKey & ( showOutgoingKey & showSentKey ) );
                        nstat = 2;
                    } else if ( stat == "STO UNSENT" ) {
                        qml = qms->queryMessages( showSmsKey & ( showOutgoingKey & ~showSentKey ) );
                        nstat = 3;
                    } else if ( stat == "ALL" ) {
                        qml = qms->queryMessages( showSmsKey );
                        nstat = 4;
                    } else {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                } else {
                    nstat = QAtUtils::parseNumber( params, posn );
                    if ( nstat == 0 ) {
                        qml = qms->queryMessages( showSmsKey & ( showIncomingKey & showReadKey ) );
                        stat = "REC READ";
                    } else if ( nstat == 1 ) {
                        qml = qms->queryMessages( showSmsKey & ( showIncomingKey & ~showReadKey ) );
                        stat = "REC UNREAD";
                    } else if ( nstat == 2 ) {
                        qml = qms->queryMessages( showSmsKey & ( showOutgoingKey & showSentKey ) );
                        stat = "STO SENT";
                    } else if ( nstat == 3 ) {
                        qml = qms->queryMessages( showSmsKey & ( showOutgoingKey & ~showSentKey ) );
                        stat = "STO UNSENT";
                    } else if ( nstat == 4 ) {
                        qml = qms->queryMessages( showSmsKey );
                        stat = "ALL";
                    } else {
                        atc->done( QAtResult::OperationNotAllowed );
                        return;
                    }
                }
            } else {
                //qml = qms->queryMessages( showSmsKey );
                qml = qms->queryMessages( );
                stat = "ALL";
                nstat = 4;
            }

            QList<QMailMessageId>::ConstIterator it;
            for ( it = qml.begin(); it != qml.end(); ++it ) {
                QMailMessage qmm = qms->message(*it);
                QString address = "";
                QString alpha_address = "";
                QString service_centre = "";

                if ( qmm.status() & QMailMessage::Incoming ) {
                    // if it wasn't read before, it is now.
                    qmm.setStatus( qmm.status() | QMailMessage::Read );
                    stat = "REC READ";

                    // get the sender's address.
                    address = qmm.from().address();
                    alpha_address = qmm.from().displayName();

                    // and the service centre's address.
                    // note that the QtopiaMail API doesn't
                    // save this from the QSmsReader, so null.
                    service_centre = "";

                } else {
                    if ( stat == "ALL" ) {
                        if ( qmm.status() & QMailMessage::Sent ) {
                            stat = "STO SENT";
                            nstat = 3;
                        } else {
                            stat = "STO UNSENT";
                            nstat = 3;
                        }
                    }

                    // get the recipient's address
                    QList<QMailAddress> qmal = qmm.to();
                    address = qmal.at(0).address();
                    alpha_address = qmal.at(0).displayName();
                }

                // OK, build our response and send it back.
                QString status = "";
                if ( it != qml.begin() ) status += "\r\n";
                status += "+CMGL: " + QString::number( qmm.id().toULongLong() );
                if ( atc->options()->messageFormat ) {
                    status += ",\"" + stat + "\"";
                    status += "," + address;
                } else {
                    status += "," + QString::number( nstat );
                }
                status += "," + alpha_address;

                if ( atc->options()->messageFormat ) {
                    status += "," + service_centre;
                } else {
                    status += ",";// + pdu_length;
                }

                // now for the AT+CSDH optional parameters
                if ( atc->options()->csdh ) {
                    // too bad - QtopiaMail API doesn't save the data.
                }

                // now now the data.
                status += "\r\n";
                status += qmm.body().data();

                // send and grab the next one.
                atc->send( status );
            }

            // if we get to here, we have dealt with all messages in store.
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMGL: (\"REC READ\",\"REC UNREAD\",\"STO SENT\",\"STO UNSENT\",\"ALL\")" );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGR Read From Storage Area}
    \compat

    This command is used to read an SMS message from a certain
    location of the message storage area.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGR=<index>}
         \o \list
              \o \c{+CMGR: <message_status>,<address>,[<address_text>],<service_centre_timestamp>[,<address_type>,<TPDU_first_octet>,<protocol_identifier>,<data_coding_scheme>,<service_centre_address>,<service_centre_address_type>,<sms_message_body_length>]<CR><LF><sms_message_body>}
              \o \c{+CMGR: <message_status>,<address>,[<address_text>][,<address_type>,<TPDU_first_octet>,<protocol_identifier>,<data_coding_scheme>,[<validity_period>],<service_centre_address>,<service_centre_address_type>,<sms_message_body_length>]<CR><LF><sms_message_body>}
            \endlist
    \row \o \c{AT+CMGR=?} \o \c{+CMGR: }(list of valid \c{<index>}s)
    \endtable

    Note that the response to the set command depends on whether the
    message stored at the given index is a message that was received
    by the MT or one that is to be (or has been) sent by the MT.

    In the first case, it will contain a \c{<service_centre_timestamp>},
    and if the second, it may contain a \c{<validity_period>}.

    The \c{<address>} field will contain the sender's address if
    the message stored at the given index is a message that was received
    by the MT, and the recipient's address if the message stored at
    the given index is a message that is to be (or has been) sent
    by the MT.

    The fields \c{<address_type>}, \c{<TPDU_first_octet>},
    \c{<protocol_identifier>}, \c{<data_coding_scheme>},
    \c{<service_centre_address>}, \c{<service_centre_address_type>},
    \c{<sms_message_body_length>} and \c{<validity_period>} are
    optional, and enabled by the \c{AT+CSDH} command.

    The \c{<address_text>} field is optional and contains the string
    associated with the \c{<address>} in the address book of the
    MT.

    The \c{<message_status>} field can have the following values:
    \table
    \row \o \c{REC UNREAD} \o A received, incoming message that is unread
    \row \o \c{REC READ} \o A received, incoming message that has been read
    \row \o \c{STO UNSENT} \o A stored, outgoing message that is yet to be sent
    \row \o \c{STO SENT} \o A stored, outgoing message that has been sent
    \endtable

    Does not conform completely with 3GPP TS 27.005;
    this implementation only allows reading of SMS
    messages in text mode (AT+CMGF=1).
*/
void AtSmsCommands::atcmgr( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::Set:
        {
            uint posn = 1;
            uint index = 0;
            if ( posn < (uint)params.length() ) {
                index = QAtUtils::parseNumber( params, posn );
                wantedSmsIndex = index;

                // the following function allows the use of AT+CSDH
                // however, qtopia handles the message store badly.
                // hence we don't use the QSmsReader - we use the 
                // QtopiaMail API (which means no +CSDH information).
                //readingSms = true;
                //smsReader->firstMessage();

                // retrieve the message from the store
                QMailStore *qms = QMailStore::instance();
                QMailMessage qmm = qms->message( QMailMessageId( index ) );

                // check that it is a valid message, and also an SMS message.
                if ( !qmm.id().isValid() || ! ( qmm.messageType() & QMailMessage::Sms ) ) {
                    // this is not an sms.
                    atc->done( QAtResult::Error );
                    break;
                }

                // this is the sms they wanted.  Build response.
                QString status = "+CMGR: ";
                QString address = "";

                // are we in text or pdu mode?
                if ( atc->options()->messageFormat ) {

                    // append the message status and address info.
                    if ( qmm.status() & QMailMessage::Incoming ) {
                        // message_status
                        if ( qmm.status() & QMailMessage::Read ) {
                            status += "\"REC READ\"";
                        } else {
                            status += "\"REC UNREAD\"";

                            // also, set the status to REC READ as per 3GPP 27.005
                            qmm.setStatus( qmm.status() | QMailMessage::Read );
                        }

                        // ,address,address_text
                        address = qmm.from().address();
                        status += "," + address;
                        status += "," + qmm.from().displayName();

                        // ,timestamp
                        status += ",\"" + qmm.date().toUTC().toString( "yy/MM/dd,hh:mm:ss" );
                        status += "+00\""; // we're in UTC, so no timezone.
                    } else {
                        // message_status
                        if ( qmm.status() & QMailMessage::Sent ) {
                            status += "\"STO SENT\"";
                        } else {
                            status += "\"STO UNSENT\"";
                        }

                        // ,address,address_text
                        QList<QMailAddress> qmal = qmm.to();
                        address = qmal.at(0).address();
                        status += ",\"" + address + "\"";
                        status += ",\"" + qmal.at(0).displayName() + "\"";
                    }

                    // AT+CSDH activated optional parameters.
                    if ( atc->options()->csdh ) {
                        // too bad - QMailStore/QMailMessage doesn't
                        // store service centre addresses, data coding
                        // scheme, allow toPDU() translation, etc.
                    }

                    // <cr><lf>body_text
                    status += "\r\n";
                    status += qmm.body().data();

                    // we are finished.
                    atc->send( status );
                    atc->done();
                    return;
                } else {
                    // we're in PDU mode.
                    atc->done( QAtResult::OperationNotSupported ); // currently, text mode only.
                }
            } else {
                // not enough parameters.
                atc->done( QAtResult::OperationNotAllowed );
                break;
            }
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGS Send Message}
    \compat

    The \c{AT+CMGS} command can be used to send SMS messages in
    either PDU (\c{AT+CMGF=0}) or text (\c{AT+CMGF=1}) mode.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGS=<da>[,<toda>]<CR>message<ctrl-Z/ESC>}
         \o \c{+CMGS: <mr>}, \c{+CME ERROR: <err>} - if in text mode.
    \row \o \c{AT+CMGS=<length><CR>pdu<ctrl-Z/ESC>}
         \o \c{+CMGS: <mr>}, \c{+CME ERROR: <err>} - if in pdu mode.
    \row \o \c{AT+CMGS=?} \o \c{OK}
    \endtable

    Execution command sends the message to the network.  The message
    reference value \c{<mr>} is returned to the TE upon successful message
    delivery.

    Conforms with: 3GPP TS 27.005.
*/
void AtSmsCommands::atcmgs( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            // Extract the destination number if we are sending in text mode.
            // If we are sending in PDU mode, we ignore the length as it will
            // be implicit in the data that follows.
            if ( atc->options()->messageFormat ) {
                uint posn = 1;
                smsNumber = QAtUtils::nextString( params, posn );
                uint type = QAtUtils::parseNumber( params, posn );
                if ( type == 0 )
                    type = 129;
                smsNumber = QAtUtils::decodeNumber( smsNumber, type );
            }

            // Request an extra line of data for the text/pdu.
            sendingSms = true;
            atc->frontEnd()->requestExtra();
        }
        break;

        case AtParseUtils::Support:
        {
            // Just need to say "OK" to indicate that we support this command.
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGW Write Message to Memory}
    \compat

    The \c{AT+CMGW} command stores a message to memory storage.
    The memory location \c{<index>} of the stored message
    is returned.  By default, the message status will
    be set to "REC UNREAD", however other status values
    may be given.

    \table
    \header \o Command \o Possible Responses
    \row \o \bold{if text mode (AT+CGMF=1):}
            \c{AT+CMGW=[=<address>[,<address_type>[,<stat>]]]<CR>}
            \bold{text is entered}\c{ctrl-z/ESC}
         \o \list \o \c{+CMGW: <index>} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT+CMGW=?} \o
    \endtable

    Does not conform completely with 3GPP TS 27.005;
    in this implementation, SMS message may only
    be written in text mode.
*/
void AtSmsCommands::atcmgw( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {
        case AtParseUtils::CommandOnly: // flow on
        case AtParseUtils::Set:
        {
            if ( atc->options()->messageFormat ) {
                uint posn = 1;
                if ( posn < (uint)params.length() ) {
                    smsNumber = QAtUtils::nextString( params, posn );
                }

                if ( posn < (uint) params.length() ) {
                    cmgw_address_type = QAtUtils::parseNumber( params, posn );
                    if ( cmgw_address_type == 0 ) {
                        cmgw_address_type = 129;
                    }
                    smsNumber = QAtUtils::decodeNumber( smsNumber, cmgw_address_type );
                }

                if ( posn < (uint)params.length() ) {
                    cmgw_status = QAtUtils::nextString( params, posn );
                } else {
                    cmgw_status = "\"STO UNSENT\"";
                }

                writingSms = true;
                atc->frontEnd()->requestExtra();
            } else {
                // we're in PDU mode.
                atc->done( QAtResult::OperationNotSupported ); // currently, text mode only.
            }
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMMS More Messages to Send}
    \compat

    The \c{AT+CMMS} command can be used to inform the modem that several
    SMS messages will be sent in quick succession, and the link should be
    held open for more efficient transmission.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMMS?} \c \c{+CMMS: 0}
    \row \o \c{AT+CMGS=?} \o \c{+CMMS: (0-2)}
    \endtable

    This implementation does not do anything special for this command.
    The underlying modem will hold the link open when it judges that it
    would be advantageous to do so.

    Conforms with: 3GPP TS 27.005.
*/
void AtSmsCommands::atcmms( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Get:
        {
            atc->send( "+CMMS: 0" );
            atc->done();
        }
        break;

        case AtParseUtils::Set:
        {
            // Allow the user to set anything - we don't do anything special.
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CMMS: (0-2)" );
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CNMI New Message Indications to TE}
    \compat

    The \c{AT+CNMI} command is used to select the procedure
    for how receiving new messages from the network is
    indicated to the TE when the TE is active.

    When the TE is inactive, message receiving is done as
    specified in 3GPP TS 23.038.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CNMI=[<mode>[,<mt>[,<bm>[,<ds>[,<bfr]]]]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CNMI?} \o \c{+CNMI: <mode>,<mt>,<bm>,<ds>,<bfr>}
    \row \o \c{AT+CNMI=?} \o \c{+CNMI: }(list of supported \c{<mode>}s),
                                        (list of supported \c{<mt>}s),
                                        (list of supported \c{<bm>}s),
                                        (list of supported \c{<ds>}s),
                                        (list of supported \c{<bfr>}s)
    \endtable

    This implementation is currently a dummy implementation;
    it does nothing, and always returns error.

    Conforms with: 3GPP TS 27.005.
*/
void AtSmsCommands::atcnmi( const QString& params )
{
    switch ( AtParseUtils::mode( params ) )
    {
        case AtParseUtils::Set:
        {
            atc->done( QAtResult::SMSOperationNotAllowed );
        }
        break;

        case AtParseUtils::Get:
        {
            atc->done( QAtResult::SMSOperationNotAllowed );
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done( QAtResult::SMSOperationNotAllowed );
        }
        break;

        default:
        {
            atc->done( QAtResult::SMSOperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CPMS Preferred Message Storage}
    \compat

    The \c{AT+CPMS} command can be used to set the preferred message storage
    for reading and writing SMS messages.  The only message storage that
    is supported by this implementation is \c{SM}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPMS=<mem1>[,<mem2>[,<mem3>]]} \o \c{+CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>,<used3>,<total3>} or \c{+CME ERROR: <err>}
    \row \o \c{AT+CPMS?} \o \c{+CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>,<used3>,<total3>}
    \row \o \c{AT+CPMS=?} \o \c{+CPMS: ((list of supported <mem1>s),(list of supported <mem2>s),(list of supported <mem3>s))}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtSmsCommands::atcpms( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            // Bail out if the SIM is not ready yet.
            if ( !smsReader->ready() ) {
                atc->done( QAtResult::USimBusy );
                break;
            }

            // All three parameters must be set to "SM".
            uint posn = 1;
            if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                atc->done( QAtResult::SMSOperationNotAllowed );
                break;
            }
            if ( posn < (uint)( params.length() ) ) {
                if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                    atc->done( QAtResult::SMSOperationNotAllowed );
                    break;
                }
                if ( posn < (uint)( params.length() ) ) {
                    if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                        atc->done( QAtResult::SMSOperationNotAllowed );
                        break;
                    }
                }
            }

            // Report the used and total messages for the "SM" store.
            QString data = QString::number( smsReader->usedMessages() ) +
                           "," +
                           QString::number( smsReader->totalMessages() );
            atc->send( "+CPMS: " + data + "," + data + "," + data );
            atc->done();
        }
        break;

        case AtParseUtils::Get:
        {
            // Bail out if the SIM is not ready yet.
            if ( smsReader->ready() ) {
                atc->done( QAtResult::USimBusy );
                break;
            }

            // Report the used and total messages for the "SM" store.
            QString data = "\"SM\"," +
                           QString::number( smsReader->usedMessages() ) +
                           "," +
                           QString::number( smsReader->totalMessages() );
            atc->send( "+CPMS: " + data + "," + data + "," + data );
            atc->done();
        }
        break;

        case AtParseUtils::Support:
        {
            atc->send( "+CPMS: ((\"SM\"),(\"SM\"),(\"SM\"))" );
            atc->done();
        }
        break;

        default: break;

    }
}


/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CRES Restore Settings}
    \compat

    The \c{AT+CRES} command restores SMS settings from EEPROM.  This is
    not supported in this implementation.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRES[=<profile>]} \o \c{OK}
    \row \o \c{AT+CRES=?} \o \c{OK}
    \endtable

    This implementation does not conform with 3GPP TS 27.005.
*/
void AtSmsCommands::atcres()
{
    // Restore SMS settings from EEPROM.  We don't support this.
    atc->done();
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSAS Save Settings}
    \compat

    The \c{AT+CSAS} command saved SMS settings to EEPROM.  This is
    not supported in this implementation.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSAS[=<profile>]} \o \c{OK}
    \row \o \c{AT+CSAS=?} \o \c{OK}
    \endtable

    This implementation does not conform with 3GPP TS 27.005.
*/
void AtSmsCommands::atcsas()
{
    // Save SMS settings to EEPROM.  We don't support this.
    atc->done();
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSCA Service Center Address}
    \compat

    The \c{AT+CSCA} command reads or modifies the SMS service center address
    on the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSCA=<sca>[,<tosca>]} \o \c{OK}
    \row \o \c{AT+CSCA?} \o \c{+CSCA: <sca>,<tosca>}
    \row \o \c{AT+CSCA=?} \o \c{OK}
    \endtable

    Set command updates the SMS service center address on the SIM.
    Read command reports the current SMS service center address on the SIM.

    Conforms with 3GPP TS 27.005.
*/
void AtSmsCommands::atcsca( const QString& params )
{
    switch ( AtParseUtils::mode( params ) ) {

        case AtParseUtils::Set:
        {
            uint posn = 1;
            QString number = QAtUtils::nextString( params, posn );
            uint type = QAtUtils::parseNumber( params, posn );
            if ( type == 0 )
                type = 129;
            number = QAtUtils::decodeNumber( number, type );
            atc->atgnc()->setNumber( QServiceNumbers::SmsServiceCenter, number );
        }
        break;

        case AtParseUtils::Get:
        {
            atc->atgnc()->queryNumber( QServiceNumbers::SmsServiceCenter );
        }
        break;

        case AtParseUtils::Support:
        {
            atc->done();
        }
        break;

        default:
        {
            atc->done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSDH Show Text Mode Parameters}
    \compat

    The \c{AT+CSDH} command controls whether extended text mode parameters
    are shown in response to \c{AT+CMGL} and \c{AT+CMGR} commands in
    text mode.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSDH=<show>} \o \list \o \c{OK} \o \c{+CME ERROR: <err>} \endlist
    \row \o \c{AT+CSDH?} \o \c{+CSDH: <show>}
    \row \o \c{AT+CSDH=?} \o \c{+CSDH: (0,1)}
    \endtable

    Conforms with 3GPP TS 27.005.
*/
void AtSmsCommands::atcsdh( const QString& params )
{
    atc->atgnc()->flagCommand( "+CSDH: ", atc->options()->csdh, params );
}

// In certain circumstances, data preceded by a <CR><LF> must
// be read as part of the previous command.  This function
// checks that we were expecting this "extra line" of data,
// and deals with it accordingly.
void AtSmsCommands::extraLine( const QString& line, bool cancel )
{
    // Check that we actually expected this to happen.
    if ( !sendingSms && !writingSms )
        return;

    ( sendingSms ) ? sendingSms = false : writingSms = false;

    // If the operation was cancelled, just say "OK" and stop.
    if ( cancel ) {
        atc->done();
        return;
    }

    if ( sendingSms ) {
        // Compose the message to be sent.
        QSMSMessage msg;
        if ( atc->options()->messageFormat ) {
            msg.setRecipient( smsNumber );
            msg.setText( QAtUtils::decode( line, atc->options()->codec ) );
        } else {
            msg = QSMSMessage::fromPdu( QAtUtils::fromHex( line ) );
        }

        // If the SMS sender is not available, then fail the request
        // by pretending that we don't have network service to send it.
        if ( !smsSender->available() ) {
            atc->done( QAtResult::SMSNoNetworkService );
            return;
        }

        // Send the message.
        smsMessageId = smsSender->send( msg );
    } else {
        // we are writing the message to the message store.
        QMailMessage *qmm = new QMailMessage();
        if ( atc->options()->messageFormat ) {
            // text format.  Grab the parameters and build the QMailMessage.
            QMailAddress qma( smsNumber );
            if ( cmgw_status == "\"STO UNSENT\"" ) {
                qmm->setTo( qma );
                qmm->setStatus( QMailMessage::Outgoing );
            } else if ( cmgw_status == "\"STO SENT\"" ) {
                qmm->setTo( qma );
                qmm->setStatus( QMailMessage::Outgoing | QMailMessage::Sent );
            } else if ( cmgw_status == "\"REC UNREAD\"" ) {
                qmm->setFrom( qma );
                qmm->setStatus( QMailMessage::Incoming );
            } else if ( cmgw_status == "\"REC READ\"" ) {
                qmm->setFrom( qma );
                qmm->setStatus( QMailMessage::Incoming | QMailMessage::Read);
            } else {
                // invalid status
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            QString bodyText = QAtUtils::decode( line, atc->options()->codec );
            QMailMessageContentType contentType( "text/plain; charset=UTF-8" );
            qmm->setBody( QMailMessageBody::fromData( bodyText, contentType, QMailMessageBody::Base64 ) );
        } else {
            // PDU format.  Build the QMailMessage from the PDU bytes.
            QSMSMessage msg = QSMSMessage::fromPdu( QAtUtils::fromHex( line ) );
            qmm->setTo( QMailAddress(msg.recipient()) );
            qmm->setFrom( QMailAddress(msg.sender()) );
            if ( cmgw_status == "\"STO UNSENT\"" ) {
                qmm->setStatus( QMailMessage::Outgoing );
            } else if ( cmgw_status == "\"STO SENT\"" ) {
                qmm->setStatus( QMailMessage::Outgoing | QMailMessage::Sent );
            } else if ( cmgw_status == "\"REC UNREAD\"" ) {
                qmm->setStatus( QMailMessage::Incoming );
            } else if ( cmgw_status == "\"REC READ\"" ) {
                qmm->setStatus( QMailMessage::Incoming | QMailMessage::Read);
            } else {
                // invalid status
                atc->done( QAtResult::OperationNotAllowed );
                return;
            }
            QMailMessageContentType contentType( "text/plain; charset=UTF-8" );
            qmm->setBody( QMailMessageBody::fromData( msg.text(), contentType, QMailMessageBody::Base64 ) );
        }

        qmm->setMessageType( QMailMessage::Sms );

        // now the QMailMessage should be completely built.
        QMailStore *qms = QMailStore::instance();
        if ( qms->addMessage( qmm ) ) {
            // success!
            atc->send( "+CMGW: " + QString::number( qmm->id().toULongLong() ) );
            atc->done();
        } else {
            // failed to write the SMS to the message store
            atc->done( QAtResult::Error );
        }

        // cleanup.
        delete qmm;
    }
}

// currently unused, since the QSmsReader class does not provide
// indexes to SMSes stored.  We use the qtmail API instead,
// but will need to switch back to this when indexing is provided
// (in order to conform to 27.005 - able to return PDU data etc).
void AtSmsCommands::smsFetched( const QString & id, const QSMSMessage & m )
{
    if ( readingSms ) {
        if ( id.length() == 0 ) {
            // there was no SMS of the specified index.
            readingSms = false;
            wantedSmsIndex = -1;
            readingSmsCount = -1;

            atc->done( QAtResult::Error );
            return;
        }

        if ( wantedSmsIndex == (++readingSmsCount) ) {
            // this is the SMS that they wanted to read with AT+CMGR
            readingSms = false; 
            wantedSmsIndex = -1;
            readingSmsCount = -1;

            // begin forming our response code string.
            QString status = "+CMGR: ";

            // need to know whether the message is incoming or outgoing.
            bool incoming = false;
            if ( m.sender() != "" ) {
                incoming = true;
            }

            // note - need some way to find message status (read|sent/unread|unsent)
            QString address = "";
            if ( false ) { // THIS NEEDS CHANGING, OBVIOUSLY
                if ( incoming ) {
                    status += "\"REC UNREAD\"";
                    address = m.sender();
                } else {
                    status += "\"STO UNSENT\"";
                    address = m.recipient();
                }
            } else {
                if ( incoming ) {
                    status += "\"REC READ\"";
                    address = m.sender();
                } else {
                    status += "\"STO SENT\"";
                    address = m.recipient();
                }
            }

            // send the sender/recipient address.
            status += ",\"" + address + "\"";

            // --- the following is one way to do it.
            // get the address_text
            //phoneBook.getEntries();
            // wait for our handler to finish finding address_text
            //mutex.wait();
            // --- fortunately, <address_text> is optional. we leave it null.

            // our phonebook handler is finished; do the rest.
            QString address_text = "";
            status += "," + address_text;

            // now print out the timestamp of the received message.
            if ( incoming ) {
                QDateTime utcSaved = m.timestamp();
                QDateTime current = QDateTime::currentDateTime();
                QDateTime localisedSaved = utcSaved.toLocalTime(); // assumes m.timestamp() is in UTC
                uint localSecs = localisedSaved.toTime_t();
                uint utcSecs = utcSaved.toTime_t();

                int offsetSeconds = 0;
                ( localSecs > utcSecs ) ? offsetSeconds = localSecs - utcSecs : offsetSeconds = -1 * ( utcSecs - localSecs );

                int offsetQHrs = ( offsetSeconds / 3600 ) * 4;

                status += ",\"" + localisedSaved.toString( "yy/MM/dd,hh:mm:ss" );
                ( offsetQHrs >= 0) ? status += "+" : status += "-";
                if ( QString::number( offsetQHrs ).length() == 1 ) {
                    status += "0";
                }
                status += QString::number( offsetQHrs ) + "\"";
            }

            // retrieve the body for future use:
            QString body = m.text();

            // now for the optional, AT+CSDH enabled fields.
            if ( atc->options()->csdh ) {
                // international or local address type?
                ( address.at(0) == '+' ) ? status += ",145" : status += ",129";
                status += "," + m.toPdu().at(0); // grab the first octet of the PDU data
                status += "," + QString::number( m.protocol() );
                status += "," + QString::number( m.dataCodingScheme() );
                ( !incoming ) ? status += "," + QString::number( m.validityPeriod() ) : status += "," ;
                status += ",\"" + m.serviceCenter() + "\"";
                ( m.serviceCenter().at(0) == '+' ) ? status += ",145" : status += ",129";
                status += "," + QString::number( body.length() );
            }

            // attach the body of the message
            status += "\r\n" + body;

            // and, finally, send it off.
            atc->send( status );
            atc->done();
        } else {
            // nope, this wasn't the one we were searching for... we need another.
            smsReader->nextMessage();
        }
    }
}

// This slot is called when the QSmsSender is finished sending an SMS.
void AtSmsCommands::smsFinished( const QString& id, QTelephony::Result result )
{
    if ( id == smsMessageId ) {
        smsMessageId = QString();
        if ( result == QTelephony::OK ) {
            // Make up a message reference for the response.
            atc->send( "+CMGS: " + QString::number( smsMessageReference++ ) );
        }
        atc->done( (QAtResult::ResultCode)result );
    }
}

