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



#ifndef MAILLIST_H
#define MAILLIST_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMailMessageId>


struct dList{
    QString serverId;
    uint size;
    QMailMessageId internalId;
    QString fromBox;
};

class MailList : public QObject
{
    Q_OBJECT

public:
    void clear();
    int count() const;
    int size() const;
    int currentSize() const;
    QMailMessageId currentId() const;
    QString currentMailbox() const;

    QString* first();
    QString* next();

    void sizeInsert(const QString& serverId, uint size, const QMailMessageId& id, const QString& box);
    void append(const QString& serverId, uint size, const QMailMessageId& id, const QString& box);

    bool contains(const QMailMessageId& id) const;

    void moveFront(const QMailMessageId& id);
    bool remove(const QMailMessageId& id);

    QStringList serverUids() const;
    QMailMessageIdList mailIds() const;

    QMailMessageId mailId(const QString& serverId) const;

private:
    QList<dList*> sortedList;
    uint currentPos;
};

#endif
