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

#ifndef BLUETOOTHFILESENDSERVICE_H
#define BLUETOOTHFILESENDSERVICE_H

#include "filetransfertask.h"
#include <qtopiaabstractservice.h>
#include <qcontent.h>
#include <QObject>

class ObexPushRequestSender;
class QDSActionRequest;
class QContact;
class QBluetoothAddress;
class BluetoothPushingServicePrivate;
class BluetoothFileSendServicePrivate;


class BluetoothFileSendService : public FileTransferTask
{
    Q_OBJECT
public:
    BluetoothFileSendService(QObject *parent = 0);
    ~BluetoothFileSendService();

    virtual QContentId transferContentId(int id) const;

public slots:
    virtual void abortTransfer(int id);

signals:
    void outgoingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void transferProgress(int id, qint64 bytes, qint64 total);
    void transferFinished(int id, bool error, bool aborted);

private:
    BluetoothFileSendServicePrivate *d;
    friend class BluetoothFileSendServicePrivate;
    friend class BluetoothPushingService;
};


class BluetoothPushingService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    BluetoothPushingService(ObexPushRequestSender *requestSender,
                            QObject *parent = 0);
    ~BluetoothPushingService();

public slots:
    void pushPersonalBusinessCard();
    void pushPersonalBusinessCard(const QBluetoothAddress &address);
    void pushBusinessCard(const QContact &contact);
    void pushBusinessCard(const QDSActionRequest& request);

    void pushFile(const QString &fileName, const QString &mimeType,
            const QString &description, bool autoDelete);
    void pushFile(const QContentId &contentId);
    void pushFile(const QBluetoothAddress &address, const QContentId &contentId);

    void pushCalendar(const QDSActionRequest &request);

private:
    BluetoothPushingServicePrivate *d;
};

#endif
