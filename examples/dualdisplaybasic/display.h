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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <qframe.h>

class QPhoneStatus;
class PhoneDisplay;

class BasicDisplay : public QFrame
{
    Q_OBJECT
public:
    BasicDisplay(QWidget *parent, Qt::WFlags f=0);
    ~BasicDisplay();

protected slots:
    void statusChanged();
    void incomingCall(const QString &number, const QString &name);

private:
    QPhoneStatus *status;
    PhoneDisplay *display;
};

#endif
