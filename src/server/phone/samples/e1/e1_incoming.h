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

#ifndef E1_INCOMING_H
#define E1_INCOMING_H

#include "e1_dialog.h"
#include <qphonecallmanager.h>
class QLabel;

class E1Incoming : public E1Dialog
{
Q_OBJECT
public:
    E1Incoming();

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
