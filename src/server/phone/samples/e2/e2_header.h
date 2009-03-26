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

#ifndef E2_HEADER_H
#define E2_HEADER_H

#include <QWidget>
#include <QPixmap>
#include <QStringList>
#include <QList>
#include <QValueSpaceItem>

class E2HeaderButton;
class E2AlertScreen;
class E2Header : public QWidget
{
Q_OBJECT
public:
    E2Header(QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);

private slots:
    void alertClicked();
    void clicked(const QString &name);
    void activeCallCount(int);
    void setAlertEnabled(bool e);

private:
    QPixmap m_fillBrush;
    QStringList m_alertStack;
    QList<E2HeaderButton *> m_buttons;
    E2HeaderButton *m_alert;
    E2AlertScreen *m_alertScreen;
    E2HeaderButton *m_phone;
    E2HeaderButton *m_phoneActive;
};

#endif
