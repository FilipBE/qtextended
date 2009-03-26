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

#ifndef QAUDIOINTERFACESERVER_H
#define QAUDIOINTERFACESERVER_H

#include <QObject>
#include <QtopiaApplication>
#include <QLocalServer>
#include <QLocalSocket>

class _QAudioInstance;

namespace mediaserver
{

class QAudioInterfaceServer : public QObject
{
    Q_OBJECT

public:
    QAudioInterfaceServer(QObject* parent = 0);
    ~QAudioInterfaceServer();

private slots:
    void acceptConnection();  // for signal: newConnection()
    void dataFromClient();    // for signal: readyRead()
    void closeConnection();   // for signal: disconnected();

private:
    void   processRequest(const QLocalSocket &id, const QByteArray &data);
    QLocalSocket* priorityResolver();

    void instanceStopped(const QLocalSocket &id);
    void instancePlayRequest(const QLocalSocket &id);
    void instanceRecordRequest(const QLocalSocket &id);
    void instancePaused(const QLocalSocket &id);
    void instanceCompleted(const QLocalSocket &id);
    void instanceResumed(const QLocalSocket &id);
    void setDomain(const QLocalSocket &id, QString dom);
    void setPriority(const QLocalSocket &id, qint32 pri);

    void sendCommand(const QLocalSocket &id, QString cmd);

    QLocalSocket             *currentId;
    QLocalSocket             *activeId;
    QLocalServer             tcpServer;
    QList<_QAudioInstance*>  instances;
};

}   // ns mediaserver

#endif
