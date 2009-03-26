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

#ifndef TST_QCOPPERF_H
#define TST_QCOPPERF_H

#include <QObject>
#include <QTime>
#include <QAbstractIpcInterface>

class QCopTestObject : public QObject
{
    Q_OBJECT
public:
    QCopTestObject( QObject *parent = 0 );
    ~QCopTestObject();

    int numberOfRuns() const { return _numberOfRuns; }
    void setNumberOfRuns(int value) { _numberOfRuns = value; }

public slots:
    virtual void start() = 0;

signals:
    void done();

private:
    int _numberOfRuns;
};

class QCopRoundTrip : public QCopTestObject
{
    Q_OBJECT
public:
    QCopRoundTrip( QObject *parent = 0 );
    ~QCopRoundTrip();

public slots:
    void start();
    void received( const QString& message, const QByteArray& data );

private:
    QTime elapse;
    int count;
    int numMsgs;
};

class QCopRoundTripAdaptor : public QCopTestObject
{
    Q_OBJECT
public:
    QCopRoundTripAdaptor( bool mini, QObject *parent = 0 );
    ~QCopRoundTripAdaptor();

public slots:
    void start();
    void receive( const QString& arg );
    void receiveMini();

signals:
    void send( const QString& arg );
    void sendMini();

private:
    QTime elapse;
    int count;
    int numMsgs;
    bool mini;
};

class QCopRoundTripInterfaceClient : public QAbstractIpcInterface
{
    Q_OBJECT
public:
    QCopRoundTripInterfaceClient
        ( QObject *parent = 0, QAbstractIpcInterface::Mode mode = Client );
    ~QCopRoundTripInterfaceClient();

public slots:
    virtual void request( const QString& arg );

signals:
    void response( const QString& arg );
};

class QCopRoundTripInterfaceServer : public QCopRoundTripInterfaceClient
{
    Q_OBJECT
public:
    QCopRoundTripInterfaceServer( QObject *parent = 0 );
    ~QCopRoundTripInterfaceServer();

    void sendResponse( const QString& arg ) { emit response( arg ); }

public slots:
    void request( const QString& arg );

signals:
    void received();
};

class QCopRoundTripInterface : public QCopTestObject
{
    Q_OBJECT
public:
    QCopRoundTripInterface( QObject *parent = 0 );
    ~QCopRoundTripInterface();

public slots:
    void start();
    void received();
    void received( const QString& arg );

private:
    QTime elapse;
    int count;
    int numMsgs;
    QCopRoundTripInterfaceClient *client;
    QCopRoundTripInterfaceServer *server;
};

class tst_QCopPerf : public QObject
{
    Q_OBJECT
public:
    tst_QCopPerf();
    virtual ~tst_QCopPerf();

private slots:
    void initTestCase();
    void channel();
    void adaptor();
    void adaptorMini();
    void interface();

private:
    void runQCopTest( QCopTestObject *testObject );
};

#endif
