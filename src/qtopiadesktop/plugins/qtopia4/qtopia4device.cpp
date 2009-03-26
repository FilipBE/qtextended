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
#include <qtopiadesktoplog.h>
#include <qcopchannel_qd.h>
#include <trace.h>
#include <QVariant>

class Qtopia4Device : public QDDevPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(Qtopia4Device,QDDevPlugin)
public:
    // QDPlugin
    QString id() { return "com.trolltech.plugin.dev.qtopia4"; }
    QString displayName() { return tr("Qtopia 4.3 Device"); }

    QDConPlugin *connection() { return mConnection; }
    QString model() { return mModel; }
    QString system() { return mSystem; }
    quint32 version() { return mVersion; }
    QString versionString() { return mVersionString; }
    QPixmap icon() { return QPixmap(":image/appicon"); }
    int port() { return 4243; }

    // Doers

    void probe( QDConPlugin *con );
    void disassociate( QDConPlugin *con );

    void requestCardInfo() {}
    void requestInstallLocations() {}
    void requestAllDocLinks() {}
    void installPackage() {}
    void removePackage() {}
    void setSyncDate() {}

private slots:
    void message( const QString &message, const QByteArray &data );

private:
    QDConPlugin *mConnection;
    QString mModel;
    QString mSystem;
    quint32 mVersion;
    QString mVersionString;
};

QD_REGISTER_PLUGIN(Qtopia4Device)

void Qtopia4Device::probe( QDConPlugin *con )
{
    if ( con->conProperty("system") == "Qtopia" && con->conProperty("protocol") == "2" ) {
        if ( con->claim( this ) ) {
            mModel = con->conProperty("model");
            mSystem = con->conProperty("system");
            mVersionString = con->conProperty("version");
            mVersion = QVariant(con->conProperty("hexversion")).toUInt();
            mConnection = con;
            connect( mConnection, SIGNAL(receivedMessage(QString,QByteArray)),
                    this, SLOT(message(QString,QByteArray)) );
            mConnection->connected( this );
        }
    }
}

void Qtopia4Device::disassociate( QDConPlugin *con )
{
    Q_ASSERT( mConnection == con );
    disconnect( mConnection, SIGNAL(receivedMessage(QString,QByteArray)),
            this, SLOT(message(QString,QByteArray)) );
    mConnection = 0;
}

void Qtopia4Device::message( const QString &message, const QByteArray &data )
{
    QDataStream stream( data );
    if ( message == "forwardedMessage(QString,QString,QByteArray)" ) {
        QString _channel;
        QString _message;
        QByteArray _data;
        stream >> _channel >> _message >> _data;
        TRACE(QDDev) << "Qtopia4Device::message" << "channel" << _channel << "message" << _message << "data" << _data;
        QCopChannel::send( _channel, _message, _data );
    }
}

#include "qtopia4device.moc"
