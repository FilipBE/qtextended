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

#ifndef QTOPIANETWORKINTERFACE_H
#define QTOPIANETWORKINTERFACE_H

#include <qtopiaglobal.h>
#include <qtopianetwork.h>

#include <QMap>
#include <QString>
#include <QVariant>
#include <qplugin.h>


class QtopiaNetworkConfiguration;
class QtopiaNetworkMonitor;
class QDialog;

class QTOPIA_EXPORT QtopiaNetworkProperties : public QMap<QString, QVariant>
{
public:
    QtopiaNetworkProperties();
    QtopiaNetworkProperties( const QtopiaNetworkProperties& list );
    ~QtopiaNetworkProperties();

    void debugDump() const;
};


class QTOPIA_EXPORT QtopiaNetworkInterface : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        Unknown = 0,  //unknown status (updated after fist call to iface->initialize() )
        Down,         //inactive
        Up,           //active
        Pending,      //the interface is in an undefined state between
                      //Up and Down
        Demand,       //the interface will be started when data traffic
                      // is present (see man pppd - demand option)
        Unavailable = 100
    };

    enum Error {
        NoError = 0,
        NotConnected = 1,
        NotInitialized = 2,
        NotAvailable = 3,
        UnknownError = 100
    };

    virtual ~QtopiaNetworkInterface();
    virtual Status status() = 0;

    virtual void initialize() = 0;
    virtual bool start( const QVariant options = QVariant() ) = 0;
    virtual bool stop() = 0;
    virtual QString device() const = 0;
    virtual void cleanup() = 0;

    virtual QtopiaNetwork::Type type() const = 0;

    virtual bool setDefaultGateway() = 0;

    virtual QtopiaNetworkConfiguration * configuration() = 0;

    virtual void setProperties(
            const QtopiaNetworkProperties& properties) = 0;

    virtual QtopiaNetworkMonitor * monitor();
};

class QTOPIA_EXPORT QtopiaNetworkConfiguration
{
public:
    virtual ~QtopiaNetworkConfiguration();
    virtual QString configFile() const = 0;

    virtual QStringList types() const = 0;
    virtual QDialog* configure(QWidget *parent, const QString& type = QString() ) = 0;

    virtual QVariant property(const QString& key) const = 0;
    virtual QtopiaNetworkProperties getProperties() const = 0;
    virtual void writeProperties( const QtopiaNetworkProperties& properties) = 0;
};

class QTOPIA_EXPORT QtopiaNetworkMonitor
{
public:
    virtual ~QtopiaNetworkMonitor();

    virtual QDialog* userInterface( QWidget* parent = 0 );
    virtual void setOption( const QVariant& options );
    virtual QVariant option( const QVariant& parameter ) const;
};

//--------------------------------------------------------
// plugin and plugin interface

struct QTOPIA_EXPORT QtopiaNetworkFactoryIface
{
    virtual ~QtopiaNetworkFactoryIface();
    virtual QPointer<QtopiaNetworkInterface> network(const QString& handle) = 0;
    virtual QtopiaNetwork::Type type() const = 0;
    virtual QByteArray customID() const = 0;
};


#define QtopiaNetworkFactoryIface_iid "com.trolltech.Qtopia.QtopiaNetworkFactoryIface"
Q_DECLARE_INTERFACE(QtopiaNetworkFactoryIface, QtopiaNetworkFactoryIface_iid)

class QTOPIA_EXPORT QtopiaNetworkPlugin : public QObject, public QtopiaNetworkFactoryIface
{
    Q_OBJECT
    Q_INTERFACES(QtopiaNetworkFactoryIface)
public:
    explicit QtopiaNetworkPlugin(QObject *parent = 0);
    virtual ~QtopiaNetworkPlugin();
};

#endif
