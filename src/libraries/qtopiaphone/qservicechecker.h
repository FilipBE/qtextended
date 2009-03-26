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

#ifndef QSERVICECHECKER_H
#define QSERVICECHECKER_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QServiceChecker : public QCommInterface
{
    Q_OBJECT
public:
    explicit QServiceChecker( const QString& service = QString(), QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QServiceChecker();

    bool isValid();

protected:
    void setValid( bool value );
};

class QTOPIAPHONE_EXPORT QServiceCheckerServer : public QServiceChecker
{
    Q_OBJECT
public:
    QServiceCheckerServer( const QString& service, bool valid,
                           QObject *parent = 0 );
    ~QServiceCheckerServer();

};

#endif
