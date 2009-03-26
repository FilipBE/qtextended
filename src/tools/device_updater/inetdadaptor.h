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

#ifndef INETDADAPTOR_H
#define INETDADAPTOR_H

#include <QList>
#include <QObject>
#include <QMutex>
#include <QProcess>

class QTcpServer;
class HttpWorker;

class InetdAdaptor : public QObject
{
    Q_OBJECT
public:
    InetdAdaptor( QObject *parent = 0 );
    ~InetdAdaptor();
    static InetdAdaptor *getInstance();
    bool isRunning() const;
    void start();
    void stop();
signals:
    void progressValue( int );
    void adaptorMessage( const QString & );
    void startedRunning();
    void stoppedRunning();
private slots:
    void newConnection();
    void httpdError( QProcess::ProcessError );
    void socketReadyRead();
    void httpdReadyRead();
    void disconnected();
private:
    void runMicroHttpd( HttpWorker *worker );
    QTcpServer *mTcpServer;
    QList<HttpWorker*> mWorkers;
    HttpWorker *mShowingProgress;
    QMutex mWorkersLock;
    QMutex mServerLock;
    QMutex mShowingProgressLock;
};

#endif
