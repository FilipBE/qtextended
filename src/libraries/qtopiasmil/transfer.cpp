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

#include "transfer.h"
#include "element.h"
#include <QMap>
#include <QIODevice>
#include <QDebug>
#include <QBasicTimer>
#include <QTimerEvent>

SmilDataSource::SmilDataSource(const QString& t)
: type(t), dev(0)
{
}

SmilDataSource::~SmilDataSource()
{
}

void SmilDataSource::setDevice(QIODevice *d)
{
    dev = d;
}

QIODevice* SmilDataSource::device() const
{
    return dev;
}

void SmilDataSource::setMimeType(const QString &t)
{
    type = t;
}

const QString& SmilDataSource::mimeType() const
{
    return type;
}

class SmilDataStore : public QObject, public SmilDataSource
{
    Q_OBJECT
public:
    SmilDataStore(SmilTransferServer *ts, const QString &s, const QString &t=QString());
    ~SmilDataStore();

    virtual void setDevice(QIODevice *d);

    void addReference(SmilElement *e) { ref.append(e); }
    void removeReference(SmilElement *e) { ref.removeAll(e); }
    int refCount() const { return ref.count(); }

    bool complete() { return done; }
    QByteArray data() const { return buffer; }

protected:
    void timerEvent(QTimerEvent *te);

private slots:
    void bytesAvailable(qint64 bytes);
    void closing();

private:
    bool done;
    QByteArray buffer;
    QString src;
    SmilElementList ref;
    SmilTransferServer *server;
    QBasicTimer timer;
};

class SmilTransferServerPrivate
{
public:
    SmilTransferServerPrivate(SmilTransferServer *s);
    ~SmilTransferServerPrivate();

    void requestData(SmilElement *e, const QString &src);
    void endData(SmilElement *e, const QString &src);

    SmilTransferServer *server;
    QMap<QString,SmilDataStore*> dataStores;
};

//===========================================================================

SmilDataStore::SmilDataStore(SmilTransferServer *ts, const QString &s, const QString &t)
    : SmilDataSource(t), done(false), src(s), server(ts)
{
    server->requestTransfer(this, src);
}

SmilDataStore::~SmilDataStore()
{
    if (!done)
        server->endTransfer(this, src);
}

void SmilDataStore::setDevice(QIODevice *d)
{
    SmilDataSource::setDevice(d);
    connect(d, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesAvailable(qint64)));
    connect(d, SIGNAL(aboutToClose()), this, SLOT(closing()));
    timer.start(0, this);
}

void SmilDataStore::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == timer.timerId()) {
        if (device()->bytesAvailable())
            bytesAvailable(device()->bytesAvailable());
        if (!device()->isSequential())
            device()->close();
        timer.stop();
    }
}

void SmilDataStore::bytesAvailable(qint64 bytes)
{
    if (bytes) {
        int oldSize = buffer.size();
        buffer.resize(oldSize+bytes);
        device()->read(buffer.data()+oldSize, bytes);
    }
}

void SmilDataStore::closing()
{
    done = true;
    SmilElementList::Iterator it = ref.begin();
    while (it != ref.end()) {
        SmilElement *e = *it;
        ++it;
        e->setData(buffer, mimeType());
    }
    server->endTransfer(this, src);
}

//---------------------------------------------------------------------------

SmilTransferServerPrivate::SmilTransferServerPrivate(SmilTransferServer *s)
    : server(s)
{
}

SmilTransferServerPrivate::~SmilTransferServerPrivate()
{
    foreach ( SmilDataStore *ds, dataStores )
        delete ds;
    dataStores.clear();
}

void SmilTransferServerPrivate::requestData(SmilElement *e, const QString &src)
{
    QMap<QString,SmilDataStore*>::Iterator it = dataStores.find(src);
    if (it != dataStores.end()) {
        // Already loaded.
        if ((*it)->complete())
            e->setData((*it)->data(), (*it)->mimeType());
        (*it)->addReference(e);
    } else {
        SmilDataStore *ds = new SmilDataStore(server, src);
        dataStores[src] = ds;
        ds->addReference(e);
    }
}

void SmilTransferServerPrivate::endData(SmilElement *e, const QString &src)
{
    QMap<QString,SmilDataStore*>::Iterator it = dataStores.find(src);
    if (it != dataStores.end()) {
        (*it)->removeReference(e);
    }
}

//---------------------------------------------------------------------------

SmilTransferServer::SmilTransferServer(QObject *parent) : QObject(parent)
{
    d = new SmilTransferServerPrivate(this);
}

SmilTransferServer::~SmilTransferServer()
{
    delete d;
}

void SmilTransferServer::requestData(SmilElement *e, const QString &src)
{
    d->requestData(e, src);
}

void SmilTransferServer::endData(SmilElement *e, const QString &src)
{
    d->endData(e, src);
}

void SmilTransferServer::requestTransfer(SmilDataSource *s, const QString &src)
{
    emit transferRequested(s, src);
}

void SmilTransferServer::endTransfer(SmilDataSource *s, const QString &src)
{
    emit transferCancelled(s, src);
}

#include "transfer.moc"
