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
#include <center.h>
#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>
#include <desktopsettings.h>
#include <qtopiadesktoplog.h>
#include <qdplugin.h>
#include <trace.h>

#include <QIODevice>
#include <QTimer>
#include <QUuid>
#include <QMap>

class QCopEmitter : public QObject
{
    Q_OBJECT
public:
    QCopEmitter( const QString &_method, const QByteArray &_data )
        : method( _method ), data( _data )
    {
    }

    ~QCopEmitter()
    {
    }

public slots:
    void doEmit()
    {
        emit receivedMessage( method, data );
        delete this;
    }

signals:
    void receivedMessage( const QString &method, const QByteArray &data );

private:
    QString method;
    QByteArray data;
};

// =====================================================================

class QCopConnection : public QDConPlugin
{
    Q_OBJECT
public:
    QCopConnection( QObject *parent = 0 );
    ~QCopConnection();

    // QDPlugin
    QString id() { return "com.trolltech.plugin.con.qcop"; }
    QString displayName() { return tr("QCop Connection"); }
    void init();

    // QDConPlugin
    QString conProperty( const QString &key );
    bool claim( QDDevPlugin *dev );
    void connected( QDDevPlugin *dev );
    QDDevPlugin *device();
    int port();

    bool tryConnect( QDLinkPlugin *link );
    void stop();

public:
    void sendMessage( const QString &channel, const QString &message, const QStringList &args = QStringList() );
    void sendMessage( const QString &channel, const QString &message, const QByteArray &data );

signals:
    void receivedMessage( const QString &message, const QByteArray &data );

private slots:
    void slotDisconnected();
    void slotReadyRead();
    void deviceMessage( const QString &message, const QByteArray &data );
    void pluginMessage( const QString &msg, const QByteArray &data );

private:
    void process( const QString &message );
    void processQCopCall( const QString &message, bool binary );
    void nextDevice( QDLinkPlugin *&lp, QDDevPlugin *&dp );

    QIODevice *mSocket;
    QMap<QString,QString> properties;
    QDDevPlugin *mDevice;
};

QD_REGISTER_PLUGIN(QCopConnection)

QCopConnection::QCopConnection( QObject *parent )
    : QDConPlugin( parent ),
    mSocket( 0 ), mDevice( 0 )
{
    mDevice = 0;

    QCopChannel *deviceChan = new QCopChannel( "QPE/*", this );
    connect( deviceChan, SIGNAL(received(QString,QByteArray)),
             this, SLOT(deviceMessage(QString,QByteArray)) );
}

QCopConnection::~QCopConnection()
{
}

void QCopConnection::init()
{
    DesktopSettings settings(id());

    // Backup property values
    QMap<QString,QVariant> save;
    QStringList properties = QStringList()
            << "port"
            << "display"
            ;
    foreach ( const QString &property, properties )
        save[property] = settings.value(QString("properties/%1/value").arg(property));

    // Clear and initialize the properties structure
    settings.remove( "properties" );
    // we care about the ordering of the properties
    settings.setValue( "/properties/order", properties );

    settings.beginGroup( "properties/port" );
    settings.setValue( "label", tr("Port") );
    settings.setValue( "default", "4245" );
    settings.setValue( "editor", "QLineEdit" );
    settings.setValue( "helptext", tr("Enter the base port that Qtopia is listening on.") );
    settings.endGroup();

    settings.beginGroup( "properties/display" );
    settings.setValue( "label", tr("Display", "the display") );
    settings.setValue( "default", "0" );
    settings.setValue( "editor", "QLineEdit" );
    settings.setValue( "helptext", tr("Enter the display number that Qtopia is running on.") );
    settings.endGroup();

    // Restore property values
    foreach( const QString &property, save.keys() ) {
        QVariant value = save[property];
        if ( !value.isValid() )
            value = settings.value(QString("properties/%1/default").arg(property));
        settings.setValue(QString("properties/%1/value").arg(property), value);
    }
}

bool QCopConnection::tryConnect( QDLinkPlugin *link )
{
    TRACE(QDCon) << "QCopConnection::tryConnect" << link->displayName();

    Q_ASSERT( !mSocket );

    mSocket = link->socket();

    if ( mSocket ) {
        connect( mSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
        connect( mSocket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
        emit setState( this, QDConPlugin::Connecting );
        // just in case anything arrived before we got our slot connected
        QTimer::singleShot( 0, this, SLOT(slotReadyRead()) );
        return true;
    }
    return false;
}

void QCopConnection::pluginMessage( const QString &msg, const QByteArray &data )
{
    TRACE(QDLink) << "QCopConnection::pluginMessage" << msg;
    QDataStream ds(data);
    if ( msg == "propertyChanged(QString)" ) {
        QString property;
        ds >> property;
        LOG() << id() << "property" << property << "has changed";
        if ( property == "port" || property == "display" )
            slotDisconnected();
    }
}

int QCopConnection::port()
{
    DesktopSettings settings(id());
    return settings.value("/properties/port/value").toInt() + settings.value("/properties/display/value").toInt();
}

void QCopConnection::stop()
{
    TRACE(QDCon) << "QCopConnection::stop";
    if ( mSocket )
        QTimer::singleShot( 0, this, SLOT(slotDisconnected()) );
}

void QCopConnection::slotDisconnected()
{
    TRACE(QDCon) << "QCopConnection::slotDisconnected";
    if ( mDevice ) {
        mDevice->disassociate( this );
        if ( mDevice->locked() ) {
            QCopEnvelope e( "QD/Connection", "setHint(QString,QString)" );
            e << tr("Do not disconnect the device while it is busy")
              << QString("busy icon");
        }
    }
    mDevice = 0;
    properties.clear();
    mSocket = 0;
    emit setState(this, QDConPlugin::Disconnected);
}

void QCopConnection::sendMessage( const QString &channel, const QString &message, const QStringList &args )
{
    if ( !mSocket )
        return;
    TRACE(QDCon) << "QCopConnection::sendMessage";// << "channel" << channel << "message" << message << "args" << args;
    QTextStream os( mSocket );
    os << "CALL " << channel.toLatin1().constData() << " " << message.toLatin1().constData() << " ";
    foreach ( QString arg, args ) {
        arg.replace( QRegExp( "&" ), "&amp;" );
        arg.replace( QRegExp( " " ), "&0x20;" );
        arg.replace( QRegExp( "\n" ), "&0x0d;" );
        arg.replace( QRegExp( "\r" ), "&0x0a;" );
        os << arg << " ";
    }
    os << endl;
}

void QCopConnection::sendMessage( const QString &channel, const QString &message, const QByteArray &data )
{
    if ( !mSocket )
        return;
    TRACE(QDCon) << "QCopConnection::sendMessage";// << "channel" << channel << "message" << message << "data" << data;
    QTextStream os( mSocket );
    os << "CALLB " << channel.toLatin1().constData() << " " << message.toLatin1().constData() << " " << data.toBase64() << endl;
}

void QCopConnection::slotReadyRead()
{
    TRACE(QDCon) << "QCopConnection::slotReadyRead";
    // The connection can be closed from in the loop
    while ( mSocket && mSocket->canReadLine() )
        process( mSocket->readLine() );
}

void QCopConnection::process( const QString &message )
{
    TRACE(QDCon) << "QCopConnection::process" << "message" << message.trimmed();
    if ( message.startsWith( "CALL " ) ) {
        // qcop call
        processQCopCall( message, false );
        return;
    } else if ( message.startsWith( "CALLB " ) ) {
        // qcop call
        processQCopCall( message, true );
        return;
    } else if ( message.startsWith( "599" ) ) {
        QStringList list = message.split( " " );
        WARNING() << "QCop channel" << list[2] << "is not registered";
        //emit channelNotRegistered( list[2] );
        return;
    }

    if ( message.startsWith( "220" ) ) {
        // 1.0-1.5 - 220 Qtopia QCop bridge ready!
        // 1.6-2.x - 220 Qtopia 2.1.1;challenge={96020319-e076-954f-bf78-25b615c682c5};loginname=user;displayname=;
        // 4.x     - 220 Qtopia 4.x.x;challenge={96020319-e076-954f-bf78-25b615c682c5};loginname=user;displayname=;protocol=2;system=Qtopia;model=Greenphone Device;hexversion=0x040300
        properties["protocol"] = "1";

        int afterSystem = message.indexOf(' ',5);
        properties["system"] = message.mid(4,afterSystem-4);

        QString message2 = message.mid(afterSystem+1);
        if ( message2.startsWith("QCop") ) {
            properties["version"] = "1.5";
            // This is a Qtopia 1.5 device
            if ( DesktopSettings::debugMode() ) {
                DesktopSettings settings(id());
                settings.beginGroup( "properties/username" );
                properties["loginname"] = settings.value("value", settings.value("default")).toString();
            }
        } else {
            int argIndex = message2.indexOf(';');
            QString version = message2.left(argIndex);
            LOG() << "connected to a version" << version << "device";
            properties["version"] = version;
            QStringList nvl = message2.mid(argIndex + 1).trimmed().split( ';', QString::SkipEmptyParts );
            foreach ( const QString &arg, nvl ) {
                int eq = arg.indexOf('=');
                if ( eq > 0 ) {
                    QString key = arg.left(eq);
                    QString value = arg.mid(eq + 1);
                    properties[key] = value;
                }
            }
        }

        LOG() << "Welcome banner identifies the device as" << properties["system"] << properties["version"] << "using protocol" << properties["protocol"];
        QTextStream os( mSocket );
        LOG() << "WRITE:" << "USER " << properties["loginname"];
        os << "USER " << properties["loginname"] << endl;
    } else if ( message.startsWith( "331" ) ) {
        // We have a request for a password
        // Qtopia 1.0-1.5 devices have no password...
        if ( !properties["challenge"].isEmpty() ) {
            DesktopSettings settings("devicepasswords");
            properties["password"] = settings.value(properties["challenge"]).toString();
            if ( properties["password"].isEmpty() ) {
                properties["password"] = QUuid::createUuid();
                // TODO how to document this stuff?
                QCopEnvelope h( "QD/Connection", "setHint(QString,QString)" );
                h << tr("You must allow Qtopia Sync Agent to connect")
                  << QString("device security");
            }
        }
        QTextStream os( mSocket );
        LOG() << "WRITE:" << "PASS " << "Qtopia15" << properties["password"];
        os << "PASS " << "Qtopia15" << properties["password"] << endl;
    } else if ( message.startsWith( "530" ) ) {
        // Bad password. Stop until the user says to go again.
        stop();
        QCopEnvelope e( "QD/Connection", "setHint(QString,QString)" );
        e << tr("Qtopia Sync Agent was denied access to the device.")
          << QString("device security");
    } else if ( message.startsWith( "230" ) ) {
        DesktopSettings settings("devicepasswords");
        settings.setValue(properties["challenge"], properties["password"]);
        emit setState( this, QDConPlugin::Matching );
    }
}

void QCopConnection::processQCopCall( const QString &message, bool binary )
{
    TRACE(QDCon) << "QCopConnection::processQCopCall";
    QStringList list = message.split( " " );
    if ( list.count() < 2 )
        return;
    list.pop_front(); // remove "CALL[B]"
    list.pop_front(); // remove "QPE/Desktop"
    QString method = list[0];
    list.pop_front(); // remove method
    QByteArray data;
    if ( binary ) {
        if ( list.count() )
            data = QByteArray::fromBase64( list.join("").toLocal8Bit() );
    } else {
        // Emulate old behaviour
        int p=method.indexOf('(')+1;
        int pend;
        QDataStream stream(&data,QIODevice::WriteOnly);
        for ( QStringList::Iterator it = list.begin();
                (pend=method.indexOf(QRegExp("[,)]"),p))>0 || it != list.end();
                ++it ) {
            QString a;
            if ( it != list.end() ) {
                QString ea=*it;
                for (int i=0; i < (int)ea.length(); ++i) {
                    if ( ea[i] == '&' ) {
                        if ( ea[i+1]=='a' ) {
                            i+=4;
                            a.append('&');
                        } else if ( ea[i+3]=='2' ) {
                            i+=5;
                            a.append(' ');
                        } else if ( ea[i+4]=='d' ) {
                            i+=5;
                            a.append('\n');
                        } else {
                            i+=5;
                            a.append('\r');
                        }
                    } else {
                        a.append(ea[i]);
                    }
                }
            }
            if ( pend > 0 ) {
                QString type = method.mid(p,pend-p);
                if ( type == "QString" ) {
                    stream << a;
                } else if ( type == "int" || type == "bool" ) {
                    stream << a.toInt();
                } else {
                    WARNING() << "Device is running new application on old Qtopia system." << endl
                              << "Old Qtopia system does not support type" << type << ".";
                    stream << a;
                }
            }
            p = pend+1;
        }
    }

    //LOG() << "received QCop message" << method;
    // Don't block now, do this some time later (once per incoming message)
    QCopEmitter *e = new QCopEmitter( method, data );
    connect( e, SIGNAL(receivedMessage(QString,QByteArray)), this, SIGNAL(receivedMessage(QString,QByteArray)) );
    QTimer::singleShot( 0, e, SLOT(doEmit()) );
}

QString QCopConnection::conProperty( const QString &key )
{
    return properties[key];
}

bool QCopConnection::claim( QDDevPlugin *dev )
{
    if ( mDevice )
        return false;
    mDevice = dev;
    return true;
}

void QCopConnection::connected( QDDevPlugin *dev )
{
    TRACE(QDCon) << "QCopConnection::connected";
    Q_ASSERT( dev == mDevice );
    emit setState( this, QDConPlugin::Connected );
}

QDDevPlugin *QCopConnection::device()
{
    return mDevice;
}

void QCopConnection::deviceMessage( const QString &message, const QByteArray &data )
{
    if ( message == "forwardedMessage(QString,QString,QByteArray)" ) {
        sendMessage( "QPE/QDSync", message, data );
    }
}

#include "qcopconnection.moc"
