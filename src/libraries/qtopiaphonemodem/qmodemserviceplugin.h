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

#ifndef QMODEMSERVICEPLUGIN_H
#define QMODEMSERVICEPLUGIN_H

#include <qfactoryinterface.h>

#include <qtopiaglobal.h>

class QModemService;
class QSerialIODeviceMultiplexer;

struct QTOPIAPHONEMODEM_EXPORT QModemServicePluginInterface : public QFactoryInterface
{
    virtual bool supports( const QString& manufacturer ) = 0;
    virtual QModemService *create
            ( const QString& service, QSerialIODeviceMultiplexer *mux,
              QObject *parent ) = 0;
};
#define QModemServicePluginInterface_iid "com.trolltech.Qtopia.QModemServicePluginInterface"
Q_DECLARE_INTERFACE(QModemServicePluginInterface, QModemServicePluginInterface_iid)

class QTOPIAPHONEMODEM_EXPORT QModemServicePlugin : public QObject, public QModemServicePluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QModemServicePluginInterface:QFactoryInterface)
public:
    explicit QModemServicePlugin( QObject* parent = 0 );
    ~QModemServicePlugin();

    QStringList keys() const;
    bool supports( const QString& manufacturer );
    QModemService *create( const QString& service,
                           QSerialIODeviceMultiplexer *mux,
                           QObject *parent );
};

#endif
