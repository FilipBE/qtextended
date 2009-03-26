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

#ifndef QTELEPHONYCONFIGURATION_H
#define QTELEPHONYCONFIGURATION_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QTelephonyConfiguration : public QCommInterface
{
    Q_OBJECT
public:
    explicit QTelephonyConfiguration( const QString& service,
                                      QObject *parent = 0,
                                      QCommInterface::Mode mode = Client );
    ~QTelephonyConfiguration();

public slots:
    virtual void update( const QString& name, const QString& value );
    virtual void request( const QString& name );

signals:
    void notification( const QString& name, const QString& value );
};

#endif
