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

#include <qsimgenericaccess.h>
#include <quuid.h>

/*!
    \class QSimGenericAccess
    \inpublicgroup QtTelephonyModule

    \brief The QSimGenericAccess class provides an interface to send commands directly to a SIM.
    \ingroup telephony

    Use of this class usually results in an \c{AT+CSIM} command being sent to the modem.
    The commands must be in the format described in 3GPP TS 51.011.

    Sending generic access commands to a SIM bypasses the normal
    mechanisms that are used to access SIM files, SIM application toolkit,
    operator data, etc.  This class should therefore be used only for
    commands that are not already covered by other classes such as
    QSimFiles, QBinarySimFile, QRecordBasedSimFile, and QSimToolkit.

    \sa QSimFiles, QBinarySimFile, QRecordBasedSimFile, QSimToolkit
*/

/*!
    Construct a new SIM generic access object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SIM generic access.  If there is more
    than one service that supports SIM generic access, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QSimGenericAccess objects for each.

    \sa QCommServiceManager::supports()
*/
QSimGenericAccess::QSimGenericAccess
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QSimGenericAccess", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this generic SIM access object.
*/
QSimGenericAccess::~QSimGenericAccess()
{
}

/*!
    Sends \a data to the SIM as a generic access command and returns a
    unique request identifier for it.  The command has the format described
    in 3GPP TS 51.011.  The telephony service will emit the response() signal
    once the command has completed.

    \sa response()
*/
QString QSimGenericAccess::command( const QByteArray& data )
{
    QString reqid = QUuid::createUuid().toString();
    command( reqid, data );
    return reqid;
}

/*!
    Sends \a data to the SIM as a generic access command.  The \a reqid
    will be returned via the response() signal to help match up commands
    and responses.  The command has the format described in 3GPP TS 51.011.

    \sa response()
*/
void QSimGenericAccess::command( const QString& reqid, const QByteArray& data )
{
    invoke( SLOT(command(QString,QByteArray)), reqid, data );
}

/*!
    \fn void QSimGenericAccess::response( const QString& reqid, QTelephony::Result result, const QByteArray& data )

    Signal that is emitted to report the \a result of issuing the request
    \a reqid via command().  If \a result is QTelephony::Ok, then \a data
    is the data returned by the SIM in response to the command.  The response
    has the format described in 3GPP TS 51.011.

    \sa command()
*/
