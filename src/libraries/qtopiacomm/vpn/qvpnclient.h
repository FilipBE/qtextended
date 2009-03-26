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

#ifndef QVPNCLIENT_H
#define QVPNCLIENT_H

#include <qtopiaglobal.h>

#include <QObject>
#include <QDialog>

class QVPNClientPrivate;

class QTOPIACOMM_EXPORT QVPNClient : public QObject
{
    Q_OBJECT
public:
    enum Type {
        OpenVPN = 0,
        IPSec = 20
    };

    enum State {
        Disconnected = 0,
        Pending,
        Connected = 20
    };

    virtual ~QVPNClient();

    virtual Type type() const = 0;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual QDialog* configure( QWidget* parent = 0 ) = 0;
    virtual State state() const = 0;
    virtual void cleanup() = 0;

    virtual QString name() const;

    QString errorString() const;
    uint id() const;

signals:
    void connectionStateChanged( bool error );

protected:
    QVPNClient( bool serverMode, QObject* parent = 0 );
    QVPNClient( bool serverMode, uint vpnID, QObject* parent = 0 );

protected:
    QVPNClientPrivate* d;
};

#endif
