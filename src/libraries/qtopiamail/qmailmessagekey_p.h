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

#ifndef QMAILMESSAGEKEY_P_H
#define QMAILMESSAGEKEY_P_H

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

#include "qmailkeyargument_p.h"

#include <QMailMessageKey>

class QMailMessageKeyPrivate : public QSharedData
{
public:
    typedef QMailKeyArgument<QMailMessageKey::Property> Argument;

    QMailMessageKeyPrivate() : QSharedData(),
                               combiner(QMailDataComparator::None),
                               negated(false)
    {};

    QMailDataComparator::Combiner combiner;
    bool negated;
    QList<Argument> arguments;
    QList<QMailMessageKey> subKeys;
};

#endif
