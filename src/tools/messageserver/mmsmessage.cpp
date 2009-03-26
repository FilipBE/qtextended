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

#include "mmsmessage.h"
#include <qtopiaapplication.h>
#include <qdatetime.h>

// Reference: OMA-MMS-ENC
//            Multimedia Messaging Service
//            Encapsulation Protocol
//
// Warning: this class is subject to change.


struct ValueMap;

class MMSHeaderCodec : public QWspHeaderCodec
{
public:
    enum Type { TextString, EncodedString, TokenText, Priority,
                Date, LongInteger, Version, MessageType, Status,
                ContentType, Uri, From, MessageClass, Expiry, YesNo,
                SenderVisibility, DeliveryTime, ResponseStatus };

    virtual QWspField decode(QWspPduDecoder &);
    virtual bool encode(QWspPduEncoder &, const QWspField &);
    virtual quint8 codePage() const { return 0xFF; }

private:
    QString decodeFrom(QWspPduDecoder &dec);
    QWspField decodeEnumerated(QWspPduDecoder &dec, const ValueMap *, const QString &name);
    QWspField decodeMessageClass(QWspPduDecoder &dec);
    QWspField decodeExpiry(QWspPduDecoder &dec);
    bool encodeFrom(QWspPduEncoder &enc, const QString &);
    bool encodeEnumerated(QWspPduEncoder &enc, const ValueMap *, const QWspField &);
    bool encodeMessageClass(QWspPduEncoder &enc, const QString &);
    bool encodeExpiry(QWspPduEncoder &enc, const QWspField &);
    int fieldFromNumber(quint8 id);
    int numberFromField(const QString &str);
};

//===========================================================================

struct HeaderField {
    quint8 number;
    const char *name;
    MMSHeaderCodec::Type type;
};

static const HeaderField headerFields[] = {
    { 0x01, "Bcc", MMSHeaderCodec::EncodedString },
    { 0x02, "Cc", MMSHeaderCodec::EncodedString },
    { 0x03, "X-Mms-Content-Location", MMSHeaderCodec::Uri },
    { 0x04, "Content-Type", MMSHeaderCodec::ContentType },
    { 0x05, "Date", MMSHeaderCodec::Date },
    { 0x06, "X-Mms-Delivery-Report", MMSHeaderCodec::YesNo },
    { 0x07, "X-Mms-Delivery-Time", MMSHeaderCodec::DeliveryTime },
    { 0x08, "X-Mms-Expiry", MMSHeaderCodec::Expiry },
    { 0x09, "From", MMSHeaderCodec::From },
    { 0x0A, "X-Mms-Message-Class", MMSHeaderCodec::MessageClass },
    { 0x0B, "Message-ID", MMSHeaderCodec::TextString },
    { 0x0C, "X-Mms-Message-Type", MMSHeaderCodec::MessageType },
    { 0x0D, "X-Mms-MMS-Version", MMSHeaderCodec::Version },
    { 0x0E, "X-Mms-Message-Size", MMSHeaderCodec::LongInteger },
    { 0x0F, "X-Mms-Priority", MMSHeaderCodec::Priority },
    { 0x10, "X-Mms-Read-Reply", MMSHeaderCodec::YesNo },
    { 0x11, "X-Mms-Report-Allowed", MMSHeaderCodec::YesNo },
    { 0x12, "X-Mms-Response-Status", MMSHeaderCodec::ResponseStatus },
    { 0x13, "X-Mms-Response-Text", MMSHeaderCodec::EncodedString },
    { 0x14, "X-Mms-Sender-Visibility", MMSHeaderCodec::SenderVisibility },
    { 0x15, "X-Mms-Status", MMSHeaderCodec::Status },
    { 0x16, "Subject", MMSHeaderCodec::EncodedString },
    { 0x17, "To", MMSHeaderCodec::EncodedString },
    { 0x18, "X-Mms-Transaction-Id", MMSHeaderCodec::TextString },
    { 0x00, "", MMSHeaderCodec::TextString }
};

struct ValueMap {
    int number;
    const char *string;
};

static const ValueMap messageTypes[] = {
    { 128, "m-send-req" },
    { 129, "m-send-conf" },
    { 130, "m-notification-ind" },
    { 131, "m-notifyresp-ind" },
    { 132, "m-retrieve-conf" },
    { 133, "m-acknowledge-ind" },
    { 134, "m-delivery-ind" },
    { -1, 0 }
};

static const ValueMap messageClassTypes[] = {
    { 128, "Personal" },
    { 129, "Advertisement" },
    { 130, "Informational" },
    { 131, "Auto" },
    { -1, 0 }
};

static const ValueMap priorityTypes[] = {
    { 128, "Low" },
    { 129, "Normal" },
    { 130, "High" },
    { -1, 0 }
};

static const ValueMap statusTypes[] = {
    { 128, "Expired" },
    { 129, "Retrieved" },
    { 130, "Rejected" },
    { 131, "Deferred" },
    { 132, "Unrecognised" },
    { -1, 0 }
};

static const ValueMap responseStatusTypes[] = {
    { 128, "Ok" },
    { 129, "Error-unspecified" },
    { 130, "Error-service-denied" },
    { 131, "Error-message-format-corrupt" },
    { 132, "Error-sending-address-unresolved" },
    { 133, "Error-message-not-found" },
    { 134, "Error-network-problem" },
    { 135, "Error-content-not-accepted" },
    { 136, "Error-unsupported-message" },
    { -1, 0 }
};

static const ValueMap senderVisibilityTypes[] = {
    { 128, "Hide" },
    { 129, "Show" },
    { -1, 0 }
};

static const ValueMap yesNoTypes[] = {
    { 128, "Yes" },
    { 129, "No" },
    { -1, 0 }
};

static int mapToNumber(const ValueMap *map, const QString &str)
{
    if (str.isEmpty())
        return -1;
    int idx = 0;
    while (map[idx].string) {
        if (map[idx].string == str)
            break;
        idx++;
    }

    return map[idx].number;
}

static const char *mapToString(const ValueMap *map, int number)
{
    int idx = 0;
    while (map[idx].string) {
        if (map[idx].number == number)
            break;
        idx++;
    }

    return map[idx].string;
}

//===========================================================================

MMSMessage::MMSMessage()
{
}

bool MMSMessage::decode(QIODevice *dev)
{
    bool ok = true;
    QWspPduDecoder dec(dev);

    // MMS headers not encoded using code page, but we can still use the
    // mechanism.
    MMSHeaderCodec *mmsCodec = new MMSHeaderCodec;
    dec.setHeaderCodec(mmsCodec);

    while (!dev->atEnd()) {
        quint8 octet = dec.peekOctet();
        if (octet >= 32) {
            QWspField field = dec.decodeField();
            fields.append(field);
        } else {
            if (type() == MSendReq || type() == MRetrieveConf) {
                const QWspField *f = field("Content-Type");
                if (f && (f->value.indexOf("application/vnd.wap.multipart.related") == 0
                        || f->value.indexOf("application/vnd.wap.multipart.mixed") == 0)) {
                    dec.setHeaderCodec(0);
                    multiPartData = dec.decodeMultipart();
                    continue;
                }
            }
            // ### handle other types
            qWarning("Don't understand format: %d", octet);
            ok = false;
            break;
        }
    }

    if (dec.status() != QWspPduDecoder::OK && dec.status() != QWspPduDecoder::Unhandled)
        ok = false;

    delete mmsCodec;
    return ok;
}

bool MMSMessage::encode(QIODevice *dev)
{
    QWspPduEncoder enc(dev);

    err = QString();

    // MMS headers not encoded using code page, but we can still use the
    // mechanism.
    MMSHeaderCodec *mmsCodec = new MMSHeaderCodec;
    enc.setHeaderCodec(mmsCodec);

    const QWspField *f = field("X-Mms-MMS-Version");
    if (!f)
        addField("X-Mms-MMS-Version", "1.0");

    bool rv = false;
    switch (type()) {
        case MSendReq:
            rv = encodeSendRequest(enc);
            break;
        case MNotificationInd:
            rv = encodeNotificationInd(enc);    // for testing
            break;
        case MNotifyResp:
            rv = encodeNotifyInd(enc);
            break;
        case MAckowledgeInd:
            rv = encodeAcknowledgeInd(enc);
            break;
        case MRetrieveConf:
        case MSendConf:
        case MDeliveryInd:
            // no need to encode these on the client
        default:
            qWarning("Cannot encode type: %d", type());
            break;
    }

    delete mmsCodec;
    return rv;
}

bool MMSMessage::encodeSendRequest(QWspPduEncoder &enc)
{
    const QWspField *f = field("X-Mms-Message-Type");
    enc.encodeField(*f);

    f = field("X-Mms-Transaction-Id");
    if (f) {
        enc.encodeField(*f);
    } else {
        err = qApp->translate("MMSMessage", "Invalid message: no X-Mms-Transaction-Id", 0);
        return false;
    }

    f = field("X-Mms-MMS-Version");
    if (f) {
        enc.encodeField(*f);
    } else {
        err = qApp->translate("MMSMessage", "Invalid message: no X-Mms-MMS-Version", 0);
        return false;
    }

    f = field("Date");
    if (f)
        enc.encodeField(*f);

    f = field("From");
    if (f) {
        enc.encodeField(*f);
    } else {
        QWspField ff;
        ff.name = "From";
        enc.encodeField(ff);
    }

    bool haveTo = false;
    QList<QWspField>::ConstIterator it;
    for (it = fields.begin(); it != fields.end(); ++it) {
        if ((*it).name == "To" || (*it).name == "Cc" || (*it).name == "Bcc") {
            enc.encodeField(*it);
            haveTo = true;
        }
    }

    if (!haveTo) {
        err = qApp->translate("MMSMessage", "Invalid message: no To, Cc or Bcc", 0);
        return false;
    }

    f = field("Subject");
    if (f)
        enc.encodeField(*f);

    static const char * const optional[] = {
        "X-Mms-Message-Class", "X-Mms-Expiry",
        "X-Mms-Delivery-Time", "X-Mms-Priority", "X-Mms-Sender-Visibility",
        "X-Mms-Delivery-Report", "X-Mms-Read-Reply", 0 };

    int i = 0;
    while (optional[i]) {
        f = field(optional[i]);
        if (f)
            enc.encodeField(*f);
        i++;
    }

    f = field("Content-Type");
    if (f) {
        enc.encodeField(*f);
    } else {
        err = qApp->translate("MMSMessage", "Invalid message: no Content-Type", 0);
        return false;
    }

    // encode body/parts
    enc.setHeaderCodec(0);
    enc.encodeMultipart(multiPartData);

    return true;
}

bool MMSMessage::encodeNotificationInd(QWspPduEncoder &enc)
{
    const QWspField *f = field("X-Mms-Message-Type");
    enc.encodeField(*f);

    f = field("X-Mms-Transaction-Id");
    if (f) {
        enc.encodeField(*f);
    } else {
        err = qApp->translate("MMSMessage", "Invalid message: no X-Mms-Transaction-Id", 0);
        return false;
    }

    f = field("X-Mms-MMS-Version");
    if (f) {
        enc.encodeField(*f);
    } else {
        err = qApp->translate("MMSMessage", "Invalid message: no X-Mms-MMS-Version", 0);
        return false;
    }

    f = field("From");
    if (f)
        enc.encodeField(*f);

    f = field("Subject");
    if (f)
        enc.encodeField(*f);

    static const char * const mandatory[] = {
        "X-Mms-Message-Class", "X-Mms-Message-Size",
        "X-Mms-Expiry", "X-Mms-Content-Location", 0 };

    int i = 0;
    while (mandatory[i]) {
        f = field(mandatory[i]);
        if (f) {
            enc.encodeField(*f);
        } else {
            err = qApp->translate("MMSMessage", "Invalid message: no %1", 0).arg(mandatory[i]);
            return false;
        }
        i++;
    }

    return true;
}

bool MMSMessage::encodeNotifyInd(QWspPduEncoder &enc)
{
    const QWspField *f;
    static const char * const mandatory[] = {
        "X-Mms-Message-Type", "X-Mms-Transaction-Id",
        "X-Mms-MMS-Version", "X-Mms-Status", 0 };

    int i = 0;
    while (mandatory[i]) {
        f = field(mandatory[i]);
        if (f) {
            enc.encodeField(*f);
        } else {
            err = qApp->translate("MMSMessage", "Invalid message: no %1", 0).arg(mandatory[i]);
            return false;
        }
        i++;
    }

    f = field("X-Mms-Report-Allowed");
    if (f)
        enc.encodeField(*f);

    return true;
}

bool MMSMessage::encodeAcknowledgeInd(QWspPduEncoder &enc)
{
    const QWspField *f;
    static const char * const mandatory[] = {
        "X-Mms-Message-Type", "X-Mms-Transaction-Id",
        "X-Mms-MMS-Version", 0 };

    int i = 0;
    while (mandatory[i]) {
        f = field(mandatory[i]);
        if (f) {
            enc.encodeField(*f);
        } else {
            err = qApp->translate("MMSMessage", "Invalid message: no %1", 0).arg(mandatory[i]);
            return false;
        }
        i++;
    }

    f = field("X-Mms-Report-Allowed");
    if (f)
        enc.encodeField(*f);

    return true;
}

MMSMessage::Type MMSMessage::type() const
{
    const QWspField *f = field("X-Mms-Message-Type");
    if (f) {
        int t = mapToNumber(messageTypes, f->value);
        if (t >= 128 && t <= 134)
            return static_cast<Type>(t);
    }

    return Invalid;
}

void MMSMessage::setType(Type t)
{
    removeField("X-Mms-Message-Type");
    const char *str = mapToString(messageTypes, t);
    if (str)
        addField("X-Mms-Message-Type", str);
}

QString MMSMessage::txnId() const
{
    const QWspField *f = field("X-Mms-Transaction-Id");
    if (f)
        return f->value;

    return QString();
}

void MMSMessage::setTxnId(const QString& txnId)
{
    removeField("X-Mms-Transaction-Id");
    addField("X-Mms-Transaction-Id", txnId);
}

QMailMessageId MMSMessage::messageId() const
{
    return msgId;
}

void MMSMessage::setMessageId(const QMailMessageId& id)
{
    msgId = id;
}

bool MMSMessage::multipartRelated() const
{
    const QWspField *f = field("Content-Type");
    if (f && f->value.indexOf("application/vnd.wap.multipart.related") == 0)
        return true;
    return false;
}

const QWspField *MMSMessage::field(const QString &name) const
{
    QList<QWspField>::ConstIterator it;
    for (it = fields.begin(); it != fields.end(); ++it) {
        if ((*it).name == name)
            return &(*it);
    }

    return 0;
}

void MMSMessage::addField(const QString &name, const QString &value)
{
    QWspField field;
    field.name = name;
    field.value = value;
    while (field.value.length() && field.value[0].isSpace())
        field.value = field.value.mid(1);
    while (field.value.length() && field.value[field.value.length()-1].isSpace())
        field.value.truncate(field.value.length()-1);
    fields.append(field);
}

void MMSMessage::addField(const QString &name, quint32 value)
{
    QWspField field;
    field.name = name;
    field.value = QString::number(value);
    fields.append(field);
}

void MMSMessage::removeField(const QString &name)
{
    QList<QWspField>::Iterator it;
    for (it = fields.begin(); it != fields.end(); ++it) {
        if ((*it).name == name) {
            fields.erase(it);
            return;
        }
    }
}

const QWspPart &MMSMessage::messagePart(int idx) const
{
    return multiPartData.part(idx);
}

void MMSMessage::addMessagePart(const QWspPart &part)
{
    multiPartData.addPart(part);
}

//===========================================================================

QWspField MMSHeaderCodec::decode(QWspPduDecoder &dec)
{
    QWspField field;

    quint8 octet = dec.decodeShortInteger();
    int idx = fieldFromNumber(octet);
    if (idx >= 0) {
        const HeaderField &fld = headerFields[idx];
        field.name = fld.name;
        // decode value
        switch (fld.type) {
            case MMSHeaderCodec::TextString:
                field.value = dec.decodeTextString();
                break;
            case MMSHeaderCodec::EncodedString:
                field.value = dec.decodeEncodedString();
                break;
            case MMSHeaderCodec::TokenText:
                field.value = dec.decodeTokenText();
                break;
            case MMSHeaderCodec::Date:
                {
                    quint32 d = dec.decodeLongInteger();
                    QDateTime dt = QWspDateTime::fromGmtTime_t(d);
                    field.value = QWspDateTime::dateString(dt);
                }
                break;
            case MMSHeaderCodec::LongInteger:
                field.value = QString::number(dec.decodeLongInteger());
                break;
            case MMSHeaderCodec::Version:
                field.value = dec.decodeVersion();
                break;
            case MMSHeaderCodec::From:
                field.value = decodeFrom(dec);
                break;
            case MMSHeaderCodec::MessageClass:
                field = decodeMessageClass(dec);
                break;
            case MMSHeaderCodec::MessageType:
                field = decodeEnumerated(dec, messageTypes, fld.name);
                break;
            case MMSHeaderCodec::ContentType:
                field.value = dec.decodeContentType();
                break;
            case MMSHeaderCodec::Uri:
                field.value = dec.decodeTextString();
                break;
            case MMSHeaderCodec::Expiry:
            case MMSHeaderCodec::DeliveryTime:
                field = decodeExpiry(dec);
                break;
            case MMSHeaderCodec::YesNo:
                field = decodeEnumerated(dec, yesNoTypes, fld.name);
                break;
            case MMSHeaderCodec::Priority:
                field = decodeEnumerated(dec, priorityTypes, fld.name);
                break;
            case MMSHeaderCodec::Status:
                field = decodeEnumerated(dec, statusTypes, fld.name);
                break;
            case MMSHeaderCodec::ResponseStatus:
                field = decodeEnumerated(dec, responseStatusTypes, fld.name);
                break;
            case MMSHeaderCodec::SenderVisibility:
                field = decodeEnumerated(dec, senderVisibilityTypes, fld.name);
                break;
            default:
                break;
        }
    }

    return field;
}

bool MMSHeaderCodec::encode(QWspPduEncoder &enc, const QWspField &field)
{
    int idx = numberFromField(field.name);
    if (idx < 0)
        return false;

    const HeaderField &fld = headerFields[idx];
    enc.encodeShortInteger(fld.number);
    switch (fld.type) {
        case MMSHeaderCodec::TextString:
            enc.encodeTextString(field.value);
            break;
        case MMSHeaderCodec::EncodedString:
            enc.encodeTextString(field.value);
            break;
        case MMSHeaderCodec::TokenText:
            enc.encodeTokenText(field.value);
            break;
        case MMSHeaderCodec::Date:
            {
                QDateTime dt = QWspDateTime::parseDate(field.value);
                quint32 d = QWspDateTime::toGmtTime_t(dt);
                enc.encodeLongInteger(d);
            }
            break;
        case MMSHeaderCodec::LongInteger:
            enc.encodeLongInteger(field.value.toUInt());
            break;
        case MMSHeaderCodec::Version:
            enc.encodeVersion(field.value);
            break;
        case MMSHeaderCodec::From:
            return encodeFrom(enc, field.value);
        case MMSHeaderCodec::MessageType:
            return encodeEnumerated(enc, messageTypes, field);
        case MMSHeaderCodec::MessageClass:
            return encodeMessageClass(enc, field.value);
        case MMSHeaderCodec::ContentType:
            enc.encodeContentType(field.value);
            break;
        case MMSHeaderCodec::Uri:
            enc.encodeTextString(field.value);
            break;
        case MMSHeaderCodec::Expiry:
        case MMSHeaderCodec::DeliveryTime:
            return encodeExpiry(enc, field);
        case MMSHeaderCodec::Priority:
            return encodeEnumerated(enc, priorityTypes, field);
        case MMSHeaderCodec::YesNo:
            return encodeEnumerated(enc, yesNoTypes, field);
        case MMSHeaderCodec::Status:
            return encodeEnumerated(enc, statusTypes, field);
        case MMSHeaderCodec::ResponseStatus:
            return encodeEnumerated(enc, responseStatusTypes, field);
        case MMSHeaderCodec::SenderVisibility:
            return encodeEnumerated(enc, senderVisibilityTypes, field);
    }

    return true;
}

QString MMSHeaderCodec::decodeFrom(QWspPduDecoder &dec)
{
    QString addr;
    quint32 len = dec.decodeLength();
    int epos = dec.device()->pos() + len;
    quint8 tok = dec.decodeOctet();
    if (tok == 128) {
        addr = dec.decodeEncodedString();
    } else if (tok == 129) {
        // nothing
    } else {
        qWarning("Invalid token in address field");
        dec.setStatus(QWspPduDecoder::Unhandled);
    }
    while (dec.device()->pos() < epos && !dec.device()->atEnd())
        dec.decodeOctet();

    return addr;
}

QWspField MMSHeaderCodec::decodeEnumerated(QWspPduDecoder &dec, const ValueMap *map, const QString &name)
{
    QWspField field;
    quint8 octet = dec.decodeOctet();
    field.name = name;
    const char *str = mapToString(map, octet);
    if (!str) {
        qWarning("Unknown type");
        dec.setStatus(QWspPduDecoder::Unhandled);
    } else {
        field.value = str;
    }

    return field;
}

QWspField MMSHeaderCodec::decodeMessageClass(QWspPduDecoder &dec)
{
    QWspField field;
    field.name = "X-Mms-Message-Class";
    quint8 id = dec.peekOctet();
    if (id & 0x80) {
        dec.decodeOctet();
        const char *str = mapToString(messageClassTypes, id);
        if (!str) {
            qWarning("Unknown message class identifier");
            dec.setStatus(QWspPduDecoder::Unhandled);
        } else {
            field.value = str;
        }
    } else {
        field.value = dec.decodeTokenText();
    }

    return field;
}

QWspField MMSHeaderCodec::decodeExpiry(QWspPduDecoder &dec)
{
    quint32 len = dec.decodeLength();
    quint8 octet = dec.decodeOctet();
    QWspField field;
    field.name = "Expiry";
    if (octet == 128) {
        // absolute
        quint32 d = dec.decodeLongInteger();
        QDateTime dt = QWspDateTime::fromGmtTime_t(d);
        field.value = QWspDateTime::dateString(dt);
    } else if (octet == 129) {
        // relative
        field.value = QString::number(dec.decodeInteger());
    } else {
        qWarning("Unknown token in expiry field");
        while (--len > 0)
            dec.decodeOctet();
    }

    return field;
}

bool MMSHeaderCodec::encodeFrom(QWspPduEncoder &enc, const QString &str)
{
    if (str.isEmpty()) {
        // insert address
        enc.encodeLength(1);
        enc.encodeOctet(129);
    } else {
        quint32 len = str.length()+2;
        enc.encodeLength(len);
        enc.encodeOctet(128);
        enc.encodeTextString(str);
    }

    return true;
}

bool MMSHeaderCodec::encodeEnumerated(QWspPduEncoder &enc, const ValueMap *map, const QWspField &field)
{
    int id = mapToNumber(map, field.value);
    if (id > 0) {
        enc.encodeOctet(static_cast<quint8>(id));
    } else {
        qWarning("Cannot encode field: %s", field.name.toLatin1().constData());
        return false;
    }

    return true;
}

bool MMSHeaderCodec::encodeMessageClass(QWspPduEncoder &enc, const QString &str)
{
    int id = mapToNumber(messageClassTypes, str);
    if (id > 0)
        enc.encodeOctet(static_cast<quint8>(id));
    else
        enc.encodeTokenText(str);

    return true;
}

bool MMSHeaderCodec::encodeExpiry(QWspPduEncoder &enc, const QWspField &field)
{
    bool ok;
    quint32 delta = field.value.toUInt(&ok);
    if (!ok) {
        // date
        QDateTime dt = QWspDateTime::parseDate(field.value);
        quint32 d = QWspDateTime::toGmtTime_t(dt);
        quint32 len = enc.longIntegerLength(d) + 1;
        enc.encodeLength(len);
        enc.encodeOctet(128);
        enc.encodeInteger(d);
        return true;
    } else {
        // delta seconds
        quint32 len = enc.integerLength(delta) + 1;
        enc.encodeLength(len);
        enc.encodeOctet(129);
        enc.encodeInteger(delta);
    }

    return true;
}

int MMSHeaderCodec::fieldFromNumber(quint8 id)
{
    int i = 0;
    while (headerFields[i].number) {
        if (headerFields[i].number == id)
            return i;
        i++;
    }

    return -1;
}

int MMSHeaderCodec::numberFromField(const QString &str)
{
    int i = 0;
    while (headerFields[i].number) {
        if (headerFields[i].name == str)
            return i;
        i++;
    }

    return -1;
}
