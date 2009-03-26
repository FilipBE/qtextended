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

#ifndef QMODEMVIBRATEACCESSORY_H
#define QMODEMVIBRATEACCESSORY_H

#include <qvibrateaccessory.h>

class QModemService;

class QTOPIAPHONEMODEM_EXPORT QModemVibrateAccessory
        : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
    explicit QModemVibrateAccessory( QModemService *service );
    ~QModemVibrateAccessory();

public slots:
    void setVibrateOnRing( const bool value );
    void setVibrateNow( const bool value );

private:
    QModemService *service;
};

#endif
