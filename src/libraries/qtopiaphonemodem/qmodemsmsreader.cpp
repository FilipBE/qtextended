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

#include <qmodemsmsreader.h>
#include <qmodemservice.h>
#include <qmodemindicators.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qretryatchat.h>
#include <qsimenvelope.h>

/*!
    \class QModemSMSReader
    \inpublicgroup QtCellModule

    \brief The QModemSMSReader class provides SMS reading facilities for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CPMS}, \c{AT+CMGL}, \c{AT+CMGR},
    \c{AT+CMGD}, \c{AT+CNMI}, and \c{AT+CMGF} commands from
    3GPP TS 27.005.

    QModemSMSReader implements the QSMSReader telephony interface.  Client
    applications should use QSMSReader instead of this class to
    access the modem's incoming SMS message store.

    \sa QSMSReader
*/

class QSMSTaggedMessage
{
public:
    QString     identifier;
    QSMSMessage message;
    bool        isUnread;
};

class QModemSMSReaderPrivate
{
public:
    QModemSMSReaderPrivate()
    {
        totalStore = 0;
        pendingPosn = 0;
        unreadCount = 0;
        fragmentsOnly = false;
        haveCached = false;
        fetching = false;
        pendingCheck = false;
        pendingFirstMessage = false;
        needRefetch = false;
        pseudoIndex = 1;
        initializing = false;
    }

    QModemService *service;
    int totalStore;
    QList<QSMSTaggedMessage *> pending;
    QList<QSMSTaggedMessage *> pseudoInbox;
    uint pseudoIndex;
    int pendingPosn;
    uint unreadCount;
    bool fragmentsOnly;
    bool haveCached;
    bool fetching;
    bool pendingCheck;
    bool pendingFirstMessage;
    bool needRefetch;
    bool initializing;
    QStringList unreadList;
};

/*!
    Create a new modem-based SMS request object for \a service.
*/
QModemSMSReader::QModemSMSReader( QModemService *service )
    : QSMSReader( service->service(), service, QCommInterface::Server )
{
    d = new QModemSMSReaderPrivate();
    d->service = service;
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );
    service->connectToPost( "smsready", this, SLOT(smsReady()) );

    service->primaryAtChat()->registerNotificationType
        ( "+CMTI:", this, SLOT(newMessageArrived()) );
    service->primaryAtChat()->registerNotificationType
        ( "+CDSI:", this, SLOT(newMessageArrived()) );

    connect( service->primaryAtChat(),
             SIGNAL(pduNotification(QString,QByteArray)),
             this,
             SLOT(pduNotification(QString,QByteArray)) );
}

/*!
    Destroy this modem-based SMS request object.
*/
QModemSMSReader::~QModemSMSReader()
{
    delete d;
}

/*!
    \reimp
*/
void QModemSMSReader::check()
{
    check( false );
}

/*!
    \reimp
*/
void QModemSMSReader::firstMessage()
{
    if ( d->haveCached ) {
        d->pendingPosn = 0;
        nextMessage();
    } else {
        d->pendingFirstMessage = true;
        if ( !d->fetching )
            fetchMessages();
    }
}

/*!
    \reimp
*/
void QModemSMSReader::nextMessage()
{
    if ( !d->unreadList.isEmpty() ) {
        // Remove messages in the pending queue from the unread list.
        for ( int posn = 0; posn < d->pending.count(); ++posn )
            d->unreadList.removeAll( d->pending.at(posn)->identifier );
        updateUnreadList();
    }
    if ( d->pendingPosn < d->pending.count() ) {

        // Pass the next entry to the "fetched" callback.
        QSMSTaggedMessage *tmsg = d->pending.at( d->pendingPosn++ );
        fetched( tmsg->identifier, tmsg->message );

    } else {

        // We are at the end of the pending list.
        d->pendingPosn = 0;
        QSMSMessage dummyMsg;
        emit fetched( "", dummyMsg );

    }
}

/*!
    \reimp
*/
void QModemSMSReader::deleteMessage( const QString& id )
{
    if ( d->unreadList.contains( id ) ) {
        d->unreadList.removeAll( id );
        updateUnreadList();
    }
    QList<QSMSTaggedMessage *>::Iterator it;
    for ( it = d->pending.begin(); it != d->pending.end(); ++it ) {
        if ( (*it)->identifier == id ) {
            d->pending.erase( it );
            break;
        }
    }
    if ( id.indexOf( QChar(':') ) == 2 ) {

        // Extract the name of the message store.
        QString store = id.left(2);
	QString trimId = id;
	
	// Remove trailing timestamp
	int timeIndex = trimId.indexOf( QChar(':'), 3 );
	if (timeIndex != -1)
	    trimId = trimId.left( timeIndex );

        // Process the comma-separated indices in the list.  Normally
        // there will be only one, but there could be multiple if
        // the message was reconstructed from multiple parts.
        int posn = 3;
        int temp, numDeleted;
        QString index;
        if ( store != "@@" && !messageStore().isEmpty() )
            d->service->primaryAtChat()->chat( "AT+CPMS=\"" + store + "\"" );
        numDeleted = 0;
        while ( posn < (int)trimId.length() ) {
            temp = trimId.indexOf( QChar(','), posn );
            if ( temp == -1 ) {
                index = trimId.mid( posn );
                posn = trimId.length();
            } else {
                index = trimId.mid( posn, temp - posn );
                posn = temp + 1;
            }
            if ( store == "@@" ) {
                // Delete the message from the pseudo inbox.
                QList<QSMSTaggedMessage *>::Iterator it;
                QSMSTaggedMessage *tmsg;
                QString actualId = "@@:" + index;
                it = d->pseudoInbox.begin();
                while ( it != d->pseudoInbox.end() ) {
                    tmsg = *it;
                    if ( tmsg->identifier == actualId ) {
                        d->pseudoInbox.erase( it );
                        delete tmsg;
                        break;
                    }
                    ++it;
                }
            } else {
                d->service->primaryAtChat()->chat( "AT+CMGD=" + index );
                ++numDeleted;
            }
        }

        // Update the "used messages" count to reflect the change.
        if ( numDeleted > 0 ) {
            int used = usedMessages();
            used -= numDeleted;
            if ( used < 0 )
                used = 0;
            setValue( "usedMessages", used );

            // After we have deleted message(s), the SIM is no longer full
            d->service->indicators()->setSmsMemoryFull( QModemIndicators::SmsMemoryOK );
        }
    }
}

/*!
    \reimp
*/
void QModemSMSReader::setUnreadCount( int value )
{
    d->unreadCount = value;
    updateUnreadCount();
}

void QModemSMSReader::updateUnreadCount()
{
    QVariant prev = value( "unreadCount" );
    if ( prev.isNull() || prev.toInt() != (int)(d->unreadCount) ) {
        setValue( "unreadCount", (int)(d->unreadCount) );
        emit unreadCountChanged();
    }
}

void QModemSMSReader::updateUnreadList()
{
    setValue( "unreadList", d->unreadList );
}

/*!
    Returns the name of the SMS message store to scan for incoming messages.
    The default implementation returns \c SM.  If this function returns
    an empty string, then the message store will not be changed from
    the modem's power-on default with an \c{AT+CPMS} command.

    \sa messageListCommand()
*/
QString QModemSMSReader::messageStore() const
{
    return "SM";        // No tr
}

/*!
    Returns the command that lists the contents of an SMS message store.
    Normally this is \c{AT+CMGL=4}, but some modems do not support the \c{=4}.

    \sa messageStore()
*/
QString QModemSMSReader::messageListCommand() const
{
    return "AT+CMGL=4";     // No tr
}

/*!
    Process a SIM download \a message from the network.  Modem
    vendor plug-ins should override this and send the message back
    to the SIM.

    Normally SIM download messages are processed internally by the modem.
    This function is only called if the message ends up in the incoming
    SMS message store.

    The default implementation composes a \c{SMS-PP DOWNLOAD}
    envelope, according to 3GPP TS 11.14, and sends it to the
    modem using the \c{AT+CSIM command}.
*/
void QModemSMSReader::simDownload( const QSMSMessage& message )
{
    // Construct the SMS-PP DOWNLOAD envelope to be sent to the SIM.
    QSimEnvelope env;
    env.setType( QSimEnvelope::SMSPPDownload );
    QByteArray pdu = message.toPdu();
    int sclen = (pdu[0] & 0xFF);
    env.addExtensionField( 0x06, pdu.mid( 1, sclen ) );     // Service center address.
    env.addExtensionField( 0x8B, pdu.mid( sclen + 1 ) );    // SMS-TPDU for the SMS-DELIVER.
    pdu = env.toPdu();

    // Build the payload for the AT+CSIM command.
    QByteArray cmd;
    cmd += (char)0xA0;
    cmd += (char)0xC2;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)pdu.size();
    cmd += pdu;

    // Send the AT+CSIM command to the modem.
    d->service->primaryAtChat()->chat
        ( "AT+CSIM=" + QString::number( cmd.size() * 2 ) + "," + QAtUtils::toHex( cmd ) );
}

void QModemSMSReader::check(bool force)
{
    if ( d->haveCached || ( d->initializing && !force ) ) {
        emit messageCount( d->pending.count() );
        updateUnreadList();
        updateUnreadCount();
    } else {
        d->pendingCheck = true;
        if ( !d->fetching )
            fetchMessages();
    }
}

void QModemSMSReader::resetModem()
{
    if ( !d->initializing ) {
        d->initializing = true;
        d->service->post( "needsms" );
    }
}

void QModemSMSReader::smsReady()
{
    if ( d->initializing ) {
        d->initializing = false;

        // Query the available SMS message notification options.
        d->service->primaryAtChat()->chat
            ( "AT+CNMI=?", this, SLOT(nmiStatusReceived(bool,QAtResult)) );

        // Perform the initial "how many messages are there" check.
        check( true );
    }
}

static QChar cnmiOpt(int flag, const char* pref)
{
    for ( ; *pref; pref++ ) {
        int bit = *pref-'0';
        if ( flag & (1 << bit) )
            return *pref;
    }
    return '0'; // not good...
}

void QModemSMSReader::nmiStatusReceived( bool, const QAtResult& result )
{
    int flags[5] = {0, 0, 0, 0, 0};
    int index = 0;
    int lposn;
    uint ch, ch2;
    int insideb;
    QString line;
    QAtResultParser cmd( result );

    // Collect up flags that represent the supported indication types.
    if ( cmd.next( "+CNMI:" ) ) {
        line = cmd.line();
        index = 0;
        lposn = 0;
        insideb = 0;
        while ( index < 5 && lposn < line.length() ) {
            ch = line[lposn++].unicode();
            if ( ch == '(' ) {
                insideb = true;
            } else if ( ch == ',' ) {
                if ( !insideb )
                    ++index;
            } else if ( ch == ')' ) {
                insideb = false;
            } else if ( insideb && ch >= '0' && ch <= '9' ) {
                if ( lposn < ( line.length() - 1 ) &&
                     line[lposn] == '-' &&
                     line[lposn + 1] >= '0' && line[lposn + 1] <= '9' )
                {
                    ch2 = line[lposn + 1].unicode();
                    lposn += 2;
                    while ( ch <= ch2 ) {
                        flags[index] |= (1 << (ch - '0'));
                        ++ch;
                    }
                }
                else
                {
                    flags[index] |= (1 << (ch - '0'));
                }
            }
        }
    }

    // Construct the command to send to enable indications.
    // We send ordinary SMS messages into the device's queue, and
    // cell broadcast messages direct to us.
    line = "AT+CNMI=";
    line += cnmiOpt(flags[0],"2310");
    line += ",";
    line += cnmiOpt(flags[1],"10");
    line += ",";
    line += cnmiOpt(flags[2],"2301");
    line += ",";
    line += cnmiOpt(flags[3],"01");
    line += ",";
    line += cnmiOpt(flags[4],"01");

    // Send the command.  If it doesn't work, then we assume that
    // the phone acts in "won't notify" mode, so no harm done.
    //
    // Some modems fail to process this command for a small period
    // of time after the AT+CPIN request at startup, so we need
    // to repeat the command if it fails.
    new QRetryAtChat( d->service->primaryAtChat(), line, 5 );

    // We are now initialized.
    d->initializing = false;
}

void QModemSMSReader::newMessageArrived()
{
    d->fragmentsOnly = false;
    if ( d->fetching ) {
        d->needRefetch = true;
    } else {
        d->haveCached = false;
        check();
    }
}

void QModemSMSReader::pduNotification
        ( const QString& type, const QByteArray& pdu )
{
    if ( type.startsWith( "+CMT:" ) ) {

        // This is an SMS message that was delivered directly to Qtopia
        // without first going through the incoming SMS queue.  Technically,
        // this is a violation of 3GPP TS 07.05 because we explicitly sent a
        // "AT+CNMI" command to stop this.  But some modems do it for SMS
        // datagrams and WAP Push messages, while still putting text messages
        // into the normal queue.  We add it to the pseudo inbox.
        QString id = "@@:" + QString::number( d->pseudoIndex++ );
        QSMSTaggedMessage *tmsg = new QSMSTaggedMessage;
        tmsg->identifier = id;
        tmsg->isUnread = true;
        tmsg->message = QSMSMessage::fromPdu( pdu );
        d->pseudoInbox.append( tmsg );

        // Fake out a new message notification to force a check.
        newMessageArrived();

    }
}

void QModemSMSReader::extractMessages( const QString& store, const QAtResult& result )
{
    QAtResultParser cmd( result );
    while ( cmd.next( "+CMGL:" ) ) {

        // Get the message index and the PDU information.
        uint index = cmd.readNumeric();
        uint status = cmd.readNumeric();
        QByteArray pdu = QAtUtils::fromHex( cmd.readNextLine() );

        // Unpack the PDU.
        QSMSTaggedMessage *tmsg = new QSMSTaggedMessage;
        tmsg->isUnread = ( status == 0 );
        tmsg->message = QSMSMessage::fromPdu( pdu );

        // Build the message identifier from the store, index and date
        QString id = store + ":" + QString::number( index );
	id += ":" + QString::number( tmsg->message.timestamp().toTime_t(), 36 );
        tmsg->identifier = id;

        // Record the message information for later.
        d->pending.append( tmsg );

    }
}

void QModemSMSReader::cpmsDone( bool, const QAtResult& result )
{
    // Look at the used and total values to determine if the
    // incoming SMS memory store is full or not, so we can update
    // the "SMSMemoryFull" indicator on modems that don't have
    // a proprietry way of detecting the full state.
    QAtResultParser parser( result );
    QModemIndicators *indicators = d->service->indicators();
    if ( parser.next( "+CPMS:" ) ) {
        if ( parser.line().startsWith( QChar('"') ) )
            parser.readString();    // Skip store name, if present.
        uint used = parser.readNumeric();
        uint total = parser.readNumeric();
        if ( used < total ) {
            indicators->setSmsMemoryFull( QModemIndicators::SmsMemoryOK );
        } else {
            indicators->setSmsMemoryFull( QModemIndicators::SmsMemoryFull );
        }

        // Update the local value space with the actual counts.
        setValue( "usedMessages", (int)used, Delayed );
        setValue( "totalMessages", (int)total );
    }
}

void QModemSMSReader::storeListDone( bool ok, const QAtResult& result )
{
    // Read the messages from the response.
    QString store = messageStore();
    if ( store.isEmpty() )
        store = "SM";
    extractMessages( store, result );

    // The SIM is now ready to perform SMS read operations
    // if the AT+CMGL command was successful.
    if ( ok )
        setReady( true );

    // Join together messages that are sent as a multi-part.
    joinMessages();

    // We now have cached messages.
    d->haveCached = ok;
    d->fetching = false;

    // If another message arrived while we were fetching,
    // then we need to re-fetch the entire list from scratch.
    if ( d->needRefetch ) {
        d->needRefetch = false;
        fetchMessages();
        return;
    }

    // Determine what to do with the messages we just fetched.
    if ( d->pendingCheck ) {
        d->pendingCheck = false;
        emit messageCount( d->pending.count() );
        updateUnreadList();
        updateUnreadCount();
    }
    if ( d->pendingFirstMessage ) {
        d->pendingFirstMessage = false;
        d->pendingPosn = 0;
        nextMessage();
    }
}

void QModemSMSReader::fetchMessages()
{
    // Enter fetching mode.
    d->fetching = true;

    // Clear the pending message list.
    d->pending.clear();

    // Copy the pseudo inbox into the pending list because some of
    // the messages we are looking for may have arrived via "+CMT:".
    for ( int index = 0; index < d->pseudoInbox.size(); ++index ) {
        QSMSTaggedMessage *tmsg = new QSMSTaggedMessage;
        tmsg->identifier = d->pseudoInbox[index]->identifier;
        tmsg->isUnread = d->pseudoInbox[index]->isUnread;
        d->pseudoInbox[index]->isUnread = false;   // Now it has been read.
        tmsg->message = d->pseudoInbox[index]->message;
        d->pending.append( tmsg );
    }

    // Make sure that PDU mode is enabled.  If the device was reset,
    // then it may have forgotten about PDU mode.
    d->service->primaryAtChat()->chat( "AT+CMGF=0" );

    // Select the message store and request the message list.
    QString store = messageStore();
    QString cmd = messageListCommand();
    if ( !store.isEmpty() ) {
        // Select the store and inspect the results to see if it is full.
        d->service->primaryAtChat()->chat
            ( "AT+CPMS=\"" + store + "\"",
              this, SLOT(cpmsDone(bool,QAtResult)) );
    } else {
        // We don't know the store name, but we can ask if it is full.
        d->service->primaryAtChat()->chat
            ( "AT+CPMS?", this, SLOT(cpmsDone(bool,QAtResult)) );
    }
    d->service->primaryAtChat()->chat
        ( messageListCommand(), this, SLOT(storeListDone(bool,QAtResult)) );
}

void QModemSMSReader::joinMessages()
{
    int posn;

    // Join the messages together.
    QStringList toBeDeleted;
    if ( joinMessages( d->pending, toBeDeleted ) ) {
        // We only have fragments, so temporarily disable the new
        // message check until we get something else in the queue.
        d->fragmentsOnly = true;
    }

    // Delete any datagrams that were dispatched by "joinMessages".
    for ( posn = 0; posn < (int)(toBeDeleted.count()); ++posn ) {
        deleteMessage( toBeDeleted[posn] );
    }

    // Count the number of unread messages in the combined list.
    // Add any new ones to the list of unread message identifiers.
    for ( posn = 0; posn < d->pending.count(); ++posn ) {
        if ( d->pending.at(posn)->isUnread ) {
            ++d->unreadCount;
            d->unreadList += d->pending.at(posn)->identifier;
        }
    }
}

// Get the multipart identifier information for a message.
static bool getMultipartInfo( const QSMSMessage& msg, QString& multiId,
                              uint& part, uint& numParts )
{
    QByteArray headers = msg.headers();
    int posn = 0;
    uint tag;
    int len;
    while ( ( posn + 2 ) <= headers.size() ) {
        tag = (unsigned char)(headers[posn]);
        len = (unsigned char)(headers[posn + 1]);
        if ( ( posn + len + 2 ) > headers.size() )
            break;
        if ( tag == 0 && len >= 3 ) {
            // Concatenated message with 8-bit identifiers.
            multiId = msg.sender() + "&" +
                      QString::number
                        ( (uint)(unsigned char)(headers[posn + 2]) );
            numParts = (unsigned char)(headers[posn + 3]);
            part     = (unsigned char)(headers[posn + 4]);
            if ( numParts && part && part <= numParts )
                return true;
        } else if ( tag == 8 && len >= 4 ) {
            // Concatenated message with 16-bit identifiers.
            multiId = msg.sender() + "&" +
                      QString::number
                        ( ( (uint)(unsigned char)(headers[posn + 2]) << 8 ) +
                            (uint)(unsigned char)(headers[posn + 3]) );
            numParts = (unsigned char)(headers[posn + 4]);
            part     = (unsigned char)(headers[posn + 5]);
            if ( numParts && part && part <= numParts )
                return true;
        }
        posn += 2 + len;
    }
    return false;
}

// Find multipart messages within a list and join them together.
// The "messages" list will be modified in-place with the new list.
// Returns true if there were left-over fragments that could not
// be combined into full messages.  This function will also dispatch
// datagram messages to their destinations.  The "toBeDeleted" list will
// contain the identifiers of messages to be deleted because their
// corresponding datagrams have been dispatched.
bool QModemSMSReader::joinMessages
    ( QList<QSMSTaggedMessage *>& messages, QStringList& toBeDeleted )
{
    QString multiId;
    uint part, numParts;
    QString multiId2;
    uint part2, numParts2;
    int posn, posn2;
    QList<QSMSTaggedMessage *> newList;
    QSMSTaggedMessage *tmsg;
    QList<QSMSTaggedMessage> partList;
    bool leftOvers;

    // Construct a new message list.  This could probably be made
    // more efficient, but we normally won't be processing more than
    // a handful of messages at a time.
    leftOvers = false;
    for ( posn = 0; posn < messages.count(); ++posn ) {
        if ( getMultipartInfo
                ( messages.at(posn)->message, multiId, part, numParts ) ) {

            // Collect up the rest of the parts for this message.
            partList.clear();
            partList.append( *messages.at(posn) );
            for ( posn2 = posn + 1; posn2 < messages.count(); ++posn2 ) {

                // Skip this message if it isn't part of the same multipart.
                if ( !getMultipartInfo
                        ( messages.at(posn2)->message,
                          multiId2, part2, numParts2 ) ) {
                    continue;
                } else if ( multiId != multiId2 ) {
                    continue;
                }

                // Add the part to the temporary list.
                partList.append( *messages.at(posn2) );
            }

            // Do we have all of the parts that we are interested in?
            if ( partList.count() == (int)numParts ) {
                tmsg = new QSMSTaggedMessage();
                tmsg->isUnread = false;
                tmsg->identifier = messages.at(posn)->identifier.left(3);
                tmsg->message = messages.at(posn)->message;
                tmsg->message.setText( "" );
                tmsg->message.setHeaders( QByteArray() );
                for ( part = 1; part <= numParts; ++part ) {
                    for ( posn2 = 0; posn2 < partList.count(); ++posn2 ) {
                        getMultipartInfo
                            ( partList[posn2].message,
                              multiId2, part2, numParts2 );
                        if ( part == part2 ) {
                            if ( partList[posn2].isUnread )
                                tmsg->isUnread = true;
                            tmsg->message.addParts
                                ( partList[posn2].message.parts() );
			    
			    // Remove timestamp from part identifier
			    QString id = partList[posn2].identifier.mid(3);
			    int timeIndex = id.indexOf( QChar(':') );
			    if (timeIndex != -1)
				id = id.left( timeIndex );
			    
                            if ( tmsg->identifier.length() == 3 ) {
                                tmsg->identifier += id;
                            } else {
                                tmsg->identifier += "," + id;
                            }
                        }
                    }
                }
		// Append timestamp to message identifier
		uint time_t = tmsg->message.timestamp().toTime_t();
		tmsg->identifier += ":" + QString::number( time_t, 36 );
		
                if ( dispatchDatagram( tmsg ) ) {
                    toBeDeleted.append( tmsg->identifier );
                    delete tmsg;
                } else {
                    newList.append( tmsg );
                }
            } else {
                leftOvers = true;
            }

        } else {

            // Ordinary message: copy directly to the new list.
            tmsg = new QSMSTaggedMessage();
            tmsg->identifier = messages.at(posn)->identifier;
            tmsg->isUnread = messages.at(posn)->isUnread;
            tmsg->message = messages.at(posn)->message;
            if ( dispatchDatagram( tmsg ) ) {
                toBeDeleted.append( tmsg->identifier );
                delete tmsg;
            } else {
                newList.append( tmsg );
            }

        }
    }
    messages = newList;
    return leftOvers;
}

// Dispatch an SMS message that has an application port number
// to the local application that handles that kind of service.
// If the port number is not understood, then we pass the
// message to the normal message handling program (e.g. qtmail).
bool QModemSMSReader::dispatchDatagram( QSMSTaggedMessage *m )
{
    // Check for SIM download packets that were not extracted by the modem.
    if ( m->message.protocol() == 0x3F ||        // SIM data download
         m->message.protocol() == 0x3C ) {       // ANSI-136 R-DATA
        if ( ( m->message.dataCodingScheme() & 0x03 ) == 2 ) {
            // Class 2 message intended for the SIM.
            simDownload( m->message );
            return true;
        }
    }

    // Dispatch using the core dispatch code in QTelephonyService.
    return d->service->dispatchDatagram( m->message );
}
