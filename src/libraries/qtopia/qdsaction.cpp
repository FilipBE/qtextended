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
#include "qdsaction.h"
#include "qdsaction_p.h"
#include "qdsactionrequest.h"
#include <QtopiaApplication>

// Qt includes
#include <QTimer>

// Qtopia includes
#include <QtopiaApplication>
#include <QtopiaServiceRequest>
#include <qtopialog.h>

#include <sys/types.h>
#include <unistd.h>

// ============================================================================
//
//  QDSActionPrivate
//
// ============================================================================

// Created with uuidgen
QDSActionPrivate::QDSActionPrivate()
:   QObject(),
    mId(),
    mServiceInfo(),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
}

QDSActionPrivate::QDSActionPrivate( const QDSActionPrivate& other )
:   QObject(),
    mId( other.mId ),
    mServiceInfo( other.mServiceInfo ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData( other.mResponseData ),
    mErrorMsg( other.mErrorMsg ),
    mResponseCode( QDSAction::Invalid )
{
}

QDSActionPrivate::QDSActionPrivate( const QString& name,
                                    const QString& service )
:   QObject(),
    mId(),
    mServiceInfo( name, service ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
    mId = QUniqueIdGenerator::createTemporaryId();
}

QDSActionPrivate::QDSActionPrivate( const QDSServiceInfo& serviceInfo )
:   QObject(),
    mId(),
    mServiceInfo( serviceInfo ),
    mResponseChannel( 0 ),
    mTimer( 0 ),
    mEventLoop( 0 ),
    mResponseData(),
    mErrorMsg(),
    mResponseCode( QDSAction::Invalid )
{
    mId = QUniqueIdGenerator::createTemporaryId();
}

QDSActionPrivate::~QDSActionPrivate()
{
    disconnectFromChannel();
}

bool QDSActionPrivate::requestActive()
{
    if ( ( mTimer != 0 ) && ( mTimer->isActive() ) )
        return true;

    return false;
}

void QDSActionPrivate::emitRequest()
{
    connectToChannel();
    startTimer();

    QtopiaServiceRequest serviceRequest(
        mServiceInfo.serviceId(),
        mServiceInfo.name() + "(QDSActionRequest)" ); // No tr

    serviceRequest << QDSActionRequest( mServiceInfo, responseChannel() );
    serviceRequest.send();
}

void QDSActionPrivate::emitRequest( const QDSData& requestData,
                                    const QByteArray& auxiliary )
{
    connectToChannel();
    startTimer();

    QtopiaServiceRequest serviceRequest(
        mServiceInfo.serviceId(),
        mServiceInfo.name() + "(QDSActionRequest)" ); // No tr

    serviceRequest << QDSActionRequest( mServiceInfo,
                                        requestData,
                                        responseChannel(),
                                        auxiliary );
    serviceRequest.send();
}

void QDSActionPrivate::heartbeatSlot()
{
    mTimer->stop();
    mTimer->start( QDS::REQUEST_TIMEOUT );
}

void QDSActionPrivate::responseSlot()
{
    mTimer->stop();

    emit response( mId );

    mResponseCode = QDSAction::Complete;
    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::responseSlot( const QDSData& responseData )
{
    mTimer->stop();

    mResponseData = responseData;
    emit response( mId, responseData );

    mResponseCode = QDSAction::CompleteData;
    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::requestTimeoutSlot()
{
    mTimer->stop();

    mResponseCode = QDSAction::Error;
    mErrorMsg = tr( "timeout" );
    emit error( mId, mErrorMsg );

    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::errorSlot( const QString& message )
{
    mTimer->stop();

    mResponseCode = QDSAction::Error;
    mErrorMsg = message;
    emit error( mId, message );

    if ( mEventLoop != 0 )
        mEventLoop->exit();
}

void QDSActionPrivate::connectToChannel()
{
    // Connect slots to response messages
    if ( mResponseChannel == 0 ) {
        mResponseChannel = new QtopiaIpcAdaptor( responseChannel() );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( heartbeat() ),
            this,
            SLOT(heartbeatSlot()),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( response() ),
            this,
            SLOT(responseSlot()),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( response( const QDSData& ) ),
            this,
            SLOT(responseSlot(QDSData)),
            QtopiaIpcAdaptor::SenderIsChannel );

        QtopiaIpcAdaptor::connect(
            mResponseChannel,
            MESSAGE( error( const QString& ) ),
            this,
            SLOT(errorSlot(QString)),
            QtopiaIpcAdaptor::SenderIsChannel );
    }
}

void QDSActionPrivate::disconnectFromChannel()
{
    delete mResponseChannel;
    mResponseChannel = 0;
}

void QDSActionPrivate::startTimer()
{
    if ( mTimer == 0 ) {
        mTimer = new QTimer( this );
        mTimer->setSingleShot( true );
        connect( mTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(requestTimeoutSlot()) );
    }

    mTimer->start( QDS::REQUEST_TIMEOUT );
}

QString QDSActionPrivate::responseChannel()
{
    QString channel = "QPE/QDSResponse/";
    channel += QString::number(mId.toUInt()) + ":" + QString::number(::getpid());

    return channel;
}

void QDSActionPrivate::reset()
{
    mErrorMsg = QString();
    mResponseCode = QDSAction::Invalid;
    mResponseData = QDSData();
    mEventLoop = 0;

    if ( mTimer != 0 )
        mTimer->stop();
}

void QDSActionPrivate::connectToAction( QDSAction* action )
{
    connect( this,
             SIGNAL(response(QUniqueId)),
             action,
             SIGNAL(response(QUniqueId)) );

    connect( this,
             SIGNAL(response(QUniqueId,QDSData)),
             action,
             SIGNAL(response(QUniqueId,QDSData)) );

    connect( this,
             SIGNAL(error(QUniqueId,QString)),
             action,
             SIGNAL(error(QUniqueId,QString)) );
}

// ============================================================================
//
//  QDSAction
//
// ============================================================================

/*!
    \class QDSAction
    \inpublicgroup QtBaseModule

    \brief The QDSAction class provides an interface for requesting Qt Extended Data Sharing (QDS)
    services.

    Applications can use the QDSAction class to make a request for a QDS service.
    The request can be made either synchronously (using QDSAction::exec())
    or asynchronously (using QDSAction::invoke()).

    \sa QDSServiceInfo, {Qt Extended Data Sharing (QDS)}

    \ingroup ipc
*/

/*!
    \fn void QDSAction::response( const QUniqueId& actionId )

    This signal is emitted when a response is received from the service provider
    for the action identified by \a actionId.
*/

/*!
    \fn void QDSAction::response( const QUniqueId& actionId, const QDSData&
    responseData )

    This signal is emitted when a response is received from the service provider
    which contains the response data \a responseData for the action identified by
    \a actionId.
*/

/*!
    \fn void QDSAction::error( const QUniqueId& actionId, const QString& message )

    This signal is emitted when an error message \a message is received for the
    action identified by \a actionId.
*/

/*!
    \enum QDSAction::ResponseCode
    This enum describes response codes for synchronous requests.

    \value Invalid Response code has not been set.
    \value Complete The request was processed correctly.
    \value CompleteData The request was processed correctly, and response data was
    received.
    \value Error An error occured, use QDSAction::errorMessage() to view the
           error message.
*/

/*!
    Constructs an empty QDSAction object and attaches it to \a parent.
*/
QDSAction::QDSAction( QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate();
    d->connectToAction( this );
}

/*!
    Constructs a deep copy of \a other.
*/
QDSAction::QDSAction( const QDSAction& other )
:   QObject(),
    d( 0 )
{
    // Create d pointer and copy members
    d = new QDSActionPrivate( *( other.d ) );
    d->connectToAction( this );
}

/*!
    Constructs a QDSAction object for the QDS service \a name and the Qt Extended service
    \a service. The action is attached to \a parent.
*/
QDSAction::QDSAction( const QString& name,
                      const QString& service,
                      QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate( name, service );
    d->connectToAction( this );
}

/*!
    Constructs a QDSAction object for the service described in \a serviceInfo and attaches to
    \a parent.
*/
QDSAction::QDSAction( const QDSServiceInfo& serviceInfo, QObject* parent )
:   QObject( parent ),
    d( 0 )
{
    // Create d pointer
    d = new QDSActionPrivate( serviceInfo );
    d->connectToAction( this );
}

/*!
    Destroys the action.
*/
QDSAction::~QDSAction()
{
    if ( d->requestActive() )
        qLog(DataSharing) << "QDSAction object destroyed while request active";

    delete d;
}

/*!
    Makes a deep copy of \a other and assigns it to this QDSAction object.
    Returns a reference to this QDSAction object.
*/
const QDSAction& QDSAction::operator=( const QDSAction& other )
{
    d->mId = other.id();
    d->mServiceInfo = other.serviceInfo();
    d->mResponseData = other.responseData();
    d->mErrorMsg = other.errorMessage();
    d->mResponseCode = other.d->mResponseCode;

    // Don't copy mResponseChannel, mTimer or mEventLoop as they are
    // specific to an instance/operation

    return *this;
}

/*!
    Returns true if the QDSAction object represents a valid QDS service; otherwise returns false.
    The requirements of a valid QDS service are discussed in QDSServiceInfo.
*/
bool QDSAction::isValid() const
{
    return d->mServiceInfo.isValid();
}

/*!
    Returns true if the QDSAction object represents an available QDS service; otherwise returns false.
    The requirements of an available QDS service are discussed in QDSServiceInfo.
*/
bool QDSAction::isAvailable() const
{
    return d->mServiceInfo.isAvailable();
}

/*!
    Returns true if the request is still being processed by the QDS service
    provider; otherwise returns false.
*/
bool QDSAction::isActive() const
{
    return d->requestActive();
}

/*!
    Returns the unique identifier for the QDSAction
*/
QUniqueId QDSAction::id() const
{
    return d->mId;
}

/*!
    Returns the QDSServiceInfo object which describes the QDS service being
    utilised.
*/
const QDSServiceInfo& QDSAction::serviceInfo() const
{
    return d->mServiceInfo;
}

/*!
    Returns the QDSData generated by the request.
*/
QDSData QDSAction::responseData() const
{
    return d->mResponseData;
}

/*!
    Returns any error message generated during the request. Errors
    are reported from either QDSAction or the provider application, and are
    reported through QDSData::error() or the return value of
    QDSData::exec(). If no error has been reported a null string will be
    returned.
*/
QString QDSAction::errorMessage() const
{
    return d->mErrorMsg;
}

/*!
    Asynchronously initiates the QDS service request with \a requestData. Depending
    on the outcome of the request, the response from the QDS service provider
    is given by one of QDSData::response() or QDSData::error() signals.

    The request may also contain \a auxiliary data for supplementary
    information, which may be required for the request but does not
    conceptually belong to \a requestData.

    This method should only be used for QDS services which have request data, as
    discussed in QDSServiceInfo, otherwise an error will be generated.

    Returns true on successful completion of the request; otherwise returns false.
*/
bool QDSAction::invoke( const QDSData &requestData, const QByteArray& auxiliary )
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return false;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return false;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( requestData.type() ) ) {
        d->mErrorMsg = tr( "incorrect data type" );
        return false;
    }

    d->reset();
    d->emitRequest( requestData, auxiliary );

    return true;
}

/*!
    Asynchronously initiates the QDS service request. Depending on the outcome
    of the request, the response from the QDS service provider is given by one
    of QDSData::response() or QDSData::error() signals.

    This method should only be used for QDS services which don't have request
    data, as discussed in QDSServiceInfo, otherwise an error will be generated.

    Returns true on successful completion of the request; otherwise returns false.
*/
bool QDSAction::invoke()
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return false;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return false;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( QMimeType(QString()) ) ) {
        d->mErrorMsg = tr( "expecting data" );
        return false;
    }

    d->reset();
    d->emitRequest();

    return true;
}

/*!
    Synchronously performs the QDS service request. The return value provides
    the result of the request, see QDSAction::ResponseCode.

    This method should only be used for QDS services which don't have request
    data, as discussed in QDSServiceInfo, otherwise an error will be
    generated.

    \warning This call will block until a response has been received from the QDS service
    or the request times out.
*/
int QDSAction::exec()
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return QDSAction::Error;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return QDSAction::Error;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( QMimeType(QString()) ) ) {
        d->mErrorMsg = tr( "expecting data" );
        return QDSAction::Error;
    }

    if (d->mEventLoop) {
        d->mErrorMsg = tr( "QDSAction::exec: Recursive call detected" );
        qLog(DataSharing) << d->mErrorMsg.toLatin1();
        return QDSAction::Error;
    }

    invoke();

    QEventLoop eventLoop;
    QtopiaApplication::instance()->
        registerRunningTask( "QDSAction/" + d->mId.toString(), &eventLoop );
    d->mEventLoop = &eventLoop;
    (void) eventLoop.exec();
    d->mEventLoop = 0;

    return d->mResponseCode;
}

/*!
    Synchronously performs the QDS service request with \a requestData. The return
    value provides the result of the request, see QDSAction::ResponseCode.

    The request may also contain \a auxiliary data for supplementary
    information which may be required for the request but does not
    conceptually belong to \a requestData.

    This method should only be used for QDS services which have request data,
    as discussed in QDSServiceInfo, otherwise an error will be generated.

    \warning This call will block until the response has been received from the
    QDS service or the request times out.

*/
int QDSAction::exec( const QDSData& requestData, const QByteArray& auxiliary )
{
    if ( !isValid() ) {
        d->mErrorMsg = tr( "invalid service" );
        return QDSAction::Error;
    }

    if ( !isAvailable() ) {
        d->mErrorMsg = tr( "unavailable service" );
        return QDSAction::Error;
    }

    if ( !d->mServiceInfo.supportsRequestDataType( requestData.type() ) ) {
        d->mErrorMsg = tr( "incorrect data type" );
        return QDSAction::Error;
    }

    if (d->mEventLoop) {
        d->mErrorMsg = tr( "QDSAction::exec: Recursive call detected" );
        qLog(DataSharing) << d->mErrorMsg.toLatin1();
        return QDSAction::Error;
    }

    invoke( requestData, auxiliary );

    QEventLoop eventLoop;
    QtopiaApplication::instance()->
        registerRunningTask( "QDSAction/" + d->mId.toString(), &eventLoop );
    d->mEventLoop = &eventLoop;
    (void) eventLoop.exec();
    d->mEventLoop = 0;

    return d->mResponseCode;
}


