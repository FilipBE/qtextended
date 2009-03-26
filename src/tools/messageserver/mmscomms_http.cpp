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

#include "mmscomms_http.h"
#include "qmailaccount.h"
#include "mmsmessage.h"

#include <private/accountconfiguration_p.h>

#include <qmailmessage.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <QWapAccount>
#include <QUrl>
#include <QFile>
#include <QBuffer>
#include <QTimer>
#include <QHttp>
#include <QSettings>
#include <QDSActionRequest>
#include <QDSData>
#include <QDrmContent>

#ifdef DUMP_MMS_DATA
// Handy:
static QByteArray toHexDump( const char *in, int size, int offset = 0 )
{
    const char hexdigits[] = "0123456789ABCDEF";

    QByteArray tmp;
    int lines = (size + 15) & ~15;
    tmp.resize(lines * ( 9 + 16 * 3 + 1 + 2 + 16 + 1 ));
            // "offset:", 16 space-separated bytes, space for column 8,
            // two spaces before ASCII, ASCII, \n

    char *out = tmp.data();
    int posn;
    int index = -(((int)offset) & 15);
    offset &= ~((uint)15);
    while (index < size) {
        *out++ = hexdigits[((int)(offset >> 28)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 24)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 20)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 16)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 12)) & 0x0F];
        *out++ = hexdigits[((int)(offset >>  8)) & 0x0F];
        *out++ = hexdigits[((int)(offset >>  4)) & 0x0F];
        *out++ = hexdigits[((int)(offset      )) & 0x0F];
        *out++ = ':';
        for (posn = 0; posn < 16 && (index + posn) < size; ++posn) {
            if ( posn == 8 )
                *out++ = ' ';
            *out++ = ' ';
            if ((index + posn) >= 0) {
                *out++ = hexdigits[(in[index + posn] >> 4) & 0x0F];
                *out++ = hexdigits[in[index + posn] & 0x0F];
            } else {
                *out++ = ' ';
                *out++ = ' ';
            }
        }
        while ( posn < 16 ) {
            if ( posn == 8 )
                *out++ = ' ';
            *out++ = ' ';
            *out++ = ' ';
            *out++ = ' ';
            ++posn;
        }
        *out++ = ' ';
        *out++ = ' ';
        for (posn = 0; posn < 16 && (index + posn) < size; ++posn) {
            if ((index + posn) >= 0) {
                char ch = in[index + posn];
                if ( ch >= 0x20 && ch <= 0x7E )
                    *out++ = ch;
                else
                    *out++ = '.';
            } else {
                *out++ = ' ';
            }
        }
        index += 16;
        if ( index < size )
            *out++ = '\n';
        offset += 16;
    }

    tmp.truncate(out - tmp.data());
    return tmp;
}
#endif

// This is an example of how to implement a comms driver for MMS.

MmsCommsHttp::MmsCommsHttp(const QMailAccountId &id, QObject *parent)
    : MmsComms(id, parent), rhttp(0), shttp(0), rId(0), sId(0),
      rStatus(200), sStatus(200)
{
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(cleanup()));
}

MmsCommsHttp::~MmsCommsHttp()
{
}

bool MmsCommsHttp::isActive() const
{
    if ((rhttp && rhttp->currentId()) || (shttp && shttp->currentId()))
        return true;

    return false;
}

void MmsCommsHttp::clearRequests()
{
    deleteList.append(shttp);
    shttp = 0;
    sId = 0;
    sStatus = 200;
    deleteList.append(rhttp);
    rhttp = 0;
    rId = 0;
    rStatus = 200;
}

void MmsCommsHttp::sendMessage(MMSMessage &msg, const QByteArray& encoded)
{
    QWapAccount acc( networkConfig() );
    QUrl server = acc.mmsServer();
    if ( server.scheme().isEmpty() )
        server.setScheme( "http" );
    if ( server.port() == -1 )
        server.setPort( 80 );

    if (!shttp) {
        shttp = createHttpConnection(server.host(), server.port());
    }

    sStatus = 200;
    QHttpRequestHeader header("POST", server.toString());
    header.setValue("User-Agent", "Qt-Extended-MMS-Client/" + Qtopia::version());
    header.setValue("Host", server.host() + ":" + QString::number(server.port()));
    header.setContentType("application/vnd.wap.mms-message");
    header.setContentLength(encoded.size());
//    addAuth(header);
#ifdef DUMP_MMS_DATA
    qLog(Messaging) << "MmsCommsHttp: Sending" << header.toString();
    qLog(Messaging).nospace() << "MmsCommsHttp: Data:\n" << qPrintable(toHexDump(encoded.constData(), encoded.length()));
#endif

    int id = shttp->request(header, encoded);
    if (msg.type() == MMSMessage::MSendReq)
        sId = id;
}

void MmsCommsHttp::retrieveMessage(const QUrl &url)
{
    int port = 80;
    if (url.port() > 0)
        port = url.port();
    rStatus = 200;
    if (!rhttp)
        rhttp = createHttpConnection(url.host(), port);
    QHttpRequestHeader header("GET", url.toString());
    header.setValue("User-Agent", "Qt-Extended-MMS-Client/" + Qtopia::version());
    header.setValue("Host", url.host() + ":" + QString::number(port));

    QPair< QString, QString > drmHeader;
    foreach( drmHeader, QDrmContent::httpHeaders() )
        header.setValue( drmHeader.first, drmHeader.second );
    QStringList types = QDrmContent::supportedTypes();

    if( !types.isEmpty() )
        header.setValue( "Accept", types.join( ", " ) );

    rId = rhttp->request(header);
}

void MmsCommsHttp::dataReadProgress(int done, int /*total*/)
{
    emit transferSize(done);
}

void MmsCommsHttp::dataSendProgress(int done, int /*total*/)
{
    emit transferSize(done);
}

void MmsCommsHttp::done(bool error)
{
    if (sender() == shttp)
        qLog(Messaging) << "MmsClient::done POST:" << error;
    else
        qLog(Messaging) << "MmsClient::done GET:" << error;
    timer->start(1000);
}

void MmsCommsHttp::cleanup()
{
    if (shttp && !shttp->currentId() && !shttp->hasPendingRequests()) {
        destroyHttpConnection(shttp);
        shttp = 0;
    }
    if (rhttp && !rhttp->currentId() && !rhttp->hasPendingRequests()) {
        destroyHttpConnection(rhttp);
        rhttp = 0;
    }

    if (!shttp && !rhttp) {
        emit statusChange(QString::null);
        emit transfersComplete();
    }

    foreach (QHttp *http, deleteList)
        destroyHttpConnection(http);
    deleteList.clear();
}

void MmsCommsHttp::requestFinished(int id, bool err)
{
    QHttp *h = static_cast<QHttp *>(sender());
    QByteArray data = h->readAll();
    QString tmp(data);
    if (err) {
        if (h->error() != QHttp::Aborted) {
            qWarning() << "MmsClient::requestFinished: Error:" << h->error() << h->errorString();
            emit statusChange(tr("Error occurred"));
            emit error(h->error(), h->errorString());
        }
        return;
    }
    if (id == rId && rStatus != 200 || id == sId && sStatus != 200) {
        emit statusChange(tr("Error occurred"));
        emit error(id == rId ? rStatus : sStatus, tr("Transfer failed"));
        return;
    }
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    MMSMessage mmsMsg;
    mmsMsg.decode(&buffer);
    if (id == rId) {
        // m-retrieve-conf
        emit retrieveConf(mmsMsg, data.size());
        rId = 0;
    } else if (id == sId) {
        // m-send-conf.
        emit sendConf(mmsMsg);
        sId = 0;
    } else {
        // other responses ignored.
    }
}

void MmsCommsHttp::requestStarted(int id)
{
    Q_UNUSED(id);
}

void MmsCommsHttp::responseHeaderReceived(const QHttpResponseHeader &resp)
{
    if (resp.statusCode() != 200) {
        if (sender() == shttp)
            sStatus = resp.statusCode();
        else
            rStatus = resp.statusCode();
    }
}

void MmsCommsHttp::stateChanged(int state)
{
    switch (state) {
        case QHttp::Unconnected:
            break;
        case QHttp::HostLookup:
            emit statusChange(tr("Lookup host"));
            break;
        case QHttp::Connecting:
            emit statusChange(tr("Connecting..."));
            break;
        case QHttp::Sending:
            emit statusChange(tr("Sending..."));
            break;
        case QHttp::Reading:
            emit statusChange(tr("Receiving..."));
            break;
        case QHttp::Closing:
            emit statusChange(tr("Closing connection"));
            break;
        default:
            break;
    }
}

void MmsCommsHttp::addAuth(QHttpRequestHeader &header)
{
    AccountConfiguration config(accountId);

    QString user = config.mailUserName();
    if (user.isNull())
        user = "";
    QString pass = config.mailPassword();
    if (pass.isNull())
        pass = "";
    QString auth = user + ':' + pass;

    auth = "Basic " + QString::fromLatin1( auth.toLatin1().toBase64() );
    header.setValue("Authorization", auth);
}

QHttp *MmsCommsHttp::createHttpConnection(const QString &host, int port)
{
    QHttp *http = new QHttp(host, port);
    connect(http, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(dataReadProgress(int,int)));
    connect(http, SIGNAL(dataSendProgress(int,int)),
            this, SLOT(dataSendProgress(int,int)));
    connect(http, SIGNAL(done(bool)), this, SLOT(done(bool)));
    connect(http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    connect(http, SIGNAL(requestStarted(int)),
            this, SLOT(requestStarted(int)));
    connect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(responseHeaderReceived(QHttpResponseHeader)));
    connect(http, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));

    return http;
}

void MmsCommsHttp::destroyHttpConnection(QHttp *http)
{
    if (!http)
        return;
    disconnect(http, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(dataReadProgress(int,int)));
    disconnect(http, SIGNAL(dataSendProgress(int,int)),
            this, SLOT(dataSendProgress(int,int)));
    disconnect(http, SIGNAL(done(bool)), this, SLOT(done(bool)));
    disconnect(http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    disconnect(http, SIGNAL(requestStarted(int)),
            this, SLOT(requestStarted(int)));
    disconnect(http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(responseHeaderReceived(QHttpResponseHeader)));
    disconnect(http, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    delete http;
}

