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

#ifndef MMSCOMMS_H
#define MMSCOMMS_H

#include <QObject>
#include <QMailAccount>

class MMSMessage;
class QUrl;

class MmsComms : public QObject
{
    Q_OBJECT
public:
    MmsComms(const QMailAccountId &accountId, QObject *parent=0);
    virtual ~MmsComms();

    virtual void sendMessage(MMSMessage &msg, const QByteArray& encoded) = 0;
    virtual void retrieveMessage(const QUrl &url) = 0;
    virtual bool isActive() const = 0;
    virtual void clearRequests() = 0;

    QString networkConfig() const;

signals:
    void notificationInd(const MMSMessage &msg);
    void deliveryInd(const MMSMessage &msg);
    void sendConf(const MMSMessage &msg);
    void retrieveConf(const MMSMessage &msg, int size);
    void statusChange(const QString &status);
    void error(int code, const QString &msg);
    void transferSize(int size);
    void transfersComplete();

protected:
    QMailAccountId accountId;
};

#endif
