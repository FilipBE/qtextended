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
// Local includes
#include "qds_p.h"
#include "qdsactionrequest.h"
#include "qdsactionrequest_p.h"
#include "qdsserviceinfo.h"

// Qt includes
#include <QObject>
#include <QTimer>

// Qtopia includes
#include <QtopiaIpcEnvelope>
#include <QMimeType>
#include <qtopialog.h>

// ============================================================================
//
//  QDSActionRequestPrivate
//
// ============================================================================


QDSActionRequestPrivate::QDSActionRequestPrivate()
:   QObject(),
    mServiceInfo(),
    mRequestData(),
    mResponseData(),
    mAuxData(),
    mChannel(),
    mComplete(),
    mErrorMessage(),
    mHeartBeat()
{
}

QDSActionRequestPrivate::QDSActionRequestPrivate(
    const QDSActionRequestPrivate& other )
:   QObject(),
    mServiceInfo( other.mServiceInfo ),
    mRequestData( other.mRequestData ),
    mResponseData( other.mResponseData ),
    mAuxData( other.mAuxData ),
    mChannel( other.mChannel ),
    mComplete( other.mComplete ),
    mErrorMessage(),
    mHeartBeat( other.mChannel )
{
}

QDSActionRequestPrivate::QDSActionRequestPrivate(
    const QDSServiceInfo& serviceInfo,
    const QDSData& requestData,
    const QByteArray& auxiliary,
    const QString& channel )
:   QObject(),
    mServiceInfo( serviceInfo ),
    mRequestData( requestData ),
    mResponseData(),
    mAuxData( auxiliary ),
    mChannel( channel ),
    mComplete( false ),
    mErrorMessage(),
    mHeartBeat( channel )
{
}

QDSActionRequestPrivate::~QDSActionRequestPrivate()
{
}

void QDSActionRequestPrivate::emitResponse()
{
    if ( mResponseData.isValid() ) {
        QtopiaIpcEnvelope e( mChannel, "response(QDSData)" ); // No tr
        e << mResponseData;
    } else {
        QtopiaIpcEnvelope e( mChannel, "response()" ); // No tr
    }
}

void QDSActionRequestPrivate::emitError( const QString& error )
{
    QtopiaIpcEnvelope e( mChannel, "error(QString)" ); // No tr
    e << error;
}

// ============================================================================
//
//  QDSHeartBeat
//
// ============================================================================

QDSHeartBeat::QDSHeartBeat( QObject* parent )
:   QObject( parent ),
    mChannel(),
    mTimer( 0 )
{
}

QDSHeartBeat::QDSHeartBeat( const QString& channel, QObject* parent )
:   QObject( parent ),
    mChannel( channel ),
    mTimer( 0 )
{
    mTimer = new QTimer( this );
    connect( mTimer, SIGNAL(timeout()), this, SLOT(beat()) );
    mTimer->start( QDS::SERVERING_HEARTBEAT_PERIOD );
    beat();
}

QDSHeartBeat::QDSHeartBeat( const QDSHeartBeat& other )
:   QObject(),
    mTimer( 0 )
{
    this->operator=( other );
}

const QDSHeartBeat& QDSHeartBeat::operator=( const QDSHeartBeat& other )
{
    if (&other != this) {
        mChannel = other.mChannel;
        
        delete mTimer;
        if (other.mTimer != 0) {
            mTimer = new QTimer( this );
            connect( mTimer, SIGNAL(timeout()), this, SLOT(beat()) );
            mTimer->start( QDS::SERVERING_HEARTBEAT_PERIOD );
            beat();
        }
    }

    return *this;
}

void QDSHeartBeat::beat()
{
    QtopiaIpcEnvelope e( mChannel, "heartbeat()" ); // No tr
}

// ============================================================================
//
//  QDSActionRequest
//
// ============================================================================

/*!
    \class QDSActionRequest
    \inpublicgroup QtBaseModule

    \brief The QDSActionRequest class encapsulates a received action request.

    A Qt Extended Data Sharing (QDS) service provider can use the QDSActionRequest class
    to capture the context of received requests, and as an interface to respond to the request.

    Applications seeking to utilise QDS services should use the QDSAction class to
    request a service; the QDSAction class will then create a QDSActionRequest instance
    and send it to the QDS service for processing.

    \sa QDSAction, QDSServiceInfo, {Qt Extended Data Sharing (QDS)}

    \ingroup ipc
*/

/*!
    Constructs an empty request and attaches it to \a parent.
*/
QDSActionRequest::QDSActionRequest( QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    d = new QDSActionRequestPrivate();
}

/*!
    Constructs a deep copy of \a other.
*/
QDSActionRequest::QDSActionRequest( const QDSActionRequest& other )
:   QObject(),
    d( 0 )
{
    d = new QDSActionRequestPrivate( *(other.d) );
}

/*!
    Constructs an action request for a service with no request data. The service
    responding to the request is provided in \a serviceInfo and the channel
    for responding to the client is provided in \a channel. The request is
    attached to \a parent.
*/
QDSActionRequest::QDSActionRequest( const QDSServiceInfo& serviceInfo,
                                    const QString& channel,
                                    QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    d = new QDSActionRequestPrivate( serviceInfo, QDSData(), QByteArray(), channel );

    if ( !d->mServiceInfo.supportsRequestDataType( QMimeType(QString()) ) )
        respond( QString( tr( "request didn't contain data" ) ) );
}

/*!
    Constructs an action request for a service with \a requestData. The service
    responding to the request is provided in \a serviceInfo and the channel
    for responding to the client is provided in \a channel. \a auxiliary data
    can also be attached to the request. The request is attached to \a parent.
*/
QDSActionRequest::QDSActionRequest( const QDSServiceInfo& serviceInfo,
                                    const QDSData& requestData,
                                    const QString& channel,
                                    const QByteArray& auxiliary,
                                    QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    d = new QDSActionRequestPrivate( serviceInfo, requestData, auxiliary, channel );

    if ( !d->mServiceInfo.supportsRequestDataType( requestData.type() ) )
        respond( QString( tr( "request contained unexpected data" ) ) );
}

/*!
    Destructor
*/
QDSActionRequest::~QDSActionRequest()
{
    delete d;
    d = 0;
}

/*!
    Makes a deep copy of \a other and assigns it to this QDSActionRequest object.
    Returns a reference to this QDSActionRequest object.
*/
const QDSActionRequest& QDSActionRequest::operator=( const QDSActionRequest& other )
{
    d->mServiceInfo = other.serviceInfo();
    d->mRequestData = other.requestData();
    d->mResponseData = other.responseData();
    d->mAuxData = other.auxiliaryData();
    d->mChannel = other.d->mChannel;
    d->mComplete = other.isComplete();
    d->mHeartBeat = QDSHeartBeat( other.d->mChannel );

    return *this;
}

/*!
    Returns the description of the service requested.
*/
const QDSServiceInfo& QDSActionRequest::serviceInfo() const
{
    return d->mServiceInfo;
}

/*!
    Returns the validity of the request.
*/
bool QDSActionRequest::isValid() const
{
    if ( d->mServiceInfo.isValid() &&
         !d->mChannel.isEmpty() &&
         d->mErrorMessage.isEmpty() )
        return true;

    return false;
}

/*!
    Returns true if the request has been completely processed and a response
    has been sent to the client; otherwise returns false.
*/
bool QDSActionRequest::isComplete() const
{
    return d->mComplete;
}

/*!
    Returns the request data.
*/
const QDSData& QDSActionRequest::requestData() const
{
    return d->mRequestData;
}

/*!
    Returns the response data.
*/
const QDSData& QDSActionRequest::responseData() const
{
    return d->mResponseData;
}

/*!
    Returns the auxiliary data accompanying the request.
*/
const QByteArray& QDSActionRequest::auxiliaryData() const
{
    return d->mAuxData;
}

/*!
    Returns any error message generated during the request. 
    If no error has been reported a null string will be returned.
*/
QString QDSActionRequest::errorMessage() const
{
    return d->mErrorMessage;
}

/*!
    Sends a response back to the client to indicate that the request has
    been processed correctly. This method is to be used for services which
    don't have response data.

    Returns false if a response has already been sent or the service requires
    response data; otherwise returns true.
*/
bool QDSActionRequest::respond()
{
    if ( d->mComplete )
        return false;

    if ( !d->mServiceInfo.supportsResponseDataType() )
        return false;

    d->mComplete = true;
    d->emitResponse();

    return true;
}

/*!
    Sends \a responseData back to the client to indicate that the request has
    been processed correctly. This method is to be used for services which
    have response data.

    Returns false if a response has already been sent or the service doesn't require
    response data; otherwise returns true.
*/
bool QDSActionRequest::respond( const QDSData &responseData )
{
    if ( d->mComplete )
        return false;

    if ( !d->mServiceInfo.supportsResponseDataType( responseData.type() ) )
        return false;

    d->mResponseData = responseData;
    d->mComplete = true;
    d->emitResponse();

    return true;
}

/*!
    Sends the error message \a message back to the client to indicate that
    an error has occured.

    Returns false if a response has already been sent; otherwise returns true.
*/
bool QDSActionRequest::respond( const QString& message )
{
    if ( d->mComplete )
        return false;

    d->mComplete = true;
    d->mErrorMessage = message;
    d->emitError( message );

    return true;
}

/*!
    \fn void QDSActionRequest::deserialize(Stream &value)

    \internal

    Deserializes the QDSActionRequest instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSActionRequest::deserialize(Stream &stream)
{
    // Service info
    QDSServiceInfo info;
    stream >> info;
    d->mServiceInfo = info;

    // Request data
    bool validRequestData = false;
    stream >> validRequestData;
    if ( validRequestData ) {
        QDSData data;
        stream >> data;
        d->mRequestData = data;
    } else {
        d->mRequestData = QDSData();
    }

    // Response Data
    bool validResponseData = false;
    stream >> validResponseData;
    if ( validResponseData ) {
        QDSData data;
        stream >> data;
        d->mResponseData = data;
    } else {
        d->mResponseData = QDSData();
    }

    // Auxiliary data
    QByteArray auxData;
    stream >> auxData;
    d->mAuxData = auxData;

    // Now channel, complete and error message info
    QString channel;
    stream >> channel;
    d->mChannel = channel;

    bool complete = false;
    stream >> complete;
    d->mComplete = complete;

    QString errorMsg;
    stream >> errorMsg;
    d->mErrorMessage = errorMsg;
}

/*!
    \fn void QDSActionRequest::serialize(Stream &value) const

    \internal

    Serializes the QDSActionRequest instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSActionRequest::serialize(Stream &stream) const
{
    // Service info
    stream << serviceInfo();

    // Request data
    if ( requestData().isValid() ) {
        stream << true;
        stream << requestData();
    } else {
        stream << false;
    }

    // Response data
    if ( responseData().isValid() ) {
        stream << true;
        stream << responseData();
    } else {
        stream << false;
    }

    // Auxiliary data
    stream << auxiliaryData();

    // Add channel, complete and error message info
    stream << d->mChannel;
    stream << isComplete();
    stream << errorMessage();
}

// Macros
Q_IMPLEMENT_USER_METATYPE(QDSActionRequest);
