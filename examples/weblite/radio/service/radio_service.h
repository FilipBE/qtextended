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
#ifndef RADIOSERVICE_H
#define RADIOSERVICE_H
#include <QtopiaAbstractService>
#include <QUrl>
#include <QProcess>
#include <QtopiaMedia>
#include "../radio_codes.h"

class WebRadioServicePrivate;
class WebRadioService : public QtopiaAbstractService
{
    Q_OBJECT
    enum MainState
    {
        State_NoChange = -1,
        State_Idle,
        State_PreCleanup,
        State_Retrieving,
        State_Parsing,
        State_DemuxInit,
        State_DemuxRunning,
        State_PlayerInit,
        State_PlayerStart,
        State_Active,
        State_PostCleanup,
        State_Error
    } mainState;
    
    enum CleanupState
    {
        State_CleanupNoChange = -1,
        State_CleanupIdle,
        State_CleanupNeeded,
        State_PlayerStop,
        State_DemuxTerminate
    } cleanupState;
    
    enum StateMachineEvent
    {
        Event_PlayRequest,
        Event_PauseRequest,
        Event_PlaylistRetrieved,
        Event_PlaylistParsed,
        Event_DemuxProcessStarted,
        Event_DemuxProcessEnded,
        Event_MediaPipeReady,
        Event_MediaValid,
        Event_MediaStarted,
        Event_MediaStopped,
        Event_CleanupDone,
        Event_Timeout
    };
            
    public:
        WebRadioService(QObject* o);
        ~WebRadioService();
            
    public slots:
        void setPlaylistUrl (const QUrl &);
        void play ();
        void playFile (const QString &);
        void setVolume (int);
        void pause ();
        
    private slots:
        void playlistLoaded();
        void parsePlaylist();
        void readFromDemux ();
        void processLine (const QString &);
        void onProcExit(int, QProcess::ExitStatus);
        void onProcStart();
        void mediaValid();
        void initMediaPlayer();
        void startMediaPlayer();
        void stopMediaPlayer();
        void startDemux();
        void stopDemux();
        void onTimeout();
        
        void mediaInvalid ();
        void playerStateChanged(QtopiaMedia::State);
        void processEvent (StateMachineEvent);
        void transition (MainState ms = State_NoChange, CleanupState cs = State_CleanupNoChange);
        
        void output (int , const QString &);
    signals:
        void error (RadioCodes::Status, const QString &);
        void titleChanged (const QString &);
        
    private:
        WebRadioServicePrivate* d;
};
#endif
