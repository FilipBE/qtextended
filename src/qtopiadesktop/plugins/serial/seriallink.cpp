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
#include <qcopenvelope_qd.h>
#include <serialport.h>

#include <QTimer>
#include <QFileInfo>

#ifndef Q_OS_UNIX
#include "regthread.h"
#endif

class SerialLink : public QDLinkPlugin
{
    Q_OBJECT
public:
    SerialLink( QObject *parent = 0 );
    ~SerialLink();

    // QDPlugin
    QString id() { return "com.trolltech.plugin.link.serial"; }
    QString displayName() { return tr("Serial Link"); }
    void init();

    // QDLinkPlugin
    bool tryConnect( QDConPlugin *con );
    void stop();
    int ping_interval();
    bool send_ack();

private slots:
    void slotConnected();
    void slotDisconnected();
    void pluginMessage( const QString &msg, const QByteArray &data );
    void setHelpText();

private:
    SerialPort *mSocket;
#ifndef Q_OS_UNIX
    static RegThread *mRegMonitor;
    static QMutex regmutex;
    static int instances;
    bool connected;
#endif
};

QD_REGISTER_PLUGIN(SerialLink)

#ifndef Q_OS_UNIX
RegThread *SerialLink::mRegMonitor = 0;
QMutex SerialLink::regmutex;
int SerialLink::instances = 0;
#endif

SerialLink::SerialLink( QObject *parent )
    : QDLinkPlugin( parent )
#ifndef Q_OS_UNIX
    , connected( false )
#endif
    , mSocket( 0 )
{
#ifndef Q_OS_UNIX
    regmutex.lock();
    instances++;
    regmutex.unlock();
#endif
}

SerialLink::~SerialLink()
{
    TRACE(TRACE) << "SerialLink::~SerialLink";
    if ( mSocket ) {
        mSocket->deleteLater();
        mSocket = 0;
    }
#ifndef Q_OS_UNIX
    regmutex.lock();
    instances--;
    if ( !instances && mRegMonitor ) {
        SetEvent( mRegMonitor->quit );
        mRegMonitor->wait();
        delete mRegMonitor;
    }
    regmutex.unlock();
#endif
}

void SerialLink::init()
{
    DesktopSettings settings(id());

    // Backup property values
    QMap<QString,QVariant> save;
    QStringList properties = QStringList()
            << "port"
            << "greenphone"
            ;
    foreach ( const QString &property, properties )
        save[property] = settings.value(QString("properties/%1/value").arg(property));

    // Clear and initialize the properties structure
    settings.remove( "properties" );
    // we care about the ordering of the properties
    settings.setValue( "/properties/order", properties );

    settings.beginGroup( "properties/port" );
    settings.setValue( "label", tr("COM Port") );
#ifdef Q_OS_UNIX
    settings.setValue( "default", "/dev/ttyS0" );
#else
    settings.setValue( "default", "COM20" );
#endif
    settings.setValue( "editor", "QLineEdit" );
    settings.endGroup();
    setHelpText();

#ifndef Q_OS_UNIX
    settings.beginGroup( "properties/greenphone" );
    settings.setValue( "label", tr("Greenphone") );
    settings.setValue( "default", true );
    settings.setValue( "editor", "QCheckBox" );
    settings.setValue( "helptext", tr("Is the device a Greenphone?") );
    settings.endGroup();
#endif

    // Restore property values
    foreach( const QString &property, save.keys() ) {
        QVariant value = save[property];
        if ( !value.isValid() )
            value = settings.value(QString("properties/%1/default").arg(property));
        settings.setValue(QString("properties/%1/value").arg(property), value);
    }
}

void SerialLink::pluginMessage( const QString &msg, const QByteArray &data )
{
    TRACE(QDLink) << "SerialLink::pluginMessage" << msg;
    QDataStream ds(data);
    if ( msg == "propertyChanged(QString)" ) {
        QString property;
        ds >> property;
        LOG() << id() << "property" << property << "has changed";
        if ( property == "port" || property == "greenphone" )
            slotDisconnected();
    }
}

bool SerialLink::tryConnect( QDConPlugin *con )
{
    TRACE(QDLink) << "SerialLink::tryConnect" << con->displayName();

    DesktopSettings settings(id());
    QString comport = settings.value("properties/port/value").toString();

#ifdef Q_OS_UNIX
    if ( ! QFileInfo(comport).exists() ) {
        WARNING() << "Can't connect to" << comport << "because it does not exist";
        return false;
    }
#else
    if ( QRegExp("^COM\\d+$").indexIn(comport) != 0 ) {
        WARNING() << "Can't connect to" << comport << "because it is not COM1, COM2, etc...";
        return false;
    }
#endif
    LOG() << "Trying to connect to" << comport;
    mSocket = new SerialPort( comport );
    bool greenphone = false;
#ifndef Q_OS_UNIX
    greenphone = settings.value("properties/greenphone/value").toBool();
    if ( greenphone )
        mSocket->setBrokenSerial();
#endif
    connect( mSocket, SIGNAL(newConnection()), this, SLOT(slotConnected()) );
    connect( mSocket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
    if ( mSocket->open( QIODevice::ReadWrite ) ) {
        if ( greenphone )
            // Start the ping immediately... the port doesn't exist when there's nobody there
            setupSocket( mSocket );
        return true;
    } else {
        LOG() << "Could not open socket";
        delete mSocket;
        mSocket = 0;
        return false;
    }
}

void SerialLink::stop()
{
    if ( mSocket ) {
        mSocket->deleteLater();
        mSocket = 0;
    }
}

int SerialLink::ping_interval()
{
    bool greenphone = false;
#ifndef Q_OS_UNIX
    DesktopSettings settings(id());
    greenphone = settings.value("properties/greenphone/value").toBool();
#endif
    if ( greenphone )
        return 1000;
    else
        return QDLinkPlugin::ping_interval();
}

bool SerialLink::send_ack()
{
    bool greenphone = false;
#ifndef Q_OS_UNIX
    DesktopSettings settings(id());
    greenphone = settings.value("properties/greenphone/value").toBool();
#endif
    if ( greenphone )
        return true;
    else
        return QDLinkPlugin::send_ack();
}

void SerialLink::slotConnected()
{
    TRACE(QDLink) << "SerialLink::slotConnected";
    bool greenphone = false;
#ifndef Q_OS_UNIX
    DesktopSettings settings(id());
    greenphone = settings.value("properties/greenphone/value").toBool();
#endif
    if ( !greenphone )
        // Don't start the ping until we're connected... why send a message when there's nobody there?
        setupSocket( mSocket );
    emit setState(this, QDLinkPlugin::Up);
}

void SerialLink::slotDisconnected()
{
    TRACE(QDLink) << "SerialLink::slotDisconnected";
    emit setState(this, QDLinkPlugin::Down);
}

void SerialLink::setHelpText()
{
    TRACE(QDLink) << "SerialLink::setHelpText";
    QString help = tr("Enter the port that the Qtopia device is connected to.");
#ifndef Q_OS_UNIX
    regmutex.lock();
    if ( !mRegMonitor ) {
        mRegMonitor = new RegThread;
        mRegMonitor->start();
    }
    regmutex.unlock();
    if ( !connected ) {
        connected = true;
        connect( mRegMonitor, SIGNAL(comPortsChanged()), this, SLOT(setHelpText()) );
    }
    QStringList ports = mRegMonitor->ports();
    if ( ports.count() ) {
        help += QString("<br>%1%2%3%4").arg(tr("Available ports are:")).arg("<ul><li>").arg(ports.join("<li>")).arg("</ul>");
        LOG() << "helptext" << help;
    } else {
        LOG() << "NO COM PORTS";
    }
#endif
    DesktopSettings settings( id() );
    settings.beginGroup( "properties/port" );
    settings.setValue( "helptext", help );
    settings.endGroup();
    QCopEnvelope e("QD/Properties", "helptextChanged(QString,QString)");
    e << id() << QString("port");
}

#include "seriallink.moc"
