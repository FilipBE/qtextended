/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QNETWORKREPLYIMPL_P_H
#define QNETWORKREPLYIMPL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Network Access API.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qnetworkreply.h"
#include "qnetworkreply_p.h"
#include "qnetworkaccessmanager.h"
#include "qnetworkaccessbackend_p.h"
#include "qnetworkproxy.h"
#include "QtCore/qmap.h"
#include "QtCore/qqueue.h"
#include "private/qringbuffer_p.h"

QT_BEGIN_NAMESPACE

class QAbstractNetworkCache;
class QNetworkAccessBackend;

class QNetworkReplyImplPrivate;
class QNetworkReplyImpl: public QNetworkReply
{
    Q_OBJECT
public:
    QNetworkReplyImpl(QObject *parent = 0);
    ~QNetworkReplyImpl();
    virtual void abort();

    // reimplemented from QNetworkReply
    virtual void close();
    virtual qint64 bytesAvailable() const;
    virtual void setReadBufferSize(qint64 size);

    virtual qint64 readData(char *data, qint64 maxlen);
    virtual bool event(QEvent *);

#ifndef QT_NO_OPENSSL
    Q_INVOKABLE QSslConfiguration sslConfigurationImplementation() const;
    Q_INVOKABLE void setSslConfigurationImplementation(const QSslConfiguration &configuration);
    virtual void ignoreSslErrors();
#endif

    Q_DECLARE_PRIVATE(QNetworkReplyImpl)
    Q_PRIVATE_SLOT(d_func(), void _q_startOperation())
    Q_PRIVATE_SLOT(d_func(), void _q_sourceReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_sourceReadChannelFinished())
    Q_PRIVATE_SLOT(d_func(), void _q_copyReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_copyReadChannelFinished())
};

class QNetworkReplyImplPrivate: public QNetworkReplyPrivate
{
public:
    enum InternalNotifications {
        NotifyDownstreamReadyWrite,
        NotifyUpstreamReadyRead,
        NotifyCloseDownstreamChannel,
        NotifyCloseUpstreamChannel,
        NotifyCopyFinished
    };

    enum State {
        Idle,
        Opening,
        Working,
        Finished,
        Aborted
    };

    typedef QQueue<InternalNotifications> NotificationQueue;

    QNetworkReplyImplPrivate();

    void _q_startOperation();
    void _q_sourceReadyRead();
    void _q_sourceReadChannelFinished();
    void _q_copyReadyRead();
    void _q_copyReadChannelFinished();

    void setup(QNetworkAccessManager::Operation op, const QNetworkRequest &request,
               QIODevice *outgoingData);
    void setNetworkCache(QAbstractNetworkCache *networkCache);
    void backendNotify(InternalNotifications notification);
    void handleNotifications();

    // callbacks from the backend (through the manager):
    void setCachingEnabled(bool enable);
    void consume(qint64 count);
    qint64 nextDownstreamBlockSize() const;
    void feed(const QByteArray &data);
    void feed(QIODevice *data);
    void finished();
    void error(QNetworkReply::NetworkError code, const QString &errorString);
    void metaDataChanged();
    void redirectionRequested(const QUrl &target);
    void sslErrors(const QList<QSslError> &errors);

    QNetworkAccessBackend *backend;
    QIODevice *outgoingData;
    QIODevice *copyDevice;
    QAbstractNetworkCache *networkCache;

    NotificationQueue pendingNotifications;
    QUrl urlForLastAuthentication;
#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy networkProxy;
    QNetworkProxy lastProxyAuthentication;
#endif

    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;
    qint64 bytesDownloaded;
    qint64 lastBytesDownloaded;
    qint64 bytesUploaded;

    QString httpReasonPhrase;
    int httpStatusCode;

    State state;
    bool isEncrypted;

    Q_DECLARE_PUBLIC(QNetworkReplyImpl)
};

QT_END_NAMESPACE

#endif
