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

#ifndef DEFAULTOBEXPUSHSERVICE_H
#define DEFAULTOBEXPUSHSERVICE_H

#include <QObject>
#include <qobexpushservice.h>
#include <qcontent.h>

class QIODevice;
class DefaultObexPushServicePrivate;
class DefaultObexPushServiceRunnerPrivate;


class DefaultObexPushService : public QObexPushService
{
    Q_OBJECT
public:
    DefaultObexPushService(QIODevice *socket, QObject *parent = 0);
    ~DefaultObexPushService();

    bool finalizeDataTransfer();
    void cleanUpRequest();

    QContentId currentContentId() const;
    QByteArray businessCard() const;

protected:
    virtual QIODevice *acceptFile(const QString &name,
            const QString &type, qint64 size, const QString &description);

private:
    DefaultObexPushServicePrivate *d;
    friend class DefaultObexPushServicePrivate;
};

#endif
