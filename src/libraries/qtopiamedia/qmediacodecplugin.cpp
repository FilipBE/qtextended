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

#include "qmediacodecplugin.h"


/*!
    \class QMediaCodecPlugin
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaCodecPlugin class represents a codec factory in the
    Media Engine.

    \sa QMediaDecoder, QMediaDevice
*/


/*!
    Destroy the QMediaCodecPlugin object
*/

QMediaCodecPlugin::~QMediaCodecPlugin()
{
}

/*!
    \fn QString QMediaCodecPlugin::name() const;

    Return the name of the Codec package.
*/

/*!
    \fn QString QMediaCodecPlugin::comment() const;

    Return a general comment about this Codec package, there is no specified
    format.
*/

/*!
    \fn QStringList QMediaCodecPlugin::mimeTypes() const;

    Return a QStringList of mime-types that are supported by this Codec
    package.
*/

/*!
    \fn QStringList QMediaCodecPlugin::fileExtensions() const;

    Return a QStringList of the file extensions that are known to this
    Codec package.
*/

/*!
    \fn double QMediaCodecPlugin::version() const;

    Return version information for this Codec package, it is specific to the
    implmentation.
*/

/*!
    \fn bool QMediaCodecPlugin::canEncode() const;

    Return an indication of whether this Codec package supports the encoding
    of data.
*/

/*!
    \fn bool QMediaCodecPlugin::canDecode() const;

    Return an indication of whether this Codec package supports the decoding
    of data.
*/

/*!
    \fn QMediaEncoder* QMediaCodecPlugin::encoder(QString const& mimeType);

    Return a QMediaEncoder suitable for the specified \a mimeType.
*/

/*!
    \fn QMediaDecoder* QMediaCodecPlugin::decoder(QString const& mimeType);

    Return a QMediaDecoder suitable for the specified \a mimeType.
*/

