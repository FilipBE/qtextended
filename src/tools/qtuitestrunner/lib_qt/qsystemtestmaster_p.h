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

#ifndef QSYSTEMTESTMASTER_P_H
#define QSYSTEMTESTMASTER_P_H

#include "qtestprotocol_p.h"
#include <QObject>
#include <qtuitestglobal.h>

class QSystemTestPrivate;

class QTUITEST_EXPORT QSystemTestMaster : public QTestProtocol
{
public:
    QSystemTestMaster( QSystemTestPrivate *testCase );
    virtual ~QSystemTestMaster();

    QString appName();

    virtual void onReplyReceived( QTestMessage *reply );

protected:
    virtual void processMessage( QTestMessage *msg );

private:
    void queryName();
    QString app_name;
    QSystemTestPrivate *test_case;
};

/*
class QTUITEST_EXPORT QSystemTestMasterServer: public QTestServerSocket
{
    Q_OBJECT
public:
    QSystemTestMasterServer( quint16 port, QSystemTest *testCase );
    virtual ~QSystemTestMasterServer();

    //static void delayConnection();

    bool waitForApp( const QString &appName, uint timeOut );
    QSystemTestMaster *findApp( const QString &appName );
    QSystemTestMaster *getApp( uint index );
    uint appCount() { return app_count; };

protected:
    virtual void onNewConnection( int socket );
    bool appendApp( QSystemTestMaster *app );
    //static void savePortInfo( int port );

protected slots:
    void connectionClosed( QTestProtocol *socket );

signals:
    void newGuiClient( QSystemTestMaster *newApp );

private:
    // FIXME: Should use QList here
    #define MAX_APPS 50
    QSystemTestMaster *apps[MAX_APPS];
    uint app_count;
    QSystemTest *test_case;
};
*/

#endif

