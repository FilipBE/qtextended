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

#ifndef E1_TELEPHONY_H
#define E1_TELEPHONY_H

#include <QWidget>
class QString;
class QByteArray;
class E1Callscreen;
class E1Dialer;
class E1Bar;
class E1PhoneTelephonyBar;

class E1Telephony : public QWidget
{
Q_OBJECT
public:
    E1Telephony(QWidget *parent = 0);


public slots:
    void display();
    void popupDialer();
    void popupCallscreen();
    void slideToDialer();
    void slideToCallscreen();

protected:
    virtual void showEvent(QShowEvent *);
    virtual void timerEvent(QTimerEvent *);

private slots:
    void message(const QString &message, const QByteArray &);
    void doShowDialer(const QString &);

private:
    enum { CallScreen, Dialer } m_state;

    void doLayout();

    unsigned int m_scrollStep;
    static const unsigned int m_scrollSteps = 10;
    static const unsigned int m_scrollStepTime = 20;

    E1PhoneTelephonyBar *m_tbar;
    E1Callscreen *m_callscreen;
    E1Dialer *m_dialer;
    E1Bar *m_bar;
};

#endif
