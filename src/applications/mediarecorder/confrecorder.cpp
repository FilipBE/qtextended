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

#include "confrecorder.h"
#include "mediarecorder.h"
#include "pluginlist.h"

#include <qcombobox.h>
#include <qsettings.h>
#include <qtopiaapplication.h>
#include <qformlayout.h>
#include <qstorage.h>
#include <qstoragedeviceselector.h>

#include "custom.h"


// Default quality settings, if not yet set in the configuration.
static const QualitySetting DefaultQualities[] = {
    {8000, 1, "audio/x-wav", "pcm"},
    {22050, 1, "audio/x-wav", "pcm"},
    {44100, 1, "audio/x-wav", "pcm"},
    {8000, 1, "audio/x-wav", "pcm"},
};

// Section names within the configuration file.
static const char * const ConfigSections[MaxQualities] = {
    "VoiceQuality", "MusicQuality", "CDQuality", "CustomQuality"
};

ConfigureRecorder::ConfigureRecorder( QualitySetting *_qualities, MediaRecorderPluginList *_plugins, QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
    , qualities( _qualities )
    , plugins( _plugins )
    , quality( CustomQuality )
{
    setObjectName( "settings" );     // To display the correct help page.

    // Create the UI.
    QFormLayout *layout = new QFormLayout( this );

    presetQualities = new QComboBox();
    presetQualities->addItem( tr("Voice") );
#ifndef EXCLUSIVE_AUDIO_SETTING
    presetQualities->addItem( tr("Music") );
    presetQualities->addItem( tr("CD") );
    presetQualities->addItem( tr("Custom") );
#endif
    layout->addRow( tr("Quality"), presetQualities );

    channels = new QComboBox();
#ifndef EXCLUSIVE_AUDIO_SETTING
    channels->addItem( tr("Mono") );
#else
    if(EXCLUSIVE_AUDIO_CHANNELS == 1)
        channels->addItem( tr("Mono") );
    else
        channels->addItem( tr("Stereo") );
#endif
    layout->addRow( tr("Channels"), channels );

    sampleRate = new QComboBox();
#ifndef EXCLUSIVE_AUDIO_SETTING
    sampleRate->addItem( tr("8 kHz") );
    sampleRate->addItem( tr("11 kHz") );
    sampleRate->addItem( tr("22 kHz") );
    sampleRate->addItem( tr("44 kHz") );
#else
    QString str = QString("%1 kHz").arg(EXCLUSIVE_AUDIO_RATE/1000);
    sampleRate->addItem( str );
#endif
    layout->addRow( tr("Sample Rate"), sampleRate );

    format = new QComboBox();
    layout->addRow( tr("Format"), format );

    storageLocation = new QStorageDeviceSelector;
    QFileSystemFilter *fsf = new QFileSystemFilter;
    fsf->documents = QFileSystemFilter::Set;
    storageLocation->setFilter(fsf);
    layout->addRow( tr("Location"), storageLocation );

    // Load the default quality settings.
#ifndef EXCLUSIVE_AUDIO_SETTING
    int qual;
    for ( qual = 0; qual < MaxQualities; ++qual ) {
        qualities[qual].frequency = DefaultQualities[qual].frequency;
        qualities[qual].channels = DefaultQualities[qual].channels;
        qualities[qual].mimeType = DefaultQualities[qual].mimeType;
        qualities[qual].formatTag = DefaultQualities[qual].formatTag;
    }
#else
    qualities[0].frequency = EXCLUSIVE_AUDIO_RATE;
    qualities[0].channels = EXCLUSIVE_AUDIO_CHANNELS;
    qualities[0].mimeType = "audio/x-wav";
    qualities[0].formatTag = "pcm";
#endif

    // Load configuration overrides.
    loadConfig();

    // Populate the list of formats.
    if ( plugins != 0 ) {
        uint numPlugins = plugins->count();
        for ( uint plugin = 0; plugin < numPlugins; ++plugin ) {
            format->addItem( plugins->formatNameAt( plugin ) );
        }
    }

    // Hook up interesting signals.
    connect( presetQualities, SIGNAL(activated(int)), this, SLOT(setQuality(int)) );
    connect( channels, SIGNAL(activated(int)), this, SLOT(setChannels(int)) );
    connect( sampleRate, SIGNAL(activated(int)), this, SLOT(setSampleRate(int)) );
    connect( format, SIGNAL(activated(int)), this, SLOT(setFormat(int)) );

    // If preset quality is Custom, other fields are disabled
    channels->setEnabled( quality == CustomQuality );
    sampleRate->setEnabled( quality == CustomQuality );
    format->setEnabled( quality == CustomQuality );
}

ConfigureRecorder::~ConfigureRecorder()
{
}

static void copyQualities( QualitySetting *dest, QualitySetting *src, int num )
{
    while ( num > 0 ) {
        dest->frequency = src->frequency;
        dest->channels = src->channels;
        dest->mimeType = src->mimeType;
        dest->formatTag = src->formatTag;
        ++dest;
        ++src;
        --num;
    }
}

void ConfigureRecorder::processPopup()
{
    // Save the current quality settings, in case we have to cancel.
    QualitySetting savedQualities[MaxQualities];
    int savedQuality = quality;
    copyQualities( savedQualities, qualities, MaxQualities );
    QString location = storageLocation->documentPath();

    // Update the configuration dialog's display with the current state.
    setQuality( quality );

    // Process the dialog.
    if ( QtopiaApplication::execDialog( this ) != QDialog::Accepted) {
        // Copy the saved configuration back.
        copyQualities( qualities, savedQualities, MaxQualities );
        quality = savedQuality;
        storageLocation->setLocation( location );
    }
}

const QFileSystem* ConfigureRecorder::fileSystem()
{
    return storageLocation->fileSystem();
}

QString ConfigureRecorder::documentPath()
{
    return storageLocation->documentPath();
}

void ConfigureRecorder::setQuality( int index )
{
    // Set the quality value.
    quality = index;
    presetQualities->setCurrentIndex( quality );

    // Set the number of channels.
    channels->setCurrentIndex( qualities[quality].channels - 1 );

    // Set the sample rate frequency.
    switch ( qualities[quality].frequency ) {
#ifndef EXCLUSIVE_AUDIO_SETTING
        case 8000:  sampleRate->setCurrentIndex( 0 ); break;
        case 11025: sampleRate->setCurrentIndex( 1 ); break;
        case 22050: sampleRate->setCurrentIndex( 2 ); break;
        case 44100: sampleRate->setCurrentIndex( 3 ); break;
#else
        case EXCLUSIVE_AUDIO_RATE:  sampleRate->setCurrentIndex( 0 ); break;
#endif
    }

    // Set the format.
    int formatIndex = plugins->indexFromType( qualities[quality].mimeType, qualities[quality].formatTag );
    if( formatIndex >= 0 ) {
        format->setCurrentIndex( formatIndex );
    }

    channels->setEnabled( quality == CustomQuality );
    sampleRate->setEnabled( quality == CustomQuality );
    format->setEnabled( quality == CustomQuality );
}

void ConfigureRecorder::setChannels( int index )
{
    qualities[quality].channels = index + 1;
}

void ConfigureRecorder::setSampleRate( int index )
{
    int frequency;

    switch ( index ) {
#ifndef EXCLUSIVE_AUDIO_SETTING
        case 0:  frequency = 8000; break;
        case 1:  frequency = 11025; break;
        case 2:  frequency = 22050; break;
        default: frequency = 44100; break;
#else
        default:  frequency = EXCLUSIVE_AUDIO_RATE; break;
#endif
    }

    qualities[quality].frequency = frequency;
}

void ConfigureRecorder::setFormat( int index )
{
    qualities[quality].mimeType = plugins->at( (uint)index )->pluginMimeType();
    qualities[quality].formatTag = plugins->formatAt( (uint)index );
}

void ConfigureRecorder::loadConfig()
{
    QSettings cfg("Trolltech","MediaRecorder");

    cfg.beginGroup( "Options" );
    QString qvalue = cfg.value( "Quality" ).toString();
    if (qvalue == "") {
        quality = VoiceQuality;
    }

    QString l = cfg.value("Location").toString();
    if ( !l.isEmpty() )
        storageLocation->setLocation(l);
    else
        storageLocation->setLocation(QFileSystem::documentsFileSystem().documentsPath());

    for ( int qual = 0; qual < MaxQualities; ++qual ) {
        if ( qvalue == ConfigSections[qual] ) {
            quality = qual;
        }

        cfg.endGroup();

        cfg.beginGroup( ConfigSections[qual] );

        int value = cfg.value( "Channels" ).toInt();
        if ( value == 1 || value == 2 ) {
            qualities[qual].channels = value;
        }

        value = cfg.value( "Frequency" ).toInt();
        if ( value == 8000 || value == 11025 || value == 22050 || value == 44100 ) {
            qualities[qual].frequency = value;
        }

        QString svalue = cfg.value( "Type" ).toString();
        QString fvalue = cfg.value( "Format" ).toString();
        if ( svalue == QString() ) {
            svalue = qualities[qual].mimeType;
        }
        if ( fvalue == QString() ) {
            fvalue = qualities[qual].formatTag;
        }

        int index = plugins->indexFromType( svalue, fvalue );
        if( index >= 0 ) {
            qualities[qual].mimeType = plugins->at( index )->pluginMimeType();
            qualities[qual].formatTag = plugins->formatAt( index );
        } else {
            qualities[qual].mimeType = svalue;
            qualities[qual].formatTag = fvalue;
        }
    }
}

void ConfigureRecorder::saveConfig()
{
    // Write out only what is different from the defaults, which
    // makes it easier to migrate to new versions of the app
    // that change the defaults to something better.

    QSettings cfg("Trolltech","MediaRecorder");

    cfg.beginGroup( "Options" );
    if ( quality != VoiceQuality ) {
        cfg.setValue( "Quality", ConfigSections[quality] );
    } else {
        cfg.remove( "Quality" );
    }

    cfg.setValue( "Location", storageLocation->documentPath() );

    for ( int qual = 0; qual < MaxQualities; ++qual ) {
        cfg.endGroup();

        cfg.beginGroup( ConfigSections[qual] );
        cfg.remove("");

        if ( qualities[qual].channels != DefaultQualities[qual].channels ) {
            cfg.setValue( "Channels", qualities[qual].channels );
        }

        if ( qualities[qual].frequency != DefaultQualities[qual].frequency ) {
            cfg.setValue( "Frequency", qualities[qual].frequency );
        }

        if ( qualities[qual].mimeType != DefaultQualities[qual].mimeType ) {
            cfg.setValue( "Type", qualities[qual].mimeType );
        }

        if ( qualities[qual].formatTag != DefaultQualities[qual].formatTag ) {
            cfg.setValue( "Format", qualities[qual].formatTag );
        }
    }
}

