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

#ifndef QSERIALIODEVICEMULTIPLEXERPLUGIN_H
#define QSERIALIODEVICEMULTIPLEXERPLUGIN_H

#include <qfactoryinterface.h>

#include <qtopiaglobal.h>

class QSerialIODevice;
class QSerialIODeviceMultiplexer;

struct QTOPIACOMM_EXPORT QSerialIODeviceMultiplexerPluginInterface : public QFactoryInterface
{
    virtual bool forceGsm0710Stop() = 0;
    virtual bool detect( QSerialIODevice *device ) = 0;
    virtual QSerialIODeviceMultiplexer *create( QSerialIODevice *device ) = 0;
};
#define QSerialIODeviceMultiplexerPluginInterface_iid "com.trolltech.Qtopia.QSerialIODeviceMultiplexerPluginInterface"
Q_DECLARE_INTERFACE(QSerialIODeviceMultiplexerPluginInterface, QSerialIODeviceMultiplexerPluginInterface_iid)

class QTOPIACOMM_EXPORT QSerialIODeviceMultiplexerPlugin : public QObject, public QSerialIODeviceMultiplexerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QSerialIODeviceMultiplexerPluginInterface:QFactoryInterface)
public:
    QSerialIODeviceMultiplexerPlugin( QObject* parent = 0 );
    ~QSerialIODeviceMultiplexerPlugin();

    QStringList keys() const;
    bool forceGsm0710Stop();
    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

#endif
