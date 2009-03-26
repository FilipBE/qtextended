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

#ifndef TESTPHONECALL_H
#define TESTPHONECALL_H

#include <qphonecallprovider.h>

class TestPhoneCallImpl : public QPhoneCallImpl
{
    Q_OBJECT
public:
    TestPhoneCallImpl( QPhoneCallProvider *provider, const QString& identifier,
                       const QString& callType );
    ~TestPhoneCallImpl();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );
    void requestFloor( int secs );
    void releaseFloor();
    void setState( QPhoneCall::State state );

private slots:
    void connectTimeout();

private:
    QPhoneCallImpl *findCall( QPhoneCall::State state );
    bool hasCall( QPhoneCall::State state );
};

class TestPhoneCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
public:
    TestPhoneCallProvider( const QString& service, QObject *parent );
    ~TestPhoneCallProvider();

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
};

#endif
