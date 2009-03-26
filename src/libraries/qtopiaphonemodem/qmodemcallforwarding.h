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

#ifndef QMODEMCALLFORWARDING_H
#define QMODEMCALLFORWARDING_H

#include <qcallforwarding.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallForwarding : public QCallForwarding
{
    Q_OBJECT
public:
    explicit QModemCallForwarding( QModemService *service );
    ~QModemCallForwarding();

public slots:
    void requestForwardingStatus( QCallForwarding::Reason reason );
    void setForwarding( QCallForwarding::Reason reason,
                        const QCallForwarding::Status& status,
                        bool enable );

private slots:
    void requestDone( bool ok, const QAtResult& result );
    void setDone( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
