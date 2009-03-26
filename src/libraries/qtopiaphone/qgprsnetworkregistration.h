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

#ifndef QGPRSNETWORKREGISTRATION_H
#define QGPRSNETWORKREGISTRATION_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <QList>

class QTOPIAPHONE_EXPORT QGprsNetworkRegistration : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(QTelephony::RegistrationState registrationState READ registrationState)
    Q_PROPERTY(int locationAreaCode READ locationAreaCode)
    Q_PROPERTY(int cellId READ cellId)
public:
    explicit QGprsNetworkRegistration( const QString& service = QString(),
                                       QObject *parent = 0,
                                       QCommInterface::Mode mode = Client );
    ~QGprsNetworkRegistration();

    QTelephony::RegistrationState registrationState() const;
    int locationAreaCode() const;
    int cellId() const;

signals:
    void registrationStateChanged();
    void locationChanged();
};

class QTOPIAPHONE_EXPORT QGprsNetworkRegistrationServer : public QGprsNetworkRegistration
{
    Q_OBJECT
public:
    explicit QGprsNetworkRegistrationServer
            ( const QString& service, QObject *parent = 0 );
    ~QGprsNetworkRegistrationServer();

protected:
    void updateRegistrationState( QTelephony::RegistrationState state );
    void updateRegistrationState
        ( QTelephony::RegistrationState state,
          int locationAreaCode, int cellId );
};

#endif
