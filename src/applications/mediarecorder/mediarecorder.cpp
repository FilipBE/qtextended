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

#include "mediarecorder.h"
#include "audioparameters_p.h"

#include "samplebuffer.h"
#include "pluginlist.h"
#include "timeprogressbar.h"
#include "confrecorder.h"
#include "waveform.h"

#include <qdocumentselector.h>
#include <qcontent.h>
#include <qstorage.h>
#include <qstoragedeviceselector.h>
#include <qtopiaapplication.h>
#include <qmimetype.h>
#include <qcategorymanager.h>
#include <qaudioinput.h>
#include <qtopia/qsoftmenubar.h>

#include <qaction.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qmenu.h>
#include <qevent.h>

#include <QDSData>
#include <QDSActionRequest>
#include <QDSServiceInfo>

#include <QtopiaIpcEnvelope>
#include <QtopiaIpcAdaptor>

#include <qaudiointerface.h>

#include <stdlib.h>


#define MR_BUFSIZE  1024


MediaRecorder::MediaRecorder(QWidget *parent, Qt::WFlags f):
    QMainWindow( parent, f ),
    contentsWidget( NULL ),
    config( 0 ),
    recorderPlugins( NULL ),
    m_audioInput( 0 ),
    m_audioInstance( 0 ),
    audioDeviceIsReady( false ),
#ifdef RECORD_THEN_SAVE
    samples( 0 ),
#endif
    sampleBuffer( 0 ),
    io( 0 ),
    m_sound( NULL ),
    recordTime( 0 ),
    recording( false ),
    playing( false ),
    recordingsCategory( "Recordings" ),
    mRecordAudioRequest( 0 ),
    m_position( 0 )
{
    // Adjust window decorations
    setWindowTitle(tr("Voice Notes"));
    setWindowIcon(QIcon( ":image/SoundPlayer"));

    // We don't need an input method with this application.
    QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);

    // Make sure that the "Recordings" category is registered.
    QCategoryManager catman("Documents");
    // For new code a more unique id should be used instead of using the untranslated text
    // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
    catman.ensureSystemCategory(recordingsCategory, recordingsCategory);

    // Create stack widget
    stack = new QStackedWidget(this);
    stack->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setCentralWidget(stack);

    // Add the Document selector
    selector = new QDocumentSelector( stack );
    selector->enableOptions( QDocumentSelector::NewDocument );
    selector->setSortMode(QDocumentSelector::ReverseChronological);
    selector->setFilter(
            QContentFilter( QContent::Document ) &
            QContentFilter( QContentFilter::MimeType, "audio/*" ) &
            QContentFilter( QContentFilter::Category, recordingsCategory ) &
            QContentFilter( QContentFilter::DRM, QLatin1String( "Unprotected" ) ) );
    selector->setFocus( Qt::OtherFocusReason );
    stack->addWidget(selector);

    connect(selector, SIGNAL(documentSelected(QContent)),
            this, SLOT(documentSelected(QContent)));

    connect(selector, SIGNAL(newSelected()),
            this, SLOT(newSelected()));

    connect(selector, SIGNAL(currentChanged()),
            this, SLOT(currentDocumentChanged()));

    // Listen for "VoiceRecording" service messages.
    new VoiceRecordingService(this);

    m_mousePref = Qtopia::mousePreferred();
}


MediaRecorder::~MediaRecorder()
{
    if(m_audioInput)
        delete m_audioInput;
    if(m_audioInstance)
        delete m_audioInstance;

    delete recorderPlugins;

#ifdef RECORD_THEN_SAVE
    if ( samples )
        delete samples;
#endif

    if (sampleBuffer)
        delete[] sampleBuffer;

    if (io)
        delete io;

    delete m_sound;
    delete mRecordAudioRequest;
}


void MediaRecorder::initializeContents()
{
    // The progress bar initially has no time display.  This will be fixed
    // up when we start recording.
    contents->progress->setMaximum( 10 );
    contents->progress->setValue( -1 );

    if ( recorderPlugins == 0 )
        recorderPlugins = new MediaRecorderPluginList();

    // Load the initial quality settings.
    config = new ConfigureRecorder(qualities, recorderPlugins, this);

    recomputeMaxTime();

    // Create a menu with "Help" on the dialog.
    QSoftMenuBar::menuFor( config );

    // Disable the settings boxes in phone mode.
    contents->GroupBox1->hide();
}

void MediaRecorder::recomputeMaxTime()
{
    // Determine the maximum available space on the device.
    const QFileSystem *fs = config->fileSystem();

    long availBlocks;
    long blockSize;
    if ( fs ) {
        availBlocks = fs->availBlocks();
        blockSize = fs->blockSize();
    } else {
        availBlocks = 0;
        blockSize = 512;
    }

    // Calculate the number of bytes per second for the current quality,
    // by asking the plugin for an estimate.
    MediaRecorderEncoder *encoder = recorderPlugins->fromType
            ( qualities[config->currentQuality()].mimeType,
              qualities[config->currentQuality()].formatTag );
    long bytesPerSec;
    if( encoder ) {
        bytesPerSec = encoder->estimateAudioBps
            ( qualities[config->currentQuality()].frequency,
              qualities[config->currentQuality()].channels,
              qualities[config->currentQuality()].formatTag );
        if ( bytesPerSec <= 0)
            bytesPerSec = 1;
    } else {
        // We don't have an encoder, so make an estimate based on
        // assuming that the format is wav.
        bytesPerSec = qualities[config->currentQuality()].frequency *
                      qualities[config->currentQuality()].channels * 2;
    }

    // Get an estimate of the maximum number of seconds that we can record.
    // Use "double" to avoid truncation errors with 32-bit arithmetic.
    long maxSecs = (long)(((double)availBlocks) * ((double)blockSize) /
                                (double)bytesPerSec);

    // Truncate the maximum to a reasonable human-grokkable time boundary,
    // as there is no point displaying things like "5 hrs 23 mins 6 secs".
    if ( maxSecs >= (60 * 60 * 24) ) {
        // Truncate to a 1 hour boundary.
        maxSecs -= (maxSecs % (60 * 60));
    } else if ( maxSecs >= (60 * 60 * 10) ) {
        // Truncate to a 15 minute boundary.
        maxSecs -= (maxSecs % (15 * 60));
    } else if ( maxSecs >= (60 * 10) ) {
        // Tuncate to a 1 minute boundary.
        maxSecs -= (maxSecs % 60);
    } else if ( maxSecs > 60 ) {
        // Truncate to a 15 second boundary.
        maxSecs -= (maxSecs % 15);
    }

    // Format the string for the max time field.
    QString str;
    if ( maxSecs >= (60 * 60 * 24) ) {
        if ( (maxSecs % (60 * 60 * 24)) == 0 ) {
            str = tr("%1 days").arg((int)(maxSecs / (60 * 60 * 24)));
        } else {
            str = tr("%1 days %2 hrs")
                .arg((int)(maxSecs / (60 * 60 * 24)))
                .arg((int)((maxSecs / (60 * 60)) % 24));
        }
    } else if ( maxSecs >= (60 * 60) ) {
        if ( (maxSecs % (60 * 60)) == 0 ) {
            str = tr("%1 hrs").arg((int)(maxSecs / (60 * 60)));
        } else {
            str = tr("%1 hrs %2 mins")
                .arg((int)(maxSecs / (60 * 60)))
                .arg((int)((maxSecs / 60) % 60));
        }
    } else if ( maxSecs >= 60 ) {
        if ( (maxSecs % 60) == 0 ) {
            str = tr("%1 mins").arg((int)(maxSecs / 60));
        } else {
            str = tr("%1 mins %2 secs").arg((int)(maxSecs / 60)).arg((int)(maxSecs % 60));
        }
    } else {
        str = tr("%1 secs").arg((int)maxSecs);
    }

    // Update the max time field.
    contents->maxTime->setText( str );

    maxRecordTime = maxSecs;
}


bool MediaRecorder::startSave()
{
    // Find the plugin to use to save the data.
    encoder = recorderPlugins->fromType(recordQuality.mimeType,
                                        recordQuality.formatTag);

    // Open the document.
    QContent    doc;

    doc.setName(tr("Voice, %1","date")
        .arg(QTimeString::localYMDHMS(QDateTime::currentDateTime(),QTimeString::Short)));
    doc.setType( encoder->pluginMimeType() );
    doc.setMedia( config->documentPath() );

    QList<QString>  cats;
    cats.append(recordingsCategory);
    doc.setCategories(cats);

    io = doc.open(QIODevice::WriteOnly);

    // Write the sample data using the encoder.
    encoder->begin(io, recordQuality.formatTag);
    encoder->setAudioChannels(m_audioInput->channels());
    encoder->setAudioFrequency(m_audioInput->frequency());

    // Record the location of the file that we are saving.
    lastSaved = doc.fileName();

    doc.commit();

    return true;
}

void MediaRecorder::endSave()
{
    // Flush the samples if we recorded to memory.
#ifdef RECORD_THEN_SAVE
    samples->rewind();
    short *buf;
    unsigned int length;
    while ( samples->nextReadBuffer( buf, length ) ) {
        if ( !encoder->writeAudioSamples( buf, (long)length ) )
            break;
    }
#endif

    // Terminate the encode process.
    encoder->end();

    // Close the document.
    if(io != NULL) {
        io->close();
        delete io;
    }
    io = 0;

    // Clear the data for another recording.
    clearData();
}


void MediaRecorder::startRecording()
{
    if(!m_audioInstance)
        m_audioInstance = new QAudioInterface("Media", this);

    if(!m_audioInput)
        m_audioInput = new QAudioInput(this);

    m_audioInstance->setInput(*m_audioInput);

    if ( config == 0 )
        switchToRecorder();

    if ( mRecordAudioRequest == 0)
        recordQuality = qualities[config->currentQuality()];

    // Bail out if we don't have a plugin for the selected format.
    if (recorderPlugins->fromType(recordQuality.mimeType,
                                    recordQuality.formatTag) == 0) {
        noPluginError();
        return;
    }

    // Disable power save while recording so that the device
    // doesn't suspend while a long-term recording session
    // is in progress.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);

    // Configure and open device
    m_audioInput->setFrequency(recordQuality.frequency);
    m_audioInput->setChannels(recordQuality.channels);

    // TODO: move to ctor
    connect(m_audioInput, SIGNAL(readyRead()),
            this, SLOT(processAudioData()));
    connect(m_audioInstance, SIGNAL(audioStarted()), this, SLOT(audioStarted()));
    connect(m_audioInstance, SIGNAL(audioStopped()), this, SLOT(audioStopped()));
    m_audioInstance->startAudio();

    // Create the sample buffer, for recording the data temporarily.
#ifdef RECORD_THEN_SAVE
    if (samples)
        delete samples;

    samples = new SampleBuffer(audioInput->bufferSize());
#else
    if (sampleBuffer)
        delete[] sampleBuffer;

    sampleBuffer = new short[MR_BUFSIZE];
#endif

    // Reset the position, which is used to calculated the sample progress
    m_position = 0;

    // Start the save process.
    if (startSave()) {

        // Create the waveform display.
        contents->waveform->changeSettings( m_audioInput->frequency(),
                                            m_audioInput->channels());

        if (configureAction)
            configureAction->setEnabled(false);

        recordTime = 0;
        contents->progress->setMaximum( 120 );
        contents->progress->setValue( 0 );
        contents->progress->setRecording();
        recording = true;
        setContextKey( Stop );

        // Some audio devices may start sending us data immediately, but
        // others may need an initial "read" to start the ball rolling.
        // Processing at least one data block will prime the device.
        processAudioData();
    }
}


void MediaRecorder::stopRecordingNoSwitch()
{
    contents->waveform->reset();

    if (configureAction)
        configureAction->setEnabled( true );

    if(m_mousePref) contents->recordButton->setEnabled(false);
    recording = false;
    setContextKey( Record );
    if(m_mousePref) contents->recordButton->setText( tr("Record") );
    // Terminate the data save.
    endSave();

    // Re-enable power save.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    // If we were in request mode, then send the response and quit the app.
    if ( mRecordAudioRequest != 0 ) {
        QByteArray contentArray;
        {
            QDataStream stream( &contentArray, QIODevice::WriteOnly );
            stream << lastSaved;
        }

        QDSData contentData( contentArray, QMimeType( "audio/x-qstring" ) );
        mRecordAudioRequest->respond( contentData );

        delete mRecordAudioRequest;
        mRecordAudioRequest = 0;
        qApp->quit();
    }
}


void MediaRecorder::stopRecording()
{
    stopRecordingNoSwitch();
    switchToFileSelector();

    delete m_audioInstance;
    m_audioInstance = 0;
}


void MediaRecorder::startPlaying()
{
    // Reconfigure the UI to reflect the current mode.
    if (configureAction)
        configureAction->setEnabled( false );

    recordTime = 0;

    // Disable power save while playing so that the device
    // doesn't suspend before the file finishes.
    QtopiaApplication::setPowerConstraint(QtopiaApplication::DisableSuspend);

    contents->progress->setValue( 0 );
    contents->progress->setPlaying();
    playing = true;
    setContextKey( Stop );
    if(m_mousePref) contents->recordButton->setEnabled(true);
}


void MediaRecorder::stopPlayingNoSwitch()
{
    // Stop playing back the recorded sound
    if (m_sound)
    {
        delete m_sound;
        m_sound = 0;
    }

    if ( sampleBuffer ) {
        delete[] sampleBuffer;
        sampleBuffer = 0;
    }

    // Re-enable power save (that was disabled in startPlaying()).
    QtopiaApplication::setPowerConstraint(QtopiaApplication::Enable);

    // Ensure UI initialized
    getContentsWidget();

    // Return the UI to the default state.
    contents->waveform->reset();
    if ( configureAction )
        configureAction->setEnabled( true );

    if(m_mousePref)contents->recordButton->setEnabled(true);
    playing = false;
    setContextKey( Play );
    contents->progress->setValue( 0 );
}


void MediaRecorder::stopPlaying()
{
    stopPlayingNoSwitch();
    switchToFileSelector();
}


void MediaRecorder::clearData()
{
#ifdef RECORD_THEN_SAVE
    samples->clear();
#endif
    contents->waveform->reset();
    if ( configureAction )
        configureAction->setEnabled( true );
    if(m_mousePref)contents->recordButton->setEnabled(true);
    recordTime = 0;
    contents->progress->setMaximum( 120 );
    contents->progress->setValue( 0 );
}


void MediaRecorder::processAudioData()
{
    int     result;
    long    newTime;
    bool    stopped = false;

#ifdef RECORD_THEN_SAVE
    short *buf;
    unsigned int length;

    if ( samples->nextWriteBuffer( buf, length ) ) {

        // Read the next block of samples into the write buffer.
        result = m_audioInput->read(buf, length);
        if (result > 0) {
            samples->commitWriteBuffer( (unsigned int)result );

            // Update the waveform display.
            contents->waveform->newSamples( buf, result );
        }
    }
    else {

        // The sample buffer is out of space, so stop recording.
        stopRecording();
        stopped = true;
    }
#else

    result = m_audioInput->read(reinterpret_cast<char*>(sampleBuffer), MR_BUFSIZE);

    if (result > 0) {

        result /= sizeof(short) * m_audioInput->channels();

        contents->waveform->newSamples(sampleBuffer, result);

        encoder->writeAudioSamples(sampleBuffer, (long)result);

        m_position += result;
    }

#endif

    // Update the record time if another second has elapsed.
    newTime = m_position / (long)(m_audioInput->frequency());
    if (newTime != recordTime) {
        recordTime = newTime;

        if (recordTime >= contents->progress->maximum()) {
            // Change the resolution on the progress bar as we've
            // max'ed out the current limit.
            contents->progress->setMaximum(contents->progress->maximum() * 4);
        }

        contents->progress->setValue((int) recordTime);
    }

    // Stop recording if we have hit the maximum record time.
    if (recordTime >= maxRecordTime && !stopped) {
        stopRecording();
    }
}


void MediaRecorder::configure()
{
    config->processPopup();
    config->saveConfig();
    recomputeMaxTime();
}


void MediaRecorder::noPluginError()
{
    QMessageBox::critical( this, tr( "No plugin found" ),
                           tr( "<qt>Voice Recorder was unable to "
                               "locate a suitable plugin to "
                               "record in the selected format.</qt>" ) );
}


void MediaRecorder::closeEvent(QCloseEvent *e)
{
    // Shut down recording or playback.
    if (contentsWidget != NULL)
    {
        stopEverythingNoSwitch();
    }

    // Determine if we should return to the file selector screen,
    // or exit from the application.
    if (stack->currentWidget() == selector)
    {
        e->accept();
    }
    else if ( mRecordAudioRequest != 0)
    {
        QByteArray contentArray;
        {
            QDataStream stream( &contentArray, QIODevice::WriteOnly );
            stream << lastSaved;
        }

        QDSData contentData( contentArray, QMimeType( "audio/x-qstring" ) );
        mRecordAudioRequest->respond( contentData );

        delete mRecordAudioRequest;
        mRecordAudioRequest = 0;

        e->accept();
    }
    else
    {
        switchToFileSelector();
        e->ignore();
    }
}

void MediaRecorder::keyPressEvent( QKeyEvent *e )
{
    if (e->key() == Qt::Key_Select)
    {
        if (playing)
            stopPlaying();
        else if (recording)
            stopRecording();
        else
            startRecording();

        e->accept();

        return;
    }

    QMainWindow::keyPressEvent(e);
}


void MediaRecorder::setContextKey( ContextKey key )
{
    switch (key) {
        case Select:
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
            break;
        case Record:
            QSoftMenuBar::setLabel( this, Qt::Key_Select, "mediarecorder/record", tr("Record") );
            break;
        case Play:
            QSoftMenuBar::setLabel( this, Qt::Key_Select, "play", tr("Play") );
            break;
        case Stop:
            QSoftMenuBar::setLabel( this, Qt::Key_Select, "stop", tr("Stop") );
            break;
    }
}


void MediaRecorder::documentSelected(const QContent& doc)
{
    if(playing && lastSaved == doc.fileName()) {
        // Stop currently playing note
        if(m_sound->isFinished()) {
            stopPlayingNoSwitch();
            lastSaved = doc.fileName();
            m_sound = new QSound( lastSaved );
            m_sound->play();
            playing = true;
        } else {
            stopPlayingNoSwitch();
            playing = false;
        }
    } else if(playing) {
        // Stop currently playing one and start new one playing
        stopPlayingNoSwitch();
        lastSaved = doc.fileName();
        m_sound = new QSound( lastSaved );
        m_sound->play();
        playing = true;
    } else {
        // Nothing playing just play
        lastSaved = doc.fileName();
        m_sound = new QSound( lastSaved );
        m_sound->play();
        playing = true;
    }
}


void MediaRecorder::newSelected()
{
    if (recorderPlugins == NULL)
        recorderPlugins = new MediaRecorderPluginList();

    if (m_sound != NULL)        // stop playing
    {
        delete m_sound;
        m_sound = NULL;
        playing = false;
    }

    switchToRecorder();
}

void MediaRecorder::currentDocumentChanged()
{
    if( selector->newCurrent() )
        setContextKey( Select );
    else
        setContextKey( Play );
}

void MediaRecorder::toggleRecording()
{
    if ( playing )
        stopPlayingNoSwitch();

    switchToRecorder();

    if (recording)
        stopRecording();
    else
        startRecording();
}

void MediaRecorder::recordAudio( const QDSActionRequest& request )
{
    if ( mRecordAudioRequest != 0 ) {
        QDSActionRequest( request ).respond( tr( "Busy serving previous request" ) );
    }

    // Stop existing recording or playback sessions.
    stopEverythingNoSwitch();

    // Unpack the audio parameters and setup the recorder
    QDataStream stream( request.requestData().toIODevice() );
    AudioParameters parameters;
    stream >> parameters;

    recordQuality.frequency = parameters.frequency();
    recordQuality.channels = parameters.channels();
    recordQuality.mimeType = parameters.mimeType().id();
    recordQuality.formatTag = parameters.subFormat();

    // Save the request and switch to the recording mode.
    mRecordAudioRequest = new QDSActionRequest( request );
    showMaximized();
    switchToRecorder();
}

void MediaRecorder::switchToFileSelector()
{
    stack->setCurrentWidget( selector );
    selector->setFocus();

    configureAction->setEnabled( false );
    setContextKey( Select );
}


void MediaRecorder::switchToOther()
{
    stack->setCurrentIndex( stack->indexOf( getContentsWidget() ) );

    currentDocumentChanged();
}

void MediaRecorder::switchToRecorder()
{
    switchToOther();

    setContextKey( Record );
    if(m_mousePref)contents->recordButton->show();
    configureAction->setEnabled(true);
    contents->waveform->reset();
    if(m_mousePref)contents->recordButton->setFocus();
}


void MediaRecorder::switchToPlayback()
{
    switchToOther();
    setContextKey( Play );
    configureAction->setEnabled( false );
    if(m_mousePref)contents->recordButton->hide();
    contents->waveform->reset();
    if(m_mousePref)contents->recordButton->setFocus();
}

void MediaRecorder::stopEverythingNoSwitch()
{
    if (recording)
    {
        stopRecordingNoSwitch();
    }
    else if (playing)
    {
        stopPlayingNoSwitch();
    }
}

QWidget* MediaRecorder::getContentsWidget()
{
    if (contentsWidget == NULL)
    {
        // contents (buttons & graph)
        contentsWidget = new QWidget(stack);
        contents = new Ui::MediaRecorderBase();
        contents->setupUi(contentsWidget);
        stack->addWidget( contentsWidget );

        // other init
        initializeContents();

        // Create the context menu for the record/playback screen.
        QMenu *options = QSoftMenuBar::menuFor(contentsWidget);

        configureAction = new QAction(QIcon(":icon/settings"), tr( "Settings..."), this);
        connect(configureAction, SIGNAL(triggered()), this, SLOT(configure()));
        configureAction->setWhatsThis(tr("Configure the recording quality settings."));
        configureAction->setEnabled(true);
        options->addAction(configureAction);

        // Make the context key say "Record".
        setContextKey( Record );
        if(m_mousePref) {
            connect(contents->recordButton, SIGNAL(clicked()), this, SLOT(recordClicked()));
        } else
            contents->recordButton->hide();
    }

    return contentsWidget;
}

void MediaRecorder::recordClicked()
{
    if (recording) {
        contents->recordButton->setText(tr("Record"));
        stopRecording();
    } else {
        startRecording();
        contents->recordButton->setText(tr("Stop"));
    }
}

void MediaRecorder::audioStarted()
{

}

void MediaRecorder::audioStopped()
{
}

/*!
    \service VoiceRecordingService VoiceRecording
    \inpublicgroup QtPimModule
    \brief The VoiceRecordingService class provides the VoiceRecording service.

    The \i VoiceRecording service enables applications to toggle
    audio recording on or off.
*/
/*!
    \internal
*/
VoiceRecordingService::~VoiceRecordingService()
{
}

/*!
    Toggle audio recording on or off.

    This slot corresponds to the QCop service message
    \c{VoiceRecording::toggleRecording()}.
*/
void VoiceRecordingService::toggleRecording()
{
    parent->toggleRecording();
    QtopiaApplication::instance()->showMainWidget();
}

/*!
    Prepares for audio recording using the parameters specified in \a request.

    This slot corresponds to a QDS service with a request data type of
    "x-parameters/x-audioparameters" and a response data type of "audio/x-qstring".

    This slot corresponds to the QCop service message
    \c{VoiceRecording::recordAudio(QDSActionRequest)}.
*/
void VoiceRecordingService::recordAudio( const QDSActionRequest& request )
{
    parent->recordAudio( request );
}
