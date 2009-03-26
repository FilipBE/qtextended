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

#ifndef QIPSEC_P_H
#define QIPSEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTOPIA_NO_IPSEC
#include "qvpnclient.h"

#include <QWidget>

class QIPSec : public QVPNClient
{
    Q_OBJECT
public:
    explicit QIPSec( QObject* parent = 0 );
    explicit QIPSec( bool serverMode, uint vpnID, QObject* parent = 0 );
    ~QIPSec();

    QVPNClient::Type type() const;
    void connect();
    void disconnect();
    QDialog* configure( QWidget* parent = 0 );
    QVPNClient::State state() const;
    QString name() const;
    void cleanup();
};
#endif //QTOPIA_NO_IPSEC
#endif
