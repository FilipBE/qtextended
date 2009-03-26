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

#include "dialproxy.h"

#include "dialercontrol.h"
#include "qabstractmessagebox.h"
#include "qabstractcallpolicymanager.h"
#include "servercontactmodel.h"
#include "uifactory.h"
#include "windowmanagement.h"

#include "abstractaudiohandler.h"

#include <QtopiaServiceHistoryModel>
#include <QtopiaServiceRequest>
#include <QValueSpace>

static const int WarningTimeout = 5000;  // e.g. Cannot call


/*!
    \class DialProxy
    \inpublicgroup QtTelephonyModule
    \brief The DialProxy class provides generic dial string processing and enables
    the routing of dialtone and call audio.
    \ingroup QtopiaServer::Telephony

    The DialProxy implements the back-end of the DialerService. It is responsible for
    the processing of dial strings, dial related key events such as the handset hook,
    the speaker and headset button as well as the accept and hangup operations. User interface
    elements request dial operations via requestDial() and connect to the various signals
    to receive notifications related to dial requests.

    A dial operation can be initiated via DialerService's dial() functions or via requestDial()
    which can be called by user interface elements of the Qt Extended server. This class
    utilizes the DialerControl class to connect to the Telephony backend. However it provides
    a much simpler API for the most common dial operations. If specific updates about the progress of
    calls are required the DialerControl should be used instead.

    The various dial functions can process normal phone numbers as well as VoIP numbers/identifiers.
    To determine what call type is required QAbstractCallPolicyManager::handling() is used.
    If more than one call type is possible the CallTypeSelector dialog is shown and the user has
    to select the call type.

    The various implementations of the QAbstractDialerScreen and QAbstractCallScreen are the
    most likely user interface elements that would be connected to this class. For this purpose
    this class provides various signals which indicate when the dialer
    or callscreen should be shown. This is particularly important if the dial is initiated
    via the dialer service rather than via the user interface (in which case the dialer screen
    is already shown on the screen and now it is just a matter of processing the dial string).

    Most desk phones have hardware buttons which allow the user to change between a handset,
    the speaker or the headset. In addition to the obvious audio routing features each of these
    buttons may also require some interfaction with the dialer. Lifting the phone
    craddle should e.g. show the dialer and play dialtones for each pressed key on the
    hard/soft-keypad. After a certain time out (while no further digit was entered) the phone
    starts dialing and enables call audio. This behavior is
    different from a situation whereby the user requested the dialer screen to be shown (w/o
    lifting the handset), dials a number (which plays dial tones) and then explicitly presses the
    dial button to initiate the call. In this situation the phone would not automatically start
    the dial process until it was told to do so.

    The hook gesture signals onHookGesture() and offHookGesture() help to identify whether the
    current dial process requires an explicit press on the dial button or not. Both signals
    should be connected to the dialer. When onHookGesture() is emitted the dialer should
    initiate the dial process as soon as no dial button has been pressed for a given time out period
    or until the offHookGesture() signal has been received.

    This class is a Qt Extended server task and cannot be used by other Qt Extended applications.

    \sa DialerService, DialerControl, CallTypeSelector
*/

/*!
    \fn void DialProxy::doShowDialer(const QString& number)

    Emitted when the dialer should be shown. This usually happens as a result of
    DialerService::showDialer() service request or if a speaker, handset or headset action
    was performed. The dialer should be prepopulated with the given \a number.
*/

/*!
    \fn void DialProxy::showCallScreen()

    Emitted when the dial screen should be shown to inform the user about an ongoing dial
    process.
*/

/*!
    \fn void DialProxy::resetScreen()

    Emitted when the last call was stopped. User interfaces connecting to this signal
    would usually change to the idle/home screen.
*/

/*!
    \fn void DialProxy::onHookGesture()

    Emitted when the off hook period stops. During an off hook period the
    dialer immediately starts the dial process if no further digit has been entered for
    a given time period.

    \sa offHookGesture()
*/


/*!
    \fn void DialProxy::offHookGesture()

    Emitted when the off hook period starts. During an off hook period the
    dialer immediately starts the dial process if no further digit has been entered for
    a given time period.

    \sa onHookGesture()
*/


/*!
    Creates a new DialProxy instance with the given \a parent.
*/
DialProxy::DialProxy(QObject *parent)
    : DialerService(parent), waitingVoiceMailNumber(false),
    noVoiceMailNumberMsgBox(0), warningMsgBox(0), voipNoPresenceMsgBox(0), dialingMsgBox(0),
    queuedIncoming(false), incomingMsgBox(0), m_dialtoneAudio(0), m_callAudio(0)
{
    // Voice mail
    serviceNumbers = new QServiceNumbers( QString(), this );
    connect( serviceNumbers, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
            this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );

    connect(DialerControl::instance(), SIGNAL(stateChanged()),
            this, SLOT(stateChanged()));
    connect(DialerControl::instance(), SIGNAL(autoAnswerCall()),
            this, SLOT(acceptIncoming()));

    m_callScreenTriggers = (CallDialing | CallIncoming | CallAccepted | CallDropped);

    m_dialtoneAudio = AbstractAudioHandler::audioHandler("DialtoneAudio");
    if (!m_dialtoneAudio)
        qLog(Component) << "DialProxy: No dialtone audio routing available";

    m_callAudio = AbstractAudioHandler::audioHandler("CallAudio");
    if (!m_callAudio)
        qLog(Component) << "DialProxy: No call audio routing available";
}

/*!
    Destroys the Dialproxy instance.
*/
DialProxy::~DialProxy()
{
    if (warningMsgBox) delete warningMsgBox;
    if (noVoiceMailNumberMsgBox) delete noVoiceMailNumberMsgBox;
    if (dialingMsgBox) delete dialingMsgBox;
    if (voipNoPresenceMsgBox) delete voipNoPresenceMsgBox;
    if (incomingMsgBox) delete incomingMsgBox;

}

/*!
    \enum DialProxy::CallScreenTrigger
    This enum defines the call events that should trigger the call screen to be displayed.

    \value CallDialing Display the call screen when a dialing call is seen.
    \value CallIncoming Display the call screen when an incoming call is seen.
    \value CallAccepted Display the call screen when an incoming call is accepted.
    \value CallDropped Display the call screen when a call is dropped.
*/

/*!
    Sets the call screen trigger conditions to \a triggers.

    \sa callScreenTriggers()
*/
void DialProxy::setCallScreenTriggers(CallScreenTriggers triggers)
{
    m_callScreenTriggers = triggers;
}

/*!
    Returns the call screen trigger conditions.

    \sa setCallScreenTriggers()
*/
DialProxy::CallScreenTriggers DialProxy::callScreenTriggers() const
{
    return m_callScreenTriggers;
}

/*!
    \reimp
*/
void DialProxy::dialVoiceMail()
{
    waitingVoiceMailNumber = true;
    if ( !serviceNumbers->available() ) {
        // The server part of QServiceNumbers may not have been available
        // when the constructor was called.  Recreate and try again.
        delete serviceNumbers;
        serviceNumbers = new QServiceNumbers( QString(), this );
        connect(serviceNumbers, SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
                this, SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );
    }
    if ( serviceNumbers->available() )
        serviceNumbers->requestServiceNumber( QServiceNumbers::VoiceMail );
    else
        serviceNumber( QServiceNumbers::VoiceMail, QString() );
}


/*!
    \internal
*/
void DialProxy::serviceNumber(QServiceNumbers::NumberId id, const QString& number)
{
    if (id == QServiceNumbers::VoiceMail) {
        // The "waitingVoiceMailNumber" flag is used to prevent
        // false positives when the "phonesettings" program
        // queries for the voice mail number.
        if (waitingVoiceMailNumber) {
            waitingVoiceMailNumber = false;
            if (number.length() > 0) {
                requestDial( number );
            } else {
                if (!noVoiceMailNumberMsgBox) {
                    noVoiceMailNumberMsgBox = QAbstractMessageBox::messageBox(0/*TODO:this*/, tr("Voice Mail"),
                        tr("<qt>Voice mail number is not set. Do you wish to set the number now?</qt>"),
                        QAbstractMessageBox::Question, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                    connect(noVoiceMailNumberMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
                }
                QtopiaApplication::showDialog(noVoiceMailNumberMsgBox);
            }
        }
    }
}

/*!
    \reimp
*/
void DialProxy::dial(const QString& /*name*/, const QString& number)
{
    requestDial(number);
}

/*!
    \reimp
*/
void DialProxy::dial(const QString& number, const QUniqueId& contact)
{
    requestDial(number, contact);
}

/*!
    Dials the given \a number which is associated to \a contact. If \a contact
    is default constrcuted the dialing process searches for contact details
    and displays the result if there is a known contact with the given \a number.

    \a number can be any type of number as long as it can be interpreted
    by QAbstractCallPolicyManager::handling().

    This function does nothing if the \a number is empty.

    \sa QAbstractCallPolicyManager
*/
void DialProxy::requestDial(const QString &number, const QUniqueId &contact)
{
    if (number.isEmpty())
        return;

    int numberAtSymbol = number.count('@');
    if (numberAtSymbol > 1) {
        showWarning(tr("Incorrect Number format"),
                tr("<qt>Unable to make a phone call.</qt>"));
        return;
    }

    // Ask all of the call policy managers what they want to do
    // with this phone number.
    QAbstractCallPolicyManager::CallHandling handling;
    QList<QAbstractCallPolicyManager *> managers;
    QHash<QString,QAbstractCallPolicyManager *> candidates;
    QAbstractCallPolicyManager *chosenManager = 0;
    managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        handling = manager->handling(number);
        if ( handling == QAbstractCallPolicyManager::MustHandle ) {
            chosenManager = manager;
            break;
        } else if ( handling == QAbstractCallPolicyManager::NeverHandle ) {
            chosenManager = 0;
            candidates.clear();
            break;
        } else if ( handling == QAbstractCallPolicyManager::CanHandle ) {
            candidates[manager->callType()] = manager;
        }
    }

    // Bail out if nothing can dial this number at this time.
    if (!chosenManager && candidates.isEmpty()) {
        showWarning(tr("No GSM/VoIP Network"),
                tr("<qt>No phone call is possible.</qt>"));
        return;
    }

    queuedCall = number;
    queuedCallType = QString();
    queuedCallContact = contact;

    // Determine which call policy manager to use.
    if (!chosenManager) {
        if (candidates.size() == 1) {
            // Only one call policy manager is active, so use that.
            chosenManager = candidates[candidates.keys().at(0)];
        } else {
            // Clear queued call information while the dialog is up
            // to prevent the call from being automatically dialed
            // if we get to stateChanged() while
            // the dialog is on-screen.
            queuedCall = QString();
            queuedCallType = QString();
            queuedCallContact = QUniqueId();

            // More than one is active, so we have to ask the user.
            QDialog* selector = UIFactory::createDialog("CallTypeSelector", 0 );
            if ( !selector ) {
                qLog(Component) << "PhoneLauncher: CallTypeSelector component not available. Aborting call.";
                return;
            }
            QMetaObject::invokeMethod( selector, "setAvailablePolicyManagers",
                    Qt::DirectConnection, Q_ARG(QStringList, candidates.keys() ) );
            if (QtopiaApplication::execDialog(selector) != QDialog::Accepted) {
                return;
            }
            QString selected;
            QMetaObject::invokeMethod( selector, "selectedPolicyManager",
                    Qt::DirectConnection, Q_RETURN_ARG(QString, selected) );

            delete selector;
            if ( !selected.isEmpty() )
                chosenManager = candidates[selected];
            if ( !chosenManager ) {
                // Shouldn't happen, but recover gracefully anyway.
                return;
            }
         
            // Re-instate the queued call information.
            queuedCall = number;
            queuedCallType = QString();
            queuedCallContact = contact;
        }
    }

    // Get the chosen call type.
    queuedCallType = chosenManager->callType();

    // Ask the user to confirm if the called party is not present.
    if (!chosenManager->isAvailable(queuedCall)) {
        if (!voipNoPresenceMsgBox) {
            voipNoPresenceMsgBox = QAbstractMessageBox::messageBox(0, tr("Unavailable"),
                tr("<qt>The selected contact appears to be unavailable. Do you still wish to make a call?</qt>"),
                QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
            connect(voipNoPresenceMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(voipNoPresenceMsgBox);
        return;
    }

    // Direct the audio to the speaker
    // if dialing requested without off-hook guesture
    // e.g. pressing on screen call button

    if (Qtopia::hasKey( Qtopia::Key_Hook ) && m_dialtoneAudio
            && m_dialtoneAudio->audioProfile().isEmpty())
        processAudioKey( Qtopia::Key_Speaker, true );

    // Dial the specified number.
    dialNumber(queuedCall, queuedCallContact, queuedCallType);
}

/*!
    Dials number \a n.  The current call state is first checked to confirm
    that it is possible to make a call.  If it is not possible to make a
    call, the user will be notified, otherwise the call screen will be shown
    and the call initiated.
    \internal
*/
void DialProxy::dialNumber(const QString &n, const QUniqueId &cont, const QString &callType)
{
    if (callType.isEmpty() || n.isEmpty())
        return;

    // Save service request history.
    QtopiaServiceRequest req("Dialer", "dial(QString,QUniqueId)");
    req << n << cont;
    QString label;
    QString icon("phone/phone"); // no tr
    if (!cont.isNull()) {
        QContactModel *m = ServerContactModel::instance();
        QContact contact = m->contact(cont);
        label = tr("Call %1").arg(contact.label());
        QMap<QContact::PhoneType, QString> numbers = contact.phoneNumbers();
        QMap<QContact::PhoneType, QString>::iterator it;
        for (it = numbers.begin(); it != numbers.end(); ++it) {
            if (*it == n) {
                icon = contact.phoneIconResource(it.key());
                break;
            }
        }
    } else {
        label = tr("Call %1").arg(n);
    }
    QtopiaServiceHistoryModel::insert(req, label, icon);

    if (m_callScreenTriggers & CallDialing)
        emit showCallScreen();

    if (DialerControl::instance()->hasActiveCalls() && DialerControl::instance()->hasCallsOnHold()) {
        if (!dialingMsgBox) {
            dialingMsgBox = QAbstractMessageBox::messageBox(
                    0,
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before begining the new call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
            connect(dialingMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
        }
        QtopiaApplication::showDialog(dialingMsgBox);
    } else if (DialerControl::instance()->hasActiveCalls()) {
        DialerControl::instance()->hold();
    } else {
        DialerControl::instance()->dial( n, DialerControl::instance()->callerIdNeeded(n), callType, cont );
    }
}


/*!
    \reimp
*/
void DialProxy::showDialer( const QString& digits )
{
    if( !DialerControl::instance()->hasIncomingCall() &&
        !DialerControl::instance()->isDialing() ) {
        emit doShowDialer(digits);
    }
}

/*!
    \reimp
*/
void DialProxy::onHook()
{
    processAudioKey(Qtopia::Key_Hook, true);
}

/*!
    \reimp
*/
void DialProxy::offHook()
{
    processAudioKey(Qtopia::Key_Hook, false);
}

/*!
    \reimp
*/
void DialProxy::headset()
{
    processAudioKey(Qtopia::Key_Headset, false);
}

/*!
    \reimp
*/
void DialProxy::speaker()
{
    processAudioKey(Qtopia::Key_Speaker, false);
}

/*!
    \reimp
*/
void DialProxy::setDialToneOnlyHint( const QString &app )
{
    if ( !dialToneOnlyHintApps.contains( app ) )
        dialToneOnlyHintApps << app;
}

/*!
    \reimp
*/
void DialProxy::redial()
{
    QValueSpaceItem item( "/Communications/Calls" );
    QString number = item.value( "LastDialedCall" ).toString();
    if ( !number.isEmpty() )
        showDialer( number );
}

void DialProxy::showWarning(const QString &title, const QString &text)
{
    if (!warningMsgBox)
        warningMsgBox =
            QAbstractMessageBox::messageBox(0,title,text,
                                            QAbstractMessageBox::Warning);
    warningMsgBox->setWindowTitle(title);
    warningMsgBox->setText(text);
    warningMsgBox->setTimeout(WarningTimeout,QAbstractMessageBox::NoButton);
    QtopiaApplication::showDialog(warningMsgBox);
}

void DialProxy::stateChanged()
{
    if (DialerControl::instance()->hasIncomingCall()
            && (m_callScreenTriggers & CallIncoming)) {
        emit showCallScreen();
    }

    if (queuedIncoming && !DialerControl::instance()->hasIncomingCall()) {
        // incoming call has gone away
        queuedIncoming = false;
        if (incomingMsgBox) delete incomingMsgBox;
        incomingMsgBox = 0;
    }

    if (queuedIncoming || !queuedCall.isEmpty()) {
        bool haveFg = DialerControl::instance()->hasActiveCalls();
        if (haveFg && !DialerControl::instance()->hasCallsOnHold()) {
            DialerControl::instance()->hold();
        } else if (!haveFg) {
            if (!queuedCall.isEmpty() && (m_callScreenTriggers & CallDialing))
                emit showCallScreen();
            if (queuedIncoming) {
                queuedIncoming = false;
                DialerControl::instance()->accept();
            } else {
                QString n(queuedCall);
                queuedCall = QString();
                queuedCallContact = QUniqueId();
                DialerControl::instance()->dial(n,DialerControl::instance()->callerIdNeeded(n), queuedCallType, queuedCallContact);
            }
        }
    }

    static QList<QAbstractCallPolicyManager *> managers = qtopiaTasks<QAbstractCallPolicyManager>();
    foreach (QAbstractCallPolicyManager *manager, managers) {
        manager->updateOnThePhonePresence( DialerControl::instance()->hasActiveCalls() );
    }
}

/*!
    The incoming call is accepted. If more than two calls are active a message
    box opens that asks the user how to proceed.
*/
void DialProxy::acceptIncoming()
{
    if (DialerControl::instance()->hasIncomingCall()) {
        if (m_callScreenTriggers & CallAccepted)
            emit showCallScreen();
        if(DialerControl::instance()->hasActiveCalls() &&
            DialerControl::instance()->hasCallsOnHold()) {
            if (!incomingMsgBox) {
                incomingMsgBox = QAbstractMessageBox::messageBox(
                    0,
                    tr("End current call?"),
                    tr("<qt>Do you wish to end the current call before answering the incoming call?</qt>"),
                    QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No);
                connect(incomingMsgBox, SIGNAL(finished(int)), this, SLOT(messageBoxDone(int)));
            }
            QtopiaApplication::showDialog(incomingMsgBox);
            queuedIncoming = true;
        } else {
            // Direct the incoming call audio to the speaker
            // if on-screen button is pressed
            if (m_callAudio && m_callAudio->audioProfile().isEmpty())
                m_callAudio->setAudioProfile( AbstractAudioHandler::profileForKey(Qtopia::Key_Speaker) );

            DialerControl::instance()->accept();
        }
    }
}

/*!
    Hangs the active call up and returns \c true. If no call was hangup as a result of this
    function call this function returns \c false.
*/
bool DialProxy::hangupPressed()
{
    bool callDisconnected = false;
    DialerControl *control = DialerControl::instance();
    if (control->isConnected() || control->isDialing() ||
        control->hasIncomingCall()) {
        // We have an in-progress call, so hang it up rather than
        // close all running applications.
        if (m_callScreenTriggers & CallDropped)
            emit showCallScreen();
        control->endCall();
        callDisconnected = true;
    }

    if ( Qtopia::hasKey( Qtopia::Key_Hook ) && m_dialtoneAudio
            && !m_dialtoneAudio->audioProfile().isEmpty() )
        emit onHookGesture();

    return callDisconnected;
}

bool DialProxy::dialerVisible() const
{
    static QWidget *dialer = 0;
    if (!dialer) {
        QWidgetList list = QApplication::allWidgets();
        foreach(QWidget* w, list) {
            if (w->inherits("QAbstractDialerScreen")) {
                dialer = w;
                break;
            }
        }
    }

    if (!dialer)
        return false;
    else
        return dialer->isVisible();
}

bool DialProxy::callHistoryIsActiveWindow() const
{
    if (qApp->activeWindow() && qApp->activeWindow()->inherits("QAbstractCallhistory"))
        return true;

    return false;
}

/*!
    Determines the behavior when audio key is pressed.
*/
void DialProxy::processAudioKey(int key, bool isPress)
{
    if (!m_callAudio || !m_dialtoneAudio)
        return;

    // Current handset status
    static bool isHandsetUp = false;
    if ( key == Qtopia::Key_Hook )
        isHandsetUp = !isPress;

    // Is this a guesture putting the handset down
    const bool isHandsetDown = key == Qtopia::Key_Hook && isPress;

    const QByteArray newProfile = AbstractAudioHandler::profileForKey(key);

    if ( DialerControl::instance()->hasActiveCalls() ) {
        // If active calls exist
        // do audio transfer, hangup etc
        const QByteArray curProfile = m_callAudio->audioProfile();

        // Guesture, putting the handset down after audio transfer
        if ( isHandsetDown && !curProfile.isEmpty() &&
                curProfile != AbstractAudioHandler::profileForKey(Qtopia::Key_Hook) )
            return;

        // Same key pressed
        if ( curProfile == newProfile ) {
            if ( isHandsetUp ) {
                m_callAudio->transferAudio(AbstractAudioHandler::profileForKey(Qtopia::Key_Hook));
            }else {
                hangupPressed();
            }
        } else {
            m_callAudio->transferAudio( newProfile );
        }
    } else {
        // If no calls exist
        // answer incoming call, show dialer etc

        const QByteArray curProfile = m_dialtoneAudio->audioProfile();

        // Remote party hung up the call.
        if ( isHandsetDown && curProfile != AbstractAudioHandler::profileForKey(Qtopia::Key_Hook)
                && m_callAudio->audioProfile().isEmpty() ) {
            m_dialtoneAudio->activateAudio(false);
            emit onHookGesture();
            return;
        }

        m_callAudio->setAudioProfile(newProfile);
        // Answer the incoming call
        if ( DialerControl::instance()->hasIncomingCall() ) {
            acceptIncoming();
            return;
        }

        // If dialer has been idle, show dialer and play dial tone
        if ( curProfile.isEmpty() ) {
            if ( !isHandsetDown ) {
                QString activeAppName = WindowManagement::activeAppName();
                bool needToShowDialer = !dialerVisible()
                    && ( activeAppName.isEmpty() ? true : !dialToneOnlyHintApps.contains( activeAppName ) )
                    && !callHistoryIsActiveWindow();

                if ( needToShowDialer )
                    emit doShowDialer("");
                m_dialtoneAudio->activateAudio(true);
                m_dialtoneAudio->transferAudio( newProfile );
                emit offHookGesture();
            }
        } else {
            // Guesture, putting the handset down after audio transfer
            if ( isHandsetDown && curProfile != AbstractAudioHandler::profileForKey(Qtopia::Key_Hook) )
                return;

            // Hanging up guesture
            if ( curProfile == newProfile ) {
                // Same key pressed twice while handset is up. transfer to handset.
                if ( isHandsetUp )
                    m_dialtoneAudio->transferAudio( AbstractAudioHandler::profileForKey(Qtopia::Key_Hook) );
                // Otherwise hangup
                else {
                    // Called if server windows are not on top
                    DialerControl *control = DialerControl::instance();
                    if (control->isConnected() || control->isDialing() ||
                        control->hasIncomingCall())
                        // We have an in-progress call, so hang it up rather than
                        // close all running applications.
                        hangupPressed();
                    //Keep showing the dialer aware application such as contacts and call history
                    QString activeAppName = WindowManagement::activeAppName();
                    if ( ( activeAppName.isEmpty() ? true : !dialToneOnlyHintApps.contains( activeAppName ) )
                            && !callHistoryIsActiveWindow() ) {

                        emit resetScreen();
                    }

                    if (!m_dialtoneAudio->audioProfile().isEmpty()) {
                        m_dialtoneAudio->activateAudio(false);
                        emit onHookGesture();
                    }
                }
            } else {
                // Transfer dial tone to new profile
                m_dialtoneAudio->transferAudio( newProfile );
            }
        }
    }
}

void DialProxy::messageBoxDone(int r)
{
    QAbstractMessageBox *box = qobject_cast<QAbstractMessageBox*>(sender());
    if (!box) return;
    if (box == noVoiceMailNumberMsgBox) {
        if (r == QAbstractMessageBox::Yes) {
            QtopiaServiceRequest e( "VoiceMail", "setVoiceMail()" );
            e.send();
        }
    } else if (box == voipNoPresenceMsgBox) {
        if (r == QAbstractMessageBox::Yes) {
            dialNumber(queuedCall, queuedCallContact, queuedCallType);
        } else {
            queuedCall = QString();
            queuedCallType = QString();
            queuedCallContact = QUniqueId();
        }
    } else if (box == dialingMsgBox) {
        if (r == QAbstractMessageBox::Yes)
            DialerControl::instance()->endCall();
        else
        {
            queuedCall = QString();
            queuedCallType = QString();
            queuedCallContact = QUniqueId();
        }
    } else if (box == incomingMsgBox) {
        if (r == QAbstractMessageBox::Yes) {
            DialerControl::instance()->endCall();
        } else {
            queuedIncoming = false;
            if (DialerControl::instance()->hasIncomingCall())
                DialerControl::instance()->incomingCall().hangup();
        }
    }
}

QTOPIA_TASK(DialProxy,DialProxy);
QTOPIA_TASK_PROVIDES(DialProxy,DialProxy);
