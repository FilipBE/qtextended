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

#include "display.h"
#include <qtimestring.h>
#include <qphonestatus.h>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <qlayout.h>
#include <QVariant>
#include <QMouseEvent>
#include <QDesktopWidget>

/* XPM */
static const char * bell_xpm[] = {
"10 10 2 1",
"       c None",
".      c #000000",
"    ..    ",
"   ....   ",
"  ......  ",
"  ......  ",
"  ......  ",
" ........ ",
"..........",
"..........",
"    ..    ",
"    ..    "};

class PhoneDisplay : public QWidget
{
    Q_OBJECT
public:
    PhoneDisplay(QWidget *parent);

    void setSignalLevel(int l) { signalLevel = l; }
    void setBatteryLevel(int l) { batteryLevel = l; }
    void setMissedCalls(int c) { missed = c; }
    void setNewMessages(int c) { messages = c; }
    void setAlarm(bool a) { alarm = a; }
    void setIncoming(const QString &number, const QString &name) {
        callNumber = number;
        callName = name;
    }

    QSize sizeHint() const {
        return QSize(30, 22);
    }

protected:
    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);
    void drawSignal(QPainter *p, const QRect &r);
    void drawBattery(QPainter *p, const QRect &r);

private:
    int batteryLevel;
    int signalLevel;
    int missed;
    int messages;
    bool alarm;
    QString callNumber;
    QString callName;
    int tid;
};

PhoneDisplay::PhoneDisplay(QWidget *parent)
    : QWidget(parent), batteryLevel(0), signalLevel(0), missed(0), messages(0)
{
    tid = startTimer(60*1000);
}

void PhoneDisplay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPixmap pm(size());
    QPainter pmp(&pm);
    pmp.fillRect(rect(), QColor(192,208,192));
    if (alarm)
        pmp.drawPixmap(30, 1, QPixmap(bell_xpm));
    drawSignal(&pmp, QRect(40, 1, 20, 10));
    drawBattery(&pmp, QRect(60, 1, 20, 10));
    QString str;
    if (!callNumber.isEmpty()) {
        if (callName.isEmpty())
            str = callNumber;
        else
            str = callName;
    } else if (missed) {
        str = tr("Unanswered");
    } else if (messages) {
        str = tr("New Message");
    } else {
        str = QTimeString::localHM(QTime::currentTime(), QTimeString::Short);
        str += " ";
        str += QTimeString::localMD(QDate::currentDate(), QTimeString::Short);
    }
    pmp.drawText(0, 12, width(), 20, Qt::AlignCenter, str);
    p.drawPixmap(0,0,pm);
}

void PhoneDisplay::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == tid) {
        update();
    }
}

void PhoneDisplay::drawSignal(QPainter *p, const QRect &r)
{
    p->drawRect(r.x()+3, r.y(), 2, r.height());
    for (int i = 0; i < 4; i++)
        p->drawLine(r.x()+i, r.y()+i, r.x()+7-i, r.y()+i);
    int bars = 4*signalLevel/100;
    int pos = 6;
    for (int i = 0; i < bars; i++) {
        int ht = r.height() * (i+1) / 4;
        p->drawRect(r.x() + pos, r.y()+r.height()-ht, 2, ht);
        pos += 3;
    }
}

void PhoneDisplay::drawBattery(QPainter *p, const QRect &r)
{
    int w = 20;
    p->drawRect(r.x(), r.y(), w-2, r.height());
    p->drawRect(r.x()+w-2, r.y()+2, 2, r.height()-4);
    int bars = 100*batteryLevel/5;
    int pos = r.x() + 2;
    for (int i = 0; i < bars; i++) {
        p->drawRect(pos, r.y()+2, 2, r.height()-4);
        pos += 3;
    }
}

//---------------------------------------------------------------------------

class TitleBar : public QWidget
{
public:
    TitleBar(QWidget *parent) : QWidget(parent) {
        setFixedHeight(6);
        setBackgroundRole(QPalette::Highlight);
    }

    QSize sizeHint() const {
        return QSize(30, 6);
    }

protected:
    void mousePressEvent(QMouseEvent *e) {
        mousePos = e->globalPos();
    }
    void mouseMoveEvent(QMouseEvent *e) {
        QPoint diff = e->globalPos() - mousePos;
        mousePos = e->globalPos();
        topLevelWidget()->move(topLevelWidget()->pos() + diff);
    }

private:
    QPoint mousePos;
};

//---------------------------------------------------------------------------

BasicDisplay::BasicDisplay(QWidget *parent, Qt::WFlags f)
    : QFrame(parent, f | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
{
    setFrameStyle(Plain|Box);
    status = new QPhoneStatus(this);
    connect(status, SIGNAL(statusChanged()), this, SLOT(statusChanged()));
    connect(status, SIGNAL(incomingCall(QString,QString)),
        this, SLOT(incomingCall(QString,QString)));
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(1);
    TitleBar *tb = new TitleBar(this);
    vb->addWidget(tb);
    display = new PhoneDisplay(this);
    vb->addWidget(display);
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desk = desktop->availableGeometry(desktop->primaryScreen());
    setFixedSize(83, 46);
    setGeometry((desk.width()-83)/2, (desk.height()-46)/2, 83, 46);

}

BasicDisplay::~BasicDisplay()
{
}

void BasicDisplay::statusChanged()
{
    QVariant value;
    value = status->value(QPhoneStatus::BatteryLevel);
    if (value.isValid())
        display->setBatteryLevel(value.toInt());
    value = status->value(QPhoneStatus::SignalLevel);
    if (value.isValid())
        display->setSignalLevel(value.toInt());
    value = status->value(QPhoneStatus::MissedCalls);
    if (value.isValid())
        display->setMissedCalls(value.toInt());
    value = status->value(QPhoneStatus::NewMessages);
    if (value.isValid())
        display->setNewMessages(value.toInt());
    value = status->value(QPhoneStatus::Alarm);
    if (value.isValid())
        display->setAlarm(value.toBool());
    display->update();
}

void BasicDisplay::incomingCall(const QString &number, const QString &name)
{
    display->setIncoming(number, name);
}


#include "display.moc"
