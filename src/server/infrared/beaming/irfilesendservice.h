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

#ifndef IRFILESENDSERVICE_H
#define IRFILESENDSERVICE_H

#include "filetransfertask.h"
#include <qtopiaabstractservice.h>
#include <qcontent.h>
#include <QObject>

class ObexPushRequestSender;
class QDSActionRequest;
class QContact;
class IrBeamingServicePrivate;
class IrFileSendServicePrivate;

class IrFileSendService : public FileTransferTask
{
    Q_OBJECT
public:
    IrFileSendService(QObject *parent = 0);
    ~IrFileSendService();

    virtual QContentId transferContentId(int id) const;

public slots:
    virtual void abortTransfer(int id);

signals:
    void outgoingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void transferProgress(int id, qint64 bytes, qint64 total);
    void transferFinished(int id, bool error, bool aborted);

private:
    IrFileSendServicePrivate *d;
    friend class IrFileSendServicePrivate;
    friend class IrBeamingService;
};


class IrBeamingService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    IrBeamingService(ObexPushRequestSender *requestSender,
                     QObject *parent = 0);
    ~IrBeamingService();

public slots:
    void beamPersonalBusinessCard();
    void beamBusinessCard(const QContact &contact);
    void beamBusinessCard(const QDSActionRequest& request);

    void beamFile(const QString &filePath, const QString &mimeType,
            const QString &description, bool autoDelete);
    void beamFile(const QContentId &contentId);

    void beamCalendar(const QDSActionRequest &request);

private:
    IrBeamingServicePrivate *d;
};

#endif
