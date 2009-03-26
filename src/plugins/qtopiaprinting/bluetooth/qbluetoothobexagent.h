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

#ifndef QBLUETOOTHOBEXAGENT_H
#define QBLUETOOTHOBEXAGENT_H

#include <qbluetoothremotedevice.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothsdpquery.h>
#include <qcontent.h>

class QBluetoothObexAgentPrivate;

class QBluetoothObexAgent : public QObject
{
    Q_OBJECT
public:
    QBluetoothObexAgent( const QBluetoothRemoteDevice &remoteDevice,
            QBluetooth::SDPProfile profile = QBluetooth::ObjectPushProfile,
            QObject *parent = 0 );
    ~QBluetoothObexAgent();

    void send( const QString &fileName, const QString &mimeType = QString() );
    void send( const QContent &content );
    void send( const QByteArray &array, const QString &fileName, const QString &mimeType = QString() );
    void sendHtml( const QString &html );

    void setAutoDelete( const bool autoDelete );

public slots:
    void abort();

signals:
    void done( bool error );

private slots:
    void searchComplete( const QBluetoothSdpQueryResult &result );
    void progress( qint64, qint64 );

private:
    void startSearch();
    bool inProgress();

    QBluetoothObexAgentPrivate *d;
};

#endif
