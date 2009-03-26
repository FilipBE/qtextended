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

#ifndef MMSMESSAGE_H
#define MMSMESSAGE_H

#include <qwsppdu.h>
#include <qstring.h>

#include <qtopiaglobal.h>
#include <QMailMessageId>

class MMSMessage
{
public:
    MMSMessage();

    enum Type { Invalid=0, MSendReq=128, MSendConf=129, MNotificationInd=130,
                MNotifyResp=131, MRetrieveConf=132, MAckowledgeInd=133,
                MDeliveryInd=134 };

    bool decode(QIODevice *d);
    bool encode(QIODevice *d);

    Type type() const;
    void setType(Type t);

    QString txnId() const;
    void setTxnId(const QString& id);

    QMailMessageId messageId() const;
    void setMessageId(const QMailMessageId& id);

    const QList<QWspField> &headers() const { return fields; }
    const QWspField *field(const QString &name) const;
    void addField(const QString &name, const QString &value);
    void addField(const QString &name, quint32 value);
    void removeField(const QString &name);

    bool multipartRelated() const;

    int messagePartCount() const { return multiPartData.count(); }
    const QWspPart &messagePart(int idx) const;
    void addMessagePart(const QWspPart &part);

    const QString &errorMessage() const { return err; }

private:
    bool encodeSendRequest(QWspPduEncoder &);
    bool encodeNotificationInd(QWspPduEncoder &enc);
    bool encodeNotifyInd(QWspPduEncoder &enc);
    bool encodeAcknowledgeInd(QWspPduEncoder &enc);

private:
    QList<QWspField> fields;
    QWspMultipart multiPartData;
    QString err;
    QMailMessageId msgId;
};

#endif
