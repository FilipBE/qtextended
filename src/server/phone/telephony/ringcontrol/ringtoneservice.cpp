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

#include "ringtoneservice.h"

/*!
    \service RingtoneService Ringtone
    \inpublicgroup QtTelephonyModule
    \brief The RingtoneService class provides the Ringtone service.

    The \i Ringtone service enables applications to request ringtones playback to 
    be started or stopped.
*/

/*!
    \fn RingtoneService::RingtoneService( QObject *parent )
    \internal
*/

/*!
    \internal
*/
RingtoneService::~RingtoneService()
{
}

/*!
    \fn void RingtoneService::startMessageRingtone()

    Requests that the ringtone associated with message arrival should start playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::startMessageRingtone()}.
*/

/*!
    \fn void RingtoneService::stopMessageRingtone()

    Requests that the ringtone associated with message arrival should stop playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::stopMessageRingtone()}.
*/

/*!
    \fn void RingtoneService::startRingtone( const QString& fileName )

    Requests that the ringtone specified by \a fileName should start playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::startRingtone(QString)}.
*/

/*!
    \fn void RingtoneService::stopRingtone( const QString& fileName )

    Requests that the ringtone specified by \a fileName should stop playing.

    This slot corresponds to the QCop service message
    \c{Ringtone::stopRingtone(QString)}.
*/

/*!
  \fn void RingtoneService::muteRing()

  Requests that the currently playing ringtone stops, but vibration will continue if it was already in
  progress. Does nothing if no ring is in progress.
  */
