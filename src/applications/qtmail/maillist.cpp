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

#include "maillist.h"

#include <QStringList>

void MailList::clear()
{
    QListIterator<dList*> it(sortedList);
    while ( it.hasNext() )
        delete it.next();
    sortedList.clear();
    currentPos = 0;
}

int MailList::count() const
{
    return sortedList.count();
}

int MailList::size() const
{
    int total = 0;
    QListIterator<dList*> it(sortedList);
    while ( it.hasNext() )
        total += it.next()->size;

    return total;
}

int MailList::currentSize() const
{
    dList *mPtr;

    if (currentPos == 0)
        return -1;

    mPtr = sortedList.at(currentPos - 1);
    return mPtr->size;
}

QMailMessageId MailList::currentId() const
{
    dList *mPtr;

    if (currentPos == 0)
        return QMailMessageId();

    mPtr = sortedList.at(currentPos - 1);
    return mPtr->internalId;
}

QString MailList::currentMailbox() const
{
    dList *mPtr;

    if (currentPos == 0)
        return QString();

    mPtr = sortedList.at(currentPos - 1);
    return mPtr->fromBox;
}

QString* MailList::first()
{
    dList *mPtr;

    if (sortedList.count() == 0)
        return NULL;

    mPtr = sortedList.at(0);
    currentPos = 1;
    return &(mPtr->serverId);
}

QString* MailList::next()
{
    dList *mPtr;

    if ( currentPos >= static_cast<uint>(sortedList.count()))
        return NULL;

    mPtr = sortedList.at(currentPos);
    currentPos++;
    return &(mPtr->serverId);
}

void MailList::sizeInsert(const QString& serverId, uint size, const QMailMessageId& id, const QString& box)
{
    int x = 0;

    dList *newEntry = new dList;
    newEntry->serverId = serverId;
    newEntry->size = size;
    newEntry->internalId = id;
    newEntry->fromBox = box;

    QListIterator<dList*> it(sortedList);
    while ( it.hasNext() ) {
        if (newEntry->size < it.next()->size) {
            sortedList.insert(x, newEntry);
            return;
        }
        ++x;
    }
    sortedList.append(newEntry);
}

void MailList::append(const QString& serverId, uint size, const QMailMessageId& id, const QString& box)
{
    dList *newEntry = new dList;
    newEntry->serverId = serverId;
    newEntry->size = size;
    newEntry->internalId = id;
    newEntry->fromBox = box;

    sortedList.append(newEntry);
}

bool MailList::contains(const QMailMessageId& id) const
{
    foreach (const dList* item, sortedList)
        if (item->internalId == id)
            return true;

    return false;
}

void MailList::moveFront(const QMailMessageId& id)
{
    dList *currentPtr;
    uint tempPos;

    tempPos = currentPos;
    if ( tempPos >= static_cast<uint>(sortedList.count()) )
        return;
    currentPtr = sortedList.at(tempPos);
    while ( ((tempPos+1) < static_cast<uint>(sortedList.count())) && ( currentPtr->internalId != id) ) {
        tempPos++;
        currentPtr = sortedList.at(tempPos);
    }

    if ( (currentPtr != NULL) && (currentPtr->internalId == id) ) {
        dList *itemPtr = sortedList.takeAt(tempPos);
        sortedList.insert(currentPos, itemPtr);
    }
}

//only works if mail is not already in download
bool MailList::remove(const QMailMessageId& id)
{
    dList *currentPtr;
    uint tempPos;

    tempPos = currentPos;
    if ( tempPos >= static_cast<uint>(sortedList.count()) )
        return false;
    currentPtr = sortedList.at(tempPos);
    while ( ((tempPos + 1) < static_cast<uint>(sortedList.count())) && ( currentPtr->internalId != id) ) {
        tempPos++;
        currentPtr = sortedList.at(tempPos);
    }

    if ( (currentPtr != NULL) && (currentPtr->internalId == id) ) {
        sortedList.removeAt(tempPos);
        return true;
    }
    return false;
}

QStringList MailList::serverUids() const
{
    QStringList ids;
    foreach (const dList* item, sortedList)
        ids.append(item->serverId);

    return ids;
}

QMailMessageIdList MailList::mailIds() const
{
    QMailMessageIdList ids;
    foreach (const dList* item, sortedList)
        ids.append(item->internalId);

    return ids;
}

QMailMessageId MailList::mailId(const QString& serverUid) const
{
    foreach (const dList* item, sortedList)
        if (item->serverId == serverUid)
            return item->internalId;

    return QMailMessageId();
}

