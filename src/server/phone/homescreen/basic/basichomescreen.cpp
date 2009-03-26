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

#include "basichomescreen.h"

#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QTimer>

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#include <qsoftmenubar.h>
#include <qdevicebuttonmanager.h>
#include <QValueSpaceObject>

#include "qtopiainputevents.h"
#include "uifactory.h"
#include "messageboard.h"
#ifdef QTOPIA_TELEPHONY
#include "dialercontrol.h"
#endif

/*!
    \reimp
*/
bool BasicHomeScreen::locked() const
{
    return (keyLock->locked() || screenLocked);
}

#ifdef QTOPIA_CELL
/*!
    \reimp
*/
bool BasicHomeScreen::simLocked() const
{
    return simLock->locked();
}
#endif

/*!
    \reimp
*/
void BasicHomeScreen::setLocked(bool lock)
{
    if (lock) {
        if (Qtopia::mousePreferred())
            QTimer::singleShot(0, this, SLOT(lockScreen()));
        else
            keyLock->lock();
    } else {
        if (Qtopia::mousePreferred())
            screenUnlocked();
        else
            keyLock->unlock();
    }
}


/*!
    \class BasicHomeScreen
    \inpublicgroup QtUiModule
    \brief The BasicHomeScreen class provides most functionality required by a HomeScreen except
    the actual user interface.
    \ingroup QtopiaServer::PhoneUI

    The ThemedHomeScreen class is based on this class and provides a themeable user interface
    to this class. This split increases flexibility when designing a new HomeScreen interface
    while keeping basic HomeScreen functionality such as locking, keypress handling or the correct
    context label changes.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractHomeScreen, ThemedHomeScreen
*/

/*!
  Construct a new BasicHomeScreen instance, with the specified \a parent
  and widget \a flags.
*/
BasicHomeScreen::BasicHomeScreen(QWidget *parent, Qt::WFlags flags)
    : QAbstractHomeScreen(parent, flags)
#ifdef QTOPIA_CELL
    , emLock(0)
    , simLock(0)
#endif
#ifdef QTOPIA_TELEPHONY
    , actionMessages(0)
    , actionCalls(0)
#endif
    , m_contextMenu(0)
    , lockMsgId(-1)
    , missedCalls(0)
    , newMessages(0)
    , smsMemoryFull(false)
    , screenLocked(false)
    , newMsgVsi(0)
    , smsMemFull(0)
    , touchLockScreen(0)
{
    keyLock = new BasicKeyLock(this);

    QObject::connect(keyLock, SIGNAL(stateChanged(BasicKeyLock::State)),
                     this, SLOT(showLockInformation()));

#ifdef QTOPIA_CELL
    emLock = new BasicEmergencyLock(this);
    simLock = new BasicSimPinLock(this);

    QObject::connect(emLock,
                     SIGNAL(stateChanged(BasicEmergencyLock::State,QString)),
                     this, SLOT(showLockInformation()));
    QObject::connect(simLock,
                     SIGNAL(stateChanged(BasicSimPinLock::State,QString)),
                     this, SLOT(showLockInformation()));
    QObject::connect(emLock, SIGNAL(dialEmergency(QString)),
                     this, SIGNAL(callEmergency(QString)));
    QObject::connect(emLock, SIGNAL(dialEmergency(QString)),
                     simLock, SLOT(reset()));
    QObject::connect(emLock, SIGNAL(dialEmergency(QString)),
                     keyLock, SLOT(reset()));
#endif

    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_InputMethodEnabled);

    setFocusPolicy(Qt::StrongFocus);

#ifndef QTOPIA_HOMEUI
    m_contextMenu = QSoftMenuBar::menuFor(this);
    m_contextMenu->installEventFilter(this);

    if (Qtopia::mousePreferred()) {
        actionLock = new QAction(QIcon(":icon/padlock"), tr("Screen Lock"), this);
        connect(actionLock, SIGNAL(triggered()), this, SLOT(lockScreen()));
    } else {
        actionLock = new QAction(QIcon(":icon/padlock"), tr("Key Lock"), this);
        connect(actionLock, SIGNAL(triggered()), keyLock, SLOT(lock()));
    }
    m_contextMenu->addAction(actionLock);
    QAction *actionProfile = new QAction(QIcon(":icon/Note"), tr("Profile..."), this);
    connect(actionProfile, SIGNAL(triggered()), this, SLOT(showProfileSelector()));
    m_contextMenu->addAction(actionProfile);

# ifdef QTOPIA_TELEPHONY
    connect(DialerControl::instance(), SIGNAL(stateChanged()),
            this, SLOT(phoneStateChanged()));
    connect(DialerControl::instance(), SIGNAL(missedCount(int)),
            this, SLOT(setMissedCalls(int)));

    actionCalls = new QAction(QIcon(":icon/phone/missedcall"), tr("Missed Calls..."), this);
    connect(actionCalls, SIGNAL(triggered()), this, SLOT(viewMissedCalls()));
    m_contextMenu->addAction(actionCalls);
    actionCalls->setEnabled(false);
    actionCalls->setVisible(false);

    actionMessages = new QAction(QIcon(":icon/phone/sms"), tr("New Messages..."), this);
    connect(actionMessages, SIGNAL(triggered()), this, SLOT(viewNewMessages()));
    m_contextMenu->addAction(actionMessages);
    actionMessages->setEnabled(false);
    actionMessages->setVisible(false);

    newMsgVsi = new QValueSpaceItem("/Communications/Messages/NewMessages", this);
    connect(newMsgVsi, SIGNAL(contentsChanged()), this, SLOT(newMessagesChanged()));
    smsMemFull = new QValueSpaceItem("/Telephony/Status/SMSMemoryFull", this);
    connect(smsMemFull, SIGNAL(contentsChanged()),this, SLOT(smsMemoryFullChanged()) );
    //init message and smsFull checks
    newMessagesChanged();
    smsMemoryFullChanged();
# endif
#endif

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);

// when using ts with a popup input method, this makes the input method popup.
// disabling the hint prevents shortcuts on ts, although.
//
//    if (Qtopia::mousePreferred()) {
//        QtopiaApplication::setInputMethodHint(this, "phoneonly");
//        setFocus();
//    }

    speeddialTimer = new QTimer(this);
    speeddialTimer->setSingleShot(true);
    speeddial_preedit = 0;
    connect(speeddialTimer, SIGNAL(timeout()), this, SLOT(activateSpeedDial()));

    ph = new PressHoldGate("HomeScreen", this);
    connect(ph, SIGNAL(activate(int,bool,bool)), this, SLOT(specialButton(int,bool)));

    installEventFilter(this);
    showLockInformation();
}

/*!
  \internal
  */
void BasicHomeScreen::activateSpeedDial()
{
    if (speeddialdown)
        emit speedDial(speeddial);
    if (speeddial_preedit)
        speeddial_activated_preedit = speeddial.right(speeddial_preedit);
    speeddial_preedit = 0;
    speeddial.clear();
}


/*!
  \internal
  */
BasicHomeScreen::~BasicHomeScreen()
{
}


#ifdef QTOPIA_TELEPHONY
/*!
  \internal
  */
void BasicHomeScreen::setMissedCalls(int m)
{
    if (actionCalls ) {
        actionCalls->setEnabled(m);
        actionCalls->setVisible(m);
    }
}

/*!
  \internal
  */
void BasicHomeScreen::newMessagesChanged()
{
    if (newMsgVsi)
        newMessages = newMsgVsi->value(QByteArray(),0).toInt();
    else
        newMessages = 0;

    if (actionMessages) {
        actionMessages->setEnabled(newMessages || smsMemoryFull);
        actionMessages->setVisible(newMessages || smsMemoryFull);
    }
}

/*!
  \internal
  */
void BasicHomeScreen::smsMemoryFullChanged()
{
    //zero -> space avilable
    //one -> full
    //two -> full and rejected
    if (smsMemFull)
        smsMemoryFull = (smsMemFull->value(QByteArray(), 0).toInt() != 0);
    else
        smsMemoryFull = 0;

    if (actionMessages) {
        actionMessages->setEnabled(newMessages || smsMemoryFull);
        actionMessages->setVisible(newMessages || smsMemoryFull);
    }
}

#endif

/*!
  \internal
  */
void BasicHomeScreen::setContextBarLocked(bool lock, bool waiting)
{
#ifndef QTOPIA_HOMEUI
    if ( lock || waiting ) {
        // set the phone to locked - we do it both when we _require_ a lock
        // and when we are not yet sure if we need to lock (waiting).
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), "unlock", tr("Unlock"));

#ifdef QTOPIA_TELEPHONY
#ifdef QTOPIA_CELL
        if(!simLock->number().isEmpty())
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
        else
#endif
            if ( DialerControl::instance()->allCalls().count() )
                QSoftMenuBar::setLabel(this, Qt::Key_Back, "phone/calls", tr("Calls"));
            else
                QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);
#endif

        // now blank out the context menu while we wait for the modem to respond.
        if ( waiting ) {
            // We have not yet decided whether we need a PIN or not.  Blank context menu.
            QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), QSoftMenuBar::NoLabel);
            QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);
        }
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, "qpe/menu", tr("Menu"));
        if (BasicKeyLock::lockKey() == QSoftMenuBar::menuKey()) {
            QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), QSoftMenuBar::Options);
        } else {
            QSoftMenuBar::clearLabel(this, BasicKeyLock::lockKey());
        }
    }
#else
    Q_UNUSED(lock)
    Q_UNUSED(waiting)
#endif
}

/*!
  \internal
  */
void BasicHomeScreen::showProfileSelector()
{
    QtopiaServiceRequest e("Profiles", "showProfiles()");
    e.send();
}


/*!
  \internal
  */
void BasicHomeScreen::showLockInformation()
{
#ifdef QTOPIA_CELL
    bool lock = keyLock->locked() || simLock->locked();
    bool waiting = simLock->state() == BasicSimPinLock::Waiting;
#else
    bool lock = keyLock->locked();
    bool waiting = false;
#endif
    emit lockStateChanged(lock);

    QString text;
    QString pix(":image/padlock");

    if (lock) {
        if (!Qtopia::mousePreferred())
            QtopiaInputEvents::suspendMouse();

        if (m_contextMenu)
            m_contextMenu->hide();
    }
    setContextBarLocked(lock, waiting);

    if (!lock) {
        // No lock
        if (!Qtopia::mousePreferred())
            QtopiaInputEvents::resumeMouse();
    }
#ifdef QTOPIA_CELL
    else if (emLock->emergency()) {
        // disable the lock key cause it can use the emergency number to unlock sim.
        QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), QSoftMenuBar::NoLabel);

        // Emergency dial!
        bool partial =
            (BasicEmergencyLock::PartialEmergencyNumber == emLock->state());
        QString number = emLock->emergencyNumber();

        if (partial) {
            text = QString("<b>") + number + QString("</b>");
            if (Qtopia::mousePreferred())
                QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), QSoftMenuBar::NoLabel);
        } else {
            text = tr("Emergency number <b>%1</b><br>Press "
                      "<font color=#008800>Call</font> to dial.").arg(number);
            pix = ":image/emergency";
            if (Qtopia::mousePreferred())
                QSoftMenuBar::setLabel(this, BasicKeyLock::lockKey(), "phone/calls", tr("Call"));
        }
    }
#endif
    else if (keyLock->locked()) {
        // Key lock
#ifdef QTOPIA_CELL
        // Clear partial emergency numbers or we won't be able to
        // type a new one in once the bad key has been cleared.
        emLock->reset();
#endif
        switch (keyLock->state()) {
        case BasicKeyLock::KeyLocked:
            break;
        case BasicKeyLock::KeyLockIncorrect: {
            text = tr("Press %1 then * to unlock phone");
            int lkey = BasicKeyLock::lockKey();
            if (lkey == Qt::Key_Context1)
                text = text.arg(tr("Unlock"));
            else
                text = text.arg(QString(QKeySequence(lkey)));
        }
        break;
        case BasicKeyLock::KeyLockToComplete:
            text = tr("Now press * to unlock phone");
            break;
        default:
            Q_ASSERT(!"Unknown state");
            break;
        };
    } else {
#ifdef QTOPIA_CELL
        // Sim lock
        bool waiting = false;
        switch (simLock->state()) {
        case BasicSimPinLock::VerifyingSimPuk:
        case BasicSimPinLock::VerifyingSimPin:
        case BasicSimPinLock::Waiting:
            text = tr("Please Wait...", "please wait while the phone checks to see if it needs a pin number, or if the most recently entered pin number is correct");
            waiting = true;
            break;
        case BasicSimPinLock::SimPinRequired:
            text = tr("Enter PIN then press %1<br>");
            break;
        case BasicSimPinLock::SimPukRequired:
            text = tr("Enter PUK then press %1<br>");
            break;
        case BasicSimPinLock::NewSimPinRequired:
            text = tr("Enter new PIN then press %1<br>");
            break;
        case BasicSimPinLock::Pending:
            if (!Qtopia::mousePreferred())
                text = tr("Please Wait...");
            waiting = true;
            break;
        default:
            Q_ASSERT(!"Unknown state");
            break;
        }

        if (!waiting) {
            text.append(QString(simLock->number().length(), '*'));

            int lkey = BasicSimPinLock::lockKey();
            if (lkey == Qt::Key_Context1)
                text = text.arg("Unlock");
            else
                text = text.arg(QString(QKeySequence(lkey)));
        }
#endif // QTOPIA_CELL
    }

    MessageBoard *board = qtopiaTask<MessageBoard>();
    if (lockMsgId > -1 && board ) {
        board->clearMessage(lockMsgId);
        lockMsgId = -1;
    }

    if (!text.isEmpty()) {
        if ( Qtopia::mousePreferred() ) {
            emit showLockDisplay(true, pix, text);
        }
        else if ( board )
            lockMsgId = board->postMessage(pix, text, 0);
        else
            qLog(Component) << "Cannot post message due to missing MessageBoard";
    } else {
        if ( Qtopia::mousePreferred() ) {
            emit showLockDisplay(false, pix, text);
        }
    }
}

#ifdef QTOPIA_TELEPHONY
/*!
  \internal
  */
void BasicHomeScreen::viewNewMessages()
{
    QtopiaServiceRequest req("Messages", "viewNewMessages(bool)");
    req << true;
    req.send();
}

/*!
  \internal
  */
void BasicHomeScreen::viewMissedCalls()
{
    emit showMissedCalls();
}
#endif

/*!
  \internal
  */
void BasicHomeScreen::inputMethodEvent(QInputMethodEvent *e)
{
    speeddialdown = true;
    QString ctext = e->commitString();
    QString ptext = e->preeditString();
    speeddial = speeddial.left(speeddial.length() - speeddial_preedit);
    if (!ctext.isEmpty()) {
        if (ctext != speeddial_activated_preedit) {
            speeddial += ctext;
            speeddial_preedit = 0;
        }
    }

    if (!ptext.isEmpty()) {
        speeddial += ptext;
        speeddial_preedit = ptext.length();
    }

    speeddial_activated_preedit.clear();

    if (!speeddial.isEmpty())
        speeddialTimer->start(1000);
}

/*!
  \internal
  */
void BasicHomeScreen::specialButton(int keycode, bool held)
{
    const QDeviceButton* button =
        QDeviceButtonManager::instance().buttonForKeycode(keycode, "HomeScreen");
    if (button) {
        QtopiaServiceRequest sr;
        if (held) {
            sr = button->heldAction();
        } else {
            sr = button->pressedAction();
        }
        if (!sr.isNull())
            sr.send();
    }
}

/*!
  \internal
  */
bool BasicHomeScreen::eventFilter(QObject *, QEvent *e)
{
#ifdef QTOPIA_CELL
    bool locked = keyLock->locked() || simLock->locked();
#else
    bool locked = keyLock->locked();
#endif

    if (locked && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent *)e;
        // there might be emergency calls
#ifdef QTOPIA_TELEPHONY
        if ( DialerControl::instance()->allCalls().count() ) {
           if ( ke->key() == Qt::Key_Back
#ifdef QTOPIA_CELL
                   && !simLock->number().length()
#endif
                   )
                emit showCallScreen();
            else if ( ke->key() == Qt::Key_Hangup )
                emit hangupCall();
        }
#endif
#ifdef QTOPIA_CELL
        if (!emLock->processKeyEvent((QKeyEvent *)e)) {
#endif
            if (keyLock->locked())
                keyLock->processKeyEvent((QKeyEvent *)e);
#ifdef QTOPIA_CELL
            else
                simLock->processKeyEvent((QKeyEvent *)e);
        }
#endif

        return true;
    } else if (!keyLock->locked()
#ifdef QTOPIA_CELL
                && !simLock->locked()
#endif
               && e->type() == QEvent::KeyPress) {
        keyLock->processKeyEvent((QKeyEvent *)e);
        return false;
    } else if (locked && e->type() == QEvent::KeyRelease) {
        return true;
    } else if (locked && e->type() == QEvent::WindowActivate) {
        // make sure the pin entry dialog is shown if sim is still locked.
        showLockInformation();
    }
    return false;
}

/*!
  \internal
  */
void BasicHomeScreen::focusInEvent(QFocusEvent *)
{
    // Avoid repaint.
}

/*!
  \internal
  */
void BasicHomeScreen::focusOutEvent(QFocusEvent *)
{
    // Avoid repaint.
}

/*!
  \internal
  */
void BasicHomeScreen::keyPressEvent(QKeyEvent *k)
{
    // also needs to respond to IM events....
    char ch = k->text()[0].toLatin1();
    if ((ch >= '0' && ch <= '9')
#ifdef QTOPIA_VOIP
            || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
#endif
            || ch == 'p' || ch == 'P' || ch == '+' || ch == 'w'
            || ch == 'W' || ch == '#' || ch == '*' || ch == '@') {

#ifdef QTOPIA_TELEPHONY
        if (DialerControl::instance()->isDialing()) {
            // send the digits as dtmf tones to the active calls.
            QList<QPhoneCall> activeCalls = DialerControl::instance()->activeCalls();
            if (activeCalls.count())
                activeCalls.first().tone(k->text());
            emit showCallScreen();
        } else {
            if (!k->isAutoRepeat()) {
                emit dialNumber(k->text().toLower());
            }
        }
#else
        if (ch >= '0' && ch <= '9' && !k->isAutoRepeat()) {
            speeddialdown = true;
            speeddial += ch;
            speeddialTimer->start(1000);
        }
#endif
        return;
    }
    switch (k->key()) {
#ifdef QTOPIA_TELEPHONY
    case Qt::Key_Call:
    case Qt::Key_Yes:
        if (!DialerControl::instance()->hasIncomingCall() && !locked() &&
                !DialerControl::instance()->isDialing())
            emit showCallHistory();
        break;
#endif
    case Qt::Key_Flip: {
        QSettings cfg("Trolltech", "Phone");
        cfg.beginGroup("FlipFunction");
        if (!cfg.value("hangup").toBool())
            break;
        // (else FALL THROUGH)
    }
#ifdef QTOPIA_TELEPHONY
    case Qt::Key_Hangup:
    case Qt::Key_No:
        emit hangupCall();
        break;
    case Qt::Key_Back:
        if (DialerControl::instance()->allCalls().count())
            emit showCallScreen();
        k->accept();
        break;
#else
    case Qt::Key_Back:
        k->accept();
        break;
#endif
    case Qt::Key_Select:
        k->accept();
        emit showPhoneBrowser();
        break;
    default:
        ph->filterDeviceButton(k->key(), true, k->isAutoRepeat());
        k->ignore();
    }
}

/*!
  \internal
  */
void BasicHomeScreen::keyReleaseEvent(QKeyEvent *k)
{
    if (!k->isAutoRepeat())
        speeddialdown = false;
    if (ph->filterDeviceButton(k->key(), false, k->isAutoRepeat()))
        k->accept();
}

#ifdef QTOPIA_TELEPHONY

/*!
  \internal
  */
void BasicHomeScreen::phoneStateChanged()
{
#ifndef QTOPIA_HOMEUI
    if (DialerControl::instance()->allCalls().count()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "phone/calls", tr("Calls"));
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::NoLabel);
    }
#endif
    if (DialerControl::instance()->hasIncomingCall() ||
            DialerControl::instance()->isConnected() ||
            DialerControl::instance()->isDialing()) {

        if (keyLock->locked())
            setContextBarLocked(false);

        actionLock->setEnabled(false);
    } else {
        if (keyLock->locked())
            setContextBarLocked(true);

        actionLock->setEnabled(true);
    }
}
#endif


/*!
    \internal
    Locks the screen.
*/
void BasicHomeScreen::lockScreen()
{
    if (touchLockScreen)
        return;

    touchLockScreen = UIFactory::createDialog( "TouchScreenLockDialog", this, Qt::WindowStaysOnTopHint);
    if (touchLockScreen) {
        screenLocked = true;
        connect(touchLockScreen, SIGNAL(accepted()), this, SLOT(screenUnlocked()));
        connect(touchLockScreen, SIGNAL(rejected()), this, SLOT(screenUnlocked()));
    } else {
        qLog(Component) << "BasicHomeScreen: TouchScreenLockDlg not available";
    }

}

/*!
  \internal
  */
void BasicHomeScreen::screenUnlocked()
{
    screenLocked = false;
    if (touchLockScreen) {
        touchLockScreen->deleteLater();
        touchLockScreen = 0;
    }
}

/*!
    \internal
    \fn void BasicHomeScreen::showLockDisplay(bool enable, const QString& pix, const QString &text)

    This signal is emitted to indicate the need to display user information
    regarding the lock status of the device e.g if the keypad lock is active. If \a enable
    is true the lock is active, otherwise it is not.  The value \a pix is the
    path to an appropriate icon e.g. a padlock icon,  and \a text is the informational message to be displayed.
*/








