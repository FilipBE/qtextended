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

#ifndef QMODEMPREFERREDNETWORKOPERATORS_H
#define QMODEMPREFERREDNETWORKOPERATORS_H

#include <qpreferrednetworkoperators.h>

class QModemService;
class QAtResult;
class QModemPreferredNetworkOperatorsPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemPreferredNetworkOperators
        : public QPreferredNetworkOperators
{
    Q_OBJECT
public:
    explicit QModemPreferredNetworkOperators( QModemService *service );
    ~QModemPreferredNetworkOperators();

public slots:
    void requestOperatorNames();
    void requestPreferredOperators
        ( QPreferredNetworkOperators::List list );
    void writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper );

private slots:
    void copn( bool ok, const QAtResult& result );
    void cplsQuery( bool ok, const QAtResult& result );
    void cpolQuery( bool ok, const QAtResult& result );
    void cpolSet( bool ok, const QAtResult& result );
    void cpolSet2( bool ok, const QAtResult& result );

protected:
    bool deleteBeforeUpdate() const;
    void setDeleteBeforeUpdate( bool value );
    bool quoteOperatorNumber() const;
    void setQuoteOperatorNumber( bool value );

private:
    QModemPreferredNetworkOperatorsPrivate *d;

    uint listNumber( QPreferredNetworkOperators::List list );
    void writeNextPreferredOperator();
};

#endif
