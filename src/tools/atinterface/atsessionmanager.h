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

#ifndef ATSESSIONMANAGER_H
#define ATSESSIONMANAGER_H

#include "atfrontend.h"
#include <qstringlist.h>

class AtSessionManagerPrivate;
class AtCallManager;
class QSerialSocket;

class AtSessionManager : public QObject
{
    Q_OBJECT
    friend class AtGsmNonCellCommands;
public:
    AtSessionManager( QObject *parent = 0 );
    ~AtSessionManager();

    bool addSerialPort( const QString& deviceName,
                        const QString& options = QString() );
    bool addTcpPort( int tcpPort, bool localHostOnly = true,
                     const QString& options = QString() );

    void removeSerialPort( const QString& deviceName );
    void removeTcpPort( int tcpPort );

    QStringList serialPorts() const;
    QList<int> tcpPorts() const;

    AtCallManager *callManager() const;

signals:
    void newSession( AtFrontEnd *session );
    void devicesChanged();

private slots:
    void incoming( QSerialSocket *socket );
    void serialPortDestroyed();

private:
    AtSessionManagerPrivate *d;

    void registerTaskIfNecessary();
    void unregisterTaskIfNecessary();
};

#endif
