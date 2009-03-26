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

#include "qbluetoothobexagent.h"

#include <qmimetype.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qwaitwidget.h>
#include <qbluetoothlocaldevice.h>
#include <qobexpushclient.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothrfcommsocket.h>
#include <QTextStream>

using namespace QBluetooth;

/*!
    \class QBluetoothObexAgent
    \brief The QBluetoothObexAgent class sends an object to a remote
    Bluetooth device over Obex connection.

    \ingroup qtopiabluetooth
  */

struct QBluetoothObexAgentPrivate {
    QBluetoothObexAgentPrivate();
    ~QBluetoothObexAgentPrivate();
    QBluetoothLocalDevice *m_localDevice;
    QBluetoothRemoteDevice *m_remoteDevice;
    SDPProfile m_profile;
    QString m_fileName;
    QString m_mimeType;
    QByteArray m_byteArray;
    QBluetoothSdpQuery m_sdap;
    QWaitWidget *m_waitWidget;
    bool m_autoDelete;
    bool m_inProgress;
    QObexPushClient *m_sender;
};

QBluetoothObexAgentPrivate::QBluetoothObexAgentPrivate()
    : m_localDevice( 0 ), m_remoteDevice( 0 ), m_waitWidget( 0 )
, m_autoDelete( false ), m_inProgress( false ), m_sender( 0 )
{
}

QBluetoothObexAgentPrivate::~QBluetoothObexAgentPrivate()
{
    if ( m_localDevice )
        delete m_localDevice;
    if ( m_remoteDevice )
        delete m_remoteDevice;
    if ( m_waitWidget )
        delete m_waitWidget;
}

/*!
    Constructs a QBluetoothObexAgent object which searches \a remoteDevice for \a profile.
    If \a profile is not specified, QBluetooth::ObjectPushProfile will be used.
  */
QBluetoothObexAgent::QBluetoothObexAgent( const QBluetoothRemoteDevice &remoteDevice, SDPProfile profile, QObject *parent )
    : QObject( parent )
{
    d = new QBluetoothObexAgentPrivate();

    QBluetoothLocalDeviceManager localDevManager( this );
    d->m_localDevice = new QBluetoothLocalDevice( localDevManager.defaultDevice() );
    d->m_remoteDevice = new QBluetoothRemoteDevice( remoteDevice );
    d->m_profile = profile;

    d->m_waitWidget = new QWaitWidget( 0 );
    d->m_waitWidget->setText( tr( "Searching..." ) );

    connect( &d->m_sdap, SIGNAL(searchComplete(QBluetoothSdpQueryResult)),
            this, SLOT(searchComplete(QBluetoothSdpQueryResult)) );
}

/*!
    Destructor
  */
QBluetoothObexAgent::~QBluetoothObexAgent()
{
    delete d;
}

void QBluetoothObexAgent::startSearch()
{
    d->m_inProgress = true;
    d->m_sdap.searchServices( d->m_remoteDevice->address(),
            *d->m_localDevice, d->m_profile );
    d->m_waitWidget->show();
}

bool QBluetoothObexAgent::inProgress()
{
    if ( d->m_inProgress ) {
        QMessageBox::information( 0, tr( "Sending in progress" ),
                tr( "<qt>Currently sending an object</qt>" ),
                QMessageBox::Ok );
    }
    return d->m_inProgress;
}
/*!
    Sends the object in file \a fileName to a remote device.
    If the optional \a mimeType parameter is not specified, it is
    determined by the file name suffix.
  */
void QBluetoothObexAgent::send( const QString &fileName, const QString &mimeType )
{
    if ( inProgress() )
        return;

    d->m_fileName = fileName;
    if ( mimeType.isEmpty() ) {
        QMimeType mime( fileName );
        d->m_mimeType = mime.id();
    } else
        d->m_mimeType = mimeType;
    startSearch();
}

/*!
    Sends the object in \a content to a remote device.
  */
void QBluetoothObexAgent::send( const QContent &content )
{
    if ( inProgress() )
        return;

    d->m_fileName = content.fileName();
    QMimeType mime( content );
    d->m_mimeType = mime.id();
    startSearch();
}

/*!
    Sends data in \a array to a remote device.
  */
void QBluetoothObexAgent::send( const QByteArray &array, const QString &fileName, const QString &mimeType )
{
    if ( inProgress() )
        return;

    d->m_byteArray = array;
    d->m_fileName = fileName;
    if ( mimeType.isEmpty() ) {
        QMimeType mime( fileName );
        d->m_mimeType = mime.id();
    } else
        d->m_mimeType = mimeType;
    startSearch();
}

/*!
    Creates an Xhtml object encapsulating body \a html and sends it to a remote device.
  */
void QBluetoothObexAgent::sendHtml( const QString &html )
{
    if ( inProgress() )
        return;
    // create a temp file
    d->m_fileName = Qtopia::tempDir() + "xhtml-print.xhtml";

    // ensure default namespace to be used on the root element
    QString body( html );
    if ( body.startsWith( "<html>" ) )
        body = body.remove( "<html>" );
    else if ( body.startsWith( "<html " ) )
        body = body.remove( 0, body.indexOf( ">" ) + 1 );
    else if ( !body.contains( "html" ) && !body.contains( "body" ) )
        body = "<body>" + body + "</body></html>";

    QFile file( d->m_fileName );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        return;

    QTextStream out( &file );
    // doc type decleration
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    out << "<!DOCTYPE html PUBLIC \"-//PWG//DTD XHTML-Print 1.0//EN\" ";
    out << "\"http://www.xhtml-print.org/xhtml-print/xhtml-print10.dtd\">";

    // root element namespace
    out << "<html xmlns=\"http://www.w3.org/1999/xhtml\">";

    // rest of the message
    out << body;

    d->m_mimeType = "application/vnd.pwg-xhtml-print+xml";
    startSearch();
}

/*!
    Sets the object autoDelete property to \a enable. If autoDelete is enabled
    the object will be delete automatically after operation. Otherwise the object has to be
    delete manually.
  */
void QBluetoothObexAgent::setAutoDelete( const bool enable )
{
    d->m_autoDelete = enable;
}

void QBluetoothObexAgent::searchComplete( const QBluetoothSdpQueryResult &result )
{
    qLog(Bluetooth) << "Service searching complete";

    bool success = false;
    foreach ( QBluetoothSdpRecord service, result.services() ) {
        if ( service.isInstance( d->m_profile ) ) {

            // check if file format is upported if printing
            if ( d->m_profile == DirectPrintingProfile ) {
                if ( !service.attribute( (quint16)0x0350 ).toString().toLower().contains( d->m_mimeType.toLower() ) ) {
                    if ( d->m_waitWidget->isVisible() )
                        d->m_waitWidget->hide();
                    QMessageBox::critical( 0, tr( "Format not supported" ),
                        tr( "<qt>Format (%1) is not supported.</qt>" )
                        .arg( d->m_mimeType ), QMessageBox::Ok );
                    emit done( true );
                    return;
               }
            }
            // discover REFCOMM server channel
            int channel = QBluetoothSdpRecord::rfcommChannel(service);

            // RFCOMM Connection
            QBluetoothRfcommSocket *rfcommSocket = new QBluetoothRfcommSocket;
            if (!rfcommSocket->connect(d->m_localDevice->address(), d->m_remoteDevice->address(), channel)) {
                delete rfcommSocket;
                QMessageBox::critical( 0, tr( "Bluetooth error" ),
                    tr( "<qt>Bluetooth connection error" ));
                emit done( true );
                return;
            }

            // OBEX connect
            d->m_sender = new QObexPushClient( rfcommSocket, this );
            rfcommSocket->setParent(d->m_sender); // socket is only needed for this client
            connect( d->m_sender, SIGNAL(dataTransferProgress(qint64,qint64)),
                    this, SLOT(progress(qint64,qint64)) );
            connect( d->m_sender, SIGNAL(done(bool)),
                    this, SIGNAL(done(bool)) );
            if ( d->m_autoDelete ) {
                connect( d->m_sender, SIGNAL(destroyed()),
                        this, SLOT(deleteLater()) );
            }

            // auto delete the push client when it's done
            connect(d->m_sender, SIGNAL(done(bool)), d->m_sender, SLOT(deleteLater()));

            d->m_sender->connect();

            // send file
            QFile *file = new QFile( d->m_fileName );
            if ( d->m_byteArray.isEmpty() )
                d->m_sender->send( file, d->m_fileName );
            else
                d->m_sender->send( d->m_byteArray, d->m_fileName );
            d->m_sender->disconnect();
            success = true;
            break;
        }

    }
    if ( !success ) {
        if ( d->m_waitWidget->isVisible() )
            d->m_waitWidget->hide();
        QMessageBox::critical( 0, tr( "Service not found" ),
                tr( "<qt>The selected device does not support this operation.</qt>" ),
                QMessageBox::Ok );
        emit done( true );
    }
}

void QBluetoothObexAgent::progress( qint64 sent, qint64 total )
{
    QString str;

    if ( sent > 1024 )
        str = QString::number( (int) sent / 1024 ) + tr( "KB", "kilobyte" );
    else
        str = QString::number( sent ) + tr( "B", "byte" );

    str += "/";

    if ( total > 1024 )
        str += QString::number( (int) total / 1024 ) + tr( "KB", "kilobyte" );
    else
        str += QString::number( total ) + tr( "B", "byte" );

    d->m_waitWidget->setText( str );
    if ( sent == total )
        d->m_waitWidget->hide();
}

/*!
    Attempts to abort send operation.
  */
void QBluetoothObexAgent::abort()
{
    if ( d->m_sender )
        d->m_sender->abort();
}

/*!
    \fn void QBluetoothObexAgent::done( bool error )

    This signal is emitted when the request has been completed.
    If an error occurred during processing \a error is false.
*/

