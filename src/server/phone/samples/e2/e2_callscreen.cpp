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

#include "e2_callscreen.h"
#include "e2_colors.h"
#include <QtopiaChannel>
#include <QSMSReader>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "dialercontrol.h"
#include <QPainter>
#include <QTimer>
#include <QContact>
#include <QContactModel>
#include "e2_bar.h"
#include "e1_error.h"
#include <QMouseEvent>
#include <QCallList>
#include <QCallListItem>
#include <QStackedWidget>
#include <QFrame>
#include <qtopiaservices.h>

E2CallScreen::E2CallScreen(E2Button *b, QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags), m_timer(0),
  m_toDialer(":image/samples/e2_todialer"),
  m_buttons(":image/samples/e2_holdmute"),
  m_button(b)
{
    m_state = "Connected";

    QObject::connect(DialerControl::instance(),
                     SIGNAL(callConnected(QPhoneCall)),
                     this,
                     SLOT(callConnected(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                     SIGNAL(callIncoming(QPhoneCall)),
                     this,
                     SLOT(callIncoming(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                     SIGNAL(callDialing(QPhoneCall)),
                     this,
                     SLOT(callDialing(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                     SIGNAL(callMissed(QPhoneCall)),
                     this,
                     SLOT(callEnded(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                     SIGNAL(callDropped(QPhoneCall)),
                     this,
                     SLOT(callEnded(QPhoneCall)));

    QObject::connect(m_button, SIGNAL(clicked()),
                     this, SLOT(end()));
}

void E2CallScreen::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QFontMetrics fm(font());
    QRect stateRect = fm.boundingRect(m_state);
    QRect timeRect = fm.boundingRect(m_time);

    p.setPen(Qt::black);
    p.drawLine(0, 0, width(), 0);
    p.drawLine(0, 0, 0, 1 + stateRect.height());
    p.drawLine(0, 1 + stateRect.height(), width(), 1 + stateRect.height());
    p.drawLine(width() - 1, 0, width() - 1, stateRect.height());
    p.fillRect(1, 1, width() - 2, stateRect.height(), palette().highlight());

    p.drawPixmap(m_toDialer.width() + 5, (height() - m_image.height()) / 2,
                 m_image);
    p.drawText(m_toDialer.width() + 5 + m_image.width() + 5, (height() - m_image.height()) / 2 + fm.height(), m_name);

    p.drawText(5, stateRect.height() - 2, m_state);
    p.drawText(width() - 1 - timeRect.width() - 5, stateRect.height() - 2, m_time);

    p.drawPixmap(0, height() - m_toDialer.height(), m_toDialer);
}

void E2CallScreen::callConnected( const QPhoneCall &call )
{
    if(!m_timer)
        m_timer = startTimer(1000);
    m_state = "Connected";
    update();
    m_callTime = call.connectTime();
    updateCallTime();
    updateInfo(call);
    m_hasCall = call.identifier();
    setActive();
}

void E2CallScreen::callIncoming( const QPhoneCall &call )
{
    if(!m_hasCall.isEmpty())
        return;
    if(!m_timer)
        m_timer = startTimer(1000);
    m_state = "Incoming";
    update();
    m_callTime = call.startTime();
    updateCallTime();
    updateInfo(call);
    m_hasCall = call.identifier();
    setActive();
}

void E2CallScreen::callDialing( const QPhoneCall &call )
{
    if(!m_hasCall.isEmpty())
        return;
    if(!m_timer)
        m_timer = startTimer(1000);
    m_state = "Calling";
    update();
    m_callTime = call.startTime();
    updateCallTime();
    updateInfo(call);
    m_hasCall = call.identifier();
    setActive();
}

void E2CallScreen::callEnded(const QPhoneCall &call)
{
    if(call.identifier() != m_hasCall)
        return;

    if(m_timer) {
        killTimer(m_timer);
        m_timer = 0;
    }

    m_state = "Call Ended";
    update();
    m_hasCall = QString();
    QTimer::singleShot(1000, this, SIGNAL(closeMe()));
    setActive();
}

void E2CallScreen::updateCallTime()
{
    int elapsed = QDateTime::currentDateTime().toTime_t() -
                  m_callTime.toTime_t();

    int min = elapsed / 60;
    int sec = elapsed % 60;

    m_time = QString("%1:%2").arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    update();
}

void E2CallScreen::updateInfo(const QPhoneCall &call)
{
    // Get the number or name to display in the text area.
    m_name = call.number();

    QContact cnt;
    if (!call.contact().isNull()) {
        QContactModel m;
        cnt = m.contact(call.contact());
    } else if (!m_name.isEmpty()) {
        QString name;
        QContactModel m;
        cnt = m.matchPhoneNumber(m_name);
    }

    if (!cnt.uid().isNull()) {
        m_name = cnt.label();
        m_image = cnt.portrait();
        if(m_image.isNull())
            m_image = QPixmap(":image/samples/e2_contact");
    } else {
        m_image = QPixmap(":image/samples/e2_contact");
    }
}

void E2CallScreen::setActive()
{
    if(isVisible()) {
        if(m_hasCall.isEmpty()) {
            m_button->setEnabled(false);
        } else {
            m_button->setEnabled(true);
        }
        m_button->setText("End");
    }
}

void E2CallScreen::sendNumber(const QString &number)
{
    Q_UNUSED(number);
    if(!m_hasCall.isEmpty())
        return;

    QValueSpaceItem item("/Telephony/Status");
    if ( !item.value("NetworkRegistered").toBool() ) {
        E1Error::error("Not registered.");
        return;
    }

    DialerControl::instance()->dial(number, true);
    emit showMe();
}

void E2CallScreen::end()
{
    if(isVisible())
        DialerControl::instance()->endAllCalls();
}

void E2CallScreen::timerEvent(QTimerEvent *)
{
    updateCallTime();
}

QRect E2CallScreen::toDialerRect() const
{
    QRect r(0, height() - m_toDialer.height(),
            m_toDialer.width(), m_toDialer.height());
    return r;
}

void E2CallScreen::mouseReleaseEvent(QMouseEvent *e)
{
    if(toDialerRect().contains(e->pos()))
        emit toDialer();
}


E2CallHistory::E2CallHistory(QWidget *parent)
: E2TitleFrame(E2TitleFrame::GradientTitle, parent), m_state(Home)
{
    setAttribute(Qt::WA_ShowModal);
    setFixedSize(205, 257);
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    m_list = new E2ListWidget(this);
    layout->addWidget(m_list);
//    QObject::connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemActivated()));
    QObject::connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated()));

    E2Button *button = new E2Button(bar());
    button->setText("Cancel");
    bar()->addButton(button, 0);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(cancelClicked()));

    updateState();
}

void E2CallHistory::itemActivated()
{
    if(m_state == Home) {
        switch(m_list->currentRow()) {
            case 0:
                m_state = Answered;
                break;
            case 1:
                m_state = Missed;
                break;
            case 2:
                m_state = Dialed;
                break;
            default:
                qFatal("Unknown current row");
                break;
        }

    } else {
        int currentRow = m_list->currentRow();
        if(currentRow != -1 && currentRow < m_currentList.count()) {
            QtopiaServiceRequest req("Dialer", "dial(QString,QUniqueId)");
            req << m_currentList.at(currentRow) << QUniqueId();
            req.send();
        }
        m_state = Home;
        close();
    }

    updateState();
}

void E2CallHistory::cancelClicked()
{
    if(Home == m_state) {
        close();
    } else  {
        m_state = Home;
        updateState();
    }
}

void E2CallHistory::updateState()
{
    m_list->clear();
    m_currentList.clear();

    if(Home == m_state) {
        setTitleText("Recent Calls");

        int dialed = 0;
        int received = 0;
        int missed = 0;

        QCallList list;
        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            switch(list.at(ii).type()) {
                case QCallListItem::Dialed:
                    ++dialed;
                    break;
                case QCallListItem::Received:
                    ++received;
                    break;
                case QCallListItem::Missed:
                    ++missed;
                    break;
            }
        }

        m_list->addItem("Answered Calls(" + QString::number(received) + ")");
        m_list->addItem("Missed Calls(" + QString::number(missed) + ")");
        m_list->addItem("Dialed Calls(" + QString::number(dialed) + ")");

    } else {

        QContactModel contacts;

        QCallListItem::CallType match(QCallListItem::Missed);
        QString text;

        switch(m_state) {
            case Answered:
                match = QCallListItem::Received;
                text = "Answered Calls";
                break;

            case Missed:
                match = QCallListItem::Missed;
                text = "Missed Calls";
                break;

            case Dialed:
                match = QCallListItem::Dialed;
                text = "Dialed Calls";
                break;
            default:
                Q_UNUSED(match);
                qFatal("Unknown call type");
                break;
        }

        setTitleText(text);

        QCallList list;
        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            QCallListItem item = list.at(ii);
            if(item.type() == match) {
                m_currentList.append(item.number());

                QString disp;
                QString name;
                disp = QString("%1:%2").arg(item.start().time().hour(), 2, 10, QChar('0')).arg(item.start().time().minute(), 2, 10, QChar('0'));
                disp += "  ";

                if(item.contact().isNull()) {
                    QContact c = contacts.matchPhoneNumber(item.number());
                    if(c.uid().isNull()) {
                        name = item.number();
                    } else {
                        name = c.label();
                    }
                } else {
                    QContact contact = m_contacts.contact(item.contact());
                    name = contact.label();
                }

                disp += name;
                if(!name.isEmpty()) {
                    m_list->addItem(disp);
                }
            }
        }
    }
}

// define E2Incoming
E2Incoming::E2Incoming()
: E2TitleFrame(E2TitleFrame::NoTitle, 0)
{
    setAttribute(Qt::WA_ShowModal);

    QObject::connect(DialerControl::instance(),
                     SIGNAL(callIncoming(QPhoneCall)),
                     this,
                     SLOT(callIncoming(QPhoneCall)));
    QObject::connect(DialerControl::instance(),
                        SIGNAL(callMissed(QPhoneCall)),
                        this,
                        SLOT(missedIncomingCall(QPhoneCall)));

    E2Button * but = new E2Button(bar());
    but->setText("Ignore");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(ignore()));
    bar()->addButton(but, 0);

    but = new E2Button(bar());
    but->setText("Busy");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(busy()));
    bar()->addButton(but, 0);

    but = new E2Button(bar());
    but->setText("Answer");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(answer()));
    bar()->addButton(but, 0);

    QVBoxLayout *l = new QVBoxLayout(this);
    QLabel * incoming = new QLabel(this);
    incoming->setText("Incoming Call...");
    l->addWidget(incoming);

    QHBoxLayout *h = new QHBoxLayout;
    l->addLayout(h);
    image = new QLabel(this);
    h->addWidget(image);
    name = new QLabel(this);
    h->addWidget(name);

    setFixedWidth(180);
}

void E2Incoming::callIncoming( const QPhoneCall &call )
{
    if(call.identifier() == m_lastId)
        return;

    if(DialerControl::instance()->hasActiveCalls() ||
       DialerControl::instance()->hasCallsOnHold()) {
        DialerControl::instance()->sendBusy();
        return;
    }

    m_lastId = call.identifier();
    currentCall = call;
    updateLabels();
    show();
    e2Center(this);
}

void E2Incoming::missedIncomingCall( const QPhoneCall& call )
{
    if( call == currentCall )
        close();
}

void E2Incoming::ignore()
{
    close();
}

void E2Incoming::busy()
{
    DialerControl::instance()->sendBusy();
    close();
}

void E2Incoming::answer()
{
    DialerControl::instance()->accept();
    emit showCallscreen();
    close();
}

void E2Incoming::updateLabels()
{
    // Get the number or name to display in the text area.
    QString m_name = currentCall.number();

    QContact cnt;
    if (!currentCall.contact().isNull()) {
        QContactModel m;
        cnt = m.contact(currentCall.contact());
    } else if (!m_name.isEmpty()) {
        QString name;
        QContactModel m;
        cnt = m.matchPhoneNumber(m_name);
    }

    QPixmap m_image;

    if (!cnt.uid().isNull()) {
        m_name = cnt.label();
        if(!cnt.portraitFile().isEmpty())
            m_image = QPixmap(cnt.portraitFile());
        else
            m_image = QPixmap();

        if(m_image.isNull()) {
            m_image = QPixmap(":image/samples/e2_contact");
        }
    } else {
        m_image = QPixmap(":image/samples/e2_contact");
    }

    name->setText(m_name);
    if(m_image.width() > 96 || m_image.height() > 96) 
        m_image = m_image.scaled(96, 96, Qt::KeepAspectRatio); 

    image->setPixmap(m_image);
}

// declare E2FSCallHistoryNumber
class E2FSCallHistoryNumber : public QWidget
{
Q_OBJECT
public:
    E2FSCallHistoryNumber(QWidget *parent = 0);

    void setNumber(const QCallListItem &);
protected:
    virtual void paintEvent(QPaintEvent *);

private:
    QContactModel m_contacts;
    QCallListItem m_item;
    QPixmap m_image;
    QString m_name;
};

// define E2FSCallHistoryNumber
E2FSCallHistoryNumber::E2FSCallHistoryNumber(QWidget *parent)
: QWidget(parent)
{
}

void E2FSCallHistoryNumber::setNumber(const QCallListItem &item)
{
    m_item = item;

    QContact cnt = m_contacts.contact(item.contact());
    if(cnt.uid().isNull())
        cnt = m_contacts.matchPhoneNumber(item.number());

    if (!cnt.uid().isNull()) {
        m_name = cnt.label();
        m_image = cnt.portrait();
        if(m_image.isNull())
            m_image = QPixmap(":image/samples/e2_contact");
    } else {
        m_image = QPixmap(":image/samples/e2_contact");
        m_name = QString();
    }

    if(m_image.width() > 44 || m_image.height() > 44)
        m_image = m_image.scaled(44, 44, Qt::KeepAspectRatio);

    update();
}

void E2FSCallHistoryNumber::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QFontMetrics fm(font());
    QString typeText;
    switch(m_item.type()) {
        case QCallListItem::Dialed:
            typeText = "Dialed Call";
            break;
        case QCallListItem::Received:
            typeText = "Answered Call";
            break;
        case QCallListItem::Missed:
            typeText = "Missed Call";
            break;
    }

    int margin = 10;
    int drawWidth = width() - margin;

    int uptoY = margin + 5;

    p.drawText(QRect(margin, uptoY, drawWidth, fm.height()),
               Qt::AlignVCenter | Qt::AlignLeft,
               typeText);

    uptoY += fm.height() + 5;

    // Image
    p.drawPixmap(margin, uptoY, m_image);

    // Name / number

    if(!m_name.isEmpty()) {
        // Name, then number

        p.drawText(QRect(margin + 44 + 5, uptoY,
                         drawWidth - 44 - 5, fm.height()),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   m_name);

        p.drawText(QRect(margin + 44 + 5, uptoY + fm.height() + 5,
                         drawWidth - 44 - 5, fm.height()),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   m_item.number());

    } else {
        // Number only
        p.drawText(QRect(margin + 44 + 5, uptoY,
                         drawWidth - 44 - 5, fm.height()),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   m_item.number());
    }

    uptoY += 44 + margin;

    p.setPen(GREY);
    p.drawLine(margin, uptoY, drawWidth, uptoY);
    p.setPen(REALLY_LIGHT_GREY);
    p.drawLine(margin, uptoY + 1, drawWidth, uptoY + 1);

    uptoY += 2 + margin;

    p.setPen(Qt::black);

    QString text;


    text = "Start Time: " + m_item.start().time().toString();
    p.drawText(QRect(margin, uptoY, drawWidth, fm.height()),
               Qt::AlignVCenter | Qt::AlignLeft,
               text);
    uptoY += fm.height();

    text = "Date: " + m_item.start().date().toString();
    p.drawText(QRect(margin, uptoY, drawWidth, fm.height()),
               Qt::AlignVCenter | Qt::AlignLeft,
               text);
    uptoY += fm.height();

    int secs = m_item.start().secsTo(m_item.end());
    QTime duration(0, 0, 0);
    duration = duration.addSecs(secs);
    text = "Duration: " + duration.toString();
    p.drawText(QRect(margin, uptoY, drawWidth, fm.height()),
               Qt::AlignVCenter | Qt::AlignLeft,
               text);
    uptoY += fm.height();
}

// declare E2FSCallHistoryScreen
class E2FSCallHistoryScreen : public QWidget
{
Q_OBJECT
public:
    enum Type { Main, Answered, Missed, Dialed };

    E2FSCallHistoryScreen(QWidget *parent = 0);

    void setType(Type type);
    Type type() const;

public slots:
    void updateScreen();

signals:
    void showNumber(const QCallListItem &);

private slots:
    void itemActivated();

protected:
    virtual void paintEvent(QPaintEvent *);

private:
    Type m_type;
    E2ListWidget *m_list;
    QList<QCallListItem> m_currentList;
    QContactModel m_contacts;
};

// define E2FSCallHistoryScreen
E2FSCallHistoryScreen::E2FSCallHistoryScreen(QWidget *parent)
: QWidget(parent), m_type(Main), m_list(0)
{
    QFontMetrics fm(font());
    setContentsMargins(0, fm.height() + 1, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_list = new E2ListWidget(this);
    m_list->setFrameShape(QFrame::NoFrame);
    layout->addWidget(m_list);
//    QObject::connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemActivated()));
    QObject::connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemActivated()));

    updateScreen();
}

E2FSCallHistoryScreen::Type E2FSCallHistoryScreen::type() const
{
    return m_type;
}

void E2FSCallHistoryScreen::setType(Type type)
{
    m_type = type;
    updateScreen();
}

void E2FSCallHistoryScreen::updateScreen()
{
    m_list->clear();
    m_currentList.clear();

    QCallList list;
    if(Main == m_type) {
        int dialed = 0;
        int received = 0;
        int missed = 0;

        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            switch(list.at(ii).type()) {
                case QCallListItem::Dialed:
                    ++dialed;
                    break;
                case QCallListItem::Received:
                    ++received;
                    break;
                case QCallListItem::Missed:
                    ++missed;
                    break;
            }
        }

        m_list->addItem("Answered Calls(" + QString::number(received) + ")");
        m_list->addItem("Missed Calls(" + QString::number(missed) + ")");
        m_list->addItem("Dialed Calls(" + QString::number(dialed) + ")");
    } else {
        QCallListItem::CallType match(QCallListItem::Missed);

        switch(m_type) {
            case Answered:
                match = QCallListItem::Received;
                break;

            case Missed:
                match = QCallListItem::Missed;
                break;

            case Dialed:
                match = QCallListItem::Dialed;
                break;
            default:
                Q_UNUSED(match);
                qFatal("Unknown call type");
                break;
        }

        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            QCallListItem item = list.at(ii);
            if(item.type() == match) {
                m_currentList.append(item);

                QString disp;
                disp = QString("%1:%2").arg(item.start().time().hour(), 2, 10, QChar('0')).arg(item.start().time().minute(), 2, 10, QChar('0'));
                disp += "  ";

                QString name;

                if(item.contact().isNull()) {
                    QContact contact = m_contacts.matchPhoneNumber(item.number());
                    if(contact.uid().isNull()) {
                        name = item.number();
                    } else {
                        name = contact.label();
                    }
                } else {
                    QContact contact = m_contacts.contact(item.contact());
                    name = contact.label();
                }

                disp += name;

                if(!name.isEmpty())
                    m_list->addItem(disp);
            }
        }
    }
    update();
}

void E2FSCallHistoryScreen::itemActivated()
{
    int currentRow = m_list->currentRow();
    if(-1 == currentRow)
        return;

    if(m_type == Main) {
        switch(currentRow) {
            case 0:
                setType(Answered);
                break;
            case 1:
                setType(Missed);
                break;
            case 2:
                setType(Dialed);
                break;
        }
    } else {
        Q_ASSERT(currentRow < m_currentList.count());
        emit showNumber(m_currentList.at(currentRow));
    }
}

void E2FSCallHistoryScreen::paintEvent(QPaintEvent *)
{
    QFontMetrics fm(font());
    QString text;
    switch(m_type) {
        case Main:
            text = "Recent Calls";
            break;
        case Answered:
            text = "Answered Calls";
            break;
        case Missed:
            text = "Missed Calls";
            break;
        case Dialed:
            text = "Dialed Calls";
            break;
    }

    QPainter p(this);
    p.drawText(QRect(0, 0, width(), fm.height()), Qt::AlignVCenter | Qt::AlignLeft, text);
    p.setPen(Qt::black);
    p.setPen(Qt::DotLine);
    p.drawLine(0, fm.height(), width(), fm.height());
}

// define E2FSCallHistory
E2FSCallHistory::E2FSCallHistory(QWidget *parent)
: QWidget(parent), m_selector(0), m_number(0), m_stack(0), m_menu(0),
  m_button(0), m_menuButton(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);
    m_selector = new E2FSCallHistoryScreen(m_stack);
    QObject::connect(m_selector, SIGNAL(showNumber(QCallListItem)), this, SLOT(showNumber(QCallListItem)));
    m_stack->addWidget(m_selector);

    m_number = new E2FSCallHistoryNumber(m_stack);
    m_stack->addWidget(m_number);

    E2Bar *m_bar = new E2Bar(this);
    m_bar->setFixedWidth(240);
    layout->addWidget(m_bar);

    m_menuButton = new E2Button(m_bar);
    m_menuButton->setPixmap(QPixmap(":image/samples/e2_menu"));
    m_bar->addButton(m_menuButton, 42);

    m_menu = new E2Menu(0);
    m_menu->addItem("Send SMS");
    m_menu->addItem("Send MMS");
    m_menu->addItem("Delete");
    QObject::connect(m_menu, SIGNAL(itemClicked(int)), this, SLOT(menuClicked(int)));

    m_button = new E2Button(m_bar);
    m_bar->addButton(m_button, 0);
    QObject::connect(m_button, SIGNAL(clicked()), this, SLOT(button()));
    m_button->setEnabled(false);

    E2Button *back = new E2Button(m_bar);
    back->setPixmap(QPixmap(":image/samples/e2_back"));
    m_bar->addButton(back, 42);
    QObject::connect(back, SIGNAL(clicked()), this, SLOT(back()));

    m_stack->setCurrentWidget(m_selector);

    QtopiaChannel *chan = new QtopiaChannel("QPE/E2", this);
    QObject::connect(chan, SIGNAL(received(QString,QByteArray)), this, SLOT(received(QString,QByteArray)));
}

void E2FSCallHistory::back()
{
    m_button->setText(QString());
    m_button->setEnabled(false);
    m_menuButton->setMenu(0);

    if(m_stack->currentWidget() == m_number) {
        m_stack->setCurrentWidget(m_selector);
    } else if(m_selector->type() == E2FSCallHistoryScreen::Main) {
        close();
    } else {
        m_selector->setType(E2FSCallHistoryScreen::Main);
    }
}

void E2FSCallHistory::showNumber(const QCallListItem &item)
{
    m_button->setText("Call");
    m_button->setEnabled(true);
    m_currentNumber = item;
    m_menuButton->setMenu(m_menu);
    m_number->setNumber(item);
    m_stack->setCurrentWidget(m_number);
}

void E2FSCallHistory::button()
{
    QtopiaServiceRequest req("Dialer", "dial(QString,QUniqueId)");
    req << m_currentNumber.number() << QUniqueId();
    req.send();
}

void E2FSCallHistory::menuClicked(int item)
{
    Q_ASSERT(m_stack->currentWidget() == m_number);
    if(0 == item) {
        // SMS
        QtopiaServiceRequest sms("SMS", "writeSms()");
        sms.send();
    } else if(1 == item) {
        // MMS
        QtopiaServiceRequest sms("Email", "writeEmail()");
        sms.send();
    } else if(2 == item) {
        // Delete
        QCallList list;
        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            if(list.at(ii) == m_currentNumber) {
                list.removeAt(ii);
                back();
                m_selector->updateScreen();
                return;
            }
        }
    }
}

void E2FSCallHistory::received(const QString &message, const QByteArray &)
{
    if(message == "showLastMissedCall()") {
        m_selector->setType(E2FSCallHistoryScreen::Missed);
        m_stack->setCurrentWidget(m_selector);

        // Locate actual last missed call
        QCallList list;
        QDateTime latest(QDate(1, 1, 1900), QTime(0,0,0));
        QCallListItem item;

        for(int ii = 0; (unsigned)ii < list.count(); ++ii) {
            QCallListItem c = list.at(ii);
            if(c.type() == QCallListItem::Missed && c.start() > latest) {
                item = c;
                latest = c.start();
            }
        }

        if(!item.isNull())
            showNumber(item);
    }
}

// define E2NewMessage
E2NewMessage::E2NewMessage()
: E2TitleFrame(E2TitleFrame::NoTitle, 0), m_fetching(false), m_messageCount(-1)
{
    smsReq = new QSMSReader( QString(), this );
    QObject::connect( smsReq, SIGNAL(messageCount(int)), this, SLOT(messageCount(int)) );
    QObject::connect( smsReq, SIGNAL(fetched(QString,QSMSMessage)), this, SLOT(fetched(QString,QSMSMessage)) );

    E2Button *but = new E2Button(bar());
    but->setText("Cancel");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(close()));
    bar()->addButton(but, 0);
    but = new E2Button(bar());
    but->setText("Read");
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(readClicked()));
    bar()->addButton(but, 0);

    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_image = new QLabel(this);
    layout->addWidget(m_image);
    m_image->setFixedSize(44, 44);

    m_message = new QLabel(this);
    m_message->setWordWrap(true);
    m_message->setMaximumWidth(180 - 44);
    layout->addWidget(m_message);

    setFixedWidth(180);
    smsReq->check();
}

void E2NewMessage::readClicked()
{
    QtopiaServiceRequest req("SMS", "viewSms()");
    req.send();
    close();
}

void E2NewMessage::displayMessage(const QSMSMessage &message)
{
    QString name;
    QPixmap pix;
    QContactModel m;
    QContact cnt = m.matchPhoneNumber(message.sender());

    if(cnt.uid().isNull()) {
        name = message.sender();
        pix = QPixmap(":image/samples/e2_contact");
    } else {
        name = cnt.label();
        pix = cnt.portrait();
        if(pix.isNull()) {
            pix = QPixmap(":image/samples/e2_contact");
        } else {
            if(pix.width() > 44 || pix.height() > 44) {
                pix = pix.scaled(44, 44, Qt::KeepAspectRatio);
            }
        }
    }

    m_message->setText("New incoming message from " + name);
    m_image->setPixmap(pix);

    show();
    e2Center(this);
}

void E2NewMessage::fetched(const QString&, const QSMSMessage& m)
{
    if(m_fetching) {
        m_fetching = false;
        displayMessage(m);
    }
}

void E2NewMessage::messageCount( int total )
{
    if(-1 == m_messageCount) {
        m_messageCount = total;
    } else if(total <= m_messageCount) {
        m_messageCount = total;
    } else {
        // total > m_messageCount
        m_messageCount = total;
        if(!m_fetching) {
            m_fetching = true;
            smsReq->firstMessage();
        }
    }
}

#include "e2_callscreen.moc"
