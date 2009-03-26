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

#ifndef QNETWORKSTATE_H
#define QNETWORKSTATE_H

#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

#include <QNetworkInterface>
#include <QList>
#include <QString>
#include <QObject>

class QNetworkStatePrivate;

class QTOPIACOMM_EXPORT QNetworkState : public QObject
{
    Q_OBJECT
public:
    explicit QNetworkState( QObject* parent = 0 );
    virtual ~QNetworkState();

    QString gateway() const;
    QList<QString> interfacesOnline() const;

    QString defaultWapAccount() const;

    static QList<QString> availableNetworkDevices( QtopiaNetwork::Type type = QtopiaNetwork::Any);
    static QtopiaNetwork::Type deviceType( const QString& devHandle );

Q_SIGNALS:
    void defaultGatewayChanged( QString devHandle, const QNetworkInterface& localAddress );
    void connected();
    void disconnected();

private Q_SLOTS:
    void gatewayChanged( const QString& newGateway );

private:
    QNetworkStatePrivate* d;
};

#endif
