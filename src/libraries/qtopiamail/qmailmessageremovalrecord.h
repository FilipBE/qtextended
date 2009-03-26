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

#ifndef QMAILMESSAGEREMOVALRECORD_H
#define QMAILMESSAGEREMOVALRECORD_H

#include "qmailid.h"
#include <QString>
#include <QSharedData>
#include <QList>

class QMailMessageRemovalRecordPrivate;

class QTOPIAMAIL_EXPORT QMailMessageRemovalRecord
{
public:
    QMailMessageRemovalRecord();
    QMailMessageRemovalRecord(const QMailAccountId& parentAccountId,
                              const QString& serveruid,
                              const QString& fromMailbox = QString());
    QMailMessageRemovalRecord(const QMailMessageRemovalRecord& other);
    virtual ~QMailMessageRemovalRecord();

    QMailMessageRemovalRecord& operator=(const QMailMessageRemovalRecord& other);

    QMailAccountId parentAccountId() const;
    void setParentAccountId(const QMailAccountId& id);

    QString serverUid() const;
    void setServerUid(const QString& serverUid);

    QString fromMailbox() const;
    void setFromMailbox(const QString& mailbox);

private:
    QSharedDataPointer<QMailMessageRemovalRecordPrivate> d;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

};

typedef QList<QMailMessageRemovalRecord> QMailMessageRemovalRecordList;

#endif
