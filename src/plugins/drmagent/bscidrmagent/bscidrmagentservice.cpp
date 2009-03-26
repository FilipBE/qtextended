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

#include "bscidrmagentservice.h"
#include "bscidrm.h"
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <QtDebug>
#include <QTimer>
#include <QContentSet>
#include <QSettings>
#include <QTemporaryFile>
#include <QDateTime>
#include <QSoftMenuBar>
#include <QDSData>
#include <QDSActionRequest>
#include <QMimeType>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QUrl>
#include <QThread>
#include <QEventLoop>
#include <QWaitWidget>
#include "bsciprompts.h"

class BSciRoapThread : public QThread
{
    Q_OBJECT
public:
    BSciRoapThread( QObject *parent = 0 );
    virtual ~BSciRoapThread();

    int handleTrigger( const QByteArray &trigger, SBSciRoapStatus *status );

    void run();

public slots:
    void cancel();

private:
    int m_operationHandle;
    QByteArray m_trigger;
    SBSciRoapStatus *m_status;
    int m_result;
};

BSciRoapThread::BSciRoapThread( QObject *parent )
    : QThread( parent )
    , m_operationHandle( BSCICreateOperationHandle( BSciDrm::context ) )
    , m_result( BSCI_NO_ERROR )
{
}

BSciRoapThread::~BSciRoapThread()
{
}

int BSciRoapThread::handleTrigger( const QByteArray &trigger, SBSciRoapStatus *status )
{
    m_trigger = trigger;
    m_status = status;

    QEventLoop eventLoop;

    connect( this, SIGNAL(finished()), &eventLoop, SLOT(quit()), Qt::QueuedConnection );

    start();

    eventLoop.exec();

    return m_result;
}

void BSciRoapThread::run()
{
    m_result = BSCIHandleRoapTrigger( BSciDrm::context, m_operationHandle, m_trigger.constData(), m_trigger.length(), m_status );
}

void BSciRoapThread::cancel()
{
    BSCICancel( BSciDrm::context, m_operationHandle );
}

/*!
    \service BSciDrmAgentService OmaDrmAgent
    \inpublicgroup QtDrmModule
    \brief The BSciDrmAgentService class provides the OmaDrmAgent service.

    The \i OmaDrmAgent service provides QDS services for handling received OMA DRM rights objects and ROAP triggers.
*/


/*!
    Constructs a new oma drm agent service with the given \a parent.

    \internal
*/
BSciDrmAgentService::BSciDrmAgentService( QObject *parent )
    : QtopiaAbstractService( "OmaDrmAgent", parent )
{
    publishAll();
}

/*!
    Destroys the drm agent service.

    \internal
 */
BSciDrmAgentService::~BSciDrmAgentService()
{
}

/*!
    Processes a QDS \a request to handle an OMA DRM v2 protected rights object.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.ro+xml and any
    combination of the handles \c {drm}, \c {handle}, \c {push}, and \c {x-wap.application:drm.ou}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::handleProtectedRightsObject(QDSActionRequest)}.
 */
void BSciDrmAgentService::handleProtectedRightsObject( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    handleProtectedRightsObject( requestCopy.requestData().data() );

    requestCopy.respond();
}

/*!
    Processes a QDS \a request to install an OMA DRM v1 XML rights object.

    If the rights object is not associated with content that rights were expected to arrive for a notification
    will be displayed to the user informing them of the rights object's arrival.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.rights+xml and any
    combination of the handles \c {drm}, \c {handle}, \c {push}, and \c {x-wap.application:drm.ou}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::handleXmlRightsObject(QDSActionRequest)}.
*/
void BSciDrmAgentService::handleXmlRightsObject( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    handleXmlRightsObject( requestCopy.requestData().data() );

    requestCopy.respond();
}

/*!
    Processes a QDS \a request to install an OMA DRM v1 WBXML rights object.

    If the rights object is not associated with content that rights were expected to arrive for a notification
    will be displayed to the user informing them of the rights object's arrival.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.rights+wbxml and any
    combination of the handles \c {drm}, \c {handle}, \c {push}, and \c {x-wap.application:drm.ou}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::handleWbXmlRightsObject(QDSActionRequest)}.
*/
void BSciDrmAgentService::handleWbXmlRightsObject( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    handleWbXmlRightsObject( requestCopy.requestData().data() );

    requestCopy.respond();
}

/*!
    Process a QDS \a request to handle an OMA DRM v2 ROAP trigger.

    In response to the trigger the user may be asked for permission to register with a rights issuer, or join, update or leave
    a domain.  If new rights to content are acquired as a result of receiving the trigger the user will be informed of this
    when the processing completes, except in the case the rights were anticipated.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.roap-trigger+xml and any
    combination of the handles \c {drm}, \c {handle}, \c {push}, and \c {x-wap.application:drm.ou}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::handleRoapTrigger(QDSActionRequest)}.
*/
void BSciDrmAgentService::handleRoapTrigger( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    handleRoapTrigger( requestCopy.requestData().data() );

    requestCopy.respond();
}

/*!
    Processes a QDS \a request to handle an OMA DRM v2 ROAP PDU.

    In response to the PDU the user may be asked for permission to register with a rights issuer, or join, update or leave
    a domain.  If new rights to content are acquired as a result of receiving the PDU the user will be informed of this
    when the processing completes, except in the case the rights were anticipated.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.roap-pdu+xml and any
    combination of the handles \c {drm}, \c {handle}, \c {push}, and \c {x-wap.application:drm.ou}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::handleRoapPdu(QDSActionRequest)}.
*/
void BSciDrmAgentService::handleRoapPdu( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    handleRoapPdu( requestCopy.requestData().data() );

    requestCopy.respond();
}

/*!
    Converts a DRM message (\c {application/vnd.oma.drm.message}) in the QDS \a request to the OMA DRM v1 DCF format 
    (\c {application/vnd.oma.drm.content}) and stores the associated rights in the rights database.

    This slot corresonds to a QDS service with the request data type \c application/vnd.oma.drm.message, the response
    data type \c application/vnd.oma.drm.content and any combination of the handles \c {drm}, and \c {handle}.

    This slot corresponds to the QCop service message \c {OmaDrmAgent::convertMessage(QDSActionRequest)}.
*/
void BSciDrmAgentService::convertMessage( const QDSActionRequest &request )
{
    QDSActionRequest requestCopy( request );

    requestCopy.respond(
            QDSData( convertMessage( requestCopy.requestData().data() ),
                     QMimeType::fromId( QLatin1String( "application/vnd.oma.drm.content" ) ) ) );
}

/*!
    Installs an OMA DRM v1 XML rights \a object received in a web download, a WAP push, or through some other
    delivery mechanism.

    The following methods are provided for prompting the user when handling the ROAP trigger:
    \list
        \o notifyRightsObjectsReceived()
        \o notifyContentAvailable()
    \endlist
*/
void BSciDrmAgentService::handleXmlRightsObject( const QByteArray &object )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    char fileBuffer[ BSCI_MAX_FILE_PATH_LENGTH + 1 ];

    int error = BSCIHandleDrmRights( BSciDrm::context, object.constData(), object.length(), fileBuffer, BSCI_MAX_FILE_PATH_LENGTH );

    if( error != BSCI_NO_ERROR )
        BSciDrm::printError( error, __PRETTY_FUNCTION__ );
    else
    {
        QString fileName( fileBuffer );

        if( !fileName.isEmpty() )
            BSciPrompts::instance()->notifyContentAvailable( QStringList() << fileName );
    }
}

/*!
    Processes an OMA DRM v2 ROAP \a trigger received in a web download, a WAP push, or through some other
    delivery mechanism.

    The following methods are provided for prompting the user when handling the ROAP trigger:
    \list
        \o Rights issuer registration
        \list
            \o requestRegistrationPermission()
            \o notifyRegistrationSuccess()
            \o notifyRegistrationFailure()
        \endlist
        \o Rights object receipt
        \list
            \o notifyRightsObjectsReceived()
            \o notifyContentAvailable()
        \endlist
        \o Rights domains
        \list
            \o requestDomainPermission()
            \o notifyDomainSuccess()
            \o notifyDomainFailure()
        \endlist
    \endlist
*/
void BSciDrmAgentService::handleRoapTrigger( const QByteArray &trigger )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    SBSciRoapStatus status;

    memset( &status, 0, sizeof(status) );

    QWaitWidget waitWidget( 0 );

    waitWidget.setCancelEnabled ( true );

    BSciRoapThread thread;

    connect( &waitWidget, SIGNAL(cancelled()), &thread, SLOT(cancel()) );
    connect( &thread, SIGNAL(finished()), &waitWidget, SLOT(hide()) );

    waitWidget.show();

    int error = thread.handleTrigger( trigger, &status );

    if( error == BSCI_NO_ERROR )
        BSciPrompts::instance()->notifySuccess( &status );
    else if( error != BSCI_CANCEL )
        BSciPrompts::instance()->notifyFailure( &status, error );

    BSCIReleaseRoapStatus( BSciDrm::context, &status );
}

/*!
    Installs an OMA DRM v1 WBXML rights \a object received in a web download, a WAP push, or through some other
    delivery mechanism.

    The following methods are provided for prompting the user when handling the ROAP trigger:
    \list
        \o notifyRightsObjectsReceived()
        \o notifyContentAvailable()
    \endlist
*/
void BSciDrmAgentService::handleWbXmlRightsObject( const QByteArray &object )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    char fileBuffer[ BSCI_MAX_FILE_PATH_LENGTH + 1 ];

    int error = BSCIHandleDrmRights( BSciDrm::context, object.constData(), object.length(), fileBuffer, BSCI_MAX_FILE_PATH_LENGTH );

    if( error != BSCI_NO_ERROR )
        BSciDrm::printError( error, __PRETTY_FUNCTION__ );
    else
    {
        QString fileName( fileBuffer );

        if( !fileName.isEmpty() )
            BSciPrompts::instance()->notifyContentAvailable( QStringList() << fileName );
    }
}


/*!
    Converts an OMA DRM v1 \a message to the DCF format and returns the result.  Any rights associated with the message
    should be stored in the rights database.
*/
QByteArray BSciDrmAgentService::convertMessage( const QByteArray &message )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    QTemporaryFile file;

    file.open();

    QByteArray boundary = message.left( message.indexOf( '\r' ) );

    int error = BSCIHandleDrmMessage(
            BSciDrm::context,
            0,
            message.constData(),
            message.size(),
            boundary.constData(),
            file.fileName().toLocal8Bit().constData() );

    QByteArray dcf;

    if( error == BSCI_NO_ERROR )
        dcf = file.readAll();

    return dcf;
}

/*!
    Installs an OMA DRM v2 protected rights \a object received in a web download, a WAP push, or through some other
    delivery mechanism.

    The following methods are provided for prompting the user when handling the ROAP trigger:
    \list
        \o notifyRightsObjectsReceived()
        \o notifyContentAvailable()
    \endlist
*/
void BSciDrmAgentService::handleProtectedRightsObject( const QByteArray &object )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    QTemporaryFile file;

    file.open();

    file.write( object );

    int error = BSCIHandleProtectedRO( BSciDrm::context, 0, file.fileName().toLocal8Bit().constData() );

    file.close();

    if( error != BSCI_NO_ERROR )
        BSciDrm::printError( error, __PRETTY_FUNCTION__ );
}

/*!
    Processes an OMA DRM v2 ROAP \a pdu received in a web download, a WAP push, or through some other
    delivery mechanism.

    The following methods are provided for prompting the user when handling the ROAP trigger:
    \list
        \o Rights issuer registration
        \list
            \o requestRegistrationPermission()
            \o notifyRegistrationSuccess()
            \o notifyRegistrationFailure()
        \endlist
        \o Rights object receipt
        \list
            \o notifyRightsObjectsReceived()
            \o notifyContentAvailable()
        \endlist
        \o Rights domains
        \list
            \o requestDomainPermission()
            \o notifyDomainSuccess()
            \o notifyDomainFailure()
        \endlist
    \endlist
*/
void BSciDrmAgentService::handleRoapPdu( const QByteArray &pdu )
{
    qLog(DRMAgent) << __PRETTY_FUNCTION__;

    SBSciRoapStatus status;

    memset( &status, 0, sizeof(status) );

    int error = BSCIHandleROAPPDU( BSciDrm::context, 0, pdu.constData(), pdu.length(), &status );

    if( error == BSCI_NO_ERROR )
        BSciPrompts::instance()->notifySuccess( &status );
    else if( error != BSCI_CANCEL )
        BSciPrompts::instance()->notifyFailure( &status, error );

    BSCIReleaseRoapStatus( BSciDrm::context, &status );
}


#include "bscidrmagentservice.moc"
