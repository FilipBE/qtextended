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

#include "qwapaccount.h"

#include <qtopianamespace.h>

#include <QFile>
#include <QSettings>


class QWapAccountPrivate 
{
public:
    QWapAccountPrivate( const QString& configFile = QString() ) 
    {
        if ( configFile.isEmpty() || !QFile::exists( configFile ) )
            conf = new QSettings( createNewAccount(), QSettings::IniFormat );
        else
            conf = new QSettings( configFile, QSettings::IniFormat );
    }
    
    ~QWapAccountPrivate()
    {
        if ( conf ) {
            conf->sync();
            delete conf;
        }
        conf = 0;
    }
    
    static QString createNewAccount()
    {
        QString path = Qtopia::applicationFileName( QLatin1String("Network"), QLatin1String("wap") ) + "/";
        QString newName = path + "wap.conf";
        int index = 0;
        while ( QFile::exists( newName ) ) {
            index++;
            newName = path+QString("wap%1.conf").arg(index);
        }
        return newName;
    }

    QSettings* conf;
};

/*!
  \class QWapAccount
    \inpublicgroup QtBaseModule

  \brief The QWapAccount class provides a wrapper around configuration files for WAP accounts.

  Qt Extended WAP account configurations are saved under \c{$HOME/Applications/Network/wap} and can
  either be accessed via QSettings or QWapAccount. However QWapAccount should be the preferred 
  option because it does not require any knowledge of configuration file format which may change 
  in the future.

  \code
    QString configPath = Qtopia::applicationFileName( "Network", "wap" );
    QDir configDir( configPath );
    QStringList files = configDir.entryList( QStringList("*.conf") );

    //just use first entry
    foreach(QString acc, files )
    {
        QWapAccount wap( acc );
    }
  \endcode
    
  The various functions in this class reflect the various parameters which are stored in the
  WAP configuration. 
  \ingroup io

*/

/*!
  \enum QWapAccount::MMSVisibility

  This enum specifies the sender vsibility for MMS messages.

  \value Default the visibility is chosen by the transfer agent
  \value SenderHidden the sender address is not revealed
  \value SenderVisible the sender address is part of the message
  
*/

/*!
  Creates a new WAP account.
  */
QWapAccount::QWapAccount()
{
    d = new QWapAccountPrivate();
}


/*!
  Creates a new WAP account and reads the details from \a wapConfig.
  If \a wapConfig is empty or doesn't exist a new account is created.
  */
QWapAccount::QWapAccount( const QString& wapConfig )
{
    d = new QWapAccountPrivate( wapConfig );
}

/*!
  Creates a new WAP account by copying the \a other WAP account.
  */
QWapAccount::QWapAccount( const QWapAccount& other )
    : d( 0 )
{
    (*this ) = other; //use assignment operator
}

/*!
  Destroys the WAP account object.
  */
QWapAccount::~QWapAccount()
{
    delete d;
}

/*!
  Returns the configuration file (file path and name ) where the details for this
  WAP account are stored.
  */
QString QWapAccount::configuration() const
{
    return d->conf->fileName();
}

/*!
  Assignment operator. Sets data on this to be what is set on \a other.
  */
QWapAccount& QWapAccount::operator=(const QWapAccount& other)
{
    if ( d )
        delete d; //old settings saved during the process
    d = new QWapAccountPrivate( other.d->conf->fileName() );

    return (*this);
}

/*!
  Returns true if this WAP account is the same as \a other.
  Two QWapAccount object are equal if they refer to the same configuration
  file.
  */
bool QWapAccount::operator==(const QWapAccount& other )
{
    return ( d->conf->fileName() == other.d->conf->fileName() );
}

/*!
  Returns the user visible name of this WAP account.
  */
QString QWapAccount::name() const
{
    return d->conf->value( "Info/Name" ).toString();
}

/*!
  \a name becomes the new name for this WAP account.
  */
void QWapAccount::setName( const QString& name )
{
    d->conf->setValue( "Info/Name", name );
}

/*!
   Returns the data interface that is related to this WAP account.
   This is useful when providers require specific APNs, usernames or passwords
   when connecting to the WAP gateway. The returns string is the path to the 
   associated data account. The status of the data interface can be retrieved by using 
   QNetworkDevice.

   \code
        QWapAccount acc;
        ...
        if ( !acc.dataInterface().isEmpty() ) {
            //get state of associated account
            QNetworkDevice interface( acc.dataInterface() );
            QtopiaNetworkInterface::Status s = interface.state();
            ...
            //start interface so that we can use this WAP account
            if ( s != QtopiaNetworkInterface::Up )
                QtopiaNetwork::startInterface( acc.dataInterface() );
        }
   \endcode

   This function returns an empty string if there is no associated account.

   \sa setDataInterface(), QNetworkDevice, QtopiaNetwork
   */
QString QWapAccount::dataInterface() const
{
    return d->conf->value( "Info/DataAccount", "" ).toString();
}

/*!
  \a ifaceHandle becomes the default data interface for this WAP account.
 
  \sa dataInterface() 
*/
void QWapAccount::setDataInterface( const QString& ifaceHandle )
{
    if ( ifaceHandle.isEmpty() || !QFile::exists( ifaceHandle ) )
        return;

    d->conf->setValue( "Info/DataAccount", ifaceHandle );
}

/*!
  Returns the URL of the WAP gateway (including port, login and password).

  \code
    QWapAccount acc(...)
    ...
    QUrl gateway = acc.gateway();
    QString password = gateway.password();
    QString username = gateway.userName();
  \endcode

  \sa setGateway(), QUrl
*/
QUrl QWapAccount::gateway() const
{
    QUrl url;
    url.setHost( d->conf->value( QLatin1String("Wap/Gateway") ).toString());
    url.setUserName( d->conf->value( QLatin1String("Wap/UserName") ).toString() );
    url.setPassword( d->conf->value( QLatin1String("Wap/Password") ).toString() );
    url.setPort( d->conf->value( QLatin1String("Wap/Port")).toInt() );

    return url;
}

/*!
  \a url points to the new WAP gateway.

  \sa gateway()
*/
void QWapAccount::setGateway( const QUrl& url )
{
    d->conf->setValue( QLatin1String("Wap/Gateway"), url.host() );
    d->conf->setValue( QLatin1String("Wap/UserName"), url.userName() );
    d->conf->setValue( QLatin1String("Wap/Password"), url.password() );
    d->conf->setValue( QLatin1String("Wap/Port"), url.port() );
}

/*!
  Returns the URL of the MMS serveri (including the port).

  \sa setMmsServer(), QUrl
*/
QUrl QWapAccount::mmsServer() const
{
    QUrl url;

    QString host( d->conf->value( QLatin1String("MMS/Server") ).toString() );
    QString port( d->conf->value( QLatin1String("MMS/Port") ).toString() );

    if ( !host.isEmpty() ) {
        QUrl configured( host );
        url.setHost( configured.host() );
        if ( configured.port() != -1 )
            url.setPort( configured.port() );
        if ( !configured.scheme().isEmpty() )
            url.setScheme( configured.scheme() );
        if ( !configured.path().isEmpty() )
            url.setPath( configured.path() );
    }
    if ( !port.isEmpty() ) {
        url.setPort( port.toInt() );
    }

    return url;
}

/*!
  \a url points to the new MMS server.

  \sa mmsServer()
*/
void QWapAccount::setMmsServer( const QUrl& url )
{
    QUrl server;
    if ( !url.host().isEmpty() )
        server.setHost( url.host() );
    if ( !url.scheme().isEmpty() )
        server.setScheme( url.scheme() );
    if ( !url.path().isEmpty() )
        server.setPath( url.path() );

    d->conf->setValue( QLatin1String("MMS/Server"), server.toString() );
    d->conf->setValue( QLatin1String("MMS/Port"), url.port() );
}

/*!
  Returns the expiry time for MMS messages in hours.

  \sa setMmsExpiry()
  */
int QWapAccount::mmsExpiry() const
{
    return d->conf->value( QLatin1String("MMS/Expiry"), 0 ).toInt();
}

/*!
  Sets the expiry time for MMS messages to \a mmsExpiry hours.
*/
void QWapAccount::setMmsExpiry( int mmsExpiry )
{
    d->conf->setValue( QLatin1String("MMS/Expiry"), mmsExpiry );
}

/*!
  Returns the sender visibility for MMS messages.

  \sa setMmsSenderVisibility()
  */
QWapAccount::MMSVisibility QWapAccount::mmsSenderVisibility() const
{
    QByteArray v = d->conf->value( QLatin1String("MMS/Visibility"), "default" ).toByteArray(); 
    if ( v == "show" )
        return SenderVisible;
    else if ( v == "hidden" )
        return SenderHidden;
    else
        return Default;
}

/*!
  Sets the MMS sender visibility to \a v.

  \sa mmsSenderVisibility()
  */
void QWapAccount::setMmsSenderVisibility( QWapAccount::MMSVisibility v )
{
    QByteArray vis = "default";
    switch ( v ) {
        case SenderVisible:
            vis = "show";
            break;
        case SenderHidden:
            vis = "hidden";
            break;
        default:
            break;
    }
    d->conf->setValue( QLatin1String("MMS/Visibility"), vis );
}

/*!
  Returns true when the sender permits delivery reports for send MMS messages.

  \sa setMmsDeliveryReport()
  */
bool QWapAccount::mmsDeliveryReport() const
{
    QByteArray b = d->conf->value( QLatin1String("MMS/AllowDeliveryReport"), "n" ).toByteArray();
    if ( b == "n" )
        return false;
    else 
        return true;
}

/*!
  Sets the MMS delivery report permission to \a delRpt.
*/
void QWapAccount::setMmsDeliveryReport( bool delRpt )
{
    d->conf->setValue( QLatin1String("MMS/AllowDeliveryReport"), delRpt ? "y" : "n" );
}
