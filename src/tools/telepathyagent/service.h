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

#ifndef SERVICE_H
#define SERVICE_H

#include <qobject.h>
#include <qtopiaabstractservice.h>
#include <qtelephonyservice.h>

#include <QString>

class TelepathyConnection;
class TelepathyConnectionManager;
class TelepathyLayer;
class TelepathyAvatarProvider;

class TelepathyTelephonyService : public QTelephonyService
{
    Q_OBJECT

public:
    explicit TelepathyTelephonyService(const QString &configuration,
                                       const QString &manager, const QString &protocol,
                                       QObject *parent = 0);
    ~TelepathyTelephonyService();

    void initialize();

    TelepathyConnection *connection();
    void setConnection(TelepathyConnection *conn);
    TelepathyLayer *layer();

    QString configuration() const;
    TelepathyConnectionManager *manager();
    QString protocol() const;

    const QMap<QString, QVariant> & parameters() const;
    virtual void updateRegistrationConfig();
    virtual bool parametersAreValid() const;

protected:
    QMap<QString, QVariant> m_parameters;

private:
    QString m_configuration;
    QString m_protocol;
    TelepathyConnection *m_connection;
    TelepathyConnectionManager *m_manager;
    bool m_paramsValid;
    TelepathyLayer *m_layer;
};

class TelepathyTelephonyServiceQCop : public QtopiaAbstractService
{
    Q_OBJECT

public:
    TelepathyTelephonyServiceQCop( QObject *parent = 0 );
    ~TelepathyTelephonyServiceQCop();

public slots:
    void start();
    void stop();

private:
    TelepathyTelephonyService *service;
};

#endif
