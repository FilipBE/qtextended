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

#include "e2_header.h"
#include "e2_bar.h"
#include <QPainter>
#include <QtopiaServiceRequest>
#include "windowmanagement.h"
#include "dialercontrol.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtopiaIpcEnvelope>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

// declare E2AlertScreen
class E2AlertScreen : public QWidget
{
Q_OBJECT
public:
    E2AlertScreen(QWidget *parent = 0);

public slots:
    void refresh();

signals:
    void showMe(bool);

private slots:
    void clearAll();
    void itemClicked();

private:
    QValueSpaceItem m_messages;
    QValueSpaceItem m_calls;
    E2ListWidget *m_list;
    int m_newMessagesCount;

    int m_messageItem;
    int m_missedItem;
};

// define E2AlertScreen
E2AlertScreen::E2AlertScreen(QWidget *parent)
: QWidget(parent),
  m_messages("/Communications/Messages/NewMessages"),
  m_calls("/Communications/Calls/MissedCalls"),
  m_newMessagesCount(0), m_messageItem(-1), m_missedItem(-1)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    m_list = new E2ListWidget(this);
    layout->addWidget(m_list);

    E2Bar *m_bar = new E2Bar(this);
    m_bar->setFixedWidth(240);
    layout->addWidget(m_bar);

    E2Button *but = new E2Button(m_bar);
    but->setPixmap(QPixmap(":image/samples/e2_menu"));
    E2Menu *menu = new E2Menu(0);
    menu->addItem("Clear All");
    QObject::connect(menu, SIGNAL(itemClicked(int)), this, SLOT(clearAll()));
    but->setMenu(menu);
    m_bar->addButton(but, 42);

    but = new E2Button(m_bar);
    but->setEnabled(false);
    m_bar->addButton(but, 0);

    but = new E2Button(m_bar);
    but->setPixmap(QPixmap(":image/samples/e2_back"));
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(close()));
    m_bar->addButton(but, 42);

    QObject::connect(&m_messages, SIGNAL(contentsChanged()),
                     this, SLOT(refresh()));
    QObject::connect(&m_calls, SIGNAL(contentsChanged()),
                     this, SLOT(refresh()));

    QObject::connect(m_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked()));
    QObject::connect(m_list, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemClicked()));
}

void E2AlertScreen::refresh()
{
    int messages = m_messages.value().toInt();
    if(messages < m_newMessagesCount)
        m_newMessagesCount = messages;

    int missed = m_calls.value().toInt();

    if(missed || (messages - m_newMessagesCount)) {
        emit showMe(true);
    } else {
        emit showMe(false);
        close();
    }

    m_list->clear();
    if(messages - m_newMessagesCount) {
        m_list->addItem(QString::number(messages - m_newMessagesCount) + " new message" + (((messages - m_newMessagesCount) == 1)?QString():QString("s")));
        m_messageItem = 0;
    } else {
        m_messageItem = -1;
    }

    if(missed) {
        m_list->addItem(QString::number(missed) + " missed call" + ((missed == 1)?QString():QString("s")));
        m_missedItem = m_messageItem + 1;
    } else {
        m_missedItem = -1;
    }
}

void E2AlertScreen::clearAll()
{
    DialerControl::instance()->resetMissedCalls();
    m_newMessagesCount = m_messages.value().toInt();
    emit showMe(false);
    close();
}

void E2AlertScreen::itemClicked()
{
    int c = m_list->currentRow();

    if(c == -1) return;
    if(c == m_missedItem) {
        DialerControl::instance()->resetMissedCalls();
        { QtopiaIpcEnvelope env("QPE/Application/callhistory", "raise()"); }
        { QtopiaIpcEnvelope env("QPE/E2", "showLastMissedCall()"); }
        refresh();
    }
    if(c == m_messageItem) {
        m_newMessagesCount = m_messages.value().toInt();
        QtopiaServiceRequest req("SMS", "viewSms()");
        req.send();
        refresh();
    }
}

// declare E2HeaderButton
class E2HeaderButton : public QWidget
{
Q_OBJECT
public:
    E2HeaderButton(const QString &name, QWidget *parent);

    void setSmallMode(bool);

signals:
    void clicked(const QString &);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);

protected:
    bool m_smallMode;
    bool m_pressed;
    QString m_name;
    QPixmap m_back;
    QPixmap m_pressedBack;
    QPixmap m_smallBack;
    QPixmap m_smallPressedBack;

    QPixmap m_image;
    QPixmap m_pressedImage;
};

E2HeaderButton::E2HeaderButton(const QString &name, QWidget *parent)
: QWidget(parent), m_smallMode(false), m_pressed(false), m_name(name),
  m_back(":image/samples/e2_headerlarge"),
  m_pressedBack(":image/samples/e2_headerlarge_pressed"),
  m_smallBack(":image/samples/e2_headersmall"),
  m_smallPressedBack(":image/samples/e2_headersmall_pressed")
{
    setSmallMode(false);
    m_image = QPixmap(":image/samples/e2_" + name);
    m_pressedImage = QPixmap(":image/samples/e2_" + name + "_pressed");
}

void E2HeaderButton::mousePressEvent(QMouseEvent *)
{
    m_pressed = true;
    update();
}

void E2HeaderButton::mouseReleaseEvent(QMouseEvent *e)
{
    if(rect().contains(e->pos()))
        emit clicked(m_name);

    m_pressed = false;
    update();
}

void E2HeaderButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if(m_smallMode) {
        if(m_pressed) {
            p.drawPixmap(0, 0, m_smallPressedBack);
        } else {
            p.drawPixmap(0, 0, m_smallBack);
        }
    } else {
        if(m_pressed) {
            p.drawPixmap(0, 0, m_pressedBack);
        } else {
            p.drawPixmap(0, 0, m_back);
        }
    }

    if(m_pressed) {
        p.drawPixmap((width() - m_pressedImage.width()) / 2,
                     (height() - m_pressedImage.height()) / 2,
                     m_pressedImage);
    } else {
        p.drawPixmap((width() - m_image.width()) / 2,
                     (height() - m_image.height()) / 2,
                     m_image);
    }
}

void E2HeaderButton::setSmallMode(bool smallMode)
{
    m_smallMode = smallMode;
    if(m_smallMode) {
        setFixedSize(m_smallBack.width(), m_smallBack.height());
    } else {
        setFixedSize(m_back.width(), m_back.height());
    }
    update();
}

// declare E2HeaderAlertButton
class E2HeaderAlertButton : public E2HeaderButton
{
Q_OBJECT
public:
    E2HeaderAlertButton(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

private slots:
    void toggle();

private:
    bool m_alertToggle;
    QTimer m_timer;
    QPixmap m_alert1;
    QPixmap m_alert2;
};

// define E2HeaderAlertButton
E2HeaderAlertButton::E2HeaderAlertButton(QWidget *parent)
: E2HeaderButton("alert", parent), m_alertToggle(false),
  m_alert1(":image/samples/e2_alert_1"),
  m_alert2(":image/samples/e2_alert_2")
{
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(toggle()));
}

void E2HeaderAlertButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    if(m_smallMode) {
        if(m_pressed) {
            p.drawPixmap(0, 0, m_smallPressedBack);
        } else {
            p.drawPixmap(0, 0, m_smallBack);
        }
    } else {
        if(m_pressed) {
            p.drawPixmap(0, 0, m_pressedBack);
        } else {
            p.drawPixmap(0, 0, m_back);
        }
    }

    if(m_alertToggle) {
        p.drawPixmap((width() - m_alert1.width()) / 2,
                     (height() - m_alert1.height()) / 2,
                     m_alert1);
    } else {
        p.drawPixmap((width() - m_alert2.width()) / 2,
                     (height() - m_alert2.height()) / 2,
                     m_alert2);
    }
}

void E2HeaderAlertButton::showEvent(QShowEvent *e)
{
    m_timer.start(300);
    E2HeaderButton::showEvent(e);
}

void E2HeaderAlertButton::hideEvent(QHideEvent *e)
{
    m_timer.stop();
    E2HeaderButton::hideEvent(e);
}

void E2HeaderAlertButton::toggle()
{
    if(m_alertToggle) m_alertToggle = false;
    else m_alertToggle = true;
    update();
}

// define E2Header
E2Header::E2Header(QWidget *parent)
: QWidget(parent, Qt::FramelessWindowHint | Qt::Tool |
                  Qt::WindowStaysOnTopHint),
  m_fillBrush(":image/samples/e2_header"),
  m_alertScreen(0)
{
    setFixedHeight(m_fillBrush.height());

    WindowManagement::dockWindow(this, WindowManagement::Top,
                                             QSize(240, m_fillBrush.height()));
    WindowManagement::protectWindow(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    E2HeaderButton *but = new E2HeaderButton("home", this);
    m_buttons.append(but);
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    but = new E2HeaderButton("contacts", this);
    m_buttons.append(but);
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));

    but = new E2HeaderAlertButton(this);
    m_buttons.append(but);
    m_alert = but;
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(alertClicked()));
    m_alert->hide();

    but = new E2HeaderButton("email", this);
    m_buttons.append(but);
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));

    but = new E2HeaderButton("phone", this);
    m_buttons.append(but);
    m_phone = but;
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));

    but = new E2HeaderButton("phoneactive", this);
    m_buttons.append(but);
    m_phoneActive = but;
    layout->addWidget(but);
    QObject::connect(but, SIGNAL(clicked(QString)), this, SLOT(clicked(QString)));
    m_phoneActive->hide();

    connect(DialerControl::instance(), SIGNAL(activeCount(int)),
            this, SLOT(activeCallCount(int)));

    m_alertScreen = new E2AlertScreen(0);
    QObject::connect(m_alertScreen, SIGNAL(showMe(bool)), this, SLOT(setAlertEnabled(bool)));
}

void E2Header::clicked(const QString &name)
{
    if("home" == name) {
        QtopiaIpcEnvelope env("QPE/E2", "showHome()");
    } else if("contacts" == name) {
        QtopiaIpcEnvelope env("QPE/Application/addressbook", "raise()");
    } else if("email" == name) {
        QtopiaIpcEnvelope env("QPE/Application/qtmail", "raise()");
    } else if("phone" == name || "phoneactive" == name) {
        QtopiaIpcEnvelope env("QPE/E2", "showTelephony()");
    }
}

void E2Header::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawTiledPixmap(rect(), m_fillBrush);
}

void E2Header::activeCallCount(int c)
{
    if(c) {
        m_phone->hide();
        m_phoneActive->show();
    } else {
        m_phoneActive->hide();
        m_phone->show();
    }
}

void E2Header::setAlertEnabled(bool e)
{
    for(int ii = 0; ii < m_buttons.count(); ++ii)
        m_buttons[ii]->setSmallMode(e);

    if( e ) {
        m_alert->show();
    } else {
        m_alert->hide();
    }
}

void E2Header::alertClicked()
{
    m_alertScreen->showMaximized();
    m_alertScreen->raise();
}

#include "e2_header.moc"
