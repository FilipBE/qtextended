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
#include <qmediartpsessionengine.h>

/*!
    \class QMediaRtpEngine
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpEngine class provides an interface for implementing backends to QMediaRtpSession.

    Instances of QMediaRtpEngine are provided by media engines to construct and manage RTP streams.

    New instance of QMediaRtpEngine are created using the QMediaRtpEngineFactory plug-in interface.

    \sa QMediaRtpSession, QMediaRtpEngineFactory
*/

/*!
    Destroys a QMediaRtpEngine.
*/
QMediaRtpEngine::~QMediaRtpEngine()
{
}

/*!
    \fn QMediaRtpEngine::supportedInboundPayloads(QMediaRtpStream::Type type)

    Returns a list of \a type payloads a session is capable of receiving.
*/

/*!
    \fn QMediaRtpEngine::supportedOutboundPayloads(QMediaRtpStream::Type type)

    Returns a list of \a type payloads a session is capable of sending.
*/

/*!
    \fn QMediaRtpEngine::streamCount() const

    Returns the numbe of streams in the session.
*/

/*!
    \fn QMediaRtpEngine::stream(int index) const

    Returns the stream at \a index.
*/

/*!
    \fn QMediaRtpEngine::addStream(QMediaRtpStream::Type type, QMediaRtpStream::Direction direction)

    Adds a new \a type stream to the session which sends and/or receives data according to
    \a direction.

    Returns the new stream, or a null pointer if the stream could not be created.
*/

/*!
    \fn QMediaRtpEngine::removeStream(QMediaRtpStream *stream)

    Removes a \a stream from a session.

*/

/*!
    \class QMediaRtpEngineFactory
    \inpublicgroup QtMediaModule
    \brief The QMediaRtpEngineFactory class provides a plug-in interface for creating instances of QMediaRtpEngine.

    Media engine plug-ins that provide RTP streaming support through QMediaRtpSession implement the
    QMediaRtpEngineFactory plug-in interface to create instances of QMediaRtpEngine which provide
    stream creation and management support.
*/

/*!
    Destroys a QMediaRtpEngineFactory.
*/
QMediaRtpEngineFactory::~QMediaRtpEngineFactory()
{
}

/*!
    \fn QMediaRtpEngineFactory::createRtpEngine()

    Returns a new instance of QMediaRtpEngine.
*/
