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

#ifndef DIALER_H
#define DIALER_H

#include "qabstractdialerscreen.h"

class Dialer;
class PhoneTouchDialerScreen : public QAbstractDialerScreen
{
    Q_OBJECT
public:
    PhoneTouchDialerScreen(QWidget *parent = 0, Qt::WFlags f = 0);

    virtual QString digits() const;
    virtual void reset();
    virtual void appendDigits(const QString &digits);
    virtual void setDigits(const QString &digits);
    virtual void doOffHook();
    virtual void doOnHook();

protected slots:
    void keyEntered(const QString &key);

private:
    Dialer *m_dialer;
};

#endif
