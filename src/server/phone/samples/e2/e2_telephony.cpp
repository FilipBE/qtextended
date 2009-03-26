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

#include "e2_telephony.h"
#include <QVBoxLayout>
#include "e2_callscreen.h"
#include "e2_dialer.h"
#include "e2_bar.h"
#include "themecontrol.h"
#include <qcopchannel_qws.h>
#include <QTimerEvent>
#include <QDebug>
#include "dialerservice.h"
#include <QString>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif
#include "e2_telephonybar.h"

class E2DialerServiceProxy : public DialerService
{
Q_OBJECT
public:
    E2DialerServiceProxy(QObject *parent)
        : DialerService(parent)
    {}

signals:
    void doShowDialer(const QString &);

protected:
    virtual void dialVoiceMail() {}
    virtual void dial( const QString&, const QString& number ) { emit doShowDialer(number); }
    virtual void dial( const QString& number, const QUniqueId&) { emit doShowDialer(number); }
    virtual void showDialer( const QString& digits ) { emit doShowDialer(digits); }
    virtual void onHook() {}
    virtual void offHook() {}
    virtual void headset() {}
    virtual void speaker() {}
    virtual void setDialToneOnlyHint( const QString & ) {}
    virtual void redial() {}
};

E2Telephony::E2Telephony(QWidget *parent)
: QWidget(parent), m_state(Dialer), m_scrollStep(0)
{
    m_tbar = new E2TelephonyBar(this);
    m_tbar->setFixedWidth(240);

    m_bar = new E2Bar(this);
    m_bar->setFixedWidth(240);
    E2Button *context = new E2Button(m_bar);
    context->setPixmap(QPixmap(":image/samples/e2_menu"));
    m_bar->addButton(context, 42);

    E2Button *middleButton = new E2Button(m_bar);
    m_bar->addButton(middleButton, 0);

    E2Button *but = new E2Button(m_bar);
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(close()));
    but->setPixmap(QPixmap(":image/samples/e2_cross"));
    m_bar->addButton(but, 42);


    m_callscreen = new E2CallScreen(middleButton, this);
    m_callscreen->hide();
    QObject::connect(m_callscreen, SIGNAL(toDialer()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(closeMe()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(showMe()),
                     this, SLOT(popupCallscreen()));

    m_dialer = new E2Dialer(middleButton, this);
    m_dialer->hide();

    QObject::connect(m_dialer, SIGNAL(toCallscreen()),
                     this, SLOT(slideToCallscreen()));
    QObject::connect(m_dialer, SIGNAL(sendNumber(QString)),
                     m_callscreen, SLOT(sendNumber(QString)));

    // Listen to header channel
    QCopChannel* channel = new QCopChannel( "QPE/E2", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(message(QString,QByteArray)) );

    E2DialerServiceProxy *proxy = new E2DialerServiceProxy(this);
    QObject::connect(proxy, SIGNAL(doShowDialer(QString)),
                     this, SLOT(doShowDialer(QString)));
}

void E2Telephony::showEvent(QShowEvent *)
{
    doLayout();
}

void E2Telephony::doLayout()
{
    m_tbar->setGeometry(0, 0, width(), 26);

    QRect geom(0, 27, width(), height() - 26 - 32 - 1);

    if(m_scrollStep == 0) {
        m_dialer->setGeometry(geom);
        m_dialer->show();
        m_callscreen->hide();
        m_dialer->setActive();
    } else if(m_scrollStep == m_scrollSteps) {
        m_callscreen->setGeometry(geom);
        m_dialer->hide();
        m_callscreen->show();
        m_callscreen->setActive();
    } else {
        // Somewhere in the middle
        QRect dialerGeom(-1 *((m_scrollStep * width()) / m_scrollSteps),
                         27, width(), height() - 26 - 32 - 1);
        QRect callGeom(width() + -1 *((m_scrollStep * width()) / m_scrollSteps),
                         27, width(), height() - 26 - 32 - 1);

        m_dialer->setGeometry(dialerGeom);
        m_callscreen->setGeometry(callGeom);
        m_dialer->show();
        m_callscreen->show();
    }

    m_bar->setGeometry(0, height() - 32, width(), 32);
}

void E2Telephony::timerEvent(QTimerEvent *e)
{
    if(m_state == Dialer) {

        if(m_scrollStep > 0)
            --m_scrollStep;
        if(!m_scrollStep)
            killTimer(e->timerId());
        doLayout();

    } else if(m_state == CallScreen) {

        if(m_scrollStep < m_scrollSteps)
            ++m_scrollStep;
        if(m_scrollStep == m_scrollSteps)
            killTimer(e->timerId());
        doLayout();

    }
}

void E2Telephony::message(const QString &message, const QByteArray &)
{
    if(message == "showTelephony()")
        display();
    else if(message == "showCallscreen()")
        popupCallscreen();
    else if(message == "showDialer()")
        popupDialer();
    else if(message == "slideToDialer()")
        slideToDialer();
    else if(message == "slideToCallscreen()")
        slideToCallscreen();
}

void E2Telephony::display()
{
    showMaximized();
    raise();
    if(m_state == Dialer)
        m_dialer->setActive();
    else
        m_callscreen->setActive();
}

void E2Telephony::slideToDialer()
{
    if(Dialer == m_state)
        return;
    m_state = Dialer;
    m_scrollStep = m_scrollSteps;
    startTimer(m_scrollStepTime);
    doLayout();
    m_dialer->setActive();
}

void E2Telephony::slideToCallscreen()
{
    if(CallScreen == m_state)
        return;
    m_state = CallScreen;
    m_scrollStep = 0;
    startTimer(m_scrollStepTime);
    doLayout();
    m_callscreen->setActive();
}

void E2Telephony::popupDialer()
{
    m_state = Dialer;
    m_scrollStep = 0;
    doLayout();
    m_dialer->setActive();
}

void E2Telephony::popupCallscreen()
{
    m_state = CallScreen;
    m_scrollStep = m_scrollSteps;
    doLayout();
    m_callscreen->setActive();
}

void E2Telephony::doShowDialer(const QString &num)
{
    popupDialer();
    display();
    m_dialer->setNumber(num.trimmed());
}

#include "e2_telephony.moc"
