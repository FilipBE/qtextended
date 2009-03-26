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
#include <trace.h>
QD_LOG_OPTION(QDLinkHelper)

#ifdef QTOPIA_DESKTOP
#include <qdlinkhelper.h>
#include <desktopsettings.h>
#include <qdplugin.h>
#include <qdthread.h>
#else
#include "qdlinkhelper.h"
#include "qdthread.h"
QD_EXPORT bool qdsync_send_200;
#endif

#include <QThread>
#include <QMutex>
#include <QIODevice>
#include <QTimer>
#include <QTextStream>
#include <QBuffer>
#include <QVariant>

// =====================================================================

class PingObject;

class QIODeviceWrapper : public QIODevice
{
    Q_OBJECT
public:
    QIODeviceWrapper( QIODevice *parent, PingObject *po = 0 );
    ~QIODeviceWrapper();

    bool isSequential() const;
    qint64 bytesAvailable() const;

    qint64 readData( char *data, qint64 maxSize );
    qint64 writeData( const char *data, qint64 maxSize );

    bool canReadLine() const;
    qint64 readLineData( char *data, qint64 maxSize );

    void ioThreadRead();
    void ioThreadWrite();

signals:
    void disconnected();
    void readData();
    void writeData();

private slots:
    void readMore();

private:
    QPointer<QIODevice> rawDevice;
    QList<QByteArray*> ibl;
    QByteArray ibr;
    mutable QMutex ibm;
    QByteArray ob;
    mutable QMutex obm;
    PingObject *po;
};

// =====================================================================

class PingObject : public QObject
{
    Q_OBJECT
public:
    PingObject( QObject *parent = 0 );
    ~PingObject();

public slots:
    void reset();
    void ping();
    void writeData();
    void readData();

signals:
    void timeout();

private:
    QTimer *timer;
public:
    QPointer<QIODevice> device;
    QIODeviceWrapper *wrapper;
    int ping_interval;
    bool send_ack;
};

// =====================================================================

class PingThread : public QDThread
{
    Q_OBJECT
public:
    PingThread( QObject *parent = 0 );
    ~PingThread();

    void t_init();
    void t_run();
    void t_quit();

    PingObject *object;
};

// =====================================================================

PingObject::PingObject( QObject *parent )
    : QObject( parent ), ping_interval( 0 )
{
    TRACE(QDLinkHelper) << "PingObject::PingObject";
    timer = new QTimer( this );
    timer->setSingleShot( true );
    connect( timer, SIGNAL(timeout()), this, SLOT(ping()) );
}

PingObject::~PingObject()
{
}

void PingObject::writeData()
{
    TRACE(QDLinkHelper) << "PingObject::writeData";
    wrapper->ioThreadWrite();
}

void PingObject::readData()
{
    TRACE(QDLinkHelper) << "PingObject::readData";
    wrapper->ioThreadRead();
}

void PingObject::reset()
{
    TRACE(QDLinkHelper) << "PingObject::reset";
    if ( timer->isActive() ) {
        LOG() << "resetting ping timer";
        timer->stop();
    }
#ifndef QTOPIA_DESKTOP
    if ( send_ack && device ) {
        LOG() << "send ack";
        QTextStream os( device );
        os << "HELPER_ACK\n";
    }
#endif
    if ( ping_interval ) {
#ifdef QTOPIA_DESKTOP
        LOG() << "sending next ping in" << ping_interval << "ms";
#else
        LOG() << "expecting ping in" << ping_interval << "ms";
#endif
        timer->start( ping_interval );
    }
}

void PingObject::ping()
{
    TRACE(QDLinkHelper) << "PingObject::ping";
#ifdef QTOPIA_DESKTOP
    LOG() << "Send a message so the device doesn't disconnect!";
    QByteArray data("HELPER_PING\n");
#else
    LOG() << "The host took too long to send me a message!";
    QByteArray data("HELPER_QUIT\n");
#endif
    qint64 ret = 0;
    if ( device )
        ret = device->write( data );
    if ( ret != data.count() ) {
        LOG() << "Ping failed... socket closed?";
    }
#ifdef QTOPIA_DESKTOP
    reset();
#else
    emit timeout();
#endif
}

// =====================================================================

PingThread::PingThread( QObject *parent )
    : QDThread( parent ), object( 0 )
{
    TRACE(QDLinkHelper) << "PingThread::PingThread";
}

PingThread::~PingThread()
{
}

void PingThread::t_init()
{
    TRACE(QDLinkHelper) << "PingThread::t_init";
    object = new PingObject;
}

void PingThread::t_run()
{
    TRACE(QDLinkHelper) << "PingThread::t_run";
#ifdef QTOPIA_DESKTOP
    object->reset();
#endif
    QDThread::t_run();
}

void PingThread::t_quit()
{
    TRACE(QDLinkHelper) << "PingThread::t_quit";
    delete object;
}

// =====================================================================

QIODeviceWrapper::QIODeviceWrapper( QIODevice *parent, PingObject *_po )
    : QIODevice(), rawDevice( parent ), po( _po )
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::QIODeviceWrapper";
    // this goes via ThreadObject to call ioThreadRead on the IO thread
    connect( rawDevice, SIGNAL(readyRead()), po, SLOT(readData()) );
    open( QIODevice::ReadWrite|QIODevice::Unbuffered );
    connect( parent, SIGNAL(disconnected()), this, SIGNAL(disconnected()) );
}

QIODeviceWrapper::~QIODeviceWrapper()
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::~QIODeviceWrapper";
}

bool QIODeviceWrapper::isSequential() const
{
    return true;
}

// this is called from the IO thread
void QIODeviceWrapper::ioThreadRead()
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::ioThreadRead";
    if ( !rawDevice ) return;
    QMutexLocker locker( &ibm );
    int count = -1;
    QByteArray justRead = rawDevice->readAll();
    ibr.append( justRead );
    QBuffer buffer( &ibr );
    buffer.open( QIODevice::ReadOnly );
    while ( buffer.canReadLine() ) {
        if ( count == -1 ) count = 0;
        QByteArray line = buffer.readLine();
        LOG() << "Read line" << line.trimmed();
        if ( line == "HELPER_ACK\n" || line == "HELPER_PING\n" ) {
            LOG() << "... ignored";
        } else if ( line.startsWith("HELPER_INIT") ) {
#ifdef QTOPIA_DESKTOP
            LOG() << "... ignored";
#else
            QList<QByteArray> args = line.mid(12).trimmed().split(' ');
            LOG() << "HELPER_INIT" << args;
            if ( po ) {
                int num = QVariant(args[0]).toInt();
                po->ping_interval = num;
                LOG() << "ping_interval set to" << num;
                if ( args.count() >= 2 )
                    po->send_ack = (bool)QVariant(args[1]).toInt();
                if ( args.count() >= 3 )
                    qdsync_send_200 = (bool)QVariant(args[2]).toInt();
            }
#endif
        } else if ( line == "HELPER_QUIT\n" ) {
            close();
            emit disconnected();
        } else {
            ibl << new QByteArray(line);
            count++;
        }
    }
    ibr = buffer.readAll();
    if ( ibr.count() ) {
        LOG() << "Partial line" << ibr;
#ifdef QTOPIA_DESKTOP
        // If we didn't read data, don't do this (likely to cause lockups)
        if ( justRead.count() > 0 ) {
            // Don't wait for the next ping...
            // We know there's data available (because we got a partial line)
            // so read again in a little while.
            QTimer::singleShot( 100, this, SLOT(readMore()) );
        }
#endif
    }
    buffer.close();
    locker.unlock();
    if ( count >= 0 ) {
        emit readData();
        if ( count )
            emit readyRead();
    }
}

qint64 QIODeviceWrapper::bytesAvailable() const
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::bytesAvailable";
    QMutexLocker locker( &ibm );
    qint64 count = 0;
    foreach ( QByteArray *data, ibl )
        count += data->count();
    count += ibr.count();
    return count;
}

qint64 QIODeviceWrapper::readData( char *data, qint64 maxSize )
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::readData";
    QMutexLocker locker( &ibm );
    char *cursor = data;
    qint64 bytes = qMin( bytesAvailable(), maxSize );
    qint64 b = bytes;
    foreach ( QByteArray *data, ibl ) {
        qint64 count = qMin( (qint64)data->count(), b );
        memcpy( cursor, data->constData(), count );
        cursor += count;
        b -= count;
        if ( b ) {
            ibl.removeFirst();
            delete data;
        }
    }
    if ( b ) {
        memcpy( cursor, ibr.constData(), b );
        if ( ibr.count() > b )
            ibr = ibr.mid( b );
    }
    return bytes;
}

qint64 QIODeviceWrapper::writeData( const char *data, qint64 maxSize )
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::writeData";
    QMutexLocker locker( &obm );
    QByteArray bytes = QByteArray( data, maxSize );
    LOG() << "write bytes to buffer" << bytes.trimmed();
    ob.append( bytes );
    // this goes via ThreadObject to call ioThreadWrite on the IO thread
    emit writeData();
    return maxSize;
}

// this is called from the IO thread
void QIODeviceWrapper::ioThreadWrite()
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::ioThreadWrite";
    if ( !rawDevice ) return;
    QMutexLocker locker( &obm );
    if ( ob.count() ) {
        LOG() << "write bytes to raw device" << ob.trimmed();
        qint64 bytes = rawDevice->write( ob.constData(), ob.count() );
        if ( bytes > 0 )
            ob = ob.mid( bytes );
    }
}

bool QIODeviceWrapper::canReadLine() const
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::canReadLine";
    QMutexLocker locker( &ibm );
    return ibl.count();
}

qint64 QIODeviceWrapper::readLineData( char *data, qint64 maxSize )
{
    TRACE(QDLinkHelper) << "QIODeviceWrapper::readLineData";
    QMutexLocker locker( &ibm );
    Q_ASSERT(ibl.count());
    QByteArray *b = ibl[0];
    qint64 bytes = qMin( (qint64)b->count(), maxSize );
    memcpy( data, b->constData(), bytes );
    if ( bytes == b->count() ) {
        ibl.removeFirst();
        delete b;
    } else {
        (*b) = b->mid( bytes );
    }
    return bytes;
}

void QIODeviceWrapper::readMore()
{
    ioThreadRead();
}

// =====================================================================

QDLinkHelper::QDLinkHelper( QIODevice *device, QObject *parent )
    : QObject( parent ), rawDevice( 0 ), wrapperDevice( 0 ), thread( 0 )
{
    init( device );
}

QDLinkHelper::~QDLinkHelper()
{
    TRACE(QDLinkHelper) << "QDLinkHelper::~QDLinkHelper";
    if ( rawDevice )
        rawDevice->deleteLater();
    thread->quit();
    thread->wait();
    delete thread;
    delete wrapperDevice;
}

void QDLinkHelper::init( QIODevice *device )
{
    TRACE(QDLinkHelper) << "QDLinkHelper::init";

    rawDevice = device;

    int ping_interval = 0;
    bool send_ack = false;
#ifdef QTOPIA_DESKTOP
    QDLinkPlugin *plugin = qobject_cast<QDLinkPlugin*>(parent());
    if ( plugin ) { // this should not fail...
        ping_interval = plugin->ping_interval();
        send_ack = plugin->send_ack();
    }
    if ( DesktopSettings::debugMode() ) {
        DesktopSettings settings("settings");
        ping_interval = settings.value("ping_interval", ping_interval).toInt();
    }
    LOG() << "Using ping_interval" << ping_interval << "send_ack" << send_ack;
    {
        QTextStream stream( rawDevice );
        // the device waits 5 times as long as our timeout
        // this should help to avoid dropouts due to OS paging, etc.
        int interval = ping_interval;
        if ( interval ) interval *= 5; // don't do this if the timeout is 0 (disabled)
        stream << "HELPER_INIT" << " " << interval << " " << (send_ack?"1":"0") << " 0" << endl;
    }
#endif

    LOG() << "create PingThread";
    thread = new PingThread;
    LOG() << "call init()";
    thread->init();
    LOG() << "after init()";
    thread->object->ping_interval = ping_interval;
    thread->object->send_ack = send_ack;
    thread->object->device = rawDevice;
    rawDevice->moveToThread( thread );
    wrapperDevice = new QIODeviceWrapper( rawDevice, thread->object );
    thread->object->wrapper = (QIODeviceWrapper*)((QIODevice*)wrapperDevice);
    connect( wrapperDevice, SIGNAL(writeData()), thread->object, SLOT(writeData()) );
    connect( wrapperDevice,
#ifdef QTOPIA_DESKTOP
            SIGNAL(writeData()),
#else
            SIGNAL(readData()),
#endif
            thread->object, SLOT(reset()) );
    connect( thread->object, SIGNAL(timeout()), this, SIGNAL(timeout()) );
    thread->start();
}

QIODevice *QDLinkHelper::socket()
{
    return wrapperDevice;
}

QIODevice *QDLinkHelper::rawSocket()
{
    return rawDevice;
}

#include "qdlinkhelper.moc"
