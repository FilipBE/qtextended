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

#include "webliteserver.h"
#include <QHttp>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QtopiaIpcAdaptor>
#include <QDir>
#include <QQueue>
#include <QMimeType>
#include <QNetworkInterface>
#include <QtopiaApplication>
#include <QTextDocument>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#define HTTP_DATE_FORMAT "ddd, dd MMM yyyy hh:mm:ss"

static QString cachePathByMimeType(const QString & mt)
{
    Q_UNUSED(mt);
    return QDir::homePath() + "/weblite-cache";
}

static bool verifyFile (const WebLiteLoadResponse & rsp)
{
    // file might be saved in cache, but incomplete. only retrieve complete files from cacheg
    if (rsp.filename.length())
    {
        QFile f (rsp.filename);
        if (f.exists())
        {
            int fileSize = f.size();
            return fileSize == rsp.totalBytes;
        }
    }
    return false;
}
// quota information for a specific pqth
struct CacheQuota
{
    int totalSpace;
    int usedSpace;
};

class WebLiteServerHttpHandler : public QObject
{
    Q_OBJECT
    public:

        bool firstTry, headOnly;
        QPointer<QHttp> http;
        QPointer<QFile> file;
        QSet<QUuid> activeClients;

        QString urlPath(const QUrl & url) const;
        void request(const QUrl & url, const QDateTime & lastModified);
        void get (const QUrl & url, const QDateTime & lastModified );
        void head(const QUrl & url, const QDateTime & lastModified );

        WebLiteLoadResponse rsp;
        WebLiteLoadRequest req;

        WebLiteServerPrivate* d;
        WebLiteServerHttpHandler (const WebLiteLoadRequest & r, const WebLiteLoadResponse &, QObject* o = NULL);
        virtual ~WebLiteServerHttpHandler ();
    public slots:
        void httpDone (bool error);
        void dataReadProgress(int done, int total);
        void readResponseHeader(const QHttpResponseHeader &);
        void abort (const QUuid &);
        void firstResponse ();

    signals:
        void response (const WebLiteLoadResponse &);
        void useCache (const WebLiteLoadRequest &);
        void downloadStarted ();
        void downloadFinished ();
        void cacheSpaceNeeded (const WebLiteLoadResponse &);
};

void WebLiteServerHttpHandler::firstResponse ()
{
    rsp.status = WebLiteLoadResponse::ConnectingToHost;
    emit response (rsp);
}

class WebLiteServerPrivate : public QObject
{
    Q_OBJECT
    public:
        int maxCacheValue;

        QMap <QUrl,QPointer<WebLiteServerHttpHandler> > getters;
        QMap<QUrl,WebLiteLoadResponse> cache;
        QMap<QString,CacheQuota> cacheQuota;
        QQueue <WebLiteLoadRequest> reqQueue;
        int numCurrentDownloads;
        WebLiteServerPrivate() : maxCacheValue(1),numCurrentDownloads(0) {}

        WebLiteLoadResponse query (const QUrl & url);
        void freeCacheSpace(const QString & cachePath, int neededSpace, const QUrl & u);

    signals:
        void queuedRequest (WebLiteLoadRequest);

    public slots:
        void addToCache(WebLiteLoadResponse);
        void saveCache ();
        void loadCache ();
        void dlStarted ();
        void dlFinished ();
};

void WebLiteServerPrivate::addToCache(WebLiteLoadResponse e)
{
    QString cachePath = cachePathByMimeType(e.contentType);
    freeCacheSpace(cachePath,e.totalBytes,e.url);
    CacheQuota * cq = &cacheQuota[cachePath];
    cq->usedSpace += e.totalBytes;
    e.cacheValue = ++maxCacheValue;
    cache[e.url] = e;
    saveCache();
}
WebLiteLoadResponse WebLiteServerPrivate::query(const QUrl & url)
{
    return cache[url];
}
/* XML format:

*/
void WebLiteServerPrivate::saveCache ()
{
    QFile f (QDir::homePath() + "/weblite-cache.xml");
    f.open(QIODevice::WriteOnly|QFile::Text);
    QXmlStreamWriter w(&f);
    w.writeStartDocument();
    w.writeStartElement("weblite-cache");
    w.writeStartElement("items");
    for (QMap<QUrl,WebLiteLoadResponse>::iterator it = cache.begin(); it != cache.end(); ++it)
    {
        w.writeStartElement("item");
        w.writeTextElement ("url", it->url.toString());
        w.writeTextElement ("filename", it->filename);
        w.writeTextElement ("last-modified", it->lastModified.toString(HTTP_DATE_FORMAT));
        w.writeTextElement ("content-type", it->contentType);
        w.writeTextElement ("total-size", QString("%1").arg(it->totalBytes));
        w.writeTextElement ("cache-path", it->cachePath);
        w.writeTextElement ("cache-value", QString("%1").arg(it->cacheValue));
        w.writeEndElement(); // item
    }
    w.writeEndElement (); // items
    w.writeEndElement (); // weblite-cache
    w.writeEndDocument();
}

void WebLiteServerPrivate::loadCache ()
{
    maxCacheValue = 1;
    QFile f (QDir::homePath() + "/weblite-cache.xml");
    if (f.exists())
    {
        f.open(QIODevice::ReadOnly|QFile::Text);
        QXmlStreamReader r(&f);
        WebLiteLoadResponse rsp;
        while (!r.atEnd())
        {
            r.readNext();
            if (r.isStartElement())
            {
                if (r.name() == "url")
                    rsp.url = QUrl(r.readElementText());
                else if (r.name() == "filename")
                    rsp.filename = r.readElementText();
                else if (r.name() == "last-modified")
                    rsp.lastModified = QDateTime::fromString(r.readElementText(),HTTP_DATE_FORMAT);
                else if (r.name() == "content-type")
                    rsp.contentType = r.readElementText();
                else if (r.name() == "total-size")
                {
                    rsp.totalBytes = rsp.loadedBytes = r.readElementText().toInt();
                }
                else if (r.name() == "cache-path")
                    rsp.cachePath = r.readElementText();
                else if (r.name() == "cache-value")
                {
                    rsp.cacheValue = r.readElementText().toInt();
                    maxCacheValue = qMax(rsp.cacheValue, maxCacheValue);
                }
            }
            else if (r.isEndElement() && r.name() == "item")
            {
                rsp.error = (QHttp::Error)0;
                if (verifyFile(rsp))
                {
                    cache[rsp.url] = rsp;
                    cacheQuota[rsp.cachePath].usedSpace += rsp.totalBytes;
                }
                else if (rsp.filename.length())
                    QFile::remove(rsp.filename);
            }
        }
    }
}
void WebLiteServerPrivate::freeCacheSpace(const QString & cachePath, int neededSpace, const QUrl & u)
{
    Q_UNUSED(u);

    CacheQuota * cq = &cacheQuota[cachePath];
    if (cq->totalSpace- cq->usedSpace >= neededSpace)
        return;
    QMap<int,QMap<QUrl,WebLiteLoadResponse>::iterator > lruSort;
    for (QMap<QUrl,WebLiteLoadResponse>::iterator it = cache.begin (); it != cache.end(); ++it)
    {
        QPointer<WebLiteServerHttpHandler> curHandler = getters[it.key()];
        if (curHandler && !curHandler->activeClients.empty())
        {
            continue;
        }
        else if (it->filename.length())
            lruSort[it->cacheValue] = it;
    }
    for (QMap<int,QMap<QUrl,WebLiteLoadResponse>::iterator >::iterator it2 = lruSort.begin (); it2 != lruSort.end() && (cq->totalSpace- cq->usedSpace) < neededSpace; ++it2)
    {
        WebLiteLoadResponse* rsp = &(**it2);
        cq->usedSpace -= (*it2)->totalBytes;
        QFile::remove (rsp->filename);
        cache.erase(*it2);
    }
}

void WebLiteServerPrivate::dlStarted ()
{
    numCurrentDownloads++;
}

void WebLiteServerPrivate::dlFinished ()
{
    if (!(--numCurrentDownloads))
    {
        if (!reqQueue.empty())
        {
            emit queuedRequest(reqQueue.dequeue ());
        }
    }
}


void WebLiteServerHttpHandler::abort (const QUuid & u)
{
    activeClients.remove(u);
    if (activeClients.empty())
    {
        deleteLater ();
    }
}
void WebLiteServerHttpHandler::request(const QUrl & url, const QDateTime & lastModified)
{
    QString path = urlPath(url);
    QHttpRequestHeader header(headOnly?"HEAD":"GET", urlPath(url));
    header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
    header.setValue(QLatin1String("Host"), url.host());
    if (lastModified.isValid () && !headOnly)
    {
        QString lm = lastModified.toString(HTTP_DATE_FORMAT)+" GMT";
        header.setValue("If-Modified-Since",lm);
    }
    http->request (header,0,headOnly?0:file);
}

void WebLiteServerHttpHandler::get (const QUrl & url, const QDateTime & lm)
{
    headOnly = false;
    request (url,lm);
}

void WebLiteServerHttpHandler::head(const QUrl & url, const QDateTime & lm)
{
    headOnly = true;
    request (url,lm);
}

QString WebLiteServerHttpHandler::urlPath(const QUrl & url) const
{
    QString str (url.toEncoded());
    if (firstTry)
    {
        str = str.mid(url.scheme().length() + 3 + url.host().length());
        if (str == "")
            str = "/";
    }
    return str;
}

void WebLiteServerHttpHandler::httpDone (bool error)
{
    if (error && http->error() == QHttp::Aborted)
        return;
    if (!error)
    {
        if (file)
        {
            file->flush();
            if (!req.direct)
            {
                if (rsp.filename.length())
                    file->copy(rsp.filename);
                file->remove ();
            }
            file->deleteLater ();
        }
        if (!rsp.totalBytes)
        {
            rsp.totalBytes = rsp.loadedBytes;
            emit cacheSpaceNeeded (rsp);
        }
    }
    else
    {
        rsp.error = http->error ();
    }
    rsp.status = error ? WebLiteLoadResponse::Error : WebLiteLoadResponse::Complete;
    if (http)
        http->deleteLater();
    emit response(rsp);
}
void WebLiteServerHttpHandler::dataReadProgress(int done, int total)
{
    if (total)
        rsp.totalBytes = total;
    
    rsp.loadedBytes = done;
    rsp.status = WebLiteLoadResponse::SomeData;
    emit response (rsp);
}
#define HTTP_OK 200
#define HTTP_PERMANENT_REDIRECT 301
#define HTTP_TEMP_REDIRECT 302
#define HTTP_NOT_MODIFIED 304
#define HTTP_BAD_REQUEST 400
#define HTTP_PAGE_NOT_FOUND 404

void WebLiteServerHttpHandler::readResponseHeader(const QHttpResponseHeader & hdr)
{
    switch (hdr.statusCode())
    {
        case HTTP_TEMP_REDIRECT:
        case HTTP_PERMANENT_REDIRECT:
            {
                // redirect / moved temporarily

                // find the new location
                QUrl u(hdr.value("location"));
        if (req.url != u)
        {
            req.url = u;
            http->abort ();
            http->setHost (req.url.host (), req.url.port(80));
            firstTry = true;
            head(req.url,rsp.lastModified);
            break;
        }
            }
        case HTTP_OK:
            if (headOnly)
            {
                QString str = hdr.value("Last-Modified");
                QDateTime lastMod = QDateTime::fromString(str.left(str.length()-4),HTTP_DATE_FORMAT);
//                bool cacheFileOk =  verifyFile(rsp);
                bool lmvalid = rsp.lastModified.isValid();
                if (
                    (lmvalid && lastMod <= rsp.lastModified) ||
                        (
                    (hdr.contentLength() && (hdr.contentLength() == rsp.totalBytes) && ((hdr.contentType().startsWith("image/"))|| (hdr.contentType().startsWith("audio/"))|| (hdr.contentType().startsWith("video/")))
                        )
                    )
                )
                {
                    http->abort ();
                    emit useCache (req);
                }
                else
                {
                    rsp.totalBytes = hdr.contentLength ();
                    if (!file)
                    {
                        rsp.contentType = hdr.contentType();
                        QDir dir (Qtopia::homePath());
                        QString ext;
                        QString lfile = req.url.toLocalFile();

                        if (req.url.toString().endsWith(".m4v"))
                            ext = ".mp4";
                        lfile += ext;
                        QMimeType mime = QMimeType::fromFileName(lfile);
                        if (rsp.contentType == "")
                        {
                            rsp.contentType = mime.id();
                        }
                        else if (mime.isNull())
                            mime = QMimeType::fromId(rsp.contentType);
                        rsp.cachePath = cachePathByMimeType(mime.id());
                        dir.mkpath(rsp.cachePath);
                        dir = QDir(rsp.cachePath);
                        if (!dir.exists())
                        {
                            rsp.cachePath = Qtopia::homePath()+"/weblite-cache";
                            dir.mkpath(rsp.cachePath);
                        }

                        rsp.filename = rsp.cachePath + "/" + Qt::escape(QUrl::toPercentEncoding(req.url.toString()).constData()) + ext;
//                        if (overrideSuffix.size() && !rsp.filename.endsWith (QString(".")+overrideSuffix))
  //                          rsp.filename += QString(".") + overrideSuffix;
                        bool vf = verifyFile(rsp);
                        file = new QFile (req.direct ? rsp.filename : QDir::tempPath() + "/weblite-" +req.clientId.toString(), this);
                        rsp.status = WebLiteLoadResponse::BeginningDownload;
                        emit response (rsp);

                        get(req.url,vf?rsp.lastModified:QDateTime());
                        rsp.lastModified = lastMod;
                        if (rsp.totalBytes)
                            emit cacheSpaceNeeded (rsp);
                    }
                    else
                    {
                        rsp.loadedBytes = 0;
                        rsp.status = WebLiteLoadResponse::BeginningDownload;
                        emit response (rsp);
                    }
                }
            }
            break;
        case HTTP_NOT_MODIFIED:
            // 304 = use cache, no new content
            emit useCache (req);
        break;
        case HTTP_BAD_REQUEST:
        case HTTP_PAGE_NOT_FOUND:
            if (firstTry)
            {
            // could be that the http server requires a full URL and not just a path
                firstTry = false;
                head(req.url,rsp.lastModified);
            }
            else
            {
            }
        break;
    }
}

WebLiteServerHttpHandler::WebLiteServerHttpHandler(const WebLiteLoadRequest & r, const WebLiteLoadResponse & rs, QObject* o)
    : QObject (o),firstTry (true),req(r)
{
        rsp = rs;
        rsp.url = req.url;
        rsp.status = WebLiteLoadResponse::ConnectingToHost;
        QTimer::singleShot(0,this,SLOT(firstResponse()));
        http = new QHttp(r.url.host(), r.url.port(80),this);
        firstTry = true;
        head(req.url,rsp.lastModified);
        connect(http,SIGNAL(done(bool)),this,SLOT(httpDone(bool)));
        connect(http,SIGNAL(dataReadProgress(int,int)),this,SLOT(dataReadProgress(int,int)));
        connect(http,SIGNAL(responseHeaderReceived(QHttpResponseHeader)),this,SLOT(readResponseHeader(QHttpResponseHeader)));
}

WebLiteServerHttpHandler::~WebLiteServerHttpHandler()
{
}


WebLiteServer::WebLiteServer (QObject* parent)
    :QtopiaIpcAdaptor("QPE/Application/webliteserver",parent)
{
    d = new WebLiteServerPrivate();
    connect (d, SIGNAL(queuedRequest(WebLiteLoadRequest)), this, SLOT(request(WebLiteLoadRequest)));
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
    QSettings sett ("Trolltech","weblite-server");
    CacheQuota* cq = &d->cacheQuota[QDir::homePath()+"/weblite-cache"];
    cq->usedSpace = 0;
    cq->totalSpace = sett.value("cache/quota/flash",4000000).toInt();
    cq = &d->cacheQuota["/mfs/sd/weblite-cache"];
    cq->usedSpace = 0;
    cq->totalSpace = sett.value("cache/quota/sd",200000000).toInt();
    d->loadCache();
    d->freeCacheSpace(QDir::homePath()+"/weblite-cache",0,QUrl(""));
    d->freeCacheSpace("/mfs/sd/weblite-cache",0,QUrl(""));
}
WebLiteServer::~WebLiteServer ()
{
}


void WebLiteServer::request    (const QByteArray & a)
{
    WebLiteLoadRequest req;
    QDataStream ds (a);
    ds >> req;
    request (req);
}

void WebLiteServer::request    (const WebLiteLoadRequest & req)
{
    WebLiteLoadResponse rsp;
    rsp.url = req.url;
    rsp.status = WebLiteLoadResponse::RequestAcknowledged;
    sendResponse (rsp);
    if (req.url.scheme() == "file")
    {
        QString fn = req.url.toString().mid(7);
        QFile f (fn);
        if (f.exists())
        {
            WebLiteLoadResponse rsp;
            rsp.status = WebLiteLoadResponse::Complete;
            rsp.url = req.url;
            rsp.filename = fn;
            rsp.loadedBytes = rsp.totalBytes = f.size();
            sendResponse (rsp);
        }
        else
        {
            WebLiteLoadResponse rsp;
            rsp.status = WebLiteLoadResponse::Error;
            rsp.error = (QHttp::Error)404;
            rsp.url = req.url;
            rsp.filename = fn;
            sendResponse (rsp);
        }
    }
    else if (req.url.scheme() == "http")
    {
        if (req.backgroundDownload && d->numCurrentDownloads)
        {
            d->reqQueue.enqueue(req);
        }
        else
        {
            for (QQueue<WebLiteLoadRequest>::iterator it = d->reqQueue.begin(); it != d->reqQueue.end(); ++it)
            {
                if (it->url == req.url)
                {
                    it = d->reqQueue.erase (it);
                    it--;
                }
            }

            QPointer<WebLiteServerHttpHandler> h = d->getters[req.url];
            if (h && (h->rsp.status == WebLiteLoadResponse::Complete && h->rsp.totalBytes == 0))
            {
                delete h;
                h = 0;
            }
            if (!h)
            {
                QList<QHostAddress> aa = QNetworkInterface::allAddresses();
                if (aa.size() > 1 || aa[0].toString() != "127.0.0.1")
                {
                    WebLiteLoadResponse cached = d->query(req.url);

                    h = new WebLiteServerHttpHandler(req,cached,this);
                    d->dlStarted ();
                    connect (h, SIGNAL(downloadFinished()), d, SLOT(dlFinished()));
                    connect (h,SIGNAL(useCache(WebLiteLoadRequest)),this,SLOT(useCache(WebLiteLoadRequest)));
                    connect (h,SIGNAL(response(WebLiteLoadResponse)),this,SLOT(sendResponse(WebLiteLoadResponse)));
                    connect (h,SIGNAL(cacheSpaceNeeded(WebLiteLoadResponse)), d, SLOT(addToCache(WebLiteLoadResponse)));
                    QtopiaIpcAdaptor::connect (this,MESSAGE(abort (QUuid)),h,SLOT(abort(QUuid)));
                    d->getters[req.url] = h;
                }
                else
                    useCache(req);
            }
            else
            {
                qWarning() << "Already downloading, adding client" << h->rsp.status << h->rsp.url << h->rsp.filename << h->rsp.error;
                sendResponse(h->rsp);
            }
            h->activeClients.insert(req.clientId);
        }
    }
}

void WebLiteServer::sendResponse( WebLiteLoadResponse r)
{
    if (r.status == WebLiteLoadResponse::Error && r.error == QHttp::Aborted)
        r.status = WebLiteLoadResponse::Aborted;
    else if (r.status == WebLiteLoadResponse::Error)
    {
        // offline support. if there was an error, look in cache
        if (d->cache.contains(r.url))
        {
            r = d->cache[r.url];
            r.status = WebLiteLoadResponse::Complete;
        }
    }
    QByteArray a;
    QDataStream ds (&a,QIODevice::WriteOnly);
    ds << r;
    if (r.status == WebLiteLoadResponse::Complete && r.url.scheme() == "http")
    {
        r.cacheValue = ++d->maxCacheValue;
        d->cache[r.url] = r;
        d->saveCache();
    }
    emit response (a);
}
void WebLiteServer::useCache (const WebLiteLoadRequest & r)
{
    WebLiteLoadResponse rsp = d->query (r.url);
    rsp.status = WebLiteLoadResponse::Complete;
    rsp.clientId = r.clientId;
    QPointer<WebLiteServerHttpHandler> h = d->getters[r.url];
    h->rsp = rsp;
    sendResponse(rsp);
}
QSXE_APP_KEY

int main( int argc, char **argv)
{
    qRegisterMetaType<WebLiteLoadRequest>("WebLiteLoadRequest");
    qRegisterMetaType<WebLiteLoadResponse>("WebLiteLoadResponse");
    QtopiaApplication app(argc, argv);
    WebLiteServer *server = new WebLiteServer;
    app.registerRunningTask("WebLiteServer", server);

    app.exec();
}

#include <webliteserver.moc>
