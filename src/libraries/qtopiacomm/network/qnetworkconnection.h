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

#ifndef QNETWORKCONNECTION_H
#define QNETWORKCONNECTION_H

#include <QObject>
#include <QList>
#include <QString>
#include <QUuid>

#include <qtopiaglobal.h>
#include <qtopianetwork.h>

class QNetworkConnectionManager;
class QNetworkConnectionPrivate;
class QTOPIACOMM_EXPORT QNetworkConnection : public QObject
{
    Q_OBJECT
public:
    class QTOPIACOMM_EXPORT Identity
    {
    public:
        Identity();
        explicit Identity( const QString& devHandle, const QUuid& vNetId );
        Identity( const Identity& other );
        ~Identity();

        //operators
        Identity &operator=(const Identity& other);
        bool operator==(const Identity& other) const;
        bool operator!=(const Identity& other) const;

        QString deviceHandle() const;
        QString name() const;
        QtopiaNetwork::Type type() const;
        bool isValid() const;
    private:
        mutable QUuid vNetId;
        mutable QString devHandle;
        friend class QNetworkConnectionPrivate;
    };
    typedef QList<Identity> Identities;

    explicit QNetworkConnection( const Identity& ident, QObject* parent = 0 );
    virtual ~QNetworkConnection();

    Identity identity() const;
    bool isConnected() const;
    bool isValid() const;
signals:
    void connectivityChanged( bool isConnected );

private:
    Q_PRIVATE_SLOT( d, void _q_deviceStateChanged(QtopiaNetworkInterface::Status,bool) );
    QNetworkConnectionPrivate* d;
    friend class QNetworkConnectionPrivate;
};


class QNetworkConnectionManagerPrivate;
class QTOPIACOMM_EXPORT QNetworkConnectionManager : public QObject
{
    Q_OBJECT
public:
    QNetworkConnectionManager( QObject* parent = 0 );
    ~QNetworkConnectionManager();

    static QNetworkConnection::Identities connections();
signals:
    void connectionAdded();
    void connectionRemoved();
private:
    QNetworkConnectionManagerPrivate* d;
    Q_PRIVATE_SLOT( d, void _q_accountsAddedRemoved() );
    Q_PRIVATE_SLOT( d, void _q_accountChanged(const QString&) );
    friend class QNetworkConnectionManagerPrivate;
};
#endif
