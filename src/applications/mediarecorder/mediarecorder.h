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

#ifndef MEDIARECORDER_H
#define MEDIARECORDER_H

#include <qmainwindow.h>
#include <qlist.h>
#include <qmap.h>
#include <qstackedwidget.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qsound.h>
#include <qtopiaabstractservice.h>

#include "ui_mediarecorderbase.h"

class QAction;
class MediaRecorderPluginList;
class SampleBuffer;
class ConfigureRecorder;
class Waveform;
class MediaRecorderEncoder;
class QtopiaChannel;
class QDocumentSelector;
class QContent;
class QAudioInput;
class QDSActionRequest;
class QAudioInterface;

// Define this to record to memory before saving to disk.
//#define RECORD_THEN_SAVE


struct QualitySetting
{
    int         frequency;
    int         channels;
    QString     mimeType;
    QString     formatTag;
};


const int VoiceQuality = 0;
const int MusicQuality = 1;
const int CDQuality = 2;
const int CustomQuality = 3;
const int MaxQualities = 4;


class MediaRecorder : public QMainWindow
{
    Q_OBJECT

public:
    MediaRecorder( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~MediaRecorder();

private:
    void initializeContents();
    void recomputeMaxTime();

    bool startSave();

private slots:
    void endSave();
    void startRecording();
    void stopRecording();
    void startPlaying();
    void stopPlaying();
    void clearData();
    void processAudioData();
    void configure();
    void noPluginError();
    void documentSelected(const QContent&);
    void newSelected();
    void currentDocumentChanged();

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );

public slots:
    void toggleRecording();
    void recordAudio( const QDSActionRequest& request );
    void recordClicked();
    void audioStarted();
    void audioStopped();

private:
    QDocumentSelector *selector;
    QWidget *contentsWidget;
    Ui::MediaRecorderBase *contents;
    ConfigureRecorder *config;
    QAction *configureAction;
    QStackedWidget *stack;
    MediaRecorderPluginList *recorderPlugins;
    QAudioInput *m_audioInput;
    QAudioInterface *m_audioInstance;
    bool audioDeviceIsReady;
#ifdef RECORD_THEN_SAVE
    SampleBuffer *samples;
#endif
    short *sampleBuffer;
    QIODevice *io;
    MediaRecorderEncoder *encoder;
    QSound *m_sound;
    QualitySetting qualities[MaxQualities];
    long recordTime;
    long maxRecordTime;
    bool recording;
    bool playing;
    QString lastSaved;
    QString recordingsCategory;
    QDSActionRequest* mRecordAudioRequest;
    QualitySetting recordQuality;

    enum ContextKey { Select, Record, Stop, Play };
    void setContextKey( ContextKey key );

    void switchToFileSelector();
    void switchToOther();
    void switchToRecorder();
    void switchToPlayback();

    void stopRecordingNoSwitch();
    void stopPlayingNoSwitch();
    void stopEverythingNoSwitch();

    QWidget* getContentsWidget();

    int     m_position;
    bool    m_mousePref;
};

class VoiceRecordingService : public QtopiaAbstractService
{
    Q_OBJECT

    friend class MediaRecorder;

public:
    ~VoiceRecordingService();

public slots:
    void toggleRecording();
    void recordAudio( const QDSActionRequest& request );

private:
    VoiceRecordingService(MediaRecorder *parent)
        : QtopiaAbstractService("VoiceRecording", parent)
    {
        this->parent = parent;
        publishAll();
    }

    MediaRecorder *parent;
};

#endif
