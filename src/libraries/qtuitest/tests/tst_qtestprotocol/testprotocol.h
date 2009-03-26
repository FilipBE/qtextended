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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#ifndef TESTPROTOCOL_H
#define TESTPROTOCOL_H

#include "qtestprotocol_p.h"

/*
    Test protocol which simply stores received messages in a list.
*/
class TestProtocol : public QTestProtocol
{
    Q_OBJECT

public:
    QList<QTestMessage>& messages();

protected:
    void processMessage(QTestMessage*);

private:
    QList<QTestMessage> m_messages;
};

#endif

