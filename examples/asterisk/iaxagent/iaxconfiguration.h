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

#ifndef IAXCONFIGURATION_H
#define IAXCONFIGURATION_H

#include <qtelephonyconfiguration.h>

class IaxTelephonyService;

class IaxConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    IaxConfiguration( IaxTelephonyService *service );
    ~IaxConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private:
    IaxTelephonyService *service;
};

#endif
