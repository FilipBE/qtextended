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

#include <QUrl>
#include <QDebug>
#include <QtopiaIpcAdaptor>
#include <QTimer>
#include "webliteclient.h"

class WebLiteClientPrivate
{
    public:
        WebLiteLoadRequest req;
        WebLiteLoadResponse rsp;
        QtopiaIpcAdaptor* ipc;
        enum
        {
            Idle,
            Initializing,
            Loading,
            SomeData,
            Done
        } state;

        int abortTimeout;
        int resendTimeout;
        QTime timeFromStart;
        QTimer* processTimer;
};
bool WebLiteClient::isBackgroundDownload () const
{
    return d->req.backgroundDownload;
}
void WebLiteClient::setBackgroundDownload (bool b)
{
    d->req.backgroundDownload = b;
}
bool WebLiteClient::isDirect() const
{
    return d->req.direct;
}
void WebLiteClient::setDirect(bool b)
{
    d->req.direct = b;
}

QUrl WebLiteClient::url () const
{
    return d->req.url;
}
void WebLiteClient::setUrl (const QUrl & u)
{
    d->req.url = u;
}

void WebLiteClient::load ()
{
    if (d->state != WebLiteClientPrivate::Idle)
        abort ();

    d->state = WebLiteClientPrivate::Initializing;
    QByteArray a;
    {
        QDataStream ds (&a,QIODevice::ReadWrite);
        ds << d->req;
    }
    d->timeFromStart.start();
    emit request (a);
}
        int WebLiteClient::loadedBytes () const { return d->rsp.loadedBytes; }
        int WebLiteClient::totalBytes () const { return d->rsp.totalBytes; }
        QHttp::Error WebLiteClient::httpError () const { return d->rsp.error; }
        WebLiteLoadResponse::Status WebLiteClient::status () const { return d->rsp.status; }

void WebLiteClient::processTime ()
{
    if ((d->resendTimeout) && (d->state == WebLiteClientPrivate::Initializing) && d->timeFromStart.elapsed() > d->resendTimeout)
        load ();
    else if ((d->abortTimeout) && (d->state == WebLiteClientPrivate::Loading) && d->timeFromStart.elapsed() > d->abortTimeout)
    {
        abort ();
        emit error ();
    }
}

void WebLiteClient::abort ()
{
    emit aborted (d->req.clientId);
}

void WebLiteClient::response (const QByteArray & a)
{
    QDataStream ds(a);
    WebLiteLoadResponse r;
    ds >> r;
    if (r.url == d->req.url)
    {
        if (d->state == WebLiteClientPrivate::Initializing)
            d->state = WebLiteClientPrivate::Loading;

        d->rsp = r;

        switch (status ())
        {
            case WebLiteLoadResponse::Complete:
                emit loaded ();
                emit done (false);
                d->state = WebLiteClientPrivate::Done;
                break;
            case WebLiteLoadResponse::Error:
                qWarning() << "Error: " << httpError();
                d->state = WebLiteClientPrivate::Done;
                emit done (true);
                emit error ();
                break;
            default:
                break;
        }
        emit progress ();
        if (r.status == WebLiteLoadResponse::SomeData || r.status == WebLiteLoadResponse::Complete)
        {
            d->state = WebLiteClientPrivate::SomeData;
        }
    }
}

WebLiteClient::WebLiteClient (QObject* parent) : QObject( parent)
{
    static bool registered = false;
    if (!registered)
    {
        qRegisterMetaType<WebLiteLoadRequest>("WebLiteLoadRequest");
        qRegisterMetaType<WebLiteLoadResponse>("WebLiteLoadResponse");
        registered = true;
    }

    d = new WebLiteClientPrivate;
    d->ipc = new QtopiaIpcAdaptor("QPE/Application/webliteserver",this);
    QtopiaIpcAdaptor::connect(this, SIGNAL(request(QByteArray)), d->ipc, MESSAGE(request(QByteArray)));
    QtopiaIpcAdaptor::connect(this, SIGNAL(aborted(QUuid)), d->ipc, MESSAGE(abort(QUuid)));
    QtopiaIpcAdaptor::connect(d->ipc, MESSAGE(response(QByteArray)), this, SLOT(response(QByteArray)));
    d->req.clientId = QUuid::createUuid();
    d->abortTimeout = 0;
    d->processTimer = new QTimer(this);
    d->resendTimeout = 2000;
    connect (d->processTimer, SIGNAL(timeout()),this,SLOT(processTime()));
    d->processTimer->start(1000);
    d->state = WebLiteClientPrivate::Idle;
}
WebLiteClient::~WebLiteClient ()
{
    abort ();
    delete d;
}
QString WebLiteClient::filename() const
{
    return d->rsp.filename;
}

bool WebLiteClient::isDone() const
{
    return d->state == WebLiteClientPrivate::Done;
}

void WebLiteClient::loadInBackground (const QUrl & url)
{
    WebLiteClient* c = new WebLiteClient();
    c->setUrl (url);
    c->setBackgroundDownload(true);
    connect (c, SIGNAL(loaded()), c, SLOT(deleteLater()));
    connect (c, SIGNAL(error()), c, SLOT(deleteLater()));
    c->load ();
}
void WebLiteClient::setTimeout(int millis)
{
    d->abortTimeout = millis;
}
        int WebLiteClient::timeout () const
{
    return d->abortTimeout;
}
