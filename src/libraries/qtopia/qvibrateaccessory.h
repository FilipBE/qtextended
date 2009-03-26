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

#ifndef QVIBRATEACCESSORY_H
#define QVIBRATEACCESSORY_H

#include "qhardwareinterface.h"

class QTOPIA_EXPORT QVibrateAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QVibrateAccessory( const QString& id = QString(), QObject *parent = 0,
                                QAbstractIpcInterface::Mode mode = Client );
    ~QVibrateAccessory();

    bool vibrateOnRing() const;
    bool vibrateNow() const;

    bool supportsVibrateOnRing() const;
    bool supportsVibrateNow() const;

public slots:
    virtual void setVibrateOnRing( const bool value );
    virtual void setVibrateNow( const bool value );

signals:
    void vibrateOnRingModified();
    void vibrateNowModified();
};

class QTOPIA_EXPORT QVibrateAccessoryProvider : public QVibrateAccessory
{
    Q_OBJECT
public:
    explicit QVibrateAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QVibrateAccessoryProvider();

protected:
    void setSupportsVibrateOnRing( bool value );
    void setSupportsVibrateNow( bool value );

public slots:
    void setVibrateOnRing( const bool value );
    void setVibrateNow( const bool value );

};

#endif
