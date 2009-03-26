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
#ifndef BUDDYSYNCERTASK_H
#define BUDDYSYNCERTASK_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QSet>

#include <QCollectivePresenceInfo>

class QContactModel;
class QCollectivePresence;
class QCommServiceManager;
class QValueSpaceObject;

class BuddySyncerTask : public QObject
{
    Q_OBJECT
public:
    BuddySyncerTask(QObject *parent = 0);
    ~BuddySyncerTask();

public slots:
    void presenceChanged(QStringList list);
    void presenceChanged(QCollectivePresence *presence, QStringList list);
    void providersChanged();
    void pendingPublishRequestsChanged();
    void pendingPublishRequestsChanged(QCollectivePresence *p);

private:
    QCollectivePresenceInfo presenceInformation(const QString& provider, const QString& uri);
    QList<QCollectivePresenceInfo> presenceInformation();
    void setPresenceInformation(const QString& provider, QList<QCollectivePresenceInfo> presences);
    void clearPresenceInformation(const QString& provider);
    void clearAllPresenceInformation();


    QContactModel *mModel;
    QCommServiceManager *mServiceManager;
    QList<QCollectivePresence *> mPresences;
    QSet<QString> mPresenceNames;

    QValueSpaceObject *mVSObject;
    uint mVSValue;
};

#endif
