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

#ifndef QMODEMSIMGENERICACCESS_H
#define QMODEMSIMGENERICACCESS_H

#include <qsimgenericaccess.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemSimGenericAccess : public QSimGenericAccess
{
    Q_OBJECT
public:
    explicit QModemSimGenericAccess( QModemService *service );
    ~QModemSimGenericAccess();

public slots:
    void command( const QString& reqid, const QByteArray& data );

private slots:
    void csim( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
