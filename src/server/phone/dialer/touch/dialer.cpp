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

#include "dialer.h"
#include "dtmfaudio.h"
#include "qabstractmessagebox.h"
#include "qtopiaserverapplication.h"
#include "savetocontacts.h"
#include "servercontactmodel.h"
#include "serverthemeview.h"
#include "themecontrol.h"
#ifdef QTOPIA_TELEPHONY
#include "abstractdialfilter.h"
#endif

#include <QLabel>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QCallList>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTimer>
#include <QBasicTimer>

#include <qtopiaservices.h>
#include <QtopiaIpcEnvelope>
#include <QThemedView>
#include <QThemeItem>
#include <QThemeWidgetItem>
#include <qcontactview.h>
#include <QTelephonyTones>
#include <qsoftmenubar.h>

class Dialer : public QThemedView
{
    Q_OBJECT

public:
    Dialer(QWidget *parent = 0, Qt::WFlags f = 0);
    virtual ~Dialer();

    QString digits() const;
    void setDigits(const QString &digits);
    void appendDigits(const QString &digits);

public slots:
    void clear();
    void doOffHook();
    void doOnHook();

signals:
    void dial(const QString &, const QUniqueId &);
    void closeMe();
    void keyEntered(const QString&);
    void filterSelect(const QString&, bool&);

protected slots:
    void msgReceived(const QString &, const QByteArray &);

private slots:
    void numberSelected(const QString &);
    void numberSelected();
    void selectContact();
    void saveToContact();
    void backspace();
    void sms();
    virtual void selectCallHistory();
    void updateIcons(const QString &text);

protected:
    virtual void themeLoaded(const QString &theme);
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *e);
    void timerEvent(QTimerEvent *e);

    QLineEdit *display;

private:
    void playDtmf(int key);

private:
    QActionGroup *m_actions;
    QAbstractMessageBox *addContactMsg;

    QBasicTimer charSelectTimer;
    int currentChar;
};

static const QString DialerChars("*+pw");

//===========================================================================

Dialer::Dialer(QWidget *parent, Qt::WFlags f)
  : QThemedView(parent, f), display(0), m_actions(0), addContactMsg(0)
    , currentChar(0)
{
    setObjectName("Dialer");
    setWindowTitle(tr("Dialer"));

    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if (ctrl)
        ctrl->registerThemedView(this, "Dialer");
    else
        qLog(Component) << "Dialer: ThemeControl not available, Theme will not work properly";

    QtopiaChannel *channel = new QtopiaChannel("Qtopia/Phone/TouchscreenDialer", this);
    connect(channel, SIGNAL(received(QString,QByteArray)), this, SLOT(msgReceived(QString,QByteArray)));

    themeLoaded("Dialer");
}

Dialer::~Dialer()
{
}

void Dialer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            e->accept();
            appendDigits(e->text());
            playDtmf(e->key());
            break;
        case Qt::Key_Asterisk:
            e->accept();
            if (charSelectTimer.isActive())
                backspace();
            charSelectTimer.start(1000, this);
            appendDigits(DialerChars[currentChar]);
            playDtmf(e->key());
            if (++currentChar >= DialerChars.length())
                currentChar = 0;
            break;
        case Qt::Key_Hangup:
        case Qt::Key_No:
        case Qt::Key_Flip:
            e->accept();
            emit closeMe();
            break;
        case Qt::Key_NumberSign:
            // XXX - if the device has a hook, then assume that this is a deskphone,
            // and not a mobile handset.  For deskphone's, # means end of number.
            // We probably need a better way of detecting this.
            if (!Qtopia::hasKey(Qtopia::Key_Hook)) {
                e->accept();
                appendDigits(e->text());
                playDtmf(e->key());
                break;
            }
            // FALL THROUGH
        case Qt::Key_Yes:
        case Qt::Key_Call:
        case Qt::Key_Select:
            if (display && display->text().length()) {
                e->accept();
                numberSelected();
                display->setText(QString());
            }
            break;
        case Qt::Key_Back:
        case Qt::Key_Backspace:
            if (display) {
                if (display->text().isEmpty())
                    emit closeMe();
                else
                    display->backspace();
                e->accept();
            }
            break;
        case Qtopia::Key_Hook:  // no key event for this key yet
            e->accept();
            emit closeMe();
            break;
        default:
            QWidget::keyPressEvent(e);
            break;
    }
    if (e->key() != Qt::Key_Asterisk)
        currentChar = 0;
}

void Dialer::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qtopia::Key_Hook:  // no key event for this key yet
        if (display && !display->text().isEmpty()) {
            numberSelected();
            display->setText(QString());
        }
        break;
    default:
        break;
    }
}

void Dialer::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == charSelectTimer.timerId()) {
        charSelectTimer.stop();
        currentChar = 0;
    }
}

void Dialer::msgReceived(const QString &str, const QByteArray &)
{
    if (str == "selectContact()") {
        selectContact();
    } else if (str == "showCallHistory()") {
        selectCallHistory();
    } else if (str == "sms()") {
        sms();
    } else if (str == "hangup()") {
        emit closeMe();
    } else if (str == "saveToContact()") {
        saveToContact();
    } else if (str == "backspace()") {
        backspace();
    } else if (str == "clear()") {
        clear();
    } else if (str == "dial()") {
        numberSelected();
    }
}

void Dialer::themeLoaded(const QString &)
{
    QThemeWidgetItem *item = qgraphicsitem_cast<QThemeWidgetItem*>(findItem("dialernumber"));
    if (item && item->widget()) {
        display = qobject_cast<QLineEdit *>(item->widget());
    } else {
        qWarning("No input field available for dialer theme.");
        display = new QLineEdit(this);
    }

    if (display) {
        connect(display, SIGNAL(textChanged(QString)), this, SLOT(updateIcons(QString)));
        connect(display, SIGNAL(textChanged(QString)), this, SIGNAL(keyEntered(QString)));
        display->setFocusPolicy(Qt::NoFocus);
        setFocus();
        setFocusPolicy(Qt::StrongFocus);
    }

    if (m_actions)
        delete m_actions;
    m_actions = new QActionGroup(this);
    m_actions->setExclusive(false);
    // if any of these items aren't provided by the theme, put them in the contextmenu
    QAction *newAction = 0;
    int actionCount = 0;
    if (!findItem("selectcontact")) {
        newAction = new QAction(QIcon(":icon/addressbook/AddressBook"),
                                tr("Select Contact"), m_actions);
        connect(newAction, SIGNAL(triggered()), this, SLOT(selectContact()));
        ++actionCount;
    }
    if (!findItem("callhistory")) {
        newAction = new QAction(QPixmap(":image/callhistory/CallHistory"),
                                tr("Call History"), m_actions);
        connect(newAction, SIGNAL(triggered()), this, SLOT(selectCallHistory()));
        ++actionCount;
    }
    if (!findItem("messages")) {
        newAction = new QAction(QIcon(":icon/txt"),
                                tr("Send Message"), m_actions);
        connect(newAction, SIGNAL(triggered()), this, SLOT(sms()));
        ++actionCount;
    }
    if (!findItem("savecontact")) {
        newAction = new QAction(QIcon(":image/addressbook/AddressBook"),
                                tr("Save to Contacts"), m_actions);
        connect(newAction, SIGNAL(triggered()), this, SLOT(saveToContact()));
        ++actionCount;
    }
    if (actionCount > 0) {
        QMenu *menu = new QMenu(this);
        menu->addActions(m_actions->actions());
        QSoftMenuBar::addMenuTo(display, menu);
    } else {
        delete m_actions;
        m_actions = 0;
    }
}

void Dialer::updateIcons(const QString &text)
{

    if (m_actions) {
        m_actions->setEnabled(!text.trimmed().isEmpty());
        m_actions->setVisible(!text.trimmed().isEmpty());
    }

    if (display && display->text().length())
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::AnyFocus);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);

    if (display && !display->text().trimmed().isEmpty())
        QSoftMenuBar::setLabel(this, Qt::Key_Select, "phone/answer", "Call", QSoftMenuBar::AnyFocus);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel, QSoftMenuBar::AnyFocus);
}

void Dialer::saveToContact()
{
    if (!(display && !display->text().isEmpty()))
        return;
    SavePhoneNumberDialog::savePhoneNumber(display->text());
}

void Dialer::selectContact()
{
    QContactSelector contactSelector(false, this);
    contactSelector.setModel(ServerContactModel::instance());
    contactSelector.showMaximized();
    if (QtopiaApplication::execDialog(&contactSelector) && contactSelector.contactSelected()) {
        QContact cnt = contactSelector.selectedContact();
        QPhoneTypeSelector typeSelector(cnt, QString());
        if (QtopiaApplication::execDialog(&typeSelector))
            if (display)
                display->setText(typeSelector.selectedNumber());
    }
}

void Dialer::selectCallHistory()
{
    QtopiaServiceRequest e("CallHistory", "showCallHistory(QCallList::ListType,QString)");
    //we don't want to depend on telephony hence we dont use QCallList directly
    e << (int)(QCallList::All);
    if (display)
        e << display->text();
    else
        e << QString();

    e.send();
}

void Dialer::sms()
{
    QtopiaServiceRequest req("SMS", "writeSms(QString,QString)");
    req << QString() << (display ? display->text() : QString());
    req.send();
}

void Dialer::numberSelected()
{
    numberSelected((display ? display->text() : QString()));
}

void Dialer::numberSelected(const QString &number)
{
    // Filter for special GSM key sequences.
    if (!number.isEmpty()) {
#ifdef QTOPIA_TELEPHONY
        AbstractDialFilter::Action action = AbstractDialFilter::Continue;
        if (AbstractDialFilter::defaultFilter()) {
            action = AbstractDialFilter::defaultFilter()->filterInput(number, true);
            if (action == AbstractDialFilter::Continue)
                emit dial(number, QUniqueId());
        } else {
            emit dial(number, QUniqueId());
        }
#else
        emit dial(number, QUniqueId());
#endif
        emit closeMe();
    }
}

void Dialer::clear()
{
    if (display) {
        display->clear();
        updateIcons(QString());
    }
}

void Dialer::backspace()
{
    if (display)
        display->backspace();
}

QString Dialer::digits() const
{
    return (display ? display->text() : QString());
}


void Dialer::setDigits(const QString &digits)
{
    if (display) {
        display->setText(digits);
        updateIcons(digits);
    }
}

void Dialer::appendDigits(const QString &digits)
{
    if (display) {
        display->setText(display->text() + digits);
        updateIcons(display->text());
    }
}

void Dialer::playDtmf(int key)
{
    DtmfAudio *dtmf = qtopiaTask<DtmfAudio>();
    if (dtmf)
        dtmf->playDtmfKeyTone(key);
}

void Dialer::doOffHook()
{
    if (!display)
        return;
    if (!display->text().isEmpty()) {
        numberSelected(display->text());
        display->setText(QString());
    }
}

void Dialer::doOnHook()
{
    if (!display)
        return;
    display->setText(QString());
    emit closeMe();
}


/*!
    \class PhoneTouchDialerScreen
    \inpublicgroup QtTelephonyModule
    \ingroup QtopiaServer::PhoneUI
    \brief The PhoneTouchDialerScreen class implements a touchscreen dialer.

    The touchscreen dialer does not support the speeddial signal.
    An image of this dialer screen using the deskphone theme can be found in
    the \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractDialerScreen
*/

// define PhoneTouchDialerScreen

/*!
  Construct a new PhoneTouchDialerScreen instance with the appropriate \a parent
  and \a flags.
 */
PhoneTouchDialerScreen::PhoneTouchDialerScreen(QWidget *parent, Qt::WFlags flags)
        : QAbstractDialerScreen(parent, flags)
{
    setWindowTitle( tr("Dialer") );
    setObjectName("Dialer");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    m_dialer = new Dialer(this);
    layout->addWidget(m_dialer);

    QObject::connect(m_dialer,
                     SIGNAL(dial(QString,QUniqueId)),
                     this,
                     SIGNAL(requestDial(QString,QUniqueId)));
    QObject::connect(m_dialer, SIGNAL(closeMe()), this, SLOT(close()));
    QObject::connect(m_dialer, SIGNAL(keyEntered(QString)), this, SLOT(keyEntered(QString)));
}

/*! \reimp */
void PhoneTouchDialerScreen::reset()
{
    m_dialer->clear();
}

/*! \reimp */
void PhoneTouchDialerScreen::appendDigits(const QString &digits)
{
    m_dialer->appendDigits(digits);
}

/*! \reimp */
void PhoneTouchDialerScreen::setDigits(const QString &digits)
{
    m_dialer->setDigits(digits);
}

/*! \reimp */
QString PhoneTouchDialerScreen::digits() const
{
    return m_dialer->digits();
}

/*! \internal */
void PhoneTouchDialerScreen::keyEntered(const QString &key)
{
#ifdef QTOPIA_TELEPHONY
    AbstractDialFilter::Action action = AbstractDialFilter::Continue;
    if (AbstractDialFilter::defaultFilter()) {
        action = AbstractDialFilter::defaultFilter()->filterInput(key);
        if (action == AbstractDialFilter::ActionTaken)
            m_dialer->setDigits(QString());
    }
#endif
}

/*! \reimp */
void PhoneTouchDialerScreen::doOffHook()
{
    m_dialer->doOffHook();
}

/*! \reimp */
void PhoneTouchDialerScreen::doOnHook()
{
    m_dialer->doOnHook();
}

QTOPIA_REPLACE_WIDGET_WHEN(QAbstractDialerScreen, PhoneTouchDialerScreen, Touchscreen);
#include "dialer.moc"
