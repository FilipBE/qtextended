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
#include "../radio_codes.h"
#include "radio_service.h"
#include "webliteclient.h"
#include <QtopiaAbstractService>
#include <QtopiaApplication>
#include <QCopChannel>

#include <QMediaControl>
#include <QMediaContent>
#include <QMediaContentContext>
struct WebRadioServicePrivate : public QObject
{
    WebLiteClient* webClient;

    QProcess* proc;
    QStringList list;
    QString curLine;
    QPointer<QMediaContent> mediaContent;
    QPointer<QMediaContentContext> ctx;
    QPointer<QMediaControl> mediaControl;
    QPointer<QMediaControlNotifier> notifier;
    QString url, mediaFilename;
    RadioCodes::Status errCode;
    QString errStr;
    QTimer *timer;
};

void WebRadioService::onProcExit(int , QProcess::ExitStatus )
{
    processEvent(Event_DemuxProcessEnded);
}


WebRadioService::WebRadioService(QObject* o)
    :QtopiaAbstractService("WebRadio",o),mainState(State_Idle),cleanupState(State_CleanupIdle)
{
    publishAll ();
    d = new WebRadioServicePrivate();
    d->webClient = new WebLiteClient(this);
    d->proc = new QProcess(this);
    d->proc->setReadChannel(QProcess::StandardError);
    connect(d->proc, SIGNAL(readyReadStandardError()), this, SLOT(readFromDemux()));
    connect(d->proc, SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onProcExit(int,QProcess::ExitStatus)));
    connect(d->proc, SIGNAL(started()),this,SLOT(onProcStart()));
    connect(d->webClient,SIGNAL(loaded()),this,SLOT(playlistLoaded()));
    d->timer = new QTimer(this);
    connect(d->timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
    d->timer->setSingleShot(true);
}

WebRadioService::~WebRadioService()
{
}
void WebRadioService::onTimeout()
{
    processEvent(Event_Timeout);
}

void WebRadioService::initMediaPlayer()
{
    if (d->ctx)
        delete d->ctx;
    if (d->notifier)
        delete d->notifier;

    d->ctx = new QMediaContentContext( this );
    d->notifier = new QMediaControlNotifier( QMediaControl::name(),this);
    connect( d->notifier, SIGNAL(valid()), this, SLOT(mediaValid()) );
    connect( d->notifier, SIGNAL(invalid()), this, SLOT(mediaInvalid()) );
    d->ctx->addObject(d->notifier);

    if (d->mediaContent)
        delete d->mediaContent;

    d->mediaContent = new QMediaContent( QUrl(QString("file://")+d->mediaFilename),QLatin1String( "Media" ), this );
    d->ctx->setMediaContent(d->mediaContent);
}
void WebRadioService::startMediaPlayer()
{
    if (d->mediaControl)
        delete d->mediaControl;
    d->mediaControl = new QMediaControl (d->mediaContent);
    d->mediaControl->setMuted(false);
    d->mediaControl->start();
    connect(d->mediaControl,SIGNAL(playerStateChanged(QtopiaMedia::State)),this,SLOT(playerStateChanged(QtopiaMedia::State)));
    d->mediaControl->setVolume(10);
}

void WebRadioService::stopMediaPlayer()
{
    if (d->mediaControl)
        d->mediaControl->stop();
    else
    {
        processEvent(Event_MediaStopped);
    }
}

void WebRadioService::stopDemux()
{
    if (d->proc && d->proc->state() == QProcess::Running)
        d->proc->terminate();
    else
        processEvent(Event_DemuxProcessEnded);
}

void WebRadioService::playerStateChanged(QtopiaMedia::State state)
{
    if (state == QtopiaMedia::Playing)
        processEvent(Event_MediaStarted);
    else
        processEvent(Event_MediaStopped);
}

void WebRadioService::mediaValid ()
{
    processEvent(Event_MediaValid);
}

void WebRadioService::mediaInvalid ()
{
}

void WebRadioService::processLine (const QString & line)
{
    if (line.length())
    {
        int s = (RadioCodes::Status)line.left(3).toInt();
        if (s > 0)
        {
            RadioCodes::Status code = (RadioCodes::Status)s;
            QString message = line.mid(4);
            switch (code)
            {
                case RadioCodes::Ready:
                    // start player!
                    d->mediaFilename = message;
                    processEvent (Event_MediaPipeReady);
                    break;
                case RadioCodes::StreamTitleChanged:
                    // notify name change
                    emit titleChanged (message);
                    output(code,message);
                    break;
                default:
                    if (code >= RadioCodes::ErrorBase)
                    {
                        // emit error
                        output(code,message);
                    }
                    break;
            }
        }
    }
}

void WebRadioService::readFromDemux()
{
    QByteArray a = d->proc->readAll();
    d->curLine += QString(a);
    QStringList ls = d->curLine.split("\n");
    for (int i=0; i < ls.count()-1; ++i)
        processLine(ls[i]);
    if (d->curLine.endsWith('\n'))
    {
        processLine(ls[ls.count()-1]);
        d->curLine = "";
    }
    else
        d->curLine = d->curLine.mid(d->curLine.lastIndexOf("\n")+1);
}

void WebRadioService::playlistLoaded()
{
    processEvent(Event_PlaylistRetrieved);
}

void WebRadioService::parsePlaylist()
{
    // parse
    QStringList l;
    QSettings sett (d->webClient->filename(),QSettings::IniFormat);
    sett.beginGroup("playlist");
    int numEntries = sett.value("numberofentries").toInt();
    if (numEntries > 0)
    {
        d->url = sett.value("File1").toString();
    }
    d->webClient->abort ();
    processEvent(Event_PlaylistParsed);
}

void WebRadioService::setVolume(int v)
{
    if (d->mediaControl)
        d->mediaControl->setVolume(v);
}

void WebRadioService::setPlaylistUrl (const QUrl & u)
{
    d->webClient->setUrl(u);
}

void WebRadioService::onProcStart()
{
    processEvent(Event_DemuxProcessStarted);
}

void WebRadioService::startDemux()
{
    if (d->url.length())
    {
        QString cmd = Qtopia::qtopiaDir()+"bin/radio_demux";
        QStringList args;
        args.append(d->url);
        if (d->proc->state() != QProcess::NotRunning)
        {
            d->proc->terminate();
            d->proc->waitForFinished();
        }
        d->proc->start(cmd,args);
    }
}

void WebRadioService::transition (MainState ms, CleanupState cs)
{
    if (ms != State_NoChange)
        mainState = ms;
    if (cs != State_CleanupNoChange)
        cleanupState = cs;
    switch (ms)
    {
        case State_Retrieving:
            if (d->webClient->url().toString() == "")
            {
                output(400,"No Station Selected");
                transition(State_Idle);
            }
            else
            {
                d->webClient->load ();
                d->timer->start (40000);
            }
            break;
        case State_Parsing:
            parsePlaylist();
            break;
        case State_DemuxInit:
            startDemux ();
            d->timer->start (3000);
        break;
        case State_PlayerInit:
            initMediaPlayer();
            d->timer->start (6000);
        break;
        case State_PlayerStart:
            startMediaPlayer();
            d->timer->start (4000);
            break;
        case State_Error:
            output(d->errCode,d->errStr);
            transition (State_Idle);
        case State_Active:
        case State_PostCleanup:
        case State_NoChange:
        case State_Idle:
        case State_PreCleanup:
        case State_DemuxRunning:
        default:
                break;
    }

    switch (cs)
    {
        case State_PlayerStop:
            stopMediaPlayer ();
            d->timer->start (7000);
            break;
        case State_DemuxTerminate:
            stopDemux ();
            d->timer->start (5000);
            break;
        case State_CleanupNoChange:
        case State_CleanupIdle:
        case State_CleanupNeeded:
        default:
            break;
    }
}

void WebRadioService::processEvent (StateMachineEvent e)
{
    d->timer->stop ();
    switch (e)
    {
        case Event_PlayRequest:
            if (cleanupState == State_CleanupNeeded)
            {
                transition(State_PreCleanup,State_PlayerStop);
            }
            else
                transition(State_Retrieving);
            break;
        case Event_PauseRequest:
            if (cleanupState != State_CleanupIdle)
            {
                transition (State_PostCleanup,State_PlayerStop);
            }
            break;
        case Event_PlaylistRetrieved:
            if (mainState == State_Retrieving)
                transition (State_Parsing);
            break;
        case Event_PlaylistParsed:
            if (mainState == State_Parsing)
                transition (State_DemuxInit);
            break;
        case Event_DemuxProcessStarted:
            if (mainState == State_DemuxInit)
                transition (State_DemuxRunning, State_CleanupNeeded);
            else
                transition (State_PostCleanup, State_DemuxTerminate);
            break;
        case Event_DemuxProcessEnded:
            if (cleanupState == State_DemuxTerminate)
                processEvent(Event_CleanupDone);
            else if (cleanupState != State_PlayerStop)
                transition (State_PreCleanup, State_PlayerStop);
            break;
        case Event_MediaPipeReady:
            if (mainState == State_DemuxRunning)
                transition (State_PlayerInit);
            else
                transition (State_PostCleanup, State_PlayerStop);
            break;
        case Event_MediaValid:
            if (mainState == State_PlayerInit)
                transition (State_PlayerStart);
            else
                transition (State_PostCleanup, State_PlayerStop);
            break;
        case Event_MediaStarted:
            if (mainState == State_PlayerStart)
                transition (State_Active);
            else
                transition (State_PostCleanup, State_PlayerStop);
            break;
        case Event_MediaStopped:
            if (cleanupState == State_PlayerStop)
                transition (State_NoChange,State_DemuxTerminate);
            else
                transition (State_Retrieving, State_CleanupIdle);
            break;
        case Event_CleanupDone:
            if (mainState == State_PreCleanup)
            {
                transition (State_Retrieving);
            }
            else if (mainState == State_PostCleanup)
                transition (State_Idle,State_CleanupIdle);
            break;
        case Event_Timeout:
            if (mainState == State_PostCleanup)
            {
                transition (State_Idle,State_CleanupIdle);
            }
            else if (mainState != State_Idle)
            {
                d->errCode = RadioCodes::Timeout;
                d->errStr = "Timeout";
                transition(State_Error);
            }
            break;
    }
}

void WebRadioService::playFile (const QString & fn)
{
    d->mediaFilename = fn;
    transition (State_PlayerInit,State_CleanupNeeded);
}

void WebRadioService::play ()
{
    processEvent (Event_PlayRequest);
}

void WebRadioService::pause ()
{
    processEvent (Event_PauseRequest);
}

QSXE_APP_KEY

int main (int argc, char** argv)
{
    QtopiaApplication app(argc,argv);
    WebRadioService svc (NULL);
    app.registerRunningTask("WebRadio");
    return app.exec ();
}

void WebRadioService::output(int n, const QString & s)
{
    QByteArray a;
    QDataStream ds (&a,QIODevice::WriteOnly);
    ds << n << s;
    QCopChannel::send("radiosvc","output(int,QString)",a);
}
