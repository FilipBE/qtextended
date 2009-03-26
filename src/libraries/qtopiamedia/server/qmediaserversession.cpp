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

#include "qmediaserversession.h"


/*!
    \class QMediaServerSession
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaServerSession class is used to represent and control a
    session registered in the Media Server.

    \sa QMediaEngine
*/

/*!
    Destroy a QMediaServerSession object.
*/

QMediaServerSession::~QMediaServerSession()
{
}

/*!
    \fn void QMediaServerSession::start()

    This function is called when the user is trying to start the session. It
    may have different meanings depending on the nature of the session.
    Generally the content must become active or playing, on start().
*/

/*!
    \fn void QMediaServerSession::pause()

    This function is called when the user is trying to pause the session. For
    some session types this may mean pausing decoding content, for others
    simply muting data.
*/

/*!
    \fn void QMediaServerSession::stop()

    This function is called as a result of the user trying to stop the session.
    It is similar to pause() excepting that the any position state, if
    applicable, should be reset.
*/

/*!
    \fn void QMediaServerSession::suspend()

    The session may be asked to suspend its activities, at this point it should
    keep a record of state of the session, but otherwise release resources used
    by the session.
*/

/*!
    \fn void QMediaServerSession::resume()

    After a suspend, resume() is called to return the session to its previous
    state.
*/

/*!
    \fn void QMediaServerSession::seek(quint32 ms)

    Seek the content to \a ms milliseconds from the beginning. If the session
    is not seekable it may ignore this function.
*/

/*!
    \fn quint32 QMediaServerSession::length()

    Return the length of the content.
*/

/*!
    \fn void QMediaServerSession::setVolume(int volume)

    Set the volume of the data to \a volume. This may involve adjusting device
    volume, or adjusting PCM volume, it is dependent on the nature of the
    session.
*/

/*!
    \fn int QMediaServerSession::volume() const

    Return the current volume.
*/

/*!
    \fn void QMediaServerSession::setMuted(bool mute)

    When \a mute is true the volume on the session should be muted, when false,
    the previously set volume should be restored.
*/

/*!
    \fn bool QMediaServerSession::isMuted() const

    Return the current mute state of the session.
*/

/*!
    \fn QtopiaMedia::State QMediaServerSession::playerState() const

    Return the current state of the session.
*/

/*!
    \fn QString QMediaServerSession::errorString()

    If the session is in the error state, a generic error string may be
    provided through this function.
*/

/*!
    \fn void QMediaServerSession::setDomain(QString const& domain)

    Sets the audio domain in which this session is active to \a domain.
*/

/*!
    \fn QString QMediaServerSession::domain() const

    Returns the audio domain for this session.
*/

/*!
    \fn QStringList QMediaServerSession::interfaces()

    This function should return the names of the interfaces supported by this
    session.
*/

/*!
    \fn QString QMediaServerSession::id() const

    This function should return the Id of the session.
*/

/*!
    \fn QString QMediaServerSession::reportData() const

    This is a general reporting mechanisim, the QString from this function will
    be placed in the Value Space for inspection.
*/

//signals:
/*!
    \fn void QMediaServerSession::playerStateChanged(QtopiaMedia::State state)

    Inform listeners that the current state of the session has changed. The
    new state is indicated by \a state.
*/

/*!
    \fn void QMediaServerSession::positionChanged(quint32 ms)

    Inform listeners that the current play position in the content has changed
    to \a ms, from the beginning.
*/

/*!
    \fn void QMediaServerSession::lengthChanged(quint32 ms)

    Inform listeners the length of the content is changed to \a ms milliseconds
    long.
*/

/*!
    \fn void QMediaServerSession::volumeChanged(int volume)

    Inform listeners the volume level has changed to \a volume.
*/

/*!
    \fn void QMediaServerSession::volumeMuted(bool muted)

    Inform listeners that the mute state of the session has changed to \a muted.
*/

/*!
    \fn void QMediaServerSession::interfaceAvailable(const QString& name)

    Inform listeners that an interface has been made available. The interface
    can be referenced in the media system by \a name.
*/

/*!
    \fn void QMediaServerSession::interfaceUnavailable(const QString& name)

    Inform listeners that an interface is no longer available for use. The
    interfaces is referenced in the media system by \a name

*/
