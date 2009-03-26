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

#include "inetdadaptor.h"
#include "configure.h"
#include "configuredata.h"

#include <QDir>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTcpServer>
#include <QCoreApplication>
#include <QMessageBox>

struct HttpWorker
{
    QTcpSocket *socket;
    QProcess *httpd;
};

/*!
  \class InetdAdaptor
  \brief Glue class between the micro_httpd processes and the tcp socket

  In Unix the internet super-server is a daemon process, usually provided by
  the binary /sbin/inetd, and configured by /etc/inetd.conf.

  inetd launches, reads the config file which includes listings of program
  binaries and ports, and then sets up a tcp socket for each port in an
  efficient way, and waits for input on those sockets.  When input is received
  the relevant program binary is started, and the data received on the socket
  is piped into the programs standard in.

  This scheme means that the program binaries are not resident in memory when
  the socket is not active, and provies other advantages as well.

  This class imulate4s the functionality of inetd in that it listens on a tcp
  socket - port 80, the www port - and when input is received launches
  micro_httpd to handle it.

  If multiple requests arrive at the same time, multiple copies of the micro_httpd
  binary could be running at once.
*/

/*!
  Construct a new instance of the InetdAdaptor
*/
InetdAdaptor::InetdAdaptor( QObject *parent )
    : QObject( parent )
    , mTcpServer( new QTcpServer() )
    , mShowingProgress( 0 )
{
}

/*!
  Destruct an instance of the InetdAdaptor
*/
InetdAdaptor::~InetdAdaptor()
{
}

/*!
  Return the one true instance of InetdAdaptor
*/
InetdAdaptor *InetdAdaptor::getInstance()
{
    static InetdAdaptor inetdAdaptor;

    return &inetdAdaptor;
}

/*!
  Run the tcp socket and wait for input.  Launch micro_httpd processes to
  handle the inputs.  Output from micro_httpd is sent back down the socket.
*/
void InetdAdaptor::start()
{
    if ( isRunning() )
        return;
    mServerLock.lock();
    if ( mTcpServer )   // reset requested, ditch this socket and go again
        delete mTcpServer;
    mTcpServer = new QTcpServer();
    mServerLock.unlock();

    QString host = Configure::dialog()->data().server();
    QHostAddress httpdServer( host );
    quint16 httpdPort = Configure::dialog()->data().port();

    if ( !mTcpServer->listen( httpdServer, httpdPort ))
    {
        emit adaptorMessage( QString( "Could not listen for http: %1" )
                .arg( mTcpServer->errorString() ));
        qWarning( "Could not listen for http: %s", qPrintable(
                    mTcpServer->errorString() ));
        if ( mTcpServer->serverError() ==
                QAbstractSocket::SocketAddressNotAvailableError )
        {
            QMessageBox::warning( 0, tr( "Network Unavailable" ),
                    tr( "Ensure the network %1:%2 is available, then try again" )
                    .arg( host ).arg( httpdPort ));

        }
        mServerLock.lock();
        delete mTcpServer;
        mTcpServer = 0;
        mServerLock.unlock();
        return;
    }
    connect( mTcpServer, SIGNAL(newConnection()),
            this, SLOT(newConnection()) );
    emit startedRunning();
}

/*!
  Stop the server
*/
void InetdAdaptor::stop()
{
    if ( !isRunning() )
        return;
    if ( isRunning() )
        mTcpServer->close();
    mServerLock.lock();
    if ( mTcpServer )
        delete mTcpServer;
    mTcpServer = 0;
    mServerLock.unlock();
    emit stoppedRunning();
}


/*!
  Return true if the inetd server is currently running; otherwise return
  false.
*/
bool InetdAdaptor::isRunning() const
{
    return mTcpServer && mTcpServer->isListening();
}

/*!
  Receive newConnection() signals from the tcp server, when a client
  connects.  Respond by accepting, creating a socket, and piping the
  traffic to and from a micro_httpd instance.
*/
void InetdAdaptor::newConnection()
{
    HttpWorker *worker = new HttpWorker();
    mShowingProgressLock.lock();
    if ( mShowingProgress == 0 )
    {
        // only show progress for one tcp request, if another comes in at
        // the same time, don't bother showing progress for that
        mShowingProgress = worker;
        mShowingProgressLock.unlock();
        emit adaptorMessage( tr( "Receiving http request" ));
        emit progressValue( 10 );
    }
    else
    {
        mShowingProgressLock.unlock();
    }
    worker->socket = mTcpServer->nextPendingConnection();
    Q_ASSERT( worker->socket != NULL );
    connect( worker->socket, SIGNAL(disconnected()),
             this, SLOT(disconnected()));
    connect( worker->socket, SIGNAL(readyRead()),
             this, SLOT(socketReadyRead()));
    if ( worker == mShowingProgress )
        emit progressValue( 20 );
    mWorkersLock.lock();
    mWorkers.append( worker );
    mWorkersLock.unlock();
}

/*!
  Run a micro_httpd instance to process http traffic.
*/
void InetdAdaptor::runMicroHttpd( HttpWorker *worker )
{
    worker->httpd = new QProcess();
    connect( worker->httpd, SIGNAL(readyRead()),
            this, SLOT(httpdReadyRead()) );
    connect( worker->httpd, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(disconnected()) );
    connect( worker->httpd, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(httpdError(QProcess::ProcessError)) );
    // TODO: make the directory path for the webserver configurable
    QString httpdPath = QCoreApplication::applicationDirPath() + "/micro_httpd";
    if ( !QFile::exists( httpdPath ))
        qFatal( "Could not find micro_httpd binary" );
    worker->httpd->start( httpdPath, QStringList( QDir::currentPath() ));
}

/*!
  Receive the error signal from a micro_httpd process and display appropriate
  error messages.
*/
void InetdAdaptor::httpdError( QProcess::ProcessError error )
{
    switch ( error )
    {
        case QProcess::FailedToStart:
            emit adaptorMessage( tr( "Could not start web server" ));
            break;
        case QProcess::Crashed:
            emit adaptorMessage( tr( "Web server crashed" ));
            break;
        case QProcess::Timedout:
            emit adaptorMessage( tr( "Web server timed out" ));
            break;
        case QProcess::WriteError:
        case QProcess::ReadError:
            emit adaptorMessage( tr( "Error reading/writing webserver" ));
            break;
        case QProcess::UnknownError:
        default:
            emit adaptorMessage( tr( "Unknown error with webserver" ));
    }
}

/*!
  Receive a signal from a socket that output is ready for reading
  and respond by writing all the output into the appropriate httpd process.
*/
void InetdAdaptor::socketReadyRead()
{
    QTcpSocket *sock = 0;
    HttpWorker *workerReady = 0;
    sock = qobject_cast<QTcpSocket*>( sender() );
    Q_ASSERT( sock != 0 );
    foreach ( HttpWorker *worker, mWorkers )
        if ( worker->socket == sock )
            workerReady = worker;
    Q_ASSERT( workerReady != 0 );
    runMicroHttpd( workerReady );
    workerReady->httpd->write( workerReady->socket->readAll() );
    if ( workerReady == mShowingProgress )
        emit progressValue( 30 );
}

/*!
  Receive a signal from a httpd process that output is ready for reading
  and respond by writing all the output into the appropriate socket.
*/
void InetdAdaptor::httpdReadyRead()
{
    QProcess *proc = 0;
    HttpWorker *workerReady = 0;
    proc = qobject_cast<QProcess*>( sender() );
    Q_ASSERT( proc != 0 );
    foreach ( HttpWorker *worker, mWorkers )
        if ( worker->httpd == proc )
            workerReady = worker;
    Q_ASSERT( workerReady != 0 );
    if ( workerReady == mShowingProgress )
        emit progressValue( 40 );
    workerReady->socket->write( workerReady->httpd->readAllStandardOutput() );
    if ( workerReady == mShowingProgress )
    {
        emit progressValue( 60 );
        emit adaptorMessage( tr( "Processing http request" ));
    }
}

/*!
  Receive disconnected() signals from sockets, and respond by cleaning up
  the corresponding worker.
*/
void InetdAdaptor::disconnected()
{
    QTcpSocket *sock = 0;
    QProcess *proc = 0;
    sock = qobject_cast<QTcpSocket*>( sender() );
    proc = qobject_cast<QProcess*>( sender() );
    Q_ASSERT( sock != 0 || proc != 0 );
    HttpWorker *disconnectedWorker = 0;
    foreach ( HttpWorker *worker, mWorkers )
        if ( worker->socket == sock || worker->httpd == proc )
            disconnectedWorker = worker;
    if ( disconnectedWorker == 0 )
    {
        qWarning( "Worker quit, but matching socket/process not found.  Leak?" );
        return;
    }
    if ( disconnectedWorker == mShowingProgress )
        emit progressValue( 80 );
    if ( disconnectedWorker->socket )
    {
        disconnectedWorker->socket->disconnect( mTcpServer );
        disconnectedWorker->socket->disconnect( this );
        delete disconnectedWorker->socket;
        disconnectedWorker->socket = 0;
    }
    if ( disconnectedWorker->httpd )
    {
        disconnectedWorker->httpd->disconnect( this );
        if ( disconnectedWorker->httpd->state() != QProcess::NotRunning )
        {
            disconnectedWorker->httpd->terminate();
            if ( !disconnectedWorker->httpd->waitForFinished( 100 ))
                disconnectedWorker->httpd->kill();
        }
        delete disconnectedWorker->httpd;
    }
    mWorkers.removeAll( disconnectedWorker );
    delete disconnectedWorker;
    if ( disconnectedWorker == mShowingProgress )
    {
        emit progressValue( 100 );
        mShowingProgress = 0;
    }
}
