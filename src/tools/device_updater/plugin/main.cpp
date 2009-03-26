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

#include <qtopiaglobal.h>
#include <qcopchannel_qd.h>
#include <QtopiaIpcEnvelope>

class device_updaterPlugin : public QObject
{
    Q_OBJECT
public:
    device_updaterPlugin( QObject *parent = 0 )
        : QObject( parent )
    {
        QCopChannel *chan = new QCopChannel( "QPE/Application/packagemanager", this );
        connect( chan, SIGNAL(received(QString,QByteArray)), this, SLOT(received(QString,QByteArray)) );
    }

    ~device_updaterPlugin()
    {
    }

private slots:
    void received( const QString &message, const QByteArray &data )
    {
        if ( message == "PackageManager::installPackageConfirm(QString)" ) {
            QString file;
            QDataStream stream( data );
            stream >> file;
            // No worries about circular delivery because this does not talk to the "local" QCop classes
            QtopiaIpcEnvelope e("QPE/Application/packagemanager", "PackageManager::installPackageConfirm(QString)");
            e << file;
        }
    }

};

QTOPIA_EXPORT_PLUGIN(device_updaterPlugin)

#include "main.moc"
