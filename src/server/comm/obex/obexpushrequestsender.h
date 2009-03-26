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

#ifndef OBEXPUSHREQEUSTSENDER_H
#define OBEXPUSHREQEUSTSENDER_H

#include <QObject>
#include <qcontent.h>

class SessionedObexClient;
class QContact;
class QDSActionRequest;
class ObexPushRequestSenderPrivate;

class ObexPushRequestSender : public QObject
{
    Q_OBJECT
public:
    ObexPushRequestSender(QObject *parent = 0);

    void sendFile(SessionedObexClient *client,
                  int id,
                  const QString &fileName,
                  const QString &mimeType,
                  const QString &description,
                  bool autoDelete);

    void sendContent(SessionedObexClient *client, int id,
                const QContentId &contentId);

    void sendPersonalBusinessCard(SessionedObexClient *client, int id);

    void sendBusinessCard(SessionedObexClient *client, int id,
                const QContact &contact);

    void sendBusinessCard(SessionedObexClient *client, int id,
                const QDSActionRequest &actionRequest);

    void sendCalendar(SessionedObexClient *client, int id,
                const QDSActionRequest &actionRequest);

    bool abortRequest(int id);

    QContentId requestContentId(int id) const;

signals:
    void requestStarted(int id, const QString &name,
            const QString &mimeType, const QString &description);
    void requestProgress(int id, qint64 bytes, qint64 total);
    void requestFinished(int id, bool error, bool aborted);

protected:
    virtual QIODevice *createDevice(const QContentId &contentId) const;
    virtual QIODevice *createDevice(const QContact &contact) const;
    virtual QFile *createFile(const QString &filePath) const;
    QContact ownerBusinessCard(bool *hasBusinessCard) const;

private:
    ObexPushRequestSenderPrivate *d;
    friend class ObexPushRequestSenderPrivate;
};

#endif
