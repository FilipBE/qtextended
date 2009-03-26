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

#ifndef QMODEMNETWORKREGISTRATION_H
#define QMODEMNETWORKREGISTRATION_H

#include <qnetworkregistration.h>
#include <qatresultparser.h>

class QModemService;
class QModemNetworkRegistrationPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemNetworkRegistration
        : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    explicit QModemNetworkRegistration( QModemService *service );
    ~QModemNetworkRegistration();

    bool supportsOperatorTechnology() const;
    void setSupportsOperatorTechnology( bool value );

    static QString operatorNameForId( const QString& id );

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

protected slots:
    virtual void resetModem();
    void queryRegistration();

private slots:
    void cregNotify( const QString& msg );
    void cregQuery( bool ok, const QAtResult& result );
    void copsDone( bool ok, const QAtResult& result );
    void copsNumericDone( bool ok, const QAtResult& result );
    void setDone( bool ok, const QAtResult& result );
    void availDone( bool ok, const QAtResult& result );
    void cfunDone();

protected:
    virtual QString setCurrentOperatorCommand
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology );

private:
    QModemNetworkRegistrationPrivate *d;

    void parseAvailableOperator
            ( QList<QNetworkRegistration::AvailableOperator>&list,
              const QList<QAtResultParser::Node>& values );
    void queryCurrentOperator();
};

#endif
