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

#ifndef E1_CALLSCREEN_H
#define E1_CALLSCREEN_H

#include <QWidget>
#include <QDateTime>
#include <QPixmap>
#include <QString>
class QLabel;
class QPhoneCall;
class E1Button;

class E1Callscreen : public QWidget
{
Q_OBJECT
public:
    E1Callscreen(E1Button *b, QWidget *parent);

    void setActive();

signals:
    void showMe();
    void closeMe();
    void toDialer();

public slots:
    void sendNumber(const QString &);

private slots:
    void callConnected( const QPhoneCall &call );
    void callIncoming( const QPhoneCall &call );
    void callDialing( const QPhoneCall &call );
    void callEnded(const QPhoneCall &call);

    void mute();
    void spkr();
    void hold();
    void end();

protected:
    virtual void timerEvent(QTimerEvent *);
    virtual void paintEvent(QPaintEvent *);

private:
    void updateCallTime();
    void updateInfo(const QPhoneCall &);
    int m_timer;
    QString m_hasCall;

    QString m_name;
    QPixmap m_image;

    QString m_state;
    QString m_time;

    QDateTime m_callTime;
    QLabel *nameLabel;
    QLabel *imageLabel;
    QLabel *timeLabel;
    QLabel *stateLabel;
    E1Button *m_button;
};

#endif
