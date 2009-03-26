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

#include "e1_telephony.h"
#include <QVBoxLayout>
#include "e1_callscreen.h"
#include "e1_error.h"
#include "e1_dialer.h"
#include "e1_bar.h"
#include "themecontrol.h"
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif
#include <QTimerEvent>
#include <QDebug>
#include "dialerservice.h"
#include <QString>

class E1DialerServiceProxy : public DialerService
{
Q_OBJECT
public:
    E1DialerServiceProxy(QObject *parent)
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
    virtual void setDialToneOnlyHint( const QString & /*app*/ ) {}
    virtual void redial() {}
};

E1Telephony::E1Telephony(QWidget *parent)
: QWidget(parent), m_state(Dialer), m_scrollStep(0)
{
    m_tbar = new E1PhoneTelephonyBar(this);

    m_bar = new E1Bar(this);
    E1CloseButton *but = new E1CloseButton;
    but->setMargin(3);
    m_bar->addItem(but);
    m_bar->addSeparator();
    E1Button *middleButton = new E1Button;
    middleButton->setMargin(3);
    m_bar->addItem(middleButton);
    middleButton->setFlag(E1Button::Expanding);
    m_bar->addSeparator();
    E1Menu *context = new E1Menu;
    context->setMargin(3);
    context->setPixmap(QPixmap(":image/samples/e1_context"));
    m_bar->addItem(context);

    m_callscreen = new E1Callscreen(middleButton, this);
    m_callscreen->hide();
    QObject::connect(m_callscreen, SIGNAL(toDialer()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(closeMe()),
                     this, SLOT(slideToDialer()));
    QObject::connect(m_callscreen, SIGNAL(showMe()),
                     this, SLOT(popupCallscreen()));

    m_dialer = new E1Dialer(middleButton, this);
    m_dialer->hide();
    QObject::connect(m_dialer, SIGNAL(toCallScreen()),
                     this, SLOT(slideToCallscreen()));
//    ThemeControl::instance()->registerThemedView( m_dialer, "Dialer" );

    QObject::connect(m_dialer, SIGNAL(sendNumber(QString)),
                     m_callscreen, SLOT(sendNumber(QString)));

    // Listen to header channel
    QCopChannel* channel = new QCopChannel( "QPE/E1", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(message(QString,QByteArray)) );

    E1DialerServiceProxy *proxy = new E1DialerServiceProxy(this);
    QObject::connect(proxy, SIGNAL(doShowDialer(QString)),
                     this, SLOT(doShowDialer(QString)));
}

void E1Telephony::showEvent(QShowEvent *)
{
    doLayout();
}

void E1Telephony::doLayout()
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

void E1Telephony::timerEvent(QTimerEvent *e)
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

void E1Telephony::message(const QString &message, const QByteArray &)
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
    else if(message == "error()") {
        E1Error::error("Hello world!");
    }

}

void E1Telephony::display()
{
    showMaximized();
    raise();
    if(m_state == Dialer)
        m_dialer->setActive();
    else
        m_callscreen->setActive();
}

void E1Telephony::slideToDialer()
{
    m_state = Dialer;
    m_scrollStep = m_scrollSteps;
    startTimer(m_scrollStepTime);
    doLayout();
    m_dialer->setActive();
}

void E1Telephony::slideToCallscreen()
{
    m_state = CallScreen;
    m_scrollStep = 0;
    startTimer(m_scrollStepTime);
    doLayout();
    m_callscreen->setActive();
}

void E1Telephony::popupDialer()
{
    m_state = Dialer;
    m_scrollStep = 0;
    doLayout();
    m_dialer->setActive();
}

void E1Telephony::popupCallscreen()
{
    m_state = CallScreen;
    m_scrollStep = m_scrollSteps;
    doLayout();
    m_callscreen->setActive();
}

void E1Telephony::doShowDialer(const QString &num)
{
    popupDialer();
    display();
    m_dialer->setNumber(num);
}

#include "e1_telephony.moc"
