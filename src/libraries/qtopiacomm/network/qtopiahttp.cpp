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

#include <QHttp>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>
#include <QTcpSocket>

#include <qtopianamespace.h>
#include <qtopialog.h>

#include "qtopiahttp.h"

/*
   Cookies in this file have been developed against:
    http://rfc.net/rfc2109.html
*/
static QString token(QString str, QChar c1, QChar c2, int *index)
{
    int start, stop;
    start = str.indexOf(c1, *index, Qt::CaseInsensitive);
    if (start == -1)
        return QString();
    stop = str.indexOf(c2, ++start, Qt::CaseInsensitive);
    if (stop == -1)
        return QString();

    *index = stop + 1;
    return str.mid(start, stop - start);
}

/*!
  \internal
  \class QHttpCookie
    \inpublicgroup QtBaseModule
*/
QHttpCookie::QHttpCookie(const QString &encoded) : secure(false)
{
    // ; seperatied, name=value, escaping unknown as yet, probably percent encoding though.
    QStringList list = encoded.split("; ");
    bool first = true;
    foreach(QString part, list)
    {
        int epos = part.indexOf("=");
        if (epos < 0)
            continue;
        QString k = part.left(epos);
        QString v = part.mid(epos+1);
        if (first) {
            first = false;
            name = k;
            value = v;
        } else if (k.toLower() == "expires"){
            // RFC-822 datetime
            /*
               date-time   =  [ day "," ] date time        ; dd mm yy
                                                           ;  hh:mm:ss zzz

               day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
                           /  "Fri"  / "Sat" /  "Sun"

               date        =  1*2DIGIT month 2DIGIT        ; day month year
                                                           ;  e.g. 20 Jun 82

               month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
                           /  "May"  /  "Jun" /  "Jul"  /  "Aug"
                           /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"

               time        =  hour zone                    ; ANSI and Military

               hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
                                                           ; 00:00:00 - 23:59:59

               zone        =  "UT"  / "GMT"                ; Universal Time
                                                           ; North American : UT
                           /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
                           /  "CST" / "CDT"                ;  Central:  - 6/ - 5
                           /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
                           /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
                           /  1ALPHA                       ; Military: Z = UT;
                                                           ;  A:-1; (J not used)
                                                           ;  M:-12; N:+1; Y:+12
                           / ( ("+" / "-") 4DIGIT )        ; Local differential
                                                           ;  hours+min. (HHMM)
                                                           */

            // currently copied out of src/libraries/qtopiamail/mailmessage.cpp
            const QString Months("janfebmaraprmayjunjulaugsepoctnovdec");
            QDateTime dateTime;
            QString str, org;
            int month = -1, day = -1, year = -1;
            bool ok;
            int x, index;
            uint len;
            QString time, timeDiff;

            for (int z = 0; z < v.length(); z++) {
                if (v[z] != ',') {
                    org += v[z];
                } else {
                    org += " ";
                }
            }

            org = org.simplified();
            org += " ";

            index = org.indexOf(' ');
            str = org.left((uint) index);
            while ( str != QString() ) {
                len = str.length();
                index--;
                if ( (day == -1) && (len <= 2) ) {
                    x = str.toInt(&ok);
                    if ( ok )
                        day = x;
                } else if ( (month == -1) && (len == 3) ) {
                    x = Months.indexOf( str.toLower() );
                    if ( x >= 0 )
                        month = (x + 3) / 3;
                } else if ( (year == -1) && (len == 4) ) {
                    x = str.toInt(&ok);
                    if ( ok )
                        year = x;
                } else if ( time.isEmpty() && len == 8 ) {      // time part: 14:22:22
                    time = str;
                } else if ( timeDiff.isEmpty() && len == 5 ) {  // time localizer: +1000
                    timeDiff = str;
                }

                str = token(org, ' ', ' ', &index);
            }

            if ( (day != -1) && (month != -1) && (year != -1) ) {
                dateTime.setDate( QDate(year, month, day) );

                if ( !time.isEmpty() ) {
                    int h = time.left(2).toInt();
                    int m = time.mid(3, 2).toInt();

                    dateTime.setTime(QTime(h, m));

                /* TODO for now ignore timezone information
                    int senderLocal = UTCToSecs( timeDiff );

                    //adjust sender local time to our local time
                    int localDiff = timeZoneDiff() - senderLocal;

                    //we add seconds after adding time, as it may change the date
                    dateTime.setTime( QTime(h, m) );
                    dateTime = dateTime.addSecs( localDiff);
                    */
                }
            }

            expires = dateTime;
        } else if (k.toLower() == "path") {
            path = v;
        }
    }
    if (path.isEmpty())
        path = "/";
}

QString QHttpCookie::encode() const
{
    // expires and path are set by server only.  so since this is a client cookie
    // we only set name and value when encoding
    return name + "=" + value;
}

bool QHttpCookie::supersedes(const QHttpCookie &other) const
{
    return name == other.name && domain == other.domain && path == other.path;
}

/* TODO Need to be able to limit total number of cookies.
   Based of
   limiting total memory usage
    void setMaxCookies(int);
   limiting single domain hogging of resources
    void setMaxDomainCookies(int);
    void setMaxCookieSize(int);
*/
class QHttpCookieJarData
{
public:
    QList<QHttpCookie> mCookies;
};

/*!
  \internal
  \class QHttpCookieJar
    \inpublicgroup QtBaseModule
*/
QHttpCookieJar::QHttpCookieJar()
{
    d = new QHttpCookieJarData;
}

QHttpCookieJar::~QHttpCookieJar() {}

// most allow at least 300
void QHttpCookieJar::setMaxCookies(int) {}
// must allow at least 20
void QHttpCookieJar::setMaxDomainCookies(int) {}
// must allow at least 4k encoded
void QHttpCookieJar::setMaxCookieSize(int) {}

// all return -1 meaning unlimited.  change when data structure supports the kind of
// caching structure required.
int QHttpCookieJar::maxCookies() const { return -1; }
int QHttpCookieJar::maxDomainCookies() const { return -1; }
int QHttpCookieJar::maxCookieSize() const { return -1; }

QList<QHttpCookie> QHttpCookieJar::validCookies(const QUrl &u) const
{
    QDateTime dt = QDateTime::currentDateTime();
    QString domain = u.host();
    QString path = u.path();
    QListIterator<QHttpCookie> it(d->mCookies);
    QList<QHttpCookie> result;
    while (it.hasNext()) {
        QHttpCookie c = it.next();
        if (domain.endsWith(c.domain) && path.startsWith(c.path)
                && (c.expires.isNull() || c.expires > dt))
            result.append(c);
    }
    return result;
}

void QHttpCookieJar::writeCookies(QHttpRequestHeader &header, const QList<QHttpCookie> &list)
{
    // alternative (and possibly more correct approach)
    // is one header value with '; ' separated cookie values.
    foreach(QHttpCookie c, list)
        header.addValue("cookie", c.encode());
}

QList<QHttpCookie> QHttpCookieJar::readCookies(const QUrl &u, const QHttpResponseHeader &header)
{
    /* should convert time zone to current time zone */
    QDateTime dt = QDateTime::currentDateTime();
    QString domain = u.host();
    QString path = u.path();

    QList<QHttpCookie> newCookies;
    QList< QPair<QString, QString> > list = header.values();
    QListIterator< QPair<QString, QString> > hit(list);
    while(hit.hasNext()) {
        QPair<QString, QString> item = hit.next();
        if (item.first.toLower() == "set-cookie") {
            qLog(Network) << "QHttpCookieJar::readCookies() -" << item.second;
            newCookies.append(QHttpCookie(item.second));
        }
    }
    return newCookies;
}

void QHttpCookieJar::storeCookies(const QList<QHttpCookie> &newCookies)
{
    /*
       1. remove superseded cookies from internal list
       2. add new cookies to list.
     */
    QMutableListIterator<QHttpCookie> cit(d->mCookies);
    while (cit.hasNext()) {
        QHttpCookie c = cit.next();
        QListIterator<QHttpCookie> ncit(newCookies);
        while(ncit.hasNext()) {
            QHttpCookie nc = ncit.next();
            if (nc.supersedes(c)) {
                cit.remove();
                break; // removed, so don't check the rest of the new ones.
            }
        }
    }
    // now to add
    d->mCookies += newCookies;
}

void QHttpCookieJar::removeCookie(const QHttpCookie &cookie)
{
    QMutableListIterator<QHttpCookie> cit(d->mCookies);
    while (cit.hasNext()) {
        QHttpCookie c = cit.next();
        if (cookie.supersedes(c)) {
            cit.remove();
        }
    }
}

void QHttpCookieJar::storeCookie(const QHttpCookie &cookie)
{
    removeCookie(cookie); // clear any old ones
    d->mCookies.append(cookie);
}

void QHttpCookieJar::expireCookies()
{
}

class QtopiaHttpData
{
public:
    QtopiaHttpData() : httpGetId(0), httpRequestAborted(false),
                       mCurrentSocket(0), mSslSocket(0), mTcpSocket(0), mJar(0)
    {}
    int httpGetId;
    bool httpRequestAborted;

    QTcpSocket *mCurrentSocket;
    QTcpSocket *mSslSocket;
    QTcpSocket *mTcpSocket;

    QHttpCookieJar *mJar;

    QUrl currentUrl;

    QString mAuth;
};
/*!
  \internal
  \class QtopiaHttp
    \inpublicgroup QtBaseModule

  \brief The QtopiaHttp class provides a wrapper around QHttp to make fetching redirected urls
  easier
*/
QtopiaHttp::QtopiaHttp(QObject *parent)
    : QHttp(parent)
{
    d = new QtopiaHttpData();

    connect(this, SIGNAL(requestFinished(int,bool)),
            this, SLOT(httpRequestFinished(int,bool)));
    connect(this, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(updateDataReadProgress(int,int)));
    connect(this, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(readResponseHeader(QHttpResponseHeader)));
    d->mSslSocket = 0;
    d->mTcpSocket = new QTcpSocket(this);
}

QtopiaHttp::~QtopiaHttp()
{
    /* check non object delete requiremenst */
}

QTcpSocket *QtopiaHttp::sslSocket() const
{
    return d->mSslSocket;
}

/*!
  Sets the socket used to handle https requests.
  Note that the socket set must sublcass QTcpSocket in such a way that it supports SSL.  Setting
  an unsecure socket will result in a connection error when attempting to get https pages.
*/
void QtopiaHttp::setSslSocket(QTcpSocket *socket)
{
    d->mSslSocket = socket;
}

void QtopiaHttp::setCookieJar(QHttpCookieJar *jar)
{
    d->mJar = jar;
}
QHttpCookieJar *QtopiaHttp::cookieJar() const
{
    return d->mJar;
}

void QtopiaHttp::setAuthorization(const QString &s)
{
    d->mAuth = s;
}

QStringList QtopiaHttp::supportedSchemes()
{
    QStringList supported;
    supported << "http";
    if (d->mSslSocket)
        supported << "https";
    return supported;
}

QByteArray QtopiaHttp::urlEncode(const QList< QPair< QString, QString > > &list)
{
    if (!list.isEmpty()) {
        QListIterator< QPair<QString, QString> > it(list);
        QUrl encoder;
        while (it.hasNext()) {
            QPair<QString, QString> item = it.next();
            encoder.addQueryItem(item.first, item.second);
        }
        return encoder.encodedQuery();
    }
    return QByteArray();
}

int QtopiaHttp::startFetch(const QUrl &u, QIODevice *to)
{
    QList< QPair<QString, QString> > list;
    return startFetch(u, list, to);
}

/* get or post is decided by whether post data is provided.  Post data
   does not follow redirects */
int QtopiaHttp::startFetch(const QUrl &u, const QList< QPair<QString, QString> > &formData, QIODevice *to)
{
    qLog(Network) << "QtopiaHttp::startFetch(" << u << ")";
    if (!u.isValid())
        return -1;

    qLog(Network) << "QtopiaHttp::startFetch() URL is valid";
    /* Determine socket and port
       should be in QHttp?
       */
    bool mSecure;
    if (u.scheme() == "http") {
        mSecure = false;
        d->mCurrentSocket = d->mTcpSocket;
    } else if (d->mSslSocket && u.scheme() == "https") {
        mSecure = true;
        d->mCurrentSocket = d->mSslSocket;
    } else {
        // unsupported scheme.
        qWarning("Unsupported scheme: %s", u.scheme().toLocal8Bit().constData());
        return -1;
    }

    // set state
    d->currentUrl = u;
    d->httpRequestAborted = false;

    // set socket
    QHttp::setSocket(d->mCurrentSocket);
    // port port
    int port = u.port();
    if (port < 0)
        port = mSecure ? 443 : 80;
    QHttp::setHost(u.host(), mSecure ? ConnectionModeHttps : ConnectionModeHttp, port);

    /* set post data */
    QByteArray uploadData = urlEncode(formData);

    qLog(Network) << "QtopiaHttp::startFetch() - post data:" << uploadData;

    /* set up header */
    // don't want 'path'.  Want 'thing to pass to QHttp', which is path+query+fragment.
    QString path = u.path();
    if (u.hasQuery())
        path += "?" + u.encodedQuery();
    // this shouldn't be needed
    if (path.isEmpty())
        path = "/";

    // clever for post and get, along with content-type and size.
    QHttpRequestHeader header(uploadData.size() > 0 ? "POST" : "GET", path);
    if (uploadData.size()) {
        header.setContentLength(uploadData.size());
        header.setContentType("application/x-www-form-urlencoded");
    }
    header.setValue("Host", u.host());

    if (!d->mAuth.isEmpty()) {
        header.setValue("Authorization", d->mAuth);
    }

    if (d->mJar)
        QHttpCookieJar::writeCookies(header, d->mJar->validCookies(u));

    /* do request */
    d->httpGetId = QHttp::request(header, uploadData, to);
    qLog(Network) << "QtopiaHttp::startFetch() requestId:" << d->httpGetId;
    return d->httpGetId;
}

void QtopiaHttp::abort()
{
    qLog(Network) << "QtopiaHttp::abort()";
    d->httpRequestAborted = true;
    QHttp::abort();
}

void QtopiaHttp::httpRequestFinished(int requestId, bool error)
{
    qLog(Network) << "QtopiaHttp::httpRequestFinished(" << requestId << error << ")";
    if (requestId == d->httpGetId)
        d->currentUrl = QUrl();

    if (d->httpRequestAborted) {
        emit failedFetch();
        qLog(Network) << "QtopiaHttp::httpRequestFinished() - aborted";
        return;
    }

    if (requestId != d->httpGetId)
        return;

    if (error) {
        qLog(Network) << "QtopiaHttp::httpRequestFinished() - error";
        emit failedFetch();
        return;
    }
    emit completedFetch();
}

void QtopiaHttp::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    qLog(Network) << "QtopiaHttp::readResponseHeader() -" << responseHeader.toString();
    if (d->mJar)
        d->mJar->storeCookies(QHttpCookieJar::readCookies(d->currentUrl, responseHeader));

    switch(responseHeader.statusCode()) {
        default:
            qLog(Network) << "QtopiaHttp::readResponseHeader() - unhandled response code:" << responseHeader.statusCode();
            d->httpRequestAborted = true;
            QHttp::abort();
            return; // avoid further processing.
        case 200:
            // ok
            qLog(Network) << "QtopiaHttp::readResponseHeader() - 200 OK";
            break;
        case 301:
        case 302:
            // temporary redirect.  Handle perminant ones the same way, although there
            // is an argument to simply failing with an appropriate error on 301.
            // may be bug in Qt, case dependent searching on keys that are case independent.
            qLog(Network) << "QtopiaHttp::readResponseHeader() - 30x redirect";
            QString newLocation = responseHeader.value("location");

            QUrl newUrl(newLocation);
            newUrl = d->currentUrl.resolved(newUrl);
            if (!supportedSchemes().contains(newUrl.scheme())) {
                // TODO should record reason for fail
                QHttp::abort();
                d->httpRequestAborted = true;
                qLog(Network) << "QtopiaHttp::readResponseHeader() - 30x redirect - unsupported scheme";
                return;
            }
            emit redirectedFetch(newLocation, responseHeader.statusCode(),
                    startFetch(newUrl, currentDestinationDevice()));
            break;
    }
}

void QtopiaHttp::updateDataReadProgress(int bytesRead, int totalBytes)
{
    if (d->httpRequestAborted)
        return;

    emit dataReadProgress(bytesRead, totalBytes);
}
