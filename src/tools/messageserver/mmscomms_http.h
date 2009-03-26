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

#ifndef MMSCOMMS_HTTP_H
#define MMSCOMMS_HTTP_H

#include "mmscomms.h"
#include <qlist.h>

class MMSMessage;
class QTimer;
class QUrl;
class QHttp;
class QHttpResponseHeader;
class QHttpRequestHeader;

class MmsCommsHttp : public MmsComms
{
    Q_OBJECT
public:
    MmsCommsHttp(const QMailAccountId &accountId, QObject *parent=0);
    ~MmsCommsHttp();

    virtual void sendMessage(MMSMessage &msg, const QByteArray& encoded);
    virtual void retrieveMessage(const QUrl &url);
    virtual bool isActive() const;
    virtual void clearRequests();

private slots:
    void dataReadProgress(int done, int total);
    void dataSendProgress(int done, int total);
    void done(bool error);
    void cleanup();
    void requestFinished(int id, bool error);
    void requestStarted(int id);
    void responseHeaderReceived(const QHttpResponseHeader &resp);
    void stateChanged(int state);

private:
    void addAuth(QHttpRequestHeader &header);
    QHttp *createHttpConnection(const QString &host, int port);
    void destroyHttpConnection(QHttp *http);

private:
    QHttp *rhttp;
    QHttp *shttp;
    int rId;
    int sId;
    int rStatus;
    int sStatus;
    QTimer *timer;
    QList<QHttp*> deleteList;
};

#endif
