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

#include "iaxtelephonyservice.h"
#include "iaxnetworkregistration.h"
#include "iaxcallprovider.h"
#include "iaxconfiguration.h"
#include "iaxservicechecker.h"
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qaudioinput.h>
#include <qaudiooutput.h>
#include <qvaluespace.h>
#include <qtopiaipcenvelope.h>
#include <QSocketNotifier>
#include <QTimer>

#include "iaxclient.h"
#include "iaxclient_lib.h"
#include "iax-client.h"

/*
    Implement the iaxclient library's audio device API.  Most of these
    are stubbed out because we don't need them.
*/

static int audioInitialize( struct iaxc_audio_driver *, int )
{
    return 0;
}

static int audioDestroy( struct iaxc_audio_driver * )
{
    return 0;
}

static int audioSelectDevices( struct iaxc_audio_driver *, int, int, int )
{
    return 0;
}

static int audioSelectedDevices( struct iaxc_audio_driver *, int *input, int *output, int *ring )
{
    *input = 0;
    *output = 0;
    *ring = 0;
    return 0;
}

static int audioStart( struct iaxc_audio_driver *d )
{
    return ((IaxTelephonyService *)(d->priv))->startAudio() ? 0 : -1;
}

static int audioStop( struct iaxc_audio_driver *d )
{
    ((IaxTelephonyService *)(d->priv))->stopAudio();
    return 0;
}

static int audioWrite( struct iaxc_audio_driver *d, void *samples, int nSamples )
{
    return ((IaxTelephonyService *)(d->priv))->writeAudio( samples, nSamples );
}

static int audioRead( struct iaxc_audio_driver *d, void *samples, int *nSamples )
{
    int result = ((IaxTelephonyService *)(d->priv))->readAudio( samples, *nSamples );
    if ( result >= 0 ) {
        *nSamples = result;
        return 0;
    } else {
        *nSamples = 0;
        return -1;
    }
}

static double audioInputLevelGet( struct iaxc_audio_driver * )
{
    return -1;
}

static double audioOutputLevelGet( struct iaxc_audio_driver * )
{
    return -1;
}

static int audioInputLevelSet( struct iaxc_audio_driver *, double )
{
    return -1;
}

static int audioOutputLevelSet( struct iaxc_audio_driver *, double )
{
    return -1;
}

static int audioPlaySound( struct iaxc_sound *, int )
{
    return 0;
}

static int audioStopSound( int )
{
    return 0;
}

static int audioMicBoostGet( struct iaxc_audio_driver * )
{
    return -1;
}

static int audioMicBoostSet( struct iaxc_audio_driver *, int )
{
    return -1;
}

static IaxTelephonyService *eventObject = 0;
static int eventCallback( iaxc_event e )
{
    if ( eventObject ) {
        if ( eventObject->processEvent( &e ) )
            return 1;
        else
            return 0;
    } else {
        return 0;
    }
}

IaxTelephonyService::IaxTelephonyService
        ( const QString& service, QObject *parent )
    : QTelephonyService( service, parent ), initialized(false)
{
    // Register our own audio driver with the iaxclient library.
    struct iaxc_audio_driver *driver = iaxc_get_audio_driver();
    // note driver->name pointer is not deleted in audioDestroy(..)
    driver->name = (char *)("iaxagent");
    driver->devices = 0;
    driver->nDevices = 0;
    driver->priv = (void *)this;
    driver->initialize = audioInitialize;
    driver->destroy = audioDestroy;
    driver->select_devices = audioSelectDevices;
    driver->selected_devices = audioSelectedDevices;
    driver->start = audioStart;
    driver->stop = audioStop;
    driver->output = audioWrite;
    driver->input = audioRead;
    driver->input_level_get = audioInputLevelGet;
    driver->output_level_get = audioOutputLevelGet;
    driver->input_level_set = audioInputLevelSet;
    driver->output_level_set = audioOutputLevelSet;
    driver->play_sound = audioPlaySound;
    driver->stop_sound = audioStopSound;
    driver->mic_boost_get = audioMicBoostGet;
    driver->mic_boost_set = audioMicBoostSet;
    audioInput = 0;
    audioOutput = 0;
    timer = 0;
    retryCount = 0;
    interval = 2;   // XXX

    // Create a timer for handling timed events within iaxclient.
    iaxTimer = new QTimer( this );
    iaxTimer->setSingleShot( true );
    connect( iaxTimer, SIGNAL(timeout()), this, SLOT(serviceIaxClient()) );

    // Initialize the iaxclient library.
    if ( iaxc_initialize( AUDIO_EXTERNAL, IAXAGENT_MAX_CALLS ) < 0 ) {
        qLog(VoIP) << "iaxc_initialize failed";
        return;
    }
    qLog(VoIP) << "iaxclient bound to port" << (int)iaxc_get_bind_port();

    // Register for read selects on the IAX network port.
    QSocketNotifier *notifier;
    notifier = new QSocketNotifier( iax_get_fd(), QSocketNotifier::Read, this );
    connect( notifier, SIGNAL(activated(int)), this, SLOT(serviceIaxClient()) );

    // Set the iaxclient library's event callback.
    eventObject = this;
    iaxc_set_event_callback( eventCallback );

    // Schedule an initial timed event, if requested by iaxclient.
    int time = iax_time_to_next_event();
    if ( time >= 0 )
        iaxTimer->start( time );

    initialized = true;
}

IaxTelephonyService::~IaxTelephonyService()
{
    stopAudio();
    if ( initialized )
        iaxc_shutdown();
    eventObject = 0;
}

void IaxTelephonyService::initialize()
{
    if ( !supports<QNetworkRegistration>() )
        addInterface( new IaxNetworkRegistration( this ) );

    if ( !supports<QTelephonyConfiguration>() )
        addInterface( new IaxConfiguration( this ) );

    if ( !supports<QServiceChecker>() )
        addInterface( new IaxServiceChecker( this ) );

    if ( !callProvider() )
        setCallProvider( new IaxCallProvider( this ) );

    QTelephonyService::initialize();
}

bool IaxTelephonyService::startAudio()
{
    // Bail out if already started.
    if ( audioInput )
        return true;

    qLog(VoIP) << "IaxTelephonyService::startAudio()";

    // Create the audio input and output handlers.
    audioInput = new QAudioInput( this );
    audioOutput = new QAudioOutput( this );

    audioInput->setFrequency( 8000 );
    audioInput->setChannels( 1 );
    audioInput->setSamplesPerBlock( 8000 / 50 ); // 20 ms frame size

    audioOutput->setFrequency( 8000 );
    audioOutput->setChannels( 1 );
    audioOutput->setBitsPerSample( 16 );

    if ( !audioInput->open( QIODevice::ReadOnly ) ) {
        qLog(VoIP) << "Audio device open failed";
        delete audioInput;
        delete audioOutput;
        audioInput = 0;
        audioOutput = 0;
        return false;
    }
    char buffer[256];
    audioInput->read( buffer, sizeof( buffer ) ); // Dummy read to start input.

#if defined(MEDIA_SERVER) 
    // Media server must relinquish the audio device
    {
        QtopiaIpcEnvelope e("QPE/MediaServer", "setPriority(int)" );

        e << 1; // RingTone
    }

    retryAudioOpen();
#else
    audioOutput->open( QIODevice::WriteOnly );
#endif

    connect( audioInput, SIGNAL(readyRead()), this, SLOT(serviceIaxClient()) );

    // Open and ready for use.
    return true;
}

void IaxTelephonyService::stopAudio()
{
    if ( audioInput ) {
        qLog(VoIP) << "IaxTelephonyService::stopAudio()";
        delete audioInput;
        audioInput = 0;
    }
    if ( audioOutput ) {
        delete audioOutput;
        audioOutput = 0;

#if defined(MEDIA_SERVER) 
        retryCount = 0;
        interval = 2;
        // Media server can use the audio device
        {
            QtopiaIpcEnvelope e("QPE/MediaServer", "setPriority(int)" );

            e << 0; // Default
        }
#endif
    }
}

int IaxTelephonyService::writeAudio( const void *samples, int numSamples )
{
    if ( audioOutput && audioOutput->isOpen() )
        audioOutput->write( (const char *)samples, numSamples * 2 );
    return 0;
}

int IaxTelephonyService::readAudio( void *samples, int numSamples )
{
    if ( !audioInput || !audioInput->isOpen() )
        return -1;
    int len = numSamples * 2;
    char *buf = (char *)samples;
    int readlen = 0;
    while ( len > 0 ) {
        int result = audioInput->read( buf, len );
        if ( result < 0 )
            break;      // A read error occurred.
        else if ( result == 0 && !readlen )
            break;      // Nothing in audio input buffer.
        readlen += result;
        buf += result;
        len -= result;
    }
    return readlen / 2;
}

bool IaxTelephonyService::processEvent( struct iaxc_event_struct *e )
{
    switch ( e->type ) {

        case IAXC_EVENT_TEXT:
        {
            // Status/error message from the iaxclient library.
            switch ( e->ev.text.type ) {
                case IAXC_TEXT_TYPE_STATUS:
                    qLog(VoIP) << "Iax Client Status:" << e->ev.text.message;
                    break;

                case IAXC_TEXT_TYPE_NOTICE:
                    qLog(VoIP) << "Iax Client Notice:" << e->ev.text.message;
                    break;

                case IAXC_TEXT_TYPE_ERROR:
                    qLog(VoIP) << "Iax Client Error:" << e->ev.text.message;
                    break;

                case IAXC_TEXT_TYPE_FATALERROR:
                    qLog(VoIP) << "Iax Client Fatal:" << e->ev.text.message;
                    break;

                case IAXC_TEXT_TYPE_IAX:
                    qLog(VoIP) << "Iax Server Message:" << e->ev.text.message;
                    break;
            }
        }
        break;

        case IAXC_EVENT_STATE:
        {
            // A call has changed state.
            IaxCallProvider *provider;
            provider = qobject_cast<IaxCallProvider *>( callProvider() );
            if ( provider )
                provider->stateEvent( &(e->ev.call) );
        }
        break;

        case IAXC_EVENT_REGISTRATION:
        {
            // Response to a registration request.
            IaxNetworkRegistration *reg = netReg();
            if ( reg )
                reg->registrationEvent( e->ev.reg.reply );
        }
        break;

        default: return false;
    }
    return true;
}

void IaxTelephonyService::serviceIaxClient()
{
    // There is network activity or audio input, so let iaxclient process it.
    iaxc_process_calls();

    // Determine if we need to schedule a timer event.
    int time = iax_time_to_next_event();
    if ( time >= 0 )
        iaxTimer->start( time );
    else
        iaxTimer->stop();
}

void IaxTelephonyService::updateRegistrationConfig()
{
    IaxNetworkRegistration *reg = netReg();
    if ( reg )
        reg->updateRegistrationConfig();
    IaxServiceChecker *checker =
        qobject_cast<IaxServiceChecker *>( interface<QServiceChecker>() );
    if ( checker )
        checker->updateRegistrationConfig();
}

void IaxTelephonyService::updateCallerIdConfig()
{
    IaxCallProvider *provider;
    provider = qobject_cast<IaxCallProvider *>( callProvider() );
    if ( provider )
        provider->updateCallerIdConfig();
}

static const int interval_multiplier = 10;  // XXX
static const int max_retry_count = 4;

void IaxTelephonyService::retryAudioOpen()
{
#if defined(MEDIA_SERVER) 
    if (audioOutput != 0 && !audioOutput->open(QIODevice::WriteOnly))
    {
        if (timer == 0)
        {
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(retryOpen()));
            timer->setSingleShot(true);
        }

        if (retryCount++ < max_retry_count)
            timer->start(interval *= interval_multiplier);
    }
#endif
}

IaxNetworkRegistration *IaxTelephonyService::netReg() const
{
    return qobject_cast<IaxNetworkRegistration *>
                ( interface<QNetworkRegistration>() );
}

IaxTelephonyServiceQCop::IaxTelephonyServiceQCop( QObject *parent )
    : QtopiaAbstractService( "Telephony", parent )
{
    publishAll();
    service = 0;
}

IaxTelephonyServiceQCop::~IaxTelephonyServiceQCop()
{
}

void IaxTelephonyServiceQCop::start()
{
    if ( !service ) {
        // Register a task to keep us alive while the service is running.
        QtopiaApplication::instance()->registerRunningTask
            ( "IaxTelephonyService", this );

        // Create the service handler, registered under the name "asterisk".
        qLog(VoIP) << "Starting iaxclient service handler";
        service = new IaxTelephonyService( "asterisk", this );
        service->initialize();
    }
}

void IaxTelephonyServiceQCop::stop()
{
    if ( service ) {
        // Delete the service handler.
        qLog(VoIP) << "Stopping iaxclient service handler";
        delete service;
        service = 0;

        // Deregister the task which allows this daemon to shut down.
        QtopiaApplication::instance()->unregisterRunningTask
            ( "IaxTelephonyService" );
    }
}
