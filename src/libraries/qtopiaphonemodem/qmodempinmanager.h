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

#ifndef QMODEMPINMANAGER_H
#define QMODEMPINMANAGER_H

#include <qpinmanager.h>

class QModemService;
class QAtResult;
class QModemPinManagerPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemPinManager : public QPinManager
{
    Q_OBJECT
public:
    explicit QModemPinManager( QModemService *service );
    ~QModemPinManager();

    void needPin( const QString& type, QObject *target,
                  const char *pinSlot, const char *cancelSlot );

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );
    void requestLockStatus( const QString& type );
    void setLockStatus
        ( const QString& type, const QString& password, bool enabled );

private slots:
    void sendQuery();
    void sendQueryAgain();
    void cpinQuery( bool ok, const QAtResult& result );
    void cpinResponse( bool ok, const QAtResult& result );
    void csimResponse( bool ok, const QAtResult& result );
    void cpukResponse( bool ok );
    void cpwdResponse( bool ok );
    void clckQuery( bool ok, const QAtResult& result );
    void clckSet( bool ok, const QAtResult& result );
    void lastPinTimeout();

protected:
    virtual bool emptyPinIsReady() const;
    virtual QString pinTypeToCode( const QString& type ) const;
    virtual int pinMaximum() const;

private slots:
    void simMissing();

private:
    QModemPinManagerPrivate *d;
};

#endif
