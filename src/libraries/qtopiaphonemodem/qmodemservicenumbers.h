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

#ifndef QMODEMSERVICENUMBERS_H
#define QMODEMSERVICENUMBERS_H

#include <qservicenumbers.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemServiceNumbers : public QServiceNumbers
{
    Q_OBJECT
public:
    explicit QModemServiceNumbers( QModemService *service );
    ~QModemServiceNumbers();

public slots:
    void requestServiceNumber( QServiceNumbers::NumberId id );
    void setServiceNumber
            ( QServiceNumbers::NumberId id, const QString& number );

protected:
    void requestServiceNumberFromFile( QServiceNumbers::NumberId id );
    void setServiceNumberInFile
            ( QServiceNumbers::NumberId id, const QString& number );

private slots:
    void csvm( bool ok, const QAtResult& result );
    void csca( bool ok, const QAtResult& result );
    void cnum( bool ok, const QAtResult& result );
    void csvmSet( bool ok, const QAtResult& result );
    void cscaSet( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
