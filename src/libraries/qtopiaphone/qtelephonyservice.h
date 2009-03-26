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

#ifndef QTELEPHONYSERVICE_H
#define QTELEPHONYSERVICE_H

#include <qabstractipcinterfacegroup.h>

class QPhoneCallProvider;
class QSMSMessage;

class QTOPIAPHONE_EXPORT QTelephonyService : public QAbstractIpcInterfaceGroup
{
    Q_OBJECT
public:
    explicit QTelephonyService( const QString& service, QObject *parent = 0 );
    ~QTelephonyService();

    QString service() const;

    QPhoneCallProvider *callProvider() const;
    void setCallProvider( QPhoneCallProvider *provider );

    void initialize();

#ifdef QTOPIA_CELL
    bool dispatchDatagram( const QSMSMessage& msg );
#endif

private:
    QPhoneCallProvider *provider;
};

#endif
