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

#ifndef QMODEMRFFUNCTIONALITY_H
#define QMODEMRFFUNCTIONALITY_H

#include <qphonerffunctionality.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemRfFunctionality
        : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    explicit QModemRfFunctionality( QModemService *service );
    ~QModemRfFunctionality();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );

private slots:
    void cfun( bool ok, const QAtResult& result );
    void cfunSet( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
