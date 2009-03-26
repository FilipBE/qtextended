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

#ifndef MESSAGEARRIVALSERVICE_H
#define MESSAGEARRIVALSERVICE_H

#include <QtopiaAbstractService>

#ifndef QTOPIA_NO_SMS

class QDSActionRequest;

class MessageArrivalService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    MessageArrivalService(QObject* parent);
    ~MessageArrivalService();

signals:
    void smsFlash(const QDSActionRequest&);
#ifndef QTOPIA_NO_MMS
    void mmsMessage(const QDSActionRequest&);
#endif

public slots:
    void pushMmsMessage( const QDSActionRequest& request );
    void flashSms( const QDSActionRequest& request );
};

#endif

#endif
