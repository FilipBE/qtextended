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

#ifndef E2_DIALER_H
#define E2_DIALER_H

#include <QWidget>
#include <QPixmap>
#include <QLineEdit>
#include <QList>
class E2Button;
class QGridLayout;
class E2DialerButton;
class E2CallHistory;
class E2Dialer : public QWidget
{
Q_OBJECT
public:
    E2Dialer(E2Button *b, QWidget *parent = 0, Qt::WFlags flags = 0);

    void setActive();
    void setNumber(const QString &);

signals:
    void sendNumber(const QString &);
    void toCallscreen();

private slots:
    void showHistory();
    void backspace();
    void number();
    void hash();
    void star();
    void textChanged();
    void callNow();
    void activeCallCount(int c);

private:
    void addDtmf(const QString &);

    int m_activeCallCount;
    QLineEdit *m_lineEdit;
    QList<QObject *> m_numbers;
    E2Button *m_button;
    E2DialerButton *m_callscreen;
    QGridLayout *m_grid;
    E2CallHistory *m_history;
};

class E2DialerButton : public QWidget
{
Q_OBJECT
public:
    E2DialerButton(const QPixmap &, bool highlight = false, QWidget *parent = 0);

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private:
    bool m_highlight;
    bool m_pressed;
    QPixmap m_button;
};

#endif
