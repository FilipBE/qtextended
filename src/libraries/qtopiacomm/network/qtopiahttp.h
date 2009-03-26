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

#ifndef QTOPIAHTTP_H
#define QTOPIAHTTP_H

/*
   INTERNAL CLASS

   This class will be removed in a future version of Qtopia, being replaced
   by functionality from future version of Qt.  The API's will likely not be compatible.
*/
#include <qtopiaglobal.h>

#include <QList>
#include <QPair>
#include <QString>
#include <QHttp>
#include <QUrl>
#include <QDateTime>

class QByteArray;
class QIODevice;

struct QTOPIACOMM_EXPORT QHttpCookie
{
    QHttpCookie() : secure(false), version(1) {}
    explicit QHttpCookie(const QString &encoded);

    QString encode() const;
    bool supersedes(const QHttpCookie &other) const;

    QString name;
    QString value;
    QString domain;
    QString path;
    QDateTime expires;
    QString comment;
    bool secure;
    int version;
};

class QHttpCookieJarData;
class QTOPIACOMM_EXPORT QHttpCookieJar
{
public:
    QHttpCookieJar();
    virtual ~QHttpCookieJar();

    static QList<QHttpCookie> readCookies(const QUrl &, const QHttpResponseHeader &);
    static void writeCookies(QHttpRequestHeader&, const QList<QHttpCookie> &);

    virtual void storeCookies(const QList<QHttpCookie> &);
    virtual QList<QHttpCookie> validCookies(const QUrl &) const;

    virtual void storeCookie(const QHttpCookie &);
    void removeCookie(const QHttpCookie &);

    void expireCookies();

    void setMaxCookies(int);
    void setMaxDomainCookies(int);
    void setMaxCookieSize(int);

    int maxCookies() const;
    int maxDomainCookies() const;
    int maxCookieSize() const;

    /* TODO need ability to iterate over all cookies
       Should not mean duplicating the list.
     */

    /* TODO also need option for perminent storage */

private:
    QHttpCookieJarData *d;
};

class QtopiaHttpData;
class QTOPIACOMM_EXPORT QtopiaHttp : public QHttp
{
    Q_OBJECT
public:
    explicit QtopiaHttp(QObject *parent = 0);
    ~QtopiaHttp();

    void setCookieJar(QHttpCookieJar *jar);
    QHttpCookieJar *cookieJar() const;

    void setAuthorization(const QString &);

    // fetch to distinguish from get/post/request, which do not invoke the related signals
    int startFetch(const QUrl &, const QList< QPair<QString, QString> > &, QIODevice *to = 0);
    int startFetch(const QUrl &, QIODevice *to = 0);

    QStringList supportedSchemes();

    // by default 0, doesn't take ownership.
    // note just setting a QTcpSocket won't work ;)
    QTcpSocket *sslSocket() const;
    void setSslSocket(QTcpSocket *);

    static QByteArray urlEncode(const QList< QPair<QString, QString> > &list);

public slots:
    void abort();

signals:
    void completedFetch();
    void failedFetch();
    void redirectedFetch(const QString &, int, int);

    void dataReadProgress(int, int);

private slots:
    void httpRequestFinished(int requestId, bool error);
    void readResponseHeader(const QHttpResponseHeader &responseHeader);
    void updateDataReadProgress(int bytesRead, int totalBytes);

private:
    QtopiaHttpData *d;
};
#endif
