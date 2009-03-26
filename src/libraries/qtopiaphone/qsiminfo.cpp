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

#include <qsiminfo.h>

/*!
    \class QSimInfo
    \inpublicgroup QtTelephonyModule

    \brief The QSimInfo class provides information about a SIM's identity.
    \ingroup telephony

    The identity of the SIM can be retrieved with the identity() method.
    If the identity is not yet known, identity() will return an empty string.
    The inserted() and removed() signals can be used to track SIM's as they
    are inserted and removed.

    At system start up and just after a modem reset, the SIM identity may not
    be available immediately even if a SIM is physically present in the modem.
    Client applications should monitor the inserted() signal to be notified
    when the SIM identity has been positively determined.

    If the modem eventually determines that there is no SIM present at all,
    the notInserted() signal will be emitted.  Note that some modems cannot
    tell the difference between a missing SIM and a SIM which is not yet
    ready for use, so the notInserted() signal may not be reliable.  It is
    the responsibility of modem vendor plug-ins to correctly implement
    the notInserted() feature.
*/

/*!
    Construct a new SIM information object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SIM information.  If there is more
    than one service that supports SIM information, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSimInfo objects for each.

    \sa QCommServiceManager::supports()
*/
QSimInfo::QSimInfo( const QString& service, QObject *parent,
                    QCommInterface::Mode mode )
    : QCommInterface( "QSimInfo", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SIM information object.
*/
QSimInfo::~QSimInfo()
{
}

/*!
    \property QSimInfo::identity
    \brief the identity of the SIM.
*/

/*!
    Returns the identity of the SIM.  Returns an empty string if the SIM
    has not been inserted, or the modem is currently being initialized
    and the SIM identity is not available yet.

    \sa insertedTime(), inserted(), removed()
*/
QString QSimInfo::identity() const
{
    return value( "identity" ).toString();
}

/*!
    \property QSimInfo::insertedTime
    \brief the date and time of the last time the SIM was detected as inserted.
*/

/*!
    Returns the date and time of the last time the SIM was detected
    as inserted.  This can be useful to determine if a phone book
    cache is up to date or if it needs to be reloaded because the
    contents of the SIM could have changed.  Returns a null
    QDateTime if there is no SIM inserted, or the SIM has not
    yet been detected.  The time returned is obtained from
    QDateTime::currentDateTime().

    \sa identity(), inserted(), removed()
*/
QDateTime QSimInfo::insertedTime() const
{
    return value( "insertedTime" ).toDateTime();
}

/*!
    \fn void QSimInfo::inserted()

    Signal that is emitted when a new SIM is inserted into the phone.
    The identity() function will return the identity of the SIM.

    \sa identity(), insertedTime(), removed()
*/

/*!
    \fn void QSimInfo::removed()

    Signal that is emitted when a SIM is removed from the phone.
    The identity() function will return an empty string.

    \sa identity(), insertedTime(), inserted()
*/

/*!
    \fn void QSimInfo::notInserted()

    Signal that is emitted when the modem detects that there is no
    SIM inserted because the \c{AT+CIMI} command returned an error
    return code of 10.  An inserted() signal may be received after
    this signal if a SIM is inserted later.

    Note that it is possible for identity() to return an empty string
    during initialization even if there is a SIM inserted, so it is
    not a reliable indicator of a missing SIM.  The notInserted()
    signal is more reliable.

    \sa identity(), inserted()
*/

/*!
    Sets the SIM \a identity and emit inserted() or removed() signals
    as appropriate.  This function is intended for use by server-side
    implementations of the QSimInfo interface.

    \sa identity(), inserted(), removed()
*/
void QSimInfo::setIdentity( const QString& identity )
{
    if ( identity != value( "identity" ).toString() ) {
        setValue( "identity", identity, Delayed );
        if ( identity.isEmpty() ) {
            setValue( "insertedTime", QDateTime() );
            emit removed();
        } else {
            setValue( "insertedTime", QDateTime::currentDateTime() );
            emit inserted();
        }
    }
}
