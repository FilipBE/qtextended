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

#ifndef QMODEMCALLBARRING_H
#define QMODEMCALLBARRING_H

#include <qcallbarring.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallBarring : public QCallBarring
{
    Q_OBJECT
public:
    explicit QModemCallBarring( QModemService *service );
    ~QModemCallBarring();

public slots:
    void requestBarringStatus( QCallBarring::BarringType type );
    void setBarringStatus( QCallBarring::BarringType type,
                           const QString& password,
                           QTelephony::CallClass cls,
                           bool lock );
    void unlockAll( const QString& password );
    void unlockAllIncoming( const QString& password );
    void unlockAllOutgoing( const QString& password );
    void changeBarringPassword( QCallBarring::BarringType type,
                                const QString& oldPassword,
                                const QString& newPassword );

protected:
    virtual QString typeToString( QCallBarring::BarringType type ) const;

private slots:
    void setDone( bool ok, const QAtResult& result );
    void requestDone( bool ok, const QAtResult& result );
    void unlockDone( bool ok, const QAtResult& result );
    void cpwdDone( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
