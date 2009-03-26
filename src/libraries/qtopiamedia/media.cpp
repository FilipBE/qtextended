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

#include "media.h"


/*!
    \class QtopiaMedia
    \inpublicgroup QtMediaModule

    \brief The QtopiaMedia namespace provides a container for general media types.

    The QtopiaMedia namespace defines two enumerations to deal with state
    information and seek offsets.

    \sa QtopiaMedia::State, QtopiaMedia::Offset
*/

/*!
    \enum QtopiaMedia::State

    This enum specifies the state of a media content.

    \value Playing The Media is currently playing.
    \value Paused The Media is currently paused.
    \value Stopped The Media is currently stopped.
    \value Buffering The Media is being buffered before playback.
    \value Error An error has occurred with the media
*/

/*!
    \enum QtopiaMedia::Offset

    This enum specifies the offset from which a seek operation should occur.

    \value Beginning The Beginning of the content.
    \value Current The current position in Playing or Paused content.
    \value End The end of the content.
*/


Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaMedia::State);
Q_IMPLEMENT_USER_METATYPE_ENUM(QtopiaMedia::Offset);
