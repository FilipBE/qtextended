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

#include "qaudiointerfaceserver.h"
#include <qtopialog.h>

class _QAudioInstance
{
public:
    QLocalSocket  *stream;
    qint32      state;
    qint32      type;
    qint32      priority;
    QString     domain;
};

class InterfaceStateInfo
{
public:
    enum InterfaceState { None = 0x0, Stopped = 0x01, Active = 0x02, Paused = 0x04 };

    InterfaceStateInfo():state() {}
    InterfaceState state;
};

class AudioTypeInfo
{
public:
    enum AudioType { None = 0x0, Play = 0x01, Record = 0x02 };

    AudioTypeInfo():type(None) {}
    AudioType type;
};

namespace mediaserver
{

/*!
    \class mediaserver::QAudioInterfaceServer
    \internal
*/

QAudioInterfaceServer::QAudioInterfaceServer(QObject* parent):
    QObject(parent)
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QAudioServer" )).toLocal8Bit();
    int retry = 0;
    while(!tcpServer.listen(socketPath.data()) && retry < 5) {
        retry++;
    }
    if(retry >= 5) {
        qLog(AudioState)<<"QAudioInterfaceServer() FAILED! trying to listen for Client instances";
    }
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    currentId = 0;
    activeId = 0;
}

QAudioInterfaceServer::~QAudioInterfaceServer()
{
    foreach( _QAudioInstance *instance, instances ) {
        disconnect( instance->stream, 0, this, 0 );
    }
}

void QAudioInterfaceServer::acceptConnection()
{
    _QAudioInstance *newInstance = new _QAudioInstance();

    newInstance->stream  = tcpServer.nextPendingConnection();
    newInstance->state    = InterfaceStateInfo::Stopped;
    newInstance->type     = AudioTypeInfo::None;
    newInstance->priority = 0;
    newInstance->domain   = "Media";

    instances.append(newInstance);
    connect(newInstance->stream, SIGNAL(readyRead()), this, SLOT(dataFromClient()));
    connect(newInstance->stream, SIGNAL(disconnected()), this, SLOT(closeConnection()));

    sendCommand(*newInstance->stream, QLatin1String("--- ACTIVE"));
}

void QAudioInterfaceServer::dataFromClient()
{
    QByteArray     data;
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream->bytesAvailable()) {
            data = instance->stream->readLine((qint64)512);
            currentId = const_cast<QLocalSocket*>(instance->stream);
            processRequest(*instance->stream,data);
        }
    }
}

void QAudioInterfaceServer::closeConnection()
{
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream->state() == QLocalSocket::UnconnectedState) {
            qLog(AudioState)<<instance->stream<<" close";
            instanceCompleted(instance->stream);
            instances.removeAt(i);
            return;
        }
    }
}

void QAudioInterfaceServer::processRequest(const QLocalSocket &id, const QByteArray &data)
{
    QString cmd = data.data();
    QStringList str = cmd.split(" ");

    if(cmd.contains("---",Qt::CaseInsensitive)) {
        if(!((str.size() == 2)||(str.size() == 3))) {
            qLog(AudioState)<<"QAudioInterfaceServer::processRequest() Invalid request %d",str.size();
            return;
        }
    } else {
        qLog(AudioState)<<"QAudioInterfaceServer::processRequest() Invalid request, ignoring";
        return;
    }

    bool ok;
    QString command = str.at(1);

    if(command.contains("PLAY",Qt::CaseSensitive)) {
        instancePlayRequest(id);
    } else if(command.contains("DONE",Qt::CaseSensitive)) {
        instanceCompleted(id);
    } else if(command.contains("PAUSED",Qt::CaseSensitive)) {
        instancePaused(id);
    } else if(command.contains("STOPPED",Qt::CaseSensitive)) {
        instanceStopped(id);
    } else if(command.contains("RESUMED",Qt::CaseSensitive)) {
        instanceResumed(id);
    } else if(command.contains("DOMAIN",Qt::CaseSensitive)) {
        setDomain(id,str.at(2));
    } else if(command.contains("PRIORITY",Qt::CaseSensitive)) {
        setPriority(id,str.at(2).toInt(&ok, 10));
    } else if(command.contains("RECORD",Qt::CaseSensitive)) {
        instanceRecordRequest(id);
    }
}

void QAudioInterfaceServer::instanceStopped(const QLocalSocket &id)
{
    /* On STOPPED msg from client
    mark client as stopped and send READY if last client to respond */

    int remaining = 0;
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            instance->state = InterfaceStateInfo::Stopped;
        }
        if(instance->state == InterfaceStateInfo::Active) remaining++;
    }
    if(remaining == 0) {
        for (int i = 0; i < instances.size(); ++i ) {
            instance = instances.at(i);
            if(instance->stream == activeId) {
                if(activeId != &id) {
                    instance->state = InterfaceStateInfo::Active;
                    sendCommand(*activeId, QLatin1String("--- READY"));
                    return;
                }
            }
        }
        for (int i = 0; i < instances.size(); ++i ) {
            instance = instances.at(i);
            if(instance->priority == 255) {
                instance->state = InterfaceStateInfo::Active;
                sendCommand(*instance->stream, QLatin1String("--- RESUME"));
            }
        }
    }
}

void QAudioInterfaceServer::instancePlayRequest(const QLocalSocket &id)
{
    /* On PLAY request from client
    - Set to type Play
    - SPECIAL case (mediaserver) priority 255 always resumes!!!
    (EXCLUSIVE_DOMAINS?) - If Ringtone, Phone domain: stop all Active
    - If ALSA, must be Media domain so play together
    - If OSS/Other, pause Active to play
    */

    int            playing = 0;
    QString        domain;
    _QAudioInstance *instance;

    // First, match which client msg is from
    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            domain = instance->domain;
            instance->type = AudioTypeInfo::Play;
        }
    }

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->state == InterfaceStateInfo::Active) {
#if defined(EXCLUSIVE_DOMAINS)
            if((domain.contains("RingTone",Qt::CaseSensitive))||(domain.contains("Phone",Qt::CaseSensitive))) {
                // instance is playing, RingTone or Phone exclusive access
                if(instance->priority != 255)
                    sendCommand(*instance->stream, QLatin1String("--- STOPALL"));
                else
                    sendCommand(*instance->stream, QLatin1String("--- PAUSE"));
                playing++;
            } else {
#endif
#if defined(QTOPIA_HAVE_ALSA)
                // instance is playing, ok then its alsa and Media, play together
                currentId = const_cast<QLocalSocket*>(&id);
                activeId = currentId;
                instancePaused(id);
                return;
#else
                // instance is playing, ok then its oss and Media
                if(instance->stream->isValid()) {
                    sendCommand(*instance->stream, QLatin1String("--- PAUSE"));
                    playing++;
                }
#endif
#if defined(EXCLUSIVE_DOMAINS)
            }
#endif
        }
    }
    activeId = currentId;

    if(!playing) {
        instancePaused(id);
    }
}

void QAudioInterfaceServer::instanceRecordRequest(const QLocalSocket &id)
{
    /* On RECORD request from client
    - Set to type Record
    - SPECIAL case (mediaserver) priority 255 always resumes!!!
    - Recording is exclusive access so stop all other active clients
    */

    int recording = 0;
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            instance->type = AudioTypeInfo::Record;
        }
        if(instance->state == InterfaceStateInfo::Active) {
            // instance recording is exclusive access
            if(instance->priority != 255)
                sendCommand(*instance->stream, QLatin1String("--- STOPALL"));
            else
                sendCommand(*instance->stream, QLatin1String("--- PAUSE"));
            recording++;
        }
    }
    activeId = currentId;

    if(!recording) {
        instancePaused(id);
    }
}

void QAudioInterfaceServer::instancePaused(const QLocalSocket &id)
{
    /* On PAUSED msg from client
    - Set state to Paused
    - If paused from requesting, send READY and set to Active
    */

    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == activeId) {
            sendCommand(*instance->stream, QLatin1String("--- READY"));
            instance->state = InterfaceStateInfo::Active;
            return;
        }
        if(instance->stream == &id) {
            instance->state = InterfaceStateInfo::Paused;
        }
    }
    // handle first instance
    instances.at(0)->state = InterfaceStateInfo::Active;
    sendCommand(*instances.at(0)->stream, QLatin1String("--- READY"));
}

void QAudioInterfaceServer::instanceCompleted(const QLocalSocket &id)
{
    /* On DONE msg from client
    - Set this to Stopped state
    - Use priorityResolver to set ActiveId and resume
    */

    int paused = 0;
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            instance->state = InterfaceStateInfo::Stopped;
        }
        if(instance->state == InterfaceStateInfo::Paused) {
            paused++;
        }
    }
    if(paused) {
        activeId = priorityResolver();

        for (int i = 0; i < instances.size(); ++i ) {
            instance = instances.at(i);
#if defined(QTOPIA_HAVE_ALSA)
            //Resume all paused for ALSA
            if(instance->state == InterfaceStateInfo::Paused) {
                sendCommand(*instance->stream, QLatin1String("--- RESUME"));
            }
#else
            if(instance->stream == activeId) {
                sendCommand(*instance->stream, QLatin1String("--- RESUME"));
            }
#endif
        }
    } else {
        activeId = 0;
        currentId = 0;
    }
}

void QAudioInterfaceServer::instanceResumed(const QLocalSocket &id)
{
    /* On RESUMED msg from client
    - Set state to Active
    */

    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            instance->state = InterfaceStateInfo::Active;
        }
    }
}

QLocalSocket* QAudioInterfaceServer::priorityResolver()
{
    /* This function returns next client to get access */

    QLocalSocket *id = 0;
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        qLog(AudioState)<<"QAudioInterfaceServer::priorityResolver() id="<<instance->stream<<
            ", state="<<instance->state<<", priority="<<instance->priority<<", domain="<<instance->domain;

#if defined(QTOPIA_HAVE_ALSA)
        if(instance->state == InterfaceStateInfo::Paused) {
            //last one paused, doesn't really matter as all paused will be resumed for ALSA
            id = instance->stream;
        }
#else
        if(instance->priority == 255) {
            // Always give back to priority=255 (mediaserver) if available
            id = instance->stream;
        } else if(id == 0 && instance->state == InterfaceStateInfo::Paused) {
            //last one paused
            id = instance->stream;
        }
#endif
    }
    if(id == NULL) return id;
    else  return instances.at(0)->stream;
    return 0;
}

void QAudioInterfaceServer::setDomain(const QLocalSocket &id, QString dom)
{
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            if(dom.contains("MediaServer")) {
                instance->priority = 255;
                instance->domain = "Media";
                sendCommand(*instance->stream, QLatin1String("--- ACK"));
            } else {
                instance->domain = dom;
                sendCommand(*instance->stream, QLatin1String("--- ACK"));
            }
        }
    }
}

void QAudioInterfaceServer::setPriority(const QLocalSocket &id, qint32 pri)
{
    _QAudioInstance *instance;

    for (int i = 0; i < instances.size(); ++i ) {
        instance = instances.at(i);
        if(instance->stream == &id) {
            instance->priority = pri;
            sendCommand(*instance->stream, QLatin1String("--- ACK"));
        }
    }
}

void QAudioInterfaceServer::sendCommand(const QLocalSocket &id, QString cmd)
{
    QLocalSocket* sock = const_cast<QLocalSocket*>(&id);
    qLog(AudioState)<<" S->C "<<cmd<<"        "<<&id;
    QByteArray data;
    data.append(cmd);
    data.append("\n");
    sock->write(data);
}

} // ns mediaserver
