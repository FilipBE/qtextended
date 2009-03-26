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

#ifndef IAXCALLPROVIDER_H
#define IAXCALLPROVIDER_H

#include <qphonecallprovider.h>

#define IAXAGENT_MAX_CALLS      16

class IaxCallProvider;
class IaxTelephonyService;

class IaxPhoneCall : public QPhoneCallImpl
{
    Q_OBJECT
public:
    IaxPhoneCall( IaxCallProvider *provider, const QString& identifier,
                  const QString& callType, int callNo );
    virtual ~IaxPhoneCall();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void tone( const QString& tones );
    void transfer( const QString& number );

    void stateEvent( struct iaxc_ev_call_state *e );

public:
    IaxCallProvider *provider;
    int callNo;
};

class IaxCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
public:
    IaxCallProvider( IaxTelephonyService *service );
    ~IaxCallProvider();

    void stateEvent( struct iaxc_ev_call_state *e );
    void putActiveOnHold();
    void endStateTransaction();
    void updateCallerIdConfig();
    QString callUri() const;

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );

private:
    IaxTelephonyService *iaxservice;

    IaxPhoneCall *fromCallNo( int callNo );
};

#endif
