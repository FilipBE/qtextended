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

#ifndef QHARDWAREMANAGER_H
#define QHARDWAREMANAGER_H

// Local includes
#include "qabstractipcinterfacegroupmanager.h"

class QHardwareManagerPrivate;
class QTOPIA_EXPORT QHardwareManager : public QAbstractIpcInterfaceGroupManager
{
    Q_OBJECT
public:
    explicit QHardwareManager( const QString& interface, QObject *parent = 0 );
    virtual ~QHardwareManager();

    QString interface() const;
    QStringList providers() const;

    template <typename T> static QStringList providers()
    {
        QAbstractIpcInterfaceGroupManager man( "/Hardware/Accessories" );
        return man.supports<T>();
    }

signals:
    void providerAdded( const QString& id );
    void providerRemoved( const QString& id );
private:
    QHardwareManagerPrivate* d;
};

#endif
