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

#ifndef QKEYPADLIGHTACCESSORY_H
#define QKEYPADLIGHTACCESSORY_H

#include "qhardwareinterface.h"

class QTOPIA_EXPORT QKeypadLightAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QKeypadLightAccessory( const QString& id = QString(), QObject *parent = 0,
                                    QAbstractIpcInterface::Mode mode = Client );
    ~QKeypadLightAccessory();

    bool on() const;

public slots:
    virtual void setOn( const bool value );

signals:
    void onModified();
};

class QTOPIA_EXPORT QKeypadLightAccessoryProvider
            : public QKeypadLightAccessory
{
    Q_OBJECT
public:
    explicit QKeypadLightAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QKeypadLightAccessoryProvider();

public slots:
    void setOn( const bool value );
};

#endif
