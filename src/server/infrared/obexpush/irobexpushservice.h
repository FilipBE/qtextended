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

#ifndef IROBEXPUSHSERVICE_H
#define IROBEXPUSHSERVICE_H

#include "filetransfertask.h"
#include "qtopiaserverapplication.h"
#include <QObject>

class IrObexPushServicePrivate;

class IrObexPushService : public FileTransferTask
{
    Q_OBJECT
public:
    IrObexPushService(QObject *parent = 0);

    virtual QContentId transferContentId(int id) const;

public slots:
    virtual void abortTransfer(int id);

signals:
    void incomingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void outgoingTransferStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void transferProgress(int id, qint64 bytes, qint64 total);
    void transferFinished(int id, bool error, bool aborted);

private:
    IrObexPushServicePrivate *d;
    friend class IrObexPushServicePrivate;
};

#endif
