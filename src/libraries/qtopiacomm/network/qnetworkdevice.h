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

#ifndef QNETWORKDEVICE_H
#define QNETWORKDEVICE_H

#include <qtopianetworkinterface.h>
#include <qtopianetwork.h>
#include <qtopiaglobal.h>

#include <QNetworkInterface>

class QNetworkDevicePrivate;

class QTOPIACOMM_EXPORT QNetworkDevice : public QObject
{
    Q_OBJECT
public:
    explicit QNetworkDevice( const QString& handle, QObject* parent = 0 );
    ~QNetworkDevice();

    QtopiaNetworkInterface::Status state() const;

    QtopiaNetworkInterface::Error error() const;
    QString errorString() const;

    QNetworkInterface address() const;
    QString interfaceName() const;
    QString handle() const;
    QString name() const;

Q_SIGNALS:
    void stateChanged( QtopiaNetworkInterface::Status newState, bool error );
private:
    QNetworkDevicePrivate* d;
};

#endif
