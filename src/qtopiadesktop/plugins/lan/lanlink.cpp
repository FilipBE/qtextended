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

#include <qdplugin.h>
#include <desktopsettings.h>
#include <trace.h>

#include <QTcpSocket>

class LANLink : public QDLinkPlugin
{
    Q_OBJECT
public:
    LANLink( QObject *parent = 0 );
    ~LANLink();

    // QDPlugin
    QString id() { return "com.trolltech.plugin.link.lan"; }
    QString displayName() { return tr("LAN Link"); }
    void init();

    // QDLinkPlugin
    bool tryConnect( QDConPlugin *con );
    void stop();

private slots:
    void slotConnected();
    void slotDisconnected();
    void slotError();
    void pluginMessage( const QString &msg, const QByteArray &data );

private:
    QTcpSocket *mSocket;
};

QD_REGISTER_PLUGIN(LANLink)

LANLink::LANLink( QObject *parent )
    : QDLinkPlugin( parent ), mSocket( 0 )
{
}

LANLink::~LANLink()
{
    if ( mSocket ) {
        mSocket->deleteLater();
        mSocket = 0;
    }
}

void LANLink::init()
{
    DesktopSettings settings(id());

    // Backup property values
    QMap<QString,QVariant> save;
    QStringList properties = QStringList()
            << "address"
            ;
    foreach ( const QString &property, properties )
        save[property] = settings.value(QString("properties/%1/value").arg(property));

    // Clear and initialize the properties structure
    settings.remove( "properties" );
    // we care about the ordering of the properties
    settings.setValue( "/properties/order", properties );

    settings.beginGroup( "properties/address" );
    settings.setValue( "label", tr("Address") );
    settings.setValue( "default", "localhost" );
    settings.setValue( "editor", "QLineEdit" );
    settings.setValue( "helptext", tr("Enter the IP Address or the Hostname of the "
                                      "Qtopia device you wish to connect to.") );
    settings.endGroup();

    // Restore property values
    foreach( const QString &property, save.keys() ) {
        QVariant value = save[property];
        if ( !value.isValid() )
            value = settings.value(QString("properties/%1/default").arg(property));
        settings.setValue(QString("properties/%1/value").arg(property), value);
    }
}

void LANLink::pluginMessage( const QString &msg, const QByteArray &data )
{
    TRACE(QDLink) << "LANLink::pluginMessage" << msg;
    QDataStream ds(data);
    if ( msg == "propertyChanged(QString)" ) {
        QString property;
        ds >> property;
        LOG() << id() << "property" << property << "has changed";
        if ( property == "address" )
            slotDisconnected();
    }
}

bool LANLink::tryConnect( QDConPlugin *con )
{
    TRACE(QDLink) << "LANLink::tryConnect" << con->displayName();

    Q_ASSERT( !mSocket );

    int port = con->port();
    if ( port == 0 )
        return false;

    mSocket = new QTcpSocket;
    connect( mSocket, SIGNAL(connected()), this, SLOT(slotConnected()) );
    connect( mSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    connect( mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError()) );

    DesktopSettings settings(id());
    QString ip = settings.value("properties/address/value").toString();

    LOG() << "Trying to connect to" << ip << ":" << port;
    mSocket->connectToHost( ip, port );

    return true;
}

void LANLink::stop()
{
    if ( mSocket ) {
        mSocket->deleteLater();
        mSocket = 0;
    }
}

void LANLink::slotConnected()
{
    TRACE(QDLink) << "LANLink::slotConnected";
    if ( mSocket )
        setupSocket( mSocket );
    emit setState(this, QDLinkPlugin::Up);
}

void LANLink::slotDisconnected()
{
    TRACE(QDLink) << "LANLink::slotDisconnected";
    emit setState(this, QDLinkPlugin::Down);
}

void LANLink::slotError()
{
    TRACE(QDLink) << "LANLink::slotError";
    emit setState(this, QDLinkPlugin::Down);
}

#include "lanlink.moc"
