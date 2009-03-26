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

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <qtopiaipcadaptor.h>
#include "qtopiaserverapplication.h"

class QByteArray;
class QString;
class QtopiaNetworkInterface;
class QValueSpaceObject;
class QValueSpaceItem;

//QtopiaNetworkSession
class QtopiaNetworkSession : public QObject
{
    Q_OBJECT
public:
    QtopiaNetworkSession( const QByteArray& ifaceHandle, QObject* parent = 0 );
    void addApplication( const QString& appName );
    void removeApplication( const QString& appName );
    inline bool applicationIsRegistered( const QString& appName );
    inline int subscribedApplications();
    void setExtendedLife( bool isExtended );
    inline bool extendedLife();

Q_SIGNALS:
    void sessionExpired( const QByteArray& handle );
    void sessionObsolete( const QByteArray& handle );

private slots:
    void runningAppsChanged();
    void interfaceStateChanged();

private:
    QByteArray m_handle;
    bool m_extended;
    QSet<QString> m_apps;
    QValueSpaceObject* m_sessionVso;
    QValueSpaceItem* m_appVsi;
    QValueSpaceItem* m_ifaceVsi;
};

//QtopiaSessionManager
class QtopiaSessionManager : public QObject
{
    Q_OBJECT
public:
    QtopiaSessionManager( QObject* parent );
    ~QtopiaSessionManager();

    void registerSession( const QByteArray& ifaceHandle, const QString& appName );
    void stopSession( const QByteArray& ifaceHandle, const QString& appName );
    void invalidateSession( const QByteArray& ifaceHandle );

    void setExtendedLifetime( const QByteArray& ifaceHandle, bool isExtended );
    bool isValidSession( const QByteArray& ifaceHandle ) const;

Q_SIGNALS:
    void quitInterface( const QString& handle );

private slots:
    void sessionChanged( const QByteArray& ifaceHandle );
    void sessionObsolete( const QByteArray& ifaceHandle );

private:
    QHash<QByteArray,QtopiaNetworkSession*> sessions;

};

//QtopiaNetworkServer
class QtopiaNetworkServer : public QtopiaIpcAdaptor
{
    Q_OBJECT
public:
    QtopiaNetworkServer();
    ~QtopiaNetworkServer();

    enum StartCode { NotReady, Success, Unavailable};

public slots:
    void stopInterface( const QString& appName, const QString& handle, bool deleteIface = false );
    void privilegedInterfaceStop( const QString& );
    void startInterface( const QString& appName, const QString& handle, const QVariant& options = QVariant() );
    void shutdownNetwork();
    void setDefaultGateway( const QString& handle, bool excludeInterface);
    void setExtendedInterfaceLifetime( const QString& handle, bool isExtended );
    void setLockMode( bool isLocked );

protected:
    void timerEvent(QTimerEvent *tEvent);
    StartCode activateInterface( const QString& appName, const QString& handle, const QVariant& options = QVariant() );

private slots:
    void updateNetwork();
    void cardMessage();
    void sessionManagerQuitRequest( const QString& handle );

private:
    bool interfaceDeviceBusy( const QPointer<QtopiaNetworkInterface> iface);

    QtopiaSessionManager* sessionManager;
    int tidAutoStart;
    QStringList autoStartConfigs;
    QtopiaIpcAdaptor *nwState;
    QString gateway;
    QValueSpaceObject* gatewaySpace;
    bool lockdown;
};
#endif
