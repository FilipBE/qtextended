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

#ifndef WEBLITECORE_H
#define WEBLITECORE_H

#include <QUuid>
#include <QUrl>
#include <QMetaType>
#include <QHttp>
#include <QDataStream>
#include <QDateTime>
struct WebLiteLoadRequest
{
    QUuid clientId;
    QUrl url;
    bool backgroundDownload, direct;
    WebLiteLoadRequest()            :backgroundDownload(false),direct(false){}
};

inline QDataStream & operator<<(QDataStream & ds, const WebLiteLoadRequest & r)
{
    ds << r.clientId;
    ds << r.url;
    ds << r.backgroundDownload;
    ds << r.direct;
    return ds;
}
inline QDataStream & operator>>(QDataStream & ds, WebLiteLoadRequest & r)
{
    ds >> r.clientId;
    ds >> r.url;
    ds >> r.backgroundDownload;
    ds >> r.direct;
    return ds;
}

struct WebLiteLoadResponse
{
    QUrl url;
    QString filename;
    QString contentType;
    int totalBytes;
    int loadedBytes;
    QUuid clientId;
    QHttp::Error error;
    QString cachePath;
    QDateTime   lastModified;
    int        refCount;
    int        cacheValue;
    enum Status
    {
        NoStatus,
        RequestAcknowledged,
        ConnectingToHost,
        BeginningDownload,
        SomeData,
        Complete,
        Error,
        Aborted,
        OfflineData
    } status;
    WebLiteLoadResponse() : totalBytes(0),loadedBytes(0),error(QHttp::NoError),refCount(0),cacheValue(0),status(NoStatus) {}
};
inline QDataStream & operator<<(QDataStream & ds, const WebLiteLoadResponse & r)
{
    ds << r.clientId;
    ds << r.url;
    ds << r.totalBytes;
    ds << r.loadedBytes;
    ds << (int)r.error;
    ds << r.contentType;
    ds << r.filename;
    ds << (int)r.status;
    ds << r.cachePath;
    ds << r.lastModified;
    ds << r.cacheValue;
    return ds;
}
inline QDataStream & operator>>(QDataStream & ds, WebLiteLoadResponse & r)
{
    ds >> r.clientId;
    ds >> r.url;
    ds >> r.totalBytes;
    ds >> r.loadedBytes;
    int n;
    ds >> n;
    r.error = (QHttp::Error)n;
    ds >> r.contentType;
    ds >> r.filename;
    ds >> n;
    r.status = (WebLiteLoadResponse::Status)n;
    ds >> r.cachePath;
    ds >> r.lastModified;
    ds >> r.cacheValue;
    return ds;
}

#endif
