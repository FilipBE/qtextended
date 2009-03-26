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

#ifndef E2_CALLSCREEN_H
#define E2_CALLSCREEN_H

#include <QWidget>
#include <QPhoneCall>
#include "e2_frames.h"
#include <QContactModel>
#include <QPhoneCall>
#include <QSMSReader>
#include <QCallListItem>
#include <QSMSMessage>
#include <QValueSpaceItem>

class QStackedWidget;
class QLabel;
class E2Button;
class QCallListItem;
class E2Menu;
class E2CallScreen : public QWidget
{
Q_OBJECT
public:
    E2CallScreen(E2Button *b, QWidget *parent = 0, Qt::WFlags flags = 0);

    void setActive();

signals:
    void toDialer();
    void closeMe();
    void showMe();

public slots:
    void sendNumber(const QString &);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void timerEvent(QTimerEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private slots:
    void callConnected( const QPhoneCall &call );
    void callIncoming( const QPhoneCall &call );
    void callDialing( const QPhoneCall &call );
    void callEnded(const QPhoneCall &call);
    void end();

private:
    void updateCallTime();
    void updateInfo(const QPhoneCall &);
    int m_timer;
    QString m_hasCall;

    QString m_name;
    QPixmap m_image;
    QDateTime m_callTime;
    QString m_state;
    QString m_time;

    QRect toDialerRect() const;
    QPixmap m_toDialer;
    QPixmap m_buttons;

    E2Button *m_button;
};

class E2CallHistory : public E2TitleFrame
{
Q_OBJECT
public:
   E2CallHistory(QWidget *parent);

private slots:
    void itemActivated();
    void cancelClicked();

private:
    void updateState();
    enum { Home, Answered, Missed, Dialed } m_state;
    E2ListWidget *m_list;
    QStringList m_currentList;
    QContactModel m_contacts;
};

class E2FSCallHistoryScreen;
class E2FSCallHistoryNumber;
class E2FSCallHistory : public QWidget
{
Q_OBJECT
public:
    E2FSCallHistory(QWidget *parent = 0);

private slots:
    void back();
    void button();
    void showNumber(const QCallListItem &);
    void menuClicked(int);
    void received(const QString &, const QByteArray &);

private:
    E2FSCallHistoryScreen *m_selector;
    E2FSCallHistoryNumber *m_number;

    QStackedWidget *m_stack;
    QCallListItem m_currentNumber;
    E2Menu *m_menu;

    E2Button *m_button;
    E2Button *m_menuButton;
};

class E2NewMessage : public E2TitleFrame
{
Q_OBJECT
public:
    E2NewMessage();

private slots:
    void readClicked();
    void fetched(const QString& id, const QSMSMessage& m);
    void messageCount(int total );

private:
    void displayMessage(const QSMSMessage &);

    bool m_fetching;
    QSMSReader* smsReq;
    int m_messageCount;

    QLabel *m_message;
    QLabel *m_image;
};

class E2Incoming : public E2TitleFrame
{
Q_OBJECT
public:
    E2Incoming();

signals:
    void showCallscreen();

private slots:
    void missedIncomingCall(const QPhoneCall&);
    void callIncoming( const QPhoneCall &call );
    void ignore();
    void busy();
    void answer();

private:
    void updateLabels();
    QPhoneCall currentCall;
    QLabel *image;
    QString m_lastId;
    QLabel *name;
};

#endif
