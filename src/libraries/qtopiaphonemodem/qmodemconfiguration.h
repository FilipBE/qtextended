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

#ifndef QMODEMCONFIGURATION_H
#define QMODEMCONFIGURATION_H

#include <qtelephonyconfiguration.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemConfiguration
            : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    explicit QModemConfiguration( QModemService *service );
    ~QModemConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private slots:
    void cgmi( bool ok, const QAtResult& result );
    void cgmm( bool ok, const QAtResult& result );
    void cgmr( bool ok, const QAtResult& result );
    void cgsn( bool ok, const QAtResult& result );

private:
    QModemService *service;

    static QString fixResponse( const QString& value, const QString& prefix );
};

#endif
