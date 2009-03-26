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

#include "tst_qcopperf.h"
#include <qtopiachannel.h>
#include <QApplication>
#include <QTimer>
#include <QDateTime>
#include <qvaluespace.h>
#include <qtopiaipcadaptor.h>
#include <QSignalSpy>
#include <shared/util.h>
#include <QTest>
#include <qbenchmark.h>

//TESTED_CLASS=
//TESTED_FILES=

static int DefaultNumberOfRuns = 10000;

QCopTestObject::QCopTestObject( QObject *parent )
    : QObject( parent )
{
    setNumberOfRuns( DefaultNumberOfRuns );
}

QCopTestObject::~QCopTestObject()
{
}

QCopRoundTrip::QCopRoundTrip( QObject *parent )
    : QCopTestObject( parent )
{
    QtopiaChannel *channel = new QtopiaChannel
        ( "QPE/Communications/TestInterface/Response/modem", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(received(QString,QByteArray)) );
}

QCopRoundTrip::~QCopRoundTrip()
{
}

void QCopRoundTrip::start()
{
    elapse.start();
    count = numMsgs = numberOfRuns();
    received( QString(), QByteArray() );
}

void QCopRoundTrip::received( const QString& , const QByteArray& )
{
    if ( count > 0 ) {
        QByteArray payload;
        payload.resize(64);
        QtopiaChannel::send
            ( "QPE/Communications/TestInterface/Response/modem",
              "requestSomething(QString)", payload );
        --count;
    } else {
        int elapsed = elapse.elapsed();
        qDebug( "QtopiaChannel round-tripped %d messages in %f seconds (%f seconds each)",
                numMsgs, ((double)elapsed) / 1000.0,
                ((double)elapsed) / 1000.0 / (double)numMsgs );
        emit done();
    }
}

QCopRoundTripAdaptor::QCopRoundTripAdaptor( bool mini, QObject *parent )
    : QCopTestObject( parent )
{
    this->mini = mini;
    QtopiaIpcAdaptor *adap =
            new QtopiaIpcAdaptor( "QPE/Communications/TestInterface2/Request/modem", this );
    if ( mini ) {
        QtopiaIpcAdaptor::connect
            ( this, SIGNAL(sendMini()),
              adap, MESSAGE(requestMini()) );
        QtopiaIpcAdaptor::connect
            ( adap, MESSAGE(requestMini()),
              this, SLOT(receiveMini()));
    } else {
        QtopiaIpcAdaptor::connect
            ( this, SIGNAL(send(QString)),
              adap, MESSAGE(requestSomething(QString)) );
        QtopiaIpcAdaptor::connect
            ( adap, MESSAGE(requestSomething(QString)),
              this, SLOT(receive(QString)));
    }
}

QCopRoundTripAdaptor::~QCopRoundTripAdaptor()
{
}

void QCopRoundTripAdaptor::start()
{
    elapse.start();
    count = numMsgs = numberOfRuns();
    if ( mini )
        emit sendMini();
    else
        emit send( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
}

void QCopRoundTripAdaptor::receive( const QString& )
{
    --count;
    if ( count > 0 ) {
        if ( mini )
            emit sendMini();
        else
            emit send( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
    } else {
        int elapsed = elapse.elapsed();
        qDebug( "QtopiaIpcAdapator round-tripped %d messages in %f seconds (%f seconds each)",
                numMsgs, ((double)elapsed) / 1000.0,
                ((double)elapsed) / 1000.0 / (double)numMsgs );
        emit done();
    }
}

void QCopRoundTripAdaptor::receiveMini()
{
    --count;
    if ( count > 0 ) {
        emit sendMini();
    } else {
        int elapsed = elapse.elapsed();
        qDebug( "QtopiaIpcAdapator round-tripped %d messages in %f seconds (%f seconds each) (mini)",
                numMsgs, ((double)elapsed) / 1000.0,
                ((double)elapsed) / 1000.0 / (double)numMsgs );
        emit done();
    }
}

QCopRoundTripInterfaceClient::QCopRoundTripInterfaceClient
        ( QObject *parent, QAbstractIpcInterface::Mode mode )
    : QAbstractIpcInterface( "/Testing", "QCopRoundTripInterfaceClient",
                             "group", parent, mode )
{
    proxyAll( staticMetaObject );
}

QCopRoundTripInterfaceClient::~QCopRoundTripInterfaceClient()
{
}

void QCopRoundTripInterfaceClient::request( const QString& arg )
{
    invoke( SLOT(request(QString)), arg );
}

QCopRoundTripInterfaceServer::QCopRoundTripInterfaceServer( QObject *parent )
    : QCopRoundTripInterfaceClient( parent, Server )
{
}

QCopRoundTripInterfaceServer::~QCopRoundTripInterfaceServer()
{
}

void QCopRoundTripInterfaceServer::request( const QString& )
{
    emit received();
}

QCopRoundTripInterface::QCopRoundTripInterface( QObject *parent )
    : QCopTestObject( parent )
{
    server = new QCopRoundTripInterfaceServer( this );
    QValueSpaceObject::sync();
    client = new QCopRoundTripInterfaceClient( this );
    connect( server, SIGNAL(received()), this, SLOT(received()) );
    connect( client, SIGNAL(response(QString)), this, SLOT(received(QString)) );
}

QCopRoundTripInterface::~QCopRoundTripInterface()
{
}

void QCopRoundTripInterface::start()
{
    elapse.start();
    count = numMsgs = numberOfRuns();
    client->request( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
}

void QCopRoundTripInterface::received()
{
    --count;
    if ( count > 0 ) {
        client->request( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
    } else {
        int elapsed = elapse.elapsed();
        qDebug( "QAbstractIpcInterface round-tripped %d slot invokes in %f seconds (%f seconds each)",
                numMsgs, ((double)elapsed) / 1000.0,
                ((double)elapsed) / 1000.0 / (double)numMsgs );

        // Start the next phase, delivering signals.
        elapse.start();
        count = numMsgs = numberOfRuns();
        server->sendResponse( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
    }
}

void QCopRoundTripInterface::received( const QString& )
{
    --count;
    if ( count > 0 ) {
        server->sendResponse( "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890" );
    } else {
        int elapsed = elapse.elapsed();
        qDebug( "QAbstractIpcInterface round-tripped %d signal emits in %f seconds (%f seconds each)",
                numMsgs, ((double)elapsed) / 1000.0,
                ((double)elapsed) / 1000.0 / (double)numMsgs );
        emit done();
    }
}

tst_QCopPerf::tst_QCopPerf()
{
}

tst_QCopPerf::~tst_QCopPerf()
{
}

void tst_QCopPerf::initTestCase()
{
    QValueSpace::initValuespaceManager();
}

void tst_QCopPerf::channel()
{
    QCopRoundTrip *testObject = new QCopRoundTrip( this );
    runQCopTest( testObject );
}

void tst_QCopPerf::adaptor()
{
    QCopRoundTripAdaptor *testObject = new QCopRoundTripAdaptor( false, this );
    runQCopTest( testObject );
}

void tst_QCopPerf::adaptorMini()
{
    QCopRoundTripAdaptor *testObject = new QCopRoundTripAdaptor( true, this );
    runQCopTest( testObject );
}

void tst_QCopPerf::interface()
{
    QCopRoundTripInterface *testObject = new QCopRoundTripInterface( this );
    runQCopTest( testObject );
}

void tst_QCopPerf::runQCopTest( QCopTestObject *testObject )
{
    QSignalSpy spy(testObject,SIGNAL(done()));
    QBENCHMARK {
        testObject->start();
        QTRY_VERIFY( spy.count() == 1 );
    }
    spy.takeLast();
}

QTEST_MAIN(tst_QCopPerf)
