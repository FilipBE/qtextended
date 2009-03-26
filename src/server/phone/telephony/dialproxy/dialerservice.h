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

#ifndef DIALERSERVICE_H
#define DIALERSERVICE_H

#include <qtopiaabstractservice.h>
class QUniqueId;
class DialerService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    DialerService( QObject *parent )
        : QtopiaAbstractService( "Dialer", parent )
        { publishAll(); }

public:
    ~DialerService();

public slots:
    virtual void dialVoiceMail() = 0;
    virtual void dial( const QString& name, const QString& number ) = 0;
    virtual void dial( const QString& number, const QUniqueId& contact ) = 0;
    virtual void showDialer( const QString& digits ) = 0;
    virtual void showDialer();
    virtual void onHook() = 0;
    virtual void offHook() = 0;
    virtual void headset() = 0;
    virtual void speaker() = 0;
    virtual void setDialToneOnlyHint( const QString &app ) = 0;
    virtual void redial() = 0;
};

#endif
