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

#include "e1_callscreen.h"
#include <QVBoxLayout>
#include <QFont>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include "e1_bar.h"
#include <qcopchannel_qws.h>
#include <QSizePolicy>
#include <QTimer>
#include "dialercontrol.h"
#include <qcontactmodel.h>
#include <qcontact.h>
#include <QPalette>
#include "e1_error.h"
#include <QPainter>

E1Callscreen::E1Callscreen(E1Button *b, QWidget *parent)
: QWidget(parent), m_timer(0), timeLabel(0), stateLabel(0),
  m_button(b)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);

    m_state = "Connected";

    QHBoxLayout *dialerLayout = new QHBoxLayout;
    vbox->addLayout(dialerLayout);

    E1Bar *dialerBar = new E1Bar(this);
    dialerBar->setBorder(E1Bar::ButtonBorder);
    E1Button *dialer = new E1Button;
    dialer->setFlag(E1Button::Expanding);
    dialer->setPixmap(QPixmap(":image/samples/e1_dialer"));
    dialerBar->addItem(dialer);
    dialerBar->setFixedSize(32, 96);
    dialerLayout->addWidget(dialerBar);
    QObject::connect(dialer, SIGNAL(clicked()), this, SIGNAL(toDialer()));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    dialerLayout->addLayout(buttonLayout);

    buttonLayout->insertStretch(-1);

    QHBoxLayout *midbox = new QHBoxLayout;
    buttonLayout->addLayout(midbox);
    nameLabel = new QLabel(this);
    imageLabel = new QLabel(this);
    imageLabel->setPixmap(QPixmap(":image/addressbook/generic-contact"));
    midbox->addWidget(imageLabel);
    midbox->addWidget(nameLabel);

    buttonLayout->insertStretch(-1);

    QHBoxLayout *buttonBox = new QHBoxLayout;
    buttonLayout->addLayout(buttonBox);

    E1Bar *holdBar = new E1Bar(this);
    holdBar->setBorder(E1Bar::ButtonBorder);
    E1Button *hold = new E1Button;
    hold->setFlag(E1Button::Expanding);
    hold->setText("Hold");
    holdBar->addItem(hold);
    holdBar->setFixedHeight(32);
    buttonBox->addWidget(holdBar);
    QObject::connect(hold, SIGNAL(clicked()), this, SLOT(hold()));

    E1Bar *muteBar = new E1Bar(this);
    muteBar->setBorder(E1Bar::ButtonBorder);
    E1Button *mute = new E1Button;
    mute->setFlag(E1Button::Expanding);
    mute->setText("Mute");
    muteBar->addItem(mute);
    muteBar->setFixedHeight(32);
    buttonBox->addWidget(muteBar);
    QObject::connect(mute, SIGNAL(clicked()), this, SLOT(mute()));

    E1Bar *spkrBar = new E1Bar(this);
    spkrBar->setBorder(E1Bar::ButtonBorder);
    E1Button *spkr = new E1Button;
    spkr->setFlag(E1Button::Expanding);
    spkr->setText("Spkr");
    spkrBar->addItem(spkr);
    spkrBar->setFixedHeight(32);
    buttonBox->addWidget(spkrBar);
    QObject::connect(spkr, SIGNAL(clicked()), this, SLOT(spkr()));

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

void E1Callscreen::callConnected( const QPhoneCall &call )
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

void E1Callscreen::callIncoming( const QPhoneCall &call )
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

void E1Callscreen::callDialing( const QPhoneCall &call )
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

void E1Callscreen::callEnded(const QPhoneCall &call)
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

void E1Callscreen::paintEvent(QPaintEvent *)
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

    p.drawText(5, stateRect.height() - 2, m_state);
    p.drawText(width() - 1 - timeRect.width() - 5, stateRect.height() - 2, m_time);

}

void E1Callscreen::timerEvent(QTimerEvent *)
{
    updateCallTime();
}

void E1Callscreen::updateCallTime()
{
    int elapsed = QDateTime::currentDateTime().toTime_t() -
                  m_callTime.toTime_t();

    int min = elapsed / 60;
    int sec = elapsed % 60;

    m_time = QString("%1:%2").arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    update();
}

void E1Callscreen::updateInfo(const QPhoneCall &call)
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

    if (!cnt.uid().isNull()) 
        m_name = cnt.label();

    m_image = cnt.thumbnail();

    nameLabel->setText(m_name);
    imageLabel->setPixmap(m_image);
}

void E1Callscreen::setActive()
{
    if(isVisible()) {
        if(m_hasCall.isEmpty())
            m_button->setText(" ");
        else
            m_button->setText("End");
    }
}

void E1Callscreen::mute()
{
    E1Error::error("Mute not supported.");
}

void E1Callscreen::spkr()
{
    E1Error::error("Speaker not supported.");
}

void E1Callscreen::hold()
{
    E1Error::error("Hold not supported.");
}

void E1Callscreen::end()
{
    if(isVisible())
        DialerControl::instance()->endAllCalls();
}

void E1Callscreen::sendNumber(const QString &number)
{
    Q_UNUSED(number)
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

