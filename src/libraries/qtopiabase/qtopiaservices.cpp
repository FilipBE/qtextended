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

#include "qtopiaservices.h"

#include <qdir.h>
#include <qfile.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qsettings.h>

static QStringList serviceList_p(const QString& d, const QString n)
{
    QStringList r;
    QDir dir(d, "???*", QDir::Unsorted, QDir::Dirs );
    QStringList dl=dir.entryList();
    for (QStringList::ConstIterator it=dl.begin(); it!=dl.end(); ++it) {
        r.append(n+*it);
        r += serviceList_p(d+"/"+*it,n+*it+"/");
    }
    return r;
}

/*!
  \class QtopiaService
    \inpublicgroup QtBaseModule


  \brief The QtopiaService class allows applications to query the services provided by other applications.

    \ingroup ipc

  A Qt Extended service is a named collection of features that an application
  may choose to provide. For example, web browsers providing
  the feature of displaying a web page given a URL will implement the \c WebAccess
  service; telephony programs providing phone call dialing support will implement
  the \c Dialer server; etc.

  See also \l{Services} for more information on implementing and configuring services.

  \sa {Services}, QtopiaServiceRequest
*/

/*!
  \class QtopiaServiceRequest
    \inpublicgroup QtBaseModule


  \brief The QtopiaServiceRequest class allows applications to request services from other applications.

    \ingroup ipc

  A QtopiaServiceRequest encapsulates a Qt Extended service name and the message to be sent to
  that service. It is similar to QtopiaIpcEnvelope, but uses service names
  rather than direct channel names.  The following example sends the \c{editTime()}
  request to the \c Time service:

  \code
  QtopiaServiceRequest req("Time", "editTime()");
  req.send();
  \endcode

  Parameter data may be written to the request prior to sending it with send().
  The following example requests that the \c WebAccess service open a specific URL:

  \code
  QtopiaServiceRequest req("WebAccess", "openURL(QString)");
  req << "http://www.example.com/";
  req.send();
  \endcode

  Applications that implement services can use QtopiaAbstractService to process
  incoming service messages.  See also {Services} for more information on
  implementing and configuring services.

  \sa {Services}, QtopiaService, QtopiaAbstractService
*/


/*!
  Returns a list of available services.
*/
QStringList QtopiaService::list()
{
    return serviceList_p(Qtopia::qtopiaDir()+"services", ""); // No tr
}

/*!
  Returns the name of the QSettings file defining the bindings which the
  user has selected for the \a service.
*/
QString QtopiaService::binding(const QString& service)
{
    QString svrc = service;
    for (int i=0; i<(int)svrc.length(); i++)
        if ( svrc[i]=='/' ) svrc[i] = '-';
    return "Service-"+svrc; // No tr
}


/*!
  Returns the name of the QSettings file defining the facilities available from
  the applications providing the \a service, or
  a null string if there is no such service.

  Applications providing the service may have additional configuration
  defined in the appConfig() file.

  \sa appConfig()
*/
QString QtopiaService::config(const QString& service)
{
    return Qtopia::qtopiaDir()+"services/"+service+".service";
}


/*!
  Returns the QSettings file defining the facilities available from
  the applications providing the \a service, or
  a null string if there is no such service.

  The QSettings can be used to extract more detailed application-specific
  information about the application providing the service.

  Note: The application always provides at least the services defined
  in the config() file - the information in the appConfig() is application-specific.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  \sa config()
*/
QString QtopiaService::appConfig(const QString& service, const QString& appname)
{
    QString a = app(service,appname);
    if ( a.isNull() )
        return a;
    return Qtopia::qtopiaDir()+"services/"+service+"/"+a;
}


/*!
  Returns all applications that offer the \a service, or
  a null string if there is no such service offered.

  \sa app()
*/
QStringList QtopiaService::apps(const QString& service)
{
    QStringList all;

    QDir dir(Qtopia::qtopiaDir()+"services/"+service, "*",
      QDir::Unsorted, QDir::Files );

    all = dir.entryList();

    return all;
}

/*!
  Returns the application providing the \a service or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  \sa apps()
*/
QString QtopiaService::app(const QString& service, const QString& appname)
{
    QSettings cfg("Trolltech",binding(service));
    QString r;
    if ( cfg.status()==QSettings::NoError ) {
        cfg.beginGroup("Service");
        if  (!appname.isEmpty())
            r = cfg.value(appname).toString();
        if ( r.isNull() )
            r = cfg.value("default").toString();
    }
    if ( r.isNull() ) {
        QDir dir(Qtopia::qtopiaDir()+"services/"+service, QString("*"),
          QDir::Unsorted, QDir::Files );

        if ( dir.count() )
        {
            r = dir[0]; //use any as a default
            for( unsigned int i = 0 ; i < dir.count() ; ++i )
                if( dir[i] == appname )
                    r = dir[i]; //but use specific if it exists
        }
    }
    return r;
}


/*!
  Returns the QCop channels for applications that offer
  \a service.

  \sa channel()
*/
QStringList QtopiaService::channels(const QString& service)
{
    QStringList r;
    QStringList rs = apps(service);
    for (QStringList::ConstIterator it = rs.begin(); it!=rs.end(); ++it)
        r.append( QString("QPE/Application/") + (*it).toLatin1() );
    return r;
}

/*!
  Returns the QCop channel for the given \a service, or
  a null string if there is no such service.

  If \a appname is provided, a specific binding to that application may
  be used rather than the global binding.

  \sa channels()
*/
QString QtopiaService::channel(const QString& service, const QString& appname)
{
    QString r = app(service,appname).toLatin1();
    return r.isEmpty() ? QString() : "QPE/Application/"+r;
}

/*!
  Returns the the list of services supported by \a appname.

  \sa list()
*/
QStringList QtopiaService::services(const QString& appname)
{
    QStringList all;

    foreach (QString service, list()) {
        QDir dir(Qtopia::qtopiaDir()+"services/"+service, appname, QDir::Unsorted, QDir::Files);
        if (dir.count())
            all << service;
    }

    return all;
}

#include <qtopiaipcenvelope.h>

/*!
  Construct a null service request.
  setService() and setMessage() must be called before send(), but the
  service may be written prior to the calls.
 */
QtopiaServiceRequest::QtopiaServiceRequest()
{
}

/*!
  Construct a service request that will send \a message to
  a \a service when send() is called. The service may be written
  prior to the calls.
*/
QtopiaServiceRequest::QtopiaServiceRequest(const QString& service, const QString& message)
    : m_Service(service), m_Message(message)
{
}

/*!
  Copy constructor. Any data previously written to the \a orig
  service will be in the copy.
*/
QtopiaServiceRequest::QtopiaServiceRequest(const QtopiaServiceRequest& orig)
{
    m_Service = orig.m_Service;
    m_Message = orig.m_Message;
    m_arguments = orig.m_arguments;
}

/*!
  Assignment operator.
  Any data previously written to the \a orig
  service will be in the copy.
*/
QtopiaServiceRequest& QtopiaServiceRequest::operator=(const QtopiaServiceRequest& orig)
{
    if( &orig == this )
        return *this;

    m_Service = orig.m_Service;
    m_Message = orig.m_Message;
    m_arguments = orig.m_arguments;

    return *this;
}

/*!
  Destructs the service request. Unlike QtopiaIpcEnvelope, the
  request is not automatically sent.
*/
QtopiaServiceRequest::~QtopiaServiceRequest()
{
}

/*!
  Returns true if either the service() or message() is not set.

  \sa service(), message()
 */
bool QtopiaServiceRequest::isNull() const
{
    return m_Service.isNull() || m_Message.isNull();
}

/*!
  Sends the request. Returns false if there was no application that could
  service the request.
*/
bool QtopiaServiceRequest::send() const
{
    if (isNull())
        return false;

#if !defined(QT_NO_COP) || defined(Q_WS_X11)
    QString ch;
    bool rawMessage = false;

    if(m_Service.startsWith("Application:")) {
        ch = "QPE/Application/" + m_Service.mid(12);
        rawMessage = true;
    } else if(m_Service.startsWith("Channel:")) {
        ch = m_Service.mid(8);
        rawMessage = true;
    } else {
        ch = QtopiaService::channel(m_Service, QString());
    }
    
    if (ch.isNull())
        return false;

    QBuffer *buffer = new QBuffer();
    QDataStream stream(buffer);
    stream.device()->open(QIODevice::WriteOnly);

    foreach(QVariant item, m_arguments) {
        QtopiaIpcAdaptorVariant copy( item );
        copy.save( stream );
    }

    QString msg = message();

    if(!rawMessage) {
        int indexOfCC = msg.indexOf("::");
        if (indexOfCC != -1) {
            int indexOfParen = msg.indexOf("(");
            if (indexOfCC > indexOfParen) {
                msg = service() + "::" + msg;
            }
        }
        else {
            msg = service() + "::" + msg;
        }
    }

    QtopiaIpcEnvelope e(ch, msg);
    delete e.device();
    e.setDevice(buffer);

    return true;
#else
    return false;
#endif
}

/*!
  Sets the \a service to which the request will be sent.

  \sa service()
 */
void QtopiaServiceRequest::setService(const QString& service)
{
    m_Service = service;
}

/*!
  \fn QString QtopiaServiceRequest::service() const

  Returns the service to which this request will be sent.

  \sa setService()
*/

/*!
    Sets the \a message to be sent to the service.

    \sa message()
*/
void QtopiaServiceRequest::setMessage(const QString& message)
{
    m_Message = message;
}

/*!
  \fn QString QtopiaServiceRequest::message() const

  Returns the message of the request.

  \sa setMessage()
*/

/*!
    \fn const QList<QVariant> &QtopiaServiceRequest::arguments() const

    Returns the complete list of arguments for this service request.
*/

/*!
    \fn void QtopiaServiceRequest::setArguments(const QList<QVariant> &arguments)

    Sets the complete list of \a arguments for this service request.
*/

/*!
    \fn QtopiaServiceRequest &QtopiaServiceRequest::operator<< (const T &var)

    Adds \a var to the list of arguments for this service request.
*/

/*!
    \fn QtopiaServiceRequest &QtopiaServiceRequest::operator<< (const char *var)

    Adds \a var to the list of arguments for this service request.
*/

/*!
    \fn void QtopiaServiceRequest::addVariantArg(const QVariant& var)

    Adds the variant \a var to the list of arguments, so that the variant's
    value is serialized in send() rather than the variant itself.
*/

/*!
    \internal
*/
QByteArray QtopiaServiceRequest::serializeArguments(const QtopiaServiceRequest &action)
{
    QByteArray ret;
    QBuffer *buffer = new QBuffer(&ret);
    buffer->open(QIODevice::WriteOnly);
    QDataStream stream(buffer);
    stream << action.m_arguments;

    delete buffer;
    return ret;
}

/*!
    \internal
*/
void QtopiaServiceRequest::deserializeArguments(QtopiaServiceRequest &action,
        const QByteArray &data)
{
    QDataStream stream(data);
    stream >> action.m_arguments;
}

/*!
    \internal
    \fn void QtopiaServiceRequest::serialize(Stream &stream) const
*/
template <typename Stream> void QtopiaServiceRequest::serialize(Stream &stream) const
{
    stream << m_arguments;
    stream << m_Service;
    stream << m_Message;
}

/*!
    \internal
    \fn void QtopiaServiceRequest::deserialize(Stream &stream)
*/
template <typename Stream> void QtopiaServiceRequest::deserialize(Stream &stream)
{
    stream >> m_arguments;
    stream >> m_Service;
    stream >> m_Message;
}

Q_IMPLEMENT_USER_METATYPE(QtopiaServiceRequest)
