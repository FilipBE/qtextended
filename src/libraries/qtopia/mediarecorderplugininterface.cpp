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

#include <mediarecorderplugininterface.h>

/*!
  \internal
  \class MediaRecorderEncoder
    \inpublicgroup QtBaseModule

  \brief The MediaRecorderEncoder class provides an abstract base class for
  Qt Extended MediaRecorder encoder plug-ins.

  Writing an encoder plug-in is achieved by subclassing this base class,
  reimplementing the pure virtual functions and exporting the class with
  the \c QTOPIA_EXPORT_PLUGIN macro. See the  \l {How to Create Qt Plugins} documentation for details.

  The functions in this class are typically used in the following order
  when recording audio:

  \code
  begin(device, tag);
  addComment("name", "value");
  ...
  addComment("name", "value");
  setAudioChannels(2);
  setAudioFrequency(44100);
  writeAudioSamples(samples, numSamples);
  ...
  writeAudioSamples(samples, numSamples);
  end();
  \endcode

  \sa QIODevice, MediaRecorderPluginInterface, begin()

  \ingroup multimedia
*/

/*!
  \fn MediaRecorderEncoder::~MediaRecorderEncoder()

  Destructs a MediaRecorderEncoder.
*/

/*!
  \fn int MediaRecorderEncoder::pluginNumFormats() const

  Returns the number of formats that are supported by this plugin.
*/

/*!
  \fn QString MediaRecorderEncoder::pluginFormatName( int format ) const

  Returns the name of one of a plugin's data format, e.g. "Wav Format".
  The data format is selected using \a format as an index.  The name may be displayed
  to the user in a list of supported recording formats.
*/

/*!
  \fn QString MediaRecorderEncoder::pluginFormatTag( int format ) const

  Returns the tag name of a plugin's data format.  For example, "pcm".
  The data format is selected using \a format as an index.  This value is used with \l MediaRecorderEncoder::begin .
*/

/*!
  \fn QString MediaRecorderEncoder::pluginComment() const

  Returns a comment that describes the purpose of the plugin.
*/

/*!
  \fn double MediaRecorderEncoder::pluginVersion() const

  Returns the version of the plugin.  Normally 1.0.
*/

//  \l {MediaRecorderEncoder::pluginExtension() file extension} .
/*!
  \fn QString MediaRecorderEncoder::pluginMimeType() const

  Returns the MIME type for the plugin's recommended file extension, e.g.
  \c audio/x-wav
*/

/*!
  \fn bool MediaRecorderEncoder::begin( QIODevice *device, const QString& formatTag )

  Begin recording on the specified output \a device, which must be
  capable of direct access (seeking) if
  \l MediaRecorderEncoder::requiresDirectAccess() returns
  true. \a formatTag selects which format to use.

  Returns true if recording has begun.  Returns false if recording
  is already in progress or if \a device is not capable of seeking.

  This call will typically be followed by calls to set the
  \l {setAudioChannels() channels} ,
  \l {setAudioFrequency() frequency} ,
  and \l {addComment() file comments} .

  \sa setAudioChannels(), setAudioFrequency(), addComment(),
  writeAudioSamples(), requiresDirectAccess()
*/

/*!
  \fn bool MediaRecorderEncoder::end()

  End recording on the current output device.  This function may
  back-patch earlier bytes in the output.  Once it has finished
  outputting the data, it will leave the device positioned after
  all bytes that were written.

  Returns true if recording was successfully terminated.
  Returns false if not currently recording, or there was an
  error writing to the device.
*/

/*!
  \fn bool MediaRecorderEncoder::isActive() const

  Returns true if the recorder is currently active; false otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::setAudioChannels( int channels )

  Sets the number of audio channels in the recorded data to either 1 or 2.

  Returns true if the channel count was set successfully.  Returns false
  if not recording, the data header has already been written, or if
  \a channels is neither 1 nor 2.

  The data header is considered written upon the first call to
  \l {writeAudioSamples() write audio samples} .

  \sa begin(), setAudioFrequency(), writeAudioSamples()
*/

/*!
  \fn bool MediaRecorderEncoder::setAudioFrequency( int frequency )

  Sets the audio sample frequency in the recorded data.

  Returns true if the frequency was set successfuly.  Returns false
  if not recording, the data header has already been written, or if
  \a frequency is less than or equal to zero.

  The data header is considered written upon the first call to
  \l {writeAudioSamples() write audio samples} .

  \sa begin(), setAudioChannels(), writeAudioSamples()
*/

/*!
  \fn bool MediaRecorderEncoder::writeAudioSamples( const short *samples, long numSamples )

  Writes a buffer of audio samples to the recorded output.

  Samples are assumed to always be 16-bit and in host byte order.
  It is the responsibility of the caller to rescale other sample sizes.

  The \a numSamples value is the number of 16-bit quantities in the
  \a samples buffer.  This will be a multiple of two for stereo data,
  with alternating channel samples.

  Returns true if the samples were successfully written.  Returns false
  if not recording or there was an error writing to the output device.

  \sa begin(), setAudioChannels(), setAudioFrequency()
*/

/*!
  \fn bool MediaRecorderEncoder::addComment( const QString& tag, const QString& contents )

  Adds a comment string \a contents tagged as \a tag to the recorded output.  The plug-in may
  ignore tags that it doesn't understand.

  Returns true if the comment was successfully added (or ignored).
  Returns false if not recording or the data header has already been
  written.

  The data header is considered written upon the first call to
  \l {writeAudioSamples() write audio samples} .  This is true
  even if plugin's data format places comments at the end of the stream,
  rather than the front.  The plug-in should cache the comments until
  it is ready to output them.

  This should only be called if \l {supportsComments() comments} are supported by the plugin.

  \sa begin(), writeAudioSamples(), supportsComments()
*/

/*!
  \fn long MediaRecorderEncoder::estimateAudioBps( int frequency, int channels, const QString& formatTag )

  Estimate the number of bytes per second that are needed to record
  audio in the \a formatTag format at a given \a frequency with the
  specified number of \a channels.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsAudio() const

  Returns true if this plug-in supports audio; false otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsVideo() const

  Returns true if this plug-in supports video; false otherwise.
*/

/*!
  \fn bool MediaRecorderEncoder::supportsComments() const

  Returns true if this plug-in supports comments; false otherwise.

  \sa addComment()
*/

/*!
  \fn bool MediaRecorderEncoder::requiresDirectAccess() const

  Returns true if this plug-in must be supplied a direct access (seekable)
  output device.

  \sa begin()
*/

/*!
  \internal
  \class MediaRecorderCodecFactoryInterface
    \inpublicgroup QtBaseModule

  \brief The MediaRecorderCodecFactoryInterface class provides a method of obtaining a multimedia encoder.

  The abstract  MediaRecorderCodecFactoryInterface class allows applications
  to obtain an encoder to record multimedia data to a file or other
  output device.

  \sa MediaRecorderEncoder
*/

/*!
  \fn MediaRecorderEncoder *MediaRecorderCodecFactoryInterface::encoder()

  Creates and returns a new plug-in encoder instance.  It is the
  responsibility of the caller to delete the instance before unloading
  the plug-in interface library.
*/

/*!
    \class MediaRecorderCodecPlugin
    \inpublicgroup QtBaseModule
    \internal
*/

MediaRecorderCodecPlugin::MediaRecorderCodecPlugin( QObject* parent )
    : QObject( parent )
{ }

MediaRecorderCodecPlugin::~MediaRecorderCodecPlugin()
{ }
