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

#include "dialercontrol.h"
#include "servercontactmodel.h"
#include "qtopiapowermanager.h"
#include "qabstractcallpolicymanager.h"

#include <qcategoryselector.h>
#include <qcontact.h>

#include <qlist.h>
#include <qdatastream.h>
#include <qfile.h>
#include <QSettings>
#include <QContactModel>
#include <QString>
#include <QtopiaApplication>
#include <QTimer>
#include <unistd.h>

static const int auto_answer_gap = 2000;

/*!
    \class DialerControl
    \inpublicgroup QtTelephonyModule
    \ingroup QtopiaServer::Telephony
    \brief The DialerControl class provides a convenient interface to
    the phone call manager.

    The DialerControl class make making calls and keeping track of calls
    simpler than dealing with QPhoneCallManager class directly.  It provides a
    reusable block of functionality that simplifies the \i common model of managing
    calls.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
  \fn void DialerControl::stateChanged()
  This signal is emitted when any call state changes.
*/

/*!
  \fn void DialerControl::callCreated(const QPhoneCall &call)
  This signal is emitted whenever any new call is created. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callDropped(const QPhoneCall &call)
  This signal is emitted when a call is dropped or missed. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callPutOnHold(const QPhoneCall &call)
  This signal is emitted when a call is placed on hold. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callConnected(const QPhoneCall &call)
  This signal is emitted when an outgoing call is connected. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callIncoming(const QPhoneCall &call)
  This signal is emitted when an incoming call arrives. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callDialing(const QPhoneCall &call)
  This signal is emitted when a call begins dialing. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callMissed(const QPhoneCall &call)
  This signal is emitted when a call is considered missed. \a call is
  the associated QPhoneCall object.
*/

/*!
  \fn void DialerControl::callControlRequested()
  This signal is emitted when a call control operations
  such as hold, resume and join are requested.
*/

/*!
  \fn void DialerControl::callControlSucceeded()
  This signal is emitted when a call control operation
  such as hold, resume and join are successful.
*/

/*!
  Return the single instance of DialerControl
 */
DialerControl * DialerControl::instance()
{
    static DialerControl * dialerControl = 0;
    if(!dialerControl)
        dialerControl = new DialerControl;
    return dialerControl;
}

/*!
  \internal
  Constructs a DialerControl object and attaches it to \a parent.
*/
DialerControl::DialerControl( )
    : QObject( 0 ), mCallList(50), mCallManager(0), aaTid(0),
      mProfiles(0), missedCalls(0), phoneValueSpace("/Communications/Calls"),
      activeCallCount(-1)
{
    mCallManager = new QPhoneCallManager( this );
    connect( mCallManager, SIGNAL(newCall(QPhoneCall)),
             this, SLOT(newCall(QPhoneCall)) );

    connect( this, SIGNAL(callIncoming(QPhoneCall)),
            this, SLOT(cacheCall(QPhoneCall)) );
    connect( this, SIGNAL(callDropped(QPhoneCall)),
                                        this, SLOT(recordCall(QPhoneCall)) );
    mProfiles = new QPhoneProfileManager(this);

    QSettings setting("Trolltech", "qpe");
    setting.beginGroup("CallControl");
    missedCalls = setting.value("MissedCalls", 0).toInt();
    phoneValueSpace.setAttribute("MissedCalls", QVariant(missedCalls));
    doActiveCalls();
    QTimer::singleShot( 0, this, SLOT(readCachedCall()) );
}

/*! \internal */
DialerControl::~DialerControl()
{
}


QList<QPhoneCall> DialerControl::findCalls( QPhoneCall::State st ) const
{
    QList<QPhoneCall> results;
    QList<QPhoneCall> calls = allCalls();
    QList<QPhoneCall>::ConstIterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ( (*iter).state() == st ) {
            results += (*iter);
        }
    }
    return results;
}

QPhoneCall DialerControl::active() const
{
    QList<QPhoneCall> localCalls = allCalls();
    QList<QPhoneCall>::ConstIterator iter;
    for ( iter = localCalls.begin(); iter != localCalls.end(); ++iter ) {
        if ( (*iter).state() == QPhoneCall::Connected ||
             (*iter).state() == QPhoneCall::Dialing ||
             (*iter).state() == QPhoneCall::Alerting ) {
            return *iter;
        }
    }
    return QPhoneCall();
}

/*!
  Returns the list of all calls in the system.
*/
QList<QPhoneCall> DialerControl::allCalls() const
{
    // supported call types by dialer control
    QList<QAbstractCallPolicyManager *> managers;
    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    QStringList supportedCallType;
    foreach (QAbstractCallPolicyManager *manager, managers)
        supportedCallType.append( manager->callType() );

    // only return calls with supported type
    QList<QPhoneCall> supportedCalls;
    QList<QPhoneCall> allCalls = mCallManager->calls();
    foreach ( QPhoneCall call, allCalls )
        if ( supportedCallType.contains( call.callType() ) )
            supportedCalls.append( call );

    return supportedCalls;
}

/*!
  Returns the list of calls in the active state (i.e. not on hold).
*/
QList<QPhoneCall> DialerControl::activeCalls() const
{
    QList<QPhoneCall> ac;
    QList<QPhoneCall> calls = allCalls();
    QList<QPhoneCall>::ConstIterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ( (*iter).state() == QPhoneCall::Connected) {
            ac += (*iter);
        }
    }
    return ac;
}

/*!
  Returns the list of calls on hold.
*/
QList<QPhoneCall> DialerControl::callsOnHold() const
{
    return findCalls( QPhoneCall::Hold );
}

/*!
  Returns true if there are any calls active or on hold.
*/
bool DialerControl::isConnected() const
{
    return hasActiveCalls() || hasCallsOnHold();
}

/*!
  Returns true if there are any active calls (i.e. calls not on hold).
*/
bool DialerControl::hasActiveCalls() const
{
    //optimized operation
    QList<QPhoneCall> calls = allCalls();
    for( QList<QPhoneCall>::ConstIterator it = calls.begin() ; it != calls.end() ;
                                                                                ++it )
        if( (*it).state() == QPhoneCall::Connected )
            return true;
    return false;
}

/*!
  Returns true if there are any calls on hold.
*/
bool DialerControl::hasCallsOnHold() const
{
    //optimized operation
    QList<QPhoneCall> calls = allCalls();
    for( QList<QPhoneCall>::ConstIterator it = calls.begin() ; it != calls.end() ;
                                                                                ++it )
        if( (*it).state() == QPhoneCall::Hold )
            return true;
    return false;
}

/*!
  Returns the number of active calls (i.e. calls not on hold).
*/
uint DialerControl::activeCallsCount() const
{
    return activeCalls().count();
}

/*!
  Returns the number of calls on hold.
*/
uint DialerControl::callsOnHoldCount() const
{
    return callsOnHold().count();
}

/*!
  \fn void DialerControl::activeCount(int activeCalls)

  Emitted whenever the number of \a activeCalls changes.
 */

/*!
  \fn void DialerControl::missedCount(int missedCalls)

  Emitted whenever the number of \a missedCalls changes.
 */

/*!
  Returns true if there is an incoming call (i.e. not yet answered).
*/
bool DialerControl::hasIncomingCall() const
{
    return ( findCalls( QPhoneCall::Incoming ).count() != 0 );
}

/*!
  Returns true if there is an outgoing call currently dialing, but not
  yet connected.
*/
bool DialerControl::isDialing() const
{
    return active().dialing();
}

/*!
  Returns the incoming call.  If there is no incoming call, a null QPhoneCall
  is returned.
*/
QPhoneCall DialerControl::incomingCall() const
{
    QList<QPhoneCall> ic = findCalls( QPhoneCall::Incoming );
    if( ic.count() )
        return ic.first();
    return QPhoneCall();
}

/*! \internal */
QPhoneCall DialerControl::createCall( const QString& callType )
{
    QPhoneCall c = mCallManager->create(callType);
    c.connectStateChanged( this, SLOT(callStateChanged(QPhoneCall)) );
    c.connectPendingTonesChanged( this, SIGNAL(pendingTonesChanged(QPhoneCall)) );
    c.connectRequestFailed( this, SIGNAL(requestFailed(QPhoneCall,QPhoneCall::Request)) );
    return c;
}

/*!
  Dials a \a number if there are no currently active calls. If a
  \a contact is specified, it is used as the contact to display and
  insert into the call history. Before calling this function,
  you should check that there are no incoming calls, or active calls.
  If \a sendcallerid is true, then send the caller's identifier as part
  of the dial sequence.

  If \a callType is "Voice" a GSM call will be dialed.  If \a callType is
  "VoIP" a VoIP call will be dialed.
*/
void DialerControl::dial( const QString &number, bool sendcallerid, const QString& callType, const QUniqueId &contact )
{

    QUniqueId matchedContact;
    if (contact.isNull() && !number.isEmpty())
        matchedContact = ServerContactModel::instance()->matchPhoneNumber(number).uid();
    else
        matchedContact = contact;

    if ( isDialing() ) {
        //qWarning("BUG! Attempt to dial while there is already a dialing call");
    }
    if( !hasActiveCalls() && !isDialing() )
    {
        // Collect up the dial options.
        QDialOptions dialopts;
        dialopts.setNumber( number );
        if ( sendcallerid )
            dialopts.setCallerId( QDialOptions::SendCallerId );
        else
            dialopts.setCallerId( QDialOptions::DefaultCallerId );
        dialopts.setContact( matchedContact );

        // Allow other parts of the server (e.g. GsmKeyActions) to
        // modify the dial options to account for supplementary services.
        bool handledAlready = false;
        emit modifyDial( dialopts, handledAlready );
        if ( handledAlready )
            return;

        // Call the specified number.
        QPhoneCall call = createCall(callType);
        phoneValueSpace.setAttribute( "LastDialedCall", QVariant(number) );
        call.dial( dialopts );

        // cache call here to preserve the information even if the battery run out.
        cacheCall( call );
    }
}

/*!
  Ends the current call(s).  The current call(s) is searched for in this order:

  1. Currently dialing call
  2. Active calls
  3. Incoming call
  4. Calls on Hold
*/
void DialerControl::endCall()
{
    QList<QPhoneCall> calls = activeCalls();
    if ( active().dialing() ) {
        active().hangup();
    } else if ( calls.count() ) {
        QList<QPhoneCall>::Iterator iter;
        for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
            if ((*iter).state() == QPhoneCall::Connected ||
                (*iter).state() == QPhoneCall::Dialing ||
                (*iter).state() == QPhoneCall::Alerting) {
                (*iter).hangup();
            }
        }
    } else if ( hasIncomingCall() ) {
        incomingCall().hangup();
    } else if ( hasCallsOnHold() ) {
        calls = callsOnHold();
        QList<QPhoneCall>::Iterator iter;
        for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
            if ((*iter).state() == QPhoneCall::Hold)
                (*iter).hangup();
        }
    }
}

/*!
    End a specific call with the modem identifier \a id.
    This will typically only work with GSM calls that set the
    QPhoneCall::modemIdentifier() value.  It is used to implement
    the GSM \c{1x SEND} key sequence where \c{x} is the identifier.
*/
void DialerControl::endCall(int id)
{
    QPhoneCall call = callManager().fromModemIdentifier(id);
    if ( !call.isNull() )
        call.hangup(QPhoneCall::CallOnly);
}

/*!
    Activate a specific call with the modem identifier \a id.
    This will typically only work with GSM calls that set the
    QPhoneCall::modemIdentifier() value.  It is used to implement
    the GSM \c{2x SEND} key sequence where \c{x} is the identifier.
*/
void DialerControl::activateCall( int id )
{
    QPhoneCall call = callManager().fromModemIdentifier(id);
    if ( !call.isNull() )
        call.activate(QPhoneCall::CallOnly);
}

/*!
  Ends all calls, whether dialing, active, incoming, or on hold.
*/
void DialerControl::endAllCalls()
{
    QList<QPhoneCall> calls = allCalls();
    QList<QPhoneCall>::Iterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        (*iter).hangup();
    }
}

/*!
    End the held calls.
*/
void DialerControl::endHeldCalls()
{
    // Individually hang up all of the held calls.
    QList<QPhoneCall> calls = callsOnHold();
    QList<QPhoneCall>::Iterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ((*iter).state() == QPhoneCall::Hold)
            (*iter).hangup(QPhoneCall::CallOnly);
    }
}

/*!
  Accepts an incoming call.  Has no effect if there is no incoming call.
*/
void DialerControl::accept()
{
    QList<QPhoneCall> incomingCalls = findCalls( QPhoneCall::Incoming );
    if( incomingCalls.count() )
    {
        QPhoneCall incoming = incomingCalls.first();
        if ( incoming.incoming() ) {
            //put active calls on hold
            if ( incoming.callType() == "Voice" ) {   // No tr
                // GSM calls will be put on hold automatically by the accept().
                // Other call types need to be put on hold explicitly.
                if( hasActiveCalls() ) {
                    if ( activeCalls().first().callType() == "Voice" ) {  // No tr
                        incoming.accept();
                    } else {
                        hold();
                        incoming.accept();
                    }
                } else {
                    incoming.accept();
                }
            } else {
                if( hasActiveCalls() )
                    hold();
                incoming.accept();
                incoming.activate();
            }
        }
    }
}

/*!
  Places the active calls on hold.  If there were calls already on hold,
  they will become active.
*/
void DialerControl::hold()
{
    if ( hasActiveCalls() ) {
        emit callControlRequested();
        active().connectStateChanged( this, SLOT(checkHoldState(QPhoneCall)) );
        active().hold();
    }
}

/*!
    \internal
    Checks if the request to hold the \a call was successful.
*/
void DialerControl::checkHoldState( const QPhoneCall &call )
{
    if ( call.state() == QPhoneCall::Hold )
        emit callControlSucceeded();
    call.disconnectStateChanged( this, SLOT(checkHoldState(QPhoneCall)) );
}

/*!
  Makes calls on hold active.
*/
void DialerControl::unhold()
{
    QList<QPhoneCall> hc = findCalls( QPhoneCall::Hold );
    if( hc.count() ) {
        emit callControlRequested();
        hc.first().connectStateChanged( this, SLOT(checkConnectedState(QPhoneCall)) );
        hc.first().activate();
    }
}

/*!
  Joins the active calls and calls on hold to form a multiparty call.
*/
void DialerControl::join()
{
    QList<QPhoneCall> coh = callsOnHold();
    if ( hasActiveCalls() && coh.count() ) {
        emit callControlRequested();
        callsOnHold().first().connectStateChanged( this, SLOT(checkConnectedState(QPhoneCall)) );
        callsOnHold().first().join();
    }
}

/*!
    \internal
    Checks if the request to connect the \a call was successful.
*/
void DialerControl::checkConnectedState( const QPhoneCall &call )
{
    if ( call.state() == QPhoneCall::Connected )
        emit callControlSucceeded();
    call.disconnectStateChanged( this, SLOT(checkConnectedState(QPhoneCall)) );
}

/*!
  Joins the active calls and calls on hold to form a multiparty call,
  and then detaches the local user.
*/
void DialerControl::transfer()
{
    QList<QPhoneCall> coh = callsOnHold();
    if (  hasActiveCalls() && coh.count() ) {
        coh.first().join( true );
    }
}

/*!
    Deflect the incoming call to a new \a number.
*/
void DialerControl::deflect( const QString& number )
{
    if ( hasIncomingCall() )
    {
        incomingCall().transfer( number );
    }
}

/*!
  Sends a busy signal to an incoming call.
*/
void DialerControl::sendBusy()
{
    if ( hasIncomingCall() )
    {
        incomingCall().hangup();
    }
}

/*!
  \fn void DialerControl::pendingTonesChanged( const QPhoneCall &call )

  Emitted whenever the pending tones for the \a call changes.
  */

/*!
  \fn void DialerControl::requestFailed(const QPhoneCall &call,QPhoneCall::Request request)

  Emitted if the \a request on \a call fails.
 */

void DialerControl::newCall( const QPhoneCall& call )
{
    QPhoneCall c = call;
    c.connectStateChanged( this, SLOT(callStateChanged(QPhoneCall)) );
    c.connectPendingTonesChanged( this, SIGNAL(pendingTonesChanged(QPhoneCall)) );
    emit callCreated( c );
    doActiveCalls();
    callStateChanged( call ); //force an update
}

/*!
  Returns the current list of calls.
*/
QCallList &DialerControl::callList()
{
    return mCallList;
}

/*!
  \fn void DialerControl::autoAnswerCall()

  Emitted whenever auto answer is enabled and the auto answer time has elapsed.
 */

/*!
  \fn void DialerControl::modifyDial( QDialOptions& options, bool& handledAlready )

  Emitted whenever a dial is about to be performed with the information
  in \a options.  This gives other parts of the server (e.g. GsmKeyActions)
  the ability to modify the request if it contains supplementary service
  information.  The \a handledAlready parameter is set to false prior to
  emitting the signal.  If a slot sets this to true, then DialerControl
  will assume that the request was already handled and the normal dial
  should not be performed.
*/

/*! \internal */
void DialerControl::timerEvent(QTimerEvent *)
{
    Q_ASSERT(aaTid);
    killTimer(aaTid);
    aaTid = 0;
    Q_ASSERT(hasIncomingCall());
    emit autoAnswerCall();
}

void DialerControl::callStateChanged( const QPhoneCall& call )
{
    // Set value space appropriately
    // XXX Optimize for redundancy!
    if(hasIncomingCall()) {
        QPhoneCall icall = incomingCall();
        QString number = icall.number();
        QString name;
        QUniqueId contact = icall.contact();
        QContactModel *m = ServerContactModel::instance();
        if(!contact.isNull()) {
            QContact cnt = m->contact(contact);
            if (!cnt.uid().isNull())
                name = cnt.label();
        } else if(!number.isEmpty()) {
            QContact cnt = m->matchPhoneNumber(number);
            if (!cnt.uid().isNull())
                name = cnt.label();
        } else {
            number = tr("Unknown", "Unknown caller");
        }
        phoneValueSpace.setAttribute("Incoming/Number", QVariant(number.trimmed()));
        phoneValueSpace.setAttribute("Incoming/Name", QVariant(name));

        if(!aaTid && mProfiles->activeProfile().autoAnswer())
            aaTid = startTimer(auto_answer_gap);
    } else {
        if(aaTid)
            killTimer(aaTid);
        phoneValueSpace.removeAttribute("Incoming");
    }

    // emit useful signals
    if( call.state() == QPhoneCall::Connected )
    {
        emit callConnected( call );
        // update cached call info.
        updateCachedCall( call );
    }
    else if( call.state() == QPhoneCall::Hold )
    {
        emit callPutOnHold( call );
    }
    else if( call.dialing() )
    {
        emit callDialing( call );
    }
    else if( call.incoming() )
    {
        // Turn off screen saver so the incoming call will be visible.
        QtopiaPowerManager::setActive(false);

        emit callIncoming( call );
    }
    else if ( call.dropped()  )
    {
        emit callDropped( call );
    }
    doActiveCalls();

    // Disable screen saver if in a call
    if (hasIncomingCall() || hasActiveCalls() || hasCallsOnHold())
        QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableLightOff);
    else
        QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    emit stateChanged();
}

/*!
  Record the \a call in the call list.
 */
void DialerControl::recordCall( const QPhoneCall &call )
{
    QCallListItem::CallType ct;
    if( call.dialed() )
        ct = QCallListItem::Dialed;
    else if( call.hasBeenConnected() )
        ct = QCallListItem::Received;
    else {
        ct = QCallListItem::Missed;
        // increase missed call count
        missedCall( call );
    }

    // QPhoneCall::connectTime() in case call has been connected
    // QPhoneCall::startTime() in other cases
    QDateTime startTime = call.hasBeenConnected() ? call.connectTime() : call.startTime();

    QCallListItem listItem( ct, call.fullNumber(),
            startTime, call.endTime(), call.contact(), call.callType() );
    mCallList.record( listItem );

    removeCachedCall( call );
}

/*! \internal */
void DialerControl::cacheCall( const QPhoneCall &call )
{
    // cache call info to perserve data when battery runs out
    QSettings setting( "Trolltech", "qpe" );
    setting.beginGroup( "CallControl" );
    setting.beginGroup( call.identifier() );
    setting.setValue( "CallType", call.dialing() ? QCallListItem::Dialed : QCallListItem::Missed );
    setting.setValue( "FullNumber", call.fullNumber() );
    setting.setValue( "StartTime", call.startTime() );
    setting.setValue( "EndTime", QDateTime() );
    setting.setValue( "Contact", call.contact().toString() );
    setting.setValue( "ServiceType", call.callType() );
    setting.endGroup();
    setting.endGroup();
    setting.sync();
    // make sure the data is written to disk
    ::sync();
}

/*! \internal */
void DialerControl::removeCachedCall( const QPhoneCall &call )
{
    QSettings setting( "Trolltech", "qpe" );
    setting.beginGroup( "CallControl" );
    setting.remove( call.identifier() );
    setting.sync();
    ::sync();
}

/*! \internal */
void DialerControl::updateCachedCall( const QPhoneCall &call )
{
    QSettings setting( "Trolltech", "qpe" );
    setting.beginGroup( "CallControl" );
    setting.beginGroup( call.identifier() );
    setting.setValue( "CallType", call.dialed() ? QCallListItem::Dialed : QCallListItem::Received );
    setting.setValue( "EndTime", QDateTime::currentDateTime() );
    setting.endGroup();
    setting.endGroup();
    setting.sync();
    ::sync();
}

/*! \internal */
void DialerControl::readCachedCall()
{
    QSettings setting( "Trolltech", "qpe" );
    setting.beginGroup( "CallControl" );
    QStringList groupList = setting.childGroups();
    foreach ( QString group, groupList ) {
        setting.beginGroup( group );
        QCallListItem::CallType ct = (QCallListItem::CallType)setting.value( "CallType" ).toInt();
        if ( ct == QCallListItem::Missed )
            missedCalls++;
        QString fullNumber = setting.value( "FullNumber" ).toString();
        QDateTime startTime = setting.value( "StartTime" ).toDateTime();
        QDateTime endTime = setting.value( "EndTime" ).toDateTime();
        QUniqueId contact = QUniqueId( setting.value( "Contact" ).toString() );
        QString serviceType = setting.value( "ServiceType" ).toString();

        QCallListItem item( ct, fullNumber, startTime, endTime, contact, serviceType );
        mCallList.record( item );
        setting.endGroup();
        setting.remove( group );
    }

    if ( missedCalls ) {
        setting.setValue( "MissedCalls", missedCalls );
        phoneValueSpace.setAttribute("MissedCalls", missedCalls);
        emit missedCount(missedCalls);
    }
    setting.sync();
    ::sync();
}

/*!  \internal */
QPhoneCallManager &DialerControl::callManager() const
{
    return *mCallManager;
}

/*!
  Reset the missed call count to 0.
  */
void DialerControl::resetMissedCalls()
{
    if( missedCalls )
    {
        missedCalls = 0;

        QSettings setting("Trolltech", "qpe");
        setting.beginGroup( "CallControl" );
        setting.setValue( "MissedCalls", missedCalls );

        phoneValueSpace.setAttribute("MissedCalls", QVariant(missedCalls));
        emit missedCount( missedCalls );
    }
}

void DialerControl::missedCall(const QPhoneCall &call)
{
    missedCalls++;

    QSettings setting("Trolltech", "qpe");
    setting.beginGroup( "CallControl" );
    setting.setValue( "MissedCalls", missedCalls );

    emit callMissed( call );
    phoneValueSpace.setAttribute("MissedCalls", missedCalls);
    emit missedCount(missedCalls);
}

/*!
  Returns the number of missed calls.
  */
int DialerControl::missedCallCount() const
{
    return missedCalls;
}

void DialerControl::doActiveCalls()
{
    QStringList activeList;
    QList<QPhoneCall> allActive = activeCalls();
    QList<QPhoneCall>::Iterator iter;

    int newActive = allActive.count();

    for ( iter = allActive.begin(); iter != allActive.end(); ++iter ) {
        activeList << (*iter).number().trimmed();
    }

    phoneValueSpace.setAttribute("ActiveCallList", activeList);

    if(newActive != activeCallCount) {
        activeCallCount = newActive;
        phoneValueSpace.setAttribute("ActiveCalls", activeCallCount);
        emit activeCount(activeCallCount);
    }
}

/*!
  Returns true if the user wants to send callerID information
  to the number \a n.
*/
bool DialerControl::callerIdNeeded(const QString &n) const
{
    QSettings cfg("Trolltech","Phone");
    cfg.beginGroup("CallerId");
    int ch = cfg.value("choice",0).toInt();
    // ch 0 = QCallSettings::Subscription
    // ch 1 = QCallSettings::Invoked
    // ch 2 = QCallSettings::Suppressed
    // ch 4 = to my contacts
    if ( ch == 0 || ch == 1 ) return false;
    if ( ch == 2 ) return true;
    QContact cnt = ServerContactModel::instance()->matchPhoneNumber(n);
    if (cnt.uid().isNull())
        return false; //no such contact -> don't send callerId

    QCategoryFilter catFilter;
    catFilter.readConfig(cfg, "category");
    if (catFilter.acceptAll())
        return true;

    QList<QString> cats = cnt.categories();
    return catFilter.accepted(cats);
}
