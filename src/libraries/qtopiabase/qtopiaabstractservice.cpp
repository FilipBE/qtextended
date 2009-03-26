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

#include <qtopiaabstractservice.h>
#include <qmetaobject.h>
#include <qtopialog.h>
#include <QByteArray>

#include <qtopiaipcadaptor.h>
#include <qtopiaservices.h>

/*!
    \class QtopiaAbstractService
    \inpublicgroup QtBaseModule

    \brief The QtopiaAbstractService class provides an interface to messages on a QCop service
    which simplifies remote slot invocations

    Service messages in Qt Extended are sent with QtopiaServiceRequest.  They consist
    of a service name, a message name, and a list of parameter values.  Qt Extended dispatches service messages to the applications associated with the service
    name, on the application's \c{QPE/Application/appname} channel, where
    \c{appname} is the application's name.

    Client applications can listen for service messages on the application
    channel directly, using QtopiaChannel, but it is cleaner and less error-prone
    to create an instance of QtopiaAbstractService instead.

    The use of QtopiaAbstractService will be demonstrated using the \c{Time}
    service.  This has a single message called \c{editTime()} which asks
    the service to pop up a dialog allowing the user to edit the current time.

    \code
    class TimeService : public QtopiaAbstractService
    {
        Q_OBJECT
    public:
        TimeService( QObject *parent = 0 );

    public slots:
        void editTime();
    };

    TimeService::TimeService( QObject *parent )
        : QtopiaAbstractService( "Time", parent )
    {
        publishAll();
    }
    \endcode

    The call to publishAll() causes all public slots within \c{TimeService}
    to be automatically registered as Service messages.  This can be
    useful if the service has many message types.

    The client can send a request to the service using QtopiaServiceRequest:

    \code
    QtopiaServiceRequest req( "Time", "editTime()" );
    req.send();
    \endcode

    \sa QtopiaService, QtopiaIpcAdaptor, QtopiaIpcEnvelope, QtopiaServiceRequest
    \sa QtopiaChannel

    \ingroup ipc
*/

class ServiceQtopiaIpcAdaptorProxy : public QtopiaIpcAdaptor
{
    Q_OBJECT

public:
    ServiceQtopiaIpcAdaptorProxy(const QString &channel, QObject *parent=0);

    QString memberToMessage( const QByteArray& member );
    QStringList sendChannels( const QString& channel );
    QString receiveChannel( const QString& channel );

    QString m_channel;
};

ServiceQtopiaIpcAdaptorProxy::ServiceQtopiaIpcAdaptorProxy(const QString &channel, QObject *parent) :
        QtopiaIpcAdaptor(channel, parent), m_channel(channel)
{

}

QString ServiceQtopiaIpcAdaptorProxy::memberToMessage( const QByteArray& member )
{
    return m_channel + "::" + QtopiaIpcAdaptor::memberToMessage( member );
}

QStringList ServiceQtopiaIpcAdaptorProxy::sendChannels( const QString& channel )
{
    return QtopiaService::channels( channel );
}

QString ServiceQtopiaIpcAdaptorProxy::receiveChannel( const QString& )
{
    return QString();
}

class QtopiaAbstractService_Private
{
public:
    QtopiaAbstractService_Private(const QString &service);

    QtopiaIpcAdaptor *m_copobject;

    QString m_service;
    bool m_publishAllCalled;
};

QtopiaAbstractService_Private::QtopiaAbstractService_Private(const QString &service) :
        m_copobject(NULL),
        m_service(service),
        m_publishAllCalled(false)
{
    m_copobject = new ServiceQtopiaIpcAdaptorProxy(service);
}

/*!
    Construct a remote service object for \a service and attach it to \a parent.
*/
QtopiaAbstractService::QtopiaAbstractService( const QString& service, QObject *parent )
    : QObject( parent )
{
    m_data = new QtopiaAbstractService_Private(service);
}

/*!
    Destroy this QCop service handling object.
*/
QtopiaAbstractService::~QtopiaAbstractService()
{
    if (m_data)
        delete m_data;
}

/*!
    Publishes all slots on this object within subclasses of QtopiaAbstractService.
    This is typically called from a subclass constructor.
*/
void QtopiaAbstractService::publishAll()
{
    const QMetaObject *meta = metaObject();
    if ( !m_data->m_publishAllCalled ) {
        int count = meta->methodCount();
        int index = QtopiaAbstractService::staticMetaObject.methodCount();
        for ( ; index < count; ++index ) {
            QMetaMethod method = meta->method( index );
            if ( method.methodType() == QMetaMethod::Slot &&
                 method.access() == QMetaMethod::Public) {
                QByteArray name = method.signature();
                QtopiaIpcAdaptor::connect(m_data->m_copobject, "3" + name, this, "1" + name);
            }
        }
        m_data->m_publishAllCalled = true;
    }
}

#include "qtopiaabstractservice.moc"
