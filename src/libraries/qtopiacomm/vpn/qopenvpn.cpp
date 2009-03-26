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

#include "qopenvpn_p.h"
#include "qvpnclientprivate_p.h"

#ifndef QTOPIA_NO_OPENVPN

#include "qopenvpngui_p.h"

#include <QByteArray>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QStackedWidget>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

/*
   Change this string if the OpenVPN binary can't be found by using the PATH variable.
   */
#define OPENVPN_BINARY "openvpn"
/*!
  \internal
  \class QOpenVPN

  \internal This class represents an OpenVPN network.

  An QOpenVPN objects operates in two distinct modes.
 */

/*!
  \internal

  Constructs a virtual private OpenVPN network with the given \a parent.
  The server mode is used by vpnmanager only and indicates that this object
  should actually start the vpn connection. An OpenVPN object that is not
  running in server mode will forward connect() and disconnect() calls to the server.

  This constructor should be used when an entirely new OpenVPN connection
  is required.
*/
QOpenVPN::QOpenVPN( QObject* parent )
    : QVPNClient( false, parent ), logIndex( 0 ), pendingID( 0 ), killID( 0 )
{
    d->createVPNConfig( QLatin1String("openvpn") );

    QSettings cfg( d->config, QSettings::IniFormat );
    cfg.setValue( "Info/Name", tr("OpenVPN connection") );
    cfg.setValue( "Info/Type", (int)type() );
    cfg.sync();

    d->vItem = new QValueSpaceItem( QLatin1String("/Network/VPN/") +
                QString::number( id() ), this );
    QObject::connect( d->vItem, SIGNAL(contentsChanged()), this, SLOT(stateChanged()) );
    lastState = QVPNClient::Disconnected;
}

/*!
  \internal

  \a vpnID allows the slection of a particular OpenVPN connection.

  This constructor should be used in order to load an existing OpenVPN connection.
  */
QOpenVPN::QOpenVPN( bool serverMode, uint vpnID, QObject* parent )
    : QVPNClient( serverMode, vpnID, parent ), logIndex( 0 ), pendingID( 0 ), killID( 0 )
{
    lastState = QVPNClient::Disconnected;
    if ( serverMode ) {
        d->vSpace = new QValueSpaceObject( QLatin1String("/Network/VPN/") +
                QString::number(id()), this );
        d->vSpace->setAttribute( QLatin1String("State"), QVPNClient::Disconnected );
    } else {
        d->vItem = new QValueSpaceItem( QLatin1String("/Network/VPN/") +
                    QString::number( id() ), this );
        QObject::connect( d->vItem, SIGNAL(contentsChanged()), this, SLOT(stateChanged()) );
    }
}

/*!
  \internal
  Destroys the virtual private network.
*/
QOpenVPN::~QOpenVPN()
{
    if ( d->serverMode && state() != Disconnected )
        disconnect();
}

/*!
  \internal
  Returns QVPNClient::OpenVPN.
  */
QVPNClient::Type QOpenVPN::type() const
{
    return QVPNClient::OpenVPN;
}

QStringList QOpenVPN::parameter( bool* error ) const
{
    QStringList parameter;
    QSettings cfg( d->config, QSettings::IniFormat );
    cfg.beginGroup("Properties");

    QString temp = cfg.value( QLatin1String("ConfigScript") ).toString();
    if ( !temp.isEmpty() ) {
        parameter << "--config";
        parameter << temp;
        return parameter; //if we use a configuration script all other options are ignored
    }
    temp = cfg.value( QLatin1String("Remote") ).toString();
    if ( temp.isEmpty() ) {
        d->errorString = tr("No remote server defined");
        *error = true;
        return parameter;
    }
    parameter << "--remote";
    parameter << temp;


    temp = cfg.value( QLatin1String("Port") ).toString();
    if ( !temp.isEmpty() ) {
        parameter << "--port";
        parameter << temp;
    }

    temp = cfg.value( QLatin1String("Protocol" ) ).toString();
    if ( !temp.isEmpty() ) {
        parameter << "--proto";
        if ( temp == "tcp-client" ) {
            parameter << "tcp-client";
        } else if ( temp == "tcp-server" ) {
            parameter << "tcp-server";
        } else {
            parameter << "udp";
            int exitRetries = cfg.value( QLatin1String("ExitNotification"), 2 ).toInt();
            if ( exitRetries>0 )
                parameter << "--explicit-exit-notify" << QString::number(exitRetries);
        }
    }

    temp = cfg.value( QLatin1String("Device"), QLatin1String("tap") ).toString();
    parameter << "--dev";
    parameter << temp;

    temp = cfg.value( QLatin1String("LocalIP") ).toString();
    if ( !temp.isEmpty() && !cfg.value( QLatin1String("RemoteIP") ).toString().isEmpty() ) {
        parameter << "--ifconfig";
        parameter << temp;
        parameter << cfg.value( QLatin1String("RemoteIP") ).toString();
    }

    temp = cfg.value( QLatin1String("Ping") ).toString();
    if ( !temp.isEmpty() ) {
        parameter << "--ping";
        parameter << temp;
    }

    temp = cfg.value( QLatin1String("PingRestart") ).toString();
    if ( !temp.isEmpty() ) {
        parameter << "--ping-restart";
        parameter << temp;
    }

    if ( cfg.value( QLatin1String("LZO"), false ).toBool() ) {
        parameter << "--comp-lzo";
    }

    if ( cfg.value( QLatin1String("Pull"), false ).toBool() ) {
        parameter << "--pull";
    }

    parameter << "--verb";
    parameter << cfg.value( QLatin1String("Verbosity"), 0 ).toString();

    parameter << "--mute";
    parameter << cfg.value( QLatin1String("Mute"), 1 ).toString();

    temp = cfg.value( QLatin1String("Authentication"), "TLS" ).toString();
    if ( temp == QLatin1String("TLS") ){ //TLS/SSL authentication
        parameter << "--client";
        temp = cfg.value( QLatin1String("CA") ).toString();
        parameter << "--ca";
        parameter << temp;
        temp = cfg.value( QLatin1String("Certificate") ).toString();
        parameter << "--cert";
        parameter << temp;
        temp = cfg.value( QLatin1String("PrivKey") ).toString();
        parameter << "--key";
        parameter << temp;
    } else if ( temp == QLatin1String("Static") ) { //static key authentication
        temp = cfg.value( QLatin1String("Secret") ).toString();
        parameter << "--secret";
        parameter << temp;
    } else { //no authentication
        //Nothing to do
    }

    cfg.endGroup();

    return parameter;
}

/*!
  \internal
  Starts a new OpenVPN connection.
  */
void QOpenVPN::connect()
{
    if ( state() != Disconnected )
        return;

    if ( d->serverMode ) {
        if ( !d->vpnProcess ) {
            d->vpnProcess = new QProcess();
            QObject::connect( d->vpnProcess, SIGNAL(stateChanged(QProcess::ProcessState)),
                    this, SLOT(stateChanged(QProcess::ProcessState)) );
            //d->vpnProcess->setReadChannelMode( QProcess::ForwardedChannels );
            QFileInfo fi( d->config);
            d->vpnProcess->setStandardOutputFile( Qtopia::tempDir() + fi.fileName() + ".log" );
        }
        if ( d->vpnProcess->state() == QProcess::NotRunning ) {
            qLog(VPN) << "Starting OpenVPN connection:" << name();
            bool error = false;
            QStringList params = parameter( &error );
            if ( !error ) {
                qLog(VPN) << "VPN parameter:" << params;
                d->vpnProcess->start( OPENVPN_BINARY, params );
            } else {
                qLog(VPN) << "Wrong VPN parameter" << d->errorString;
            }
        }
    } else {
#ifndef QT_NO_COP
        QtopiaIpcAdaptor o( QLatin1String("QPE/VPNManager") );
        o.send( MESSAGE(connectVPN(uint)), id() );
#endif
    }
}

/*!
  \internal
  Disconnects the network.
  */
void QOpenVPN::disconnect()
{
    if ( state() == Disconnected )
        return;

    if ( d->serverMode ) {
        if ( d->vpnProcess ) {
            if ( !killID ) {
                d->vpnProcess->terminate();
                killID = startTimer( 10000 );
            } else {
                qLog(VPN) << "Forcing shutdown of" << name();
                d->vpnProcess->kill();
                killTimer( killID );
                killID = 0;
            }
        }
    } else {
#ifndef QT_NO_COP
        QtopiaIpcAdaptor o( QLatin1String("QPE/VPNManager") );
        o.send( MESSAGE(disconnectVPN(uint)), id() );
#endif
    }
}

/*!
  \internal
  Returns the human-readable name of this OpenVPN connection.
  */
QString QOpenVPN::name() const
{
    if ( d->config.isEmpty() ) {
        return tr("OpenVPN connection");
    }
    return QVPNClient::name();
}

/*!
  \internal
  Returns a configuration dialog for this OpenVPN network. The caller is responsible
  for the deletion of the returned pointer.
  */
QDialog* QOpenVPN::configure( QWidget* parent )
{
    if ( !d->serverMode ) {
        return new QOpenVPNDialog( d->config, parent );
    }
    return 0;
}

/*!
  \internal
  Returns the current state of the network.
  */
QVPNClient::State QOpenVPN::state() const
{
    if ( d->serverMode ) {
        if ( d->vpnProcess && d->vpnProcess->state() == QProcess::Running) {
            if ( pendingID )
                return QVPNClient::Pending;
            else
                return QVPNClient::Connected;
        }
    } else {
        const int state = d->vItem->value("State", -1).toInt();
        if ( state >= 0 )
           return (QVPNClient::State) state;
    }
    return QVPNClient::Disconnected;
}

/*!
  \internal
  Deletes the OpenVPN configuration and all files associated to it.
  \warning This function does nothing if the connection is still active.
  */
void QOpenVPN::cleanup()
{
    if ( state() != Disconnected )
        return;

    if ( !d->serverMode && !d->config.isEmpty() ) {
        uint i = id();
        QFile::remove( d->config );
#ifndef QT_NO_COP
        QtopiaIpcAdaptor o( QLatin1String("QPE/VPNManager") );
        o.send( MESSAGE(deleteVPN(uint)), i );
#endif
    }
}

/*!
  \internal

  This slot is used in client mode only.
*/
void QOpenVPN::stateChanged() {
    if ( d->serverMode )
        return;
    int newState = d->vItem->value("State", -1).toInt();
    if ( newState == -1 ) //handle undefined as disconnected
        newState = 0;

    if ( lastState != (QVPNClient::State)(newState) ) {
        lastState = (QVPNClient::State)(newState);
        emit connectionStateChanged( false );
    }
}


void QOpenVPN::stateChanged( QProcess::ProcessState newState )
{
    if ( !d->vSpace )
        return;
    switch( newState ) {
        case QProcess::NotRunning:
            d->vSpace->setAttribute( QLatin1String("State"), QVPNClient::Disconnected );
            if ( killID ) {
                killTimer( killID );
                killID = 0;
            }
            emit connectionStateChanged( false );
            break;
        case QProcess::Running:
        case QProcess::Starting:
            d->vSpace->setAttribute( QLatin1String("State"), QVPNClient::Pending );
            emit connectionStateChanged( false );
            pendingID = startTimer( 1000 );
            break;
    }
}

void QOpenVPN::timerEvent( QTimerEvent* e )
{
    if ( e->timerId() == pendingID ) {
        QFile logf( Qtopia::tempDir() + QFileInfo(d->config).fileName() + QLatin1String(".log") );
        if ( ! logf.open( QIODevice::ReadOnly ) )
            return;
        QString log = logf.readAll();
        log.replace( "\r", "" );
        if ( logIndex > log.length() )
            logIndex = 0;
        int i = 0;
        i = log.indexOf( QRegExp( "Initialization Sequence Completed" ), logIndex );
        if ( i >= logIndex ) {
            logIndex = i+4;
            if ( d->vSpace )
                d->vSpace->setAttribute( QLatin1String("State"), QVPNClient::Connected );
            emit connectionStateChanged( false );
            if ( pendingID ) {
                killTimer( pendingID );
                pendingID = 0;
            }
        }
    } else if ( e->timerId() == killID ) {
        qLog(VPN) << "OpenVPN process" << name() << "doesn't react. Forcing shutdown.";
        killTimer( killID );
        killID = 0;
        if ( d->vpnProcess )
            d->vpnProcess->kill();
    }
}



/* OpenVPN user interface */

QOpenVPNDialog::QOpenVPNDialog( const QString& cfg, QWidget* parent )
    : QDialog( parent ), config( cfg )
{
    setModal( true );
    init();
}


QOpenVPNDialog::~QOpenVPNDialog()
{
}

void QOpenVPNDialog::init()
{
    QVBoxLayout* vbox = new QVBoxLayout( this );
    vbox->setMargin( 0 );
    vbox->setSpacing( 0 );
    setWindowTitle( tr("OpenVPN") );

    stack = new QStackedWidget( this );

    QWidget *widget = new QWidget( );
    QVBoxLayout* vl = new QVBoxLayout( widget );
    vl->setMargin( 2 );
    vl->setSpacing( 4 );
    list = new QListWidget( widget );
    list->setSpacing( 1 );
    list->setAlternatingRowColors( true );

    QListWidgetItem *item = new QListWidgetItem( QIcon(":icon/account"), tr("Account"), list );
    item->setTextAlignment( Qt::AlignHCenter );

    item = new QListWidgetItem( QIcon(":icon/exec"), tr("Device"), list );
    item->setTextAlignment( Qt::AlignHCenter );

    item = new QListWidgetItem( QIcon(":icon/padlock"), tr("Certificates"), list );
    item->setTextAlignment( Qt::AlignHCenter );

    item = new QListWidgetItem( QIcon(":icon/settings"), tr("Options"), list );
    item->setTextAlignment( Qt::AlignHCenter );

    vl->addWidget( list );
    stack->addWidget( widget );

    QHBoxLayout *hl = new QHBoxLayout();
    userHint = new QLabel( widget );
    userHint->setWordWrap( true );
    userHint->setMargin( 2 );
    hl->addWidget( userHint );

    QSpacerItem* spacer = new QSpacerItem( 1, 60, QSizePolicy::Minimum,
            QSizePolicy::Expanding );
    hl->addItem( spacer );
    vl->addLayout( hl );

    vbox->addWidget( stack );

    VPNConfigWidget* w = 0;
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    w = new GeneralOpenVPNPage( this );
    w->setConfigFile( config );
    scroll->setWidget( w );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    w = new DeviceOpenVPNPage( this );
    w->setConfigFile( config );
    scroll->setWidget( w );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    w = new CertificateOpenVPNPage( this );
    w->setConfigFile( config );
    scroll->setWidget( w );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    w = new OptionsOpenVPNPage( this );
    w->setConfigFile( config );
    scroll->setWidget( w );
    stack->addWidget( scroll );

    stack->setCurrentIndex( 0 );
    setObjectName("vpn"); 

    connect( list, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(listSelected(QListWidgetItem*)) );
    connect( list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(showUserHint(QListWidgetItem*,QListWidgetItem*)) );

    list->setCurrentRow( 0 );
}

void QOpenVPNDialog::listSelected(QListWidgetItem* item)
{
    if ( !item )
        return;
    int newIdx = list->row( item ) + 1;
    QScrollArea* scroll = 0;
    scroll = qobject_cast<QScrollArea *>(stack->widget( newIdx ));
    if ( scroll ) {
        VPNConfigWidget* w = 0;
        w = qobject_cast<VPNConfigWidget*>( scroll->widget() );
        if ( w ) {
            w->init();
            int stackIndex = list->row( item ) + 1;
            stack->setCurrentIndex( stackIndex );
            switch ( stackIndex ) {
                case 1:
                    setObjectName("openvpn-account");
                    break;
                case 2:
                    setObjectName("openvpn-device");
                    break;
                case 3:
                    setObjectName("openvpn-certs");
                    break;
                case 4:
                    setObjectName("openvpn-options");
                    break;
                default:
                    setObjectName("vpn");
            }
        }
    }
}

void QOpenVPNDialog::showUserHint( QListWidgetItem* newItem, QListWidgetItem* )
{
    if ( !newItem )
        return;

    QString hint;
    switch( list->row( newItem ) ) {
        case 0: //General
            hint = tr( "General OpenVPN parameter." );
            break;
        case 1: //Device parameter
            hint = tr( "Device type and IP parameter." );
            break;
        case 2: //Certificates
            hint = tr( "Certificate for authentication purposes." );
            break;
        case 3: //Options
            hint = tr( "Additional VPN options." );
            break;
        default:
            break;
    }
    userHint->setText( hint );
}

void QOpenVPNDialog::accept()
{
    if ( stack->currentIndex() == 0 ) {
        QDialog::accept();
    } else {
        VPNConfigWidget* w = qobject_cast<VPNConfigWidget*>( stack->currentWidget() );
        if ( w )
            w->save();
        stack->setCurrentIndex( 0 );
        setObjectName("vpn"); 
    }
}
#endif //QTOPIA_NO_OPENVPN
