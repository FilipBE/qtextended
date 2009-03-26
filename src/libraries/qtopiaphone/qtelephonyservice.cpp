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

#include <qtelephonyservice.h>
#include <qphonecallprovider.h>
#include <qservicechecker.h>
#ifdef QTOPIA_CELL
#include <qsmsmessage.h>
#include <qwsppdu.h>
#endif
#include <qtopialog.h>
#include <qdsservices.h>
#include <qdsaction.h>
#include <QBuffer>

/*!
    \class QTelephonyService
    \inpublicgroup QtTelephonyModule

    \brief The QTelephonyService class provides a convenient wrapper to create telephony services and the interfaces that they support.
    \ingroup telephony

    This class extends QAbstractIpcInterfaceGroup to add service(), callProvider(),
    and setCallProvider().

    Telephony services group together a number of related telephony interfaces,
    to make it more convenient to create them at service start up and to allow
    the interfaces to find each other easily for passing requests from one
    interface to another.

    As an example, a VoIP telephony service will typically have at least three
    telephony interfaces, for network registration, presence, and phone call
    management.  The QTelephonyService object would be declared as follows:

    \code
    class VoIPService : public QTelephonyService
    {
        Q_OBJECT
    public:
        VoIPService( QObject *parent = 0 )
            : QTelephonyService( "voip", parent ) {}
        ~VoIPService() {}
    
        void initialize();
    };

    void VoIPService::initialize()
    {
        if ( !supports<QNetworkRegistration>() )
            addInterface( new VoIPNetworkRegistration( this ) );

        if ( !supports<QCollectivePresence>() )
            addInterface( new VoIPPresenceProvider( this ) );

        if ( !callProvider() )
            setCallProvider( new VoIPCallProvider( this ) );

        QTelephonyService::initialize();
    }
    \endcode

    The VoIP telephony service creates an instance of \c VoIPService, and then calls
    the initialize() method to complete the initialization process:

    \code
    VoIPService *service = new VoIPService();
    service->initialize();
    \endcode

    During initialize(), the three telephony interfaces corresponding to
    QNetworkRegistration, QCollectivePresence, and QPhoneCallProvider, are created.
    We first check to see if there is an existing implementation using
    supports() or callProvider(), which allows the VoIP telephony service
    to be inherited by another VoIP service that provides greater functionality
    than the basic VoIP service.

    At the end of the function, the base QTelephonyService::initialize()
    method is called, which completes the initialization process and
    advertises the telephony service and all of its interfaces to the system.
    QCommServiceManager can be used by client applications to receive
    notification of when telephony services enter and leave the system.

    \sa QAbstractIpcInterfaceGroup, QCommServiceManager, QCommInterface, QNetworkRegistration
    \sa QCollectivePresence, QPhoneCallProvider
*/

/*!
    Create a new telephony service called \a service and attach
    it to \a parent.

    A call to the constructor should be followed by a call to
    initialize() to complete the initialization process.

    \sa initialize()
*/
QTelephonyService::QTelephonyService( const QString& service, QObject *parent )
    : QAbstractIpcInterfaceGroup( service, parent )
{
    provider = 0;
}

/*!
    Destroy this telephony service.
*/
QTelephonyService::~QTelephonyService()
{
}

/*!
    Returns the name of this telephony service, which is the same as
    its group name.
*/
QString QTelephonyService::service() const
{
    return groupName();
}

/*!
    Returns the phone call provider associated with this service.
    Returns null if the provider has not yet been set.

    \sa setCallProvider()
*/
QPhoneCallProvider *QTelephonyService::callProvider() const
{
    return provider;
}

/*!
    Sets the phone call provider for this service to \a provider.
    If the service already has a provider, it will be deleted.
    Ownership of \a provider will pass to this object, so that
    it will be automatically deleted when the modem service is deleted.

    The usual way to use this method is from within an override
    of initialize():

    \code
    void MyService::initialize()
    {
        if ( !callProvider() )
            setCallProvider( new MyCallProvider( this ) );
        QTelephonyService::initialize();
    }
    \endcode

    \sa callProvider()
*/
void QTelephonyService::setCallProvider( QPhoneCallProvider *provider )
{
    if ( this->provider && this->provider != provider )
        delete this->provider;
    this->provider = provider;
    if ( provider )
        provider->setParent( this );
}

/*!
    \reimp
*/
void QTelephonyService::initialize()
{
    // Create a default service checker if we don't have one yet.
    if ( !supports<QServiceChecker>() )
        addInterface( new QServiceCheckerServer( service(), true, this ) );

    QAbstractIpcInterfaceGroup::initialize();
}

#ifdef QTOPIA_CELL

// Check for and dispatch flash sms messages.
static bool dispatchFlashMessage( const QSMSMessage& msg )
{
    if ( msg.messageClass() != 0 )
        return false;
    qLog(Modem) << "SMS flash message";
    QString type = "application/x-sms-flash";
    QDSServices services( type, QString(), QStringList() << "push" );
    if ( !services.isEmpty() ) {
        QDSAction action( services.first() );
        QByteArray payload;
        {
            QDataStream stream
                ( &payload, QIODevice::WriteOnly | QIODevice::Append );
            stream << msg;
        }
        action.invoke( QDSData( payload, QMimeType( type ) ) );
        return true;
    } else {
        return false;
    }
}

/*!
    Dispatch the SMS datagram \a msg according to its SMS port number
    or WAP Push MIME type.  This is used by telephony services that
    accept incoming SMS messages to dispatch them according to the
    installed services.  Returns true if the message was dispatched,
    or false if no service exists that can handle the message.

    See the documentation for QSMSMessage::destinationPort() for more
    information on how WAP push messages and SMS datagrams are dispatched.

    \sa QSMSMessage::destinationPort()
*/
bool QTelephonyService::dispatchDatagram( const QSMSMessage& msg )
{
    QString chan, type;
    QDSServiceInfo info;
    QByteArray payload;
    QByteArray raw;

    // If the message does not have a port number, then it isn't a datagram.
    int port = msg.destinationPort();
    if ( port == -1 )
        return dispatchFlashMessage( msg );

    // Recognise port numbers that may contain WAP push datagrams.
    // We want to check the content type to see if we have a
    // specialised handler for it before dispatching by port number.
    if ( port == 2948 || port == 49999 ) {
        type = QWspPush::quickContentType( msg.applicationData() );
        qLog(Modem) << "WAP push message of type " << type;
        QDSServices services( type, QString(), QStringList() << "push" );
        if ( !services.isEmpty() ) {
            info = services.first();
            QByteArray a = msg.applicationData();
            QBuffer pushpdu(&a);
            pushpdu.open(QIODevice::ReadOnly);
            QWspPduDecoder decoder(&pushpdu);
            QWspPush push = decoder.decodePush();
            payload = push.data();
        }
    }

    // See if we have a registered service for this port number.
    if ( !info.isValid() ) {
        qLog(Modem) << "SMS datagram on port " << port;
        type = "application/x-smsapp-" + QString::number(port);
        QDSServices services( type, QString(), QStringList() << "push" );
        if ( !services.isEmpty() ) {
            info = services.first();
            payload = msg.applicationData();
        } else {
            return dispatchFlashMessage( msg );
        }
    }

    // Pack the entire SMS message into a raw datastream, to be sent
    // along with the message as auxillary data.  This allows programs
    // that need the full SMS/WAP headers to access them.
    {
        QDataStream stream
            ( &raw, QIODevice::WriteOnly | QIODevice::Append );
        stream << msg;
    }

    // Send the datagram to the specified QDS service for processing.
    QDSAction action( info );
    action.invoke( QDSData( payload, QMimeType( type ) ), raw );

    // The datagram has been dispatched.
    return true;
}

#endif // QTOPIA_CELL
