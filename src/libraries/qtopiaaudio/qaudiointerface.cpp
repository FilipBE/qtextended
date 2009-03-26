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

#include <QTimer>
#include <QIODevice>
#include <QLocalSocket>

#include <qtopialog.h>
#include <qtopianamespace.h>

#include "qaudiointerface.h"

#include <QAudioStateInfo>
#include <QAudioStateConfiguration>
#include <QtopiaIpcEnvelope>
#include <QtopiaIpcAdaptor>
#include <QAudioInput>
#include <QAudioOutput>


class QAudioInstance
{
public:
    QLocalSocket  *stream;
    qint32        state;
    qint32        type;
    QString       domain;
    bool          mediaserver;
};

class InterfaceStateInfo
{
public:
    enum InterfaceState { None = 0x0, Init = 0x01, Stop = 0x02, Active = 0x03, Resume = 0x04, UserStop = 0x08, UserStart = 0x16, Pause = 0x32 };

    InterfaceStateInfo():state() {}
    InterfaceState state;
};

class AudioTypeInfo
{
public:
    enum AudioType { None = 0x0, Input = 0x01, Output = 0x02, InputAndOutput = 0x04 };

    AudioTypeInfo():type() {}
    AudioType type;
};


class QAudioInterfacePrivate : public QObject
{
    Q_OBJECT
public:
    QAudioInterfacePrivate(QObject *parent)
    {
        MyInfo.state    = InterfaceStateInfo::None;
        MyInfo.type     = AudioTypeInfo::None;
        MyInfo.domain   = "Media";
        MyInfo.mediaserver =  false;
        MyInfo.stream   = new QLocalSocket(this);
        connect(MyInfo.stream, SIGNAL(readyRead()), this, SLOT(dataFromServer()));
        connect(MyInfo.stream, SIGNAL(error(QLocalSocket::LocalSocketError)),this,
                SLOT(socketError(QLocalSocket::LocalSocketError)));
        connect(this, SIGNAL(audioStopped()), parent,SIGNAL(audioStopped()));
        connect(this, SIGNAL(audioStarted()), parent,SIGNAL(audioStarted()));

        clientState = InterfaceStateInfo::None;
        initState = InterfaceStateInfo::None;
        audioIn = 0;
        audioOut = 0;

        active = false;
        currentDomain ="Media";

        // Handle Audio State Changes
        mgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);
        audioConf = new QAudioStateConfiguration(this);
        connect(audioConf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
                this, SLOT(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)));
        connect(audioConf, SIGNAL(availabilityChanged()),
                this, SLOT(availabilityChanged()));
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()),this,SLOT(timeout()));

        timer->start(1000);
    }

    ~QAudioInterfacePrivate()
    {
        active = false;
        MyInfo.state = InterfaceStateInfo::Stop;

        sendCommand("--- DONE");
        MyInfo.stream->flush();
        MyInfo.stream->disconnectFromServer();
        if(audioIn != 0) {
            audioIn->close();
        }
        if(audioOut != 0) {
            audioOut->close();
        }
    }

    void startAudio();
    void setMode(const QByteArray &domain);
    void setInput(const QAudioInput &input);
    void setOutput(const QAudioOutput &output);
    void stopAudio();

signals:
    void audioStarted();
    void audioStopped();

private slots:
    void dataFromServer();    // for signal: readyRead()
    void socketError(QLocalSocket::LocalSocketError socketError);
    void sendCommand(QString cmd);
    void instanceActive();
    void instancePause();
    void instanceResume();

    void availabilityChanged();
    void currentStateChanged(const QAudioStateInfo &state,
                             QAudio::AudioCapability capability);
    void timeout();
    bool connectToAudioServer();

private:
    QAudioInput              *audioIn;
    QAudioOutput             *audioOut;

    QtopiaIpcAdaptor         *mgr;
    QAudioStateConfiguration *audioConf;
    QString                  profile;

    QtopiaIpcAdaptor         *adaptor;
    QAudioInstance           MyInfo;
    QTimer                   *timer;
public:
    int                      clientState;
    int                      initState;

    bool                     active;
    QString                  currentDomain;
};

void QAudioInterfacePrivate::dataFromServer()
{
    QByteArray data;

    if(MyInfo.stream->bytesAvailable() > 0) {
        data = MyInfo.stream->readLine((qint64)512);

        QString cmd = data.data();
        QStringList str = cmd.split(" ");

        qLog(AudioState)<<"data from server, msg="<<cmd.replace("\n"," ")<<"    "<<this;

        if(cmd.contains("---",Qt::CaseInsensitive)) {
            if(!((str.size() == 2)||(str.size() == 3))) {
                qLog(AudioState)<<"QAudioInterface::dataFromServer() Invalid arguments should be 2 or 3, not %d",
                        str.size();
                return;
            }
        } else {
            qLog(AudioState)<<"QAudioInterface::dataFromServer() Invalid request, ignoring";
            return;
        }

        QString command = str.at(1);
        if(command.contains("ACTIVE",Qt::CaseSensitive)) {
            // Send DOMAIN
            MyInfo.state = InterfaceStateInfo::Init;
            instanceActive();
        } else if((command.contains("ACK",Qt::CaseSensitive)) &&
                (MyInfo.state == InterfaceStateInfo::Init)) {
            if(!initState) {
                initState = InterfaceStateInfo::Init;
                instanceResume();
            } else {
                instanceResume();
            }
        } else if(command.contains("READY",Qt::CaseSensitive)) {
            if(MyInfo.mediaserver) {
                //If mediaserver then resume
                MyInfo.state = InterfaceStateInfo::Active;
                if((currentDomain.startsWith(MyInfo.domain)) && !active) {
                    active = true;
                    emit audioStarted();
                }
            } else if(clientState == InterfaceStateInfo::UserStop) {
                //If client stopped by user don't resume
                return;
            } else
                MyInfo.state = InterfaceStateInfo::Active;
            if(MyInfo.state == InterfaceStateInfo::Active) {
                QByteArray d(MyInfo.domain.toLocal8Bit().constData());
                int capability = static_cast<int>(QAudio::OutputOnly);
                if(MyInfo.type == AudioTypeInfo::Output) {
                    capability = static_cast<int>(QAudio::OutputOnly);
                } else if(MyInfo.type == AudioTypeInfo::Input) {
                    capability = static_cast<int>(QAudio::InputOnly);
                } else {
                    capability = static_cast<int>(QAudio::InputAndOutput);
                }
                mgr->send("setDomain(QByteArray,int)",d,capability);
                QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
                e << MyInfo.domain;
                timer->start(1000);
            }
        } else if(command.contains("PAUSE",Qt::CaseSensitive)) {
            // You need to stop and then respond with STOPPED
            MyInfo.state = InterfaceStateInfo::Pause;
            instancePause();
        } else if(command.contains("RESUME",Qt::CaseSensitive)) {
            // You can continue using the audio and respond with RESUMED
            MyInfo.state = InterfaceStateInfo::Active;
            instanceResume();
        } else if(command.contains("STOPALL",Qt::CaseSensitive)) {
            // You must stop all access to the audio and respond with STOPPED
            MyInfo.state = InterfaceStateInfo::Stop;
            stopAudio();
        }
    }
}

void QAudioInterfacePrivate::socketError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
        case QLocalSocket::ConnectionRefusedError:
            qLog(AudioState)<<"QLocalSocket::ConnectionRefusedError";
            break;
        case QLocalSocket::ServerNotFoundError:
            qLog(AudioState)<<"LocalSocket::ServerNotFoundError";
            break;
        case QLocalSocket::SocketAccessError:
            qLog(AudioState)<<"QLocalSocket::SocketAccessError";
            break;
        default:
            qLog(AudioState)<<"QLocalSocket::Unknown Error";
            break;
    }
}

void QAudioInterfacePrivate::instanceActive()
{
    if(MyInfo.mediaserver)
        sendCommand("--- DOMAIN MediaServer");
    else
        sendCommand(QString("--- DOMAIN %1").arg(MyInfo.domain));
}

void QAudioInterfacePrivate::instancePause()
{
    if(audioIn != 0) {
        audioIn->close();
    }
    if(audioOut != 0) {
        audioOut->close();
    }

    sendCommand("--- PAUSED");
    if(active) {
        active = false;
        emit audioStopped();
    }
}

void QAudioInterfacePrivate::instanceResume()
{
    startAudio();
}

void QAudioInterfacePrivate::startAudio()
{
    // Handle case when startAudio() is called before connection to server
    if(MyInfo.state == InterfaceStateInfo::None) {
        clientState = InterfaceStateInfo::Init;
        return;
    }

    QString cmd;
    QByteArray data;

    if((MyInfo.state > InterfaceStateInfo::None) && (MyInfo.state != InterfaceStateInfo::Resume)) {
        if(audioIn != 0) {
            cmd = "--- RECORD";
            if(!MyInfo.type)
                MyInfo.type = AudioTypeInfo::Input;
        } else {
            cmd = "--- PLAY";
            if(!MyInfo.type)
                MyInfo.type = AudioTypeInfo::Output;
        }
        MyInfo.state = InterfaceStateInfo::Active;
    } else if(MyInfo.state == InterfaceStateInfo::Resume) {
        // handle paused case
        cmd = "--- RESUMED";
        MyInfo.state = InterfaceStateInfo::Active;
        QByteArray d(MyInfo.domain.toLocal8Bit().constData());
        int capability;
        if(MyInfo.type == AudioTypeInfo::Output) {
            capability = static_cast<int>(QAudio::OutputOnly);
        } else if(MyInfo.type == AudioTypeInfo::Input) {
            capability = static_cast<int>(QAudio::InputOnly);
        } else {
            capability = static_cast<int>(QAudio::InputAndOutput);
        }
        mgr->send("setDomain(QByteArray,int)",d,capability);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
        e << MyInfo.domain;
        timer->start(1000);
    } else
        return;

    sendCommand(cmd);
}

void QAudioInterfacePrivate::stopAudio()
{
    if(audioIn != 0) {
        audioIn->close();
    }
    if(audioOut != 0) {
        audioOut->close();
    }
    if(clientState == InterfaceStateInfo::UserStop) {
        MyInfo.state = InterfaceStateInfo::Stop;
        sendCommand("--- DONE");
    } else
        sendCommand("--- STOPPED");

    if(active) {
        active = false;
        emit audioStopped();
    }
}

void QAudioInterfacePrivate::setMode(const QByteArray &dom)
{
    if(dom.contains("MediaServer")) {
        MyInfo.domain = "Media";
        MyInfo.mediaserver = true;
    } else
        MyInfo.domain = dom;
}

void QAudioInterfacePrivate::setInput(const QAudioInput &input)
{
    audioIn = const_cast<QAudioInput*>(&input);

    if(MyInfo.type == AudioTypeInfo::Output)
        MyInfo.type = AudioTypeInfo::InputAndOutput;
    else
        MyInfo.type = AudioTypeInfo::Input;
}

void QAudioInterfacePrivate::setOutput(const QAudioOutput &output)
{
    audioOut = const_cast<QAudioOutput*>(&output);

    if(MyInfo.type == AudioTypeInfo::Input)
        MyInfo.type = AudioTypeInfo::InputAndOutput;
    else
        MyInfo.type = AudioTypeInfo::Output;
}

void QAudioInterfacePrivate::sendCommand(QString cmd)
{
    qLog(AudioState)<<" S<-C "<<cmd<<"      "<<this;
    QByteArray data;
    data.append(cmd);
    data.append("\n");
    MyInfo.stream->write(data);
    MyInfo.stream->flush();
}

void QAudioInterfacePrivate::availabilityChanged()
{
    qLog(AudioState)<<"void QAudioInterface::availabilityChanged()";
}

void QAudioInterfacePrivate::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability c)
{
    Q_UNUSED(state);
    Q_UNUSED(c);

    if(MyInfo.state == InterfaceStateInfo::Active) {
        if(audioIn != 0) {
            audioIn->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        }
        if(audioOut != 0) {
            audioOut->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
        }
        if(!active) {
            active = true;
            emit audioStarted();
        }
    }
    currentDomain = state.domain().data();
}

void QAudioInterfacePrivate::timeout()
{
    timer->stop();
    if(MyInfo.state == InterfaceStateInfo::None) {
        if(!connectToAudioServer())
            timer->start(500);
        return;
    }
    if(MyInfo.state == InterfaceStateInfo::Active) {
        if(audioIn != 0) {
            audioIn->open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        }
        if(audioOut != 0) {
            audioOut->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
        }
        if(!active) {
            active = false;
            emit audioStarted();
        }
    }
}

bool QAudioInterfacePrivate::connectToAudioServer()
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QAudioServer" )).toLocal8Bit();
    MyInfo.stream->connectToServer(socketPath.data());
    if(MyInfo.stream->waitForConnected(500)) {
        qLog(AudioState)<<"QAudioInterfacePrivate() connected";
        return true;
    }
    timer->start(500);

    return false;
}

/*!
      \class QAudioInterface
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule

      \brief The QAudioInterface class provides a way of managing access to the audio system.

      This class communicates with a server instance that is managing access to the audio system depending on domain.

      A typical implementation follows:

      \code
      QAudioInput      *audio;
      QAudioInterface  *audiomgr;

      audio = new QAudioInput(this);
      audiomgr = new QAudioInterface("Media", this);
      audiomgr->setInput(audio);
      audiomgr->audioStart();
      \endcode

      audioStarted() is emitted when audio is ready for read/write access
      audioStopped() is emitted when the server stops or pauses the instance

      see signals audioStarted() and audioStopped()

      \sa QAudioStateConfiguration, QAudioStateInfo
      \ingroup multimedia
*/

/*!
      Construct a new QAudioInterface object with \a domain
      and \a parent

      domain can be Media, RingTone or Phone
*/
QAudioInterface::QAudioInterface( const QByteArray &domain, QObject *parent)
    : QObject( parent )
{
    d = new QAudioInterfacePrivate(this);
    d->setMode(domain);
}

/*!
      Destroy this audio interface.
      */
QAudioInterface::~QAudioInterface()
{
    d->deleteLater();
}

/*!
      Start the process of getting access to the audio system.
      started() is emitted when audio is ready for reading.
*/
void QAudioInterface::startAudio()
{
    qLog(AudioState)<<"====== USER startAudio()    "<<this;
    //if(d->clientState == InterfaceStateInfo::UserStart) d->clientState = InterfaceStateInfo::Active;
    d->clientState = InterfaceStateInfo::Active;
    d->startAudio();
}

/*!
      Stop access to the audio system.
      stopped() is emitted when audio is stopped.
*/

void QAudioInterface::stopAudio()
{
    qLog(AudioState)<<"====== USER stopAudio()     "<<this;
    d->clientState = InterfaceStateInfo::UserStop;
    d->stopAudio();
}

/*!
      Pass a pointer \a input for QAudioInput class to manage.

      The class will call close() to detach from the audio device
      automatically and emit audioStopped() signal.
      The class will re-open the audio device when available again
      and emit audioStarted()
*/
void QAudioInterface::setInput(const QAudioInput &input)
{
    qLog(AudioState)<<"void QAudioInterface::setInput()";
    d->setInput(input);
}

/*!
      Pass a pointer \a output for QAudioOutput class to manage.
*/
void QAudioInterface::setOutput(const QAudioOutput &output)
{
    qLog(AudioState)<<"void QAudioInterface::setOutput()";
    d->setOutput(output);
}

/*!
  \fn QAudioInterface::audioStarted()
  This signal is emitted when the instance is ready for access.
*/

/*!
  \fn QAudioInterface::audioStopped()
  This signal is emitted when the instance has been stopped.
*/

#include "qaudiointerface.moc"
