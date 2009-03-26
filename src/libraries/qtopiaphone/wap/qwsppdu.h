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

#ifndef QWSPPDU_H
#define QWSPPDU_H

#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>

#include <qtopiaglobal.h>

class QIODevice;

class QTOPIAPHONE_EXPORT QWspField
{
public:
    QWspField();
    QWspField( const QWspField& field );
    ~QWspField();

    QWspField& operator=( const QWspField& field );

    QString name;
    QString value;
};

class QTOPIAPHONE_EXPORT QWspDateTime
{
public:
    static QDateTime parseDate(QString in);
    static QString dateString(QDateTime d);
    static QDateTime fromGmtTime_t(quint32);
    static quint32 toTime_t(const QDateTime &);
    static quint32 toGmtTime_t(const QDateTime &);
};

class QWspPduDecoder;
class QWspPduEncoder;

class QTOPIAPHONE_EXPORT QWspHeaderCodec
{
public:
    virtual ~QWspHeaderCodec();
    virtual QWspField decode(QWspPduDecoder &) = 0;
    virtual bool encode(QWspPduEncoder &, const QWspField &) = 0;
    virtual quint8 codePage() const = 0;
};

class QTOPIAPHONE_EXPORT QWspPart
{
public:
    QWspPart();
    QWspPart( const QWspPart& part );
    ~QWspPart();

    QWspPart& operator=( const QWspPart& part );

    const QList<QWspField> &headers() const { return hdr; }
    const QWspField *header(const QString &name) const;
    void addHeader(const QWspField &);
    void addHeader(const QString &name, const QString &value);

    const QByteArray &data() const { return ba; }
    void setData(const char *d, int l);
    void readData(QIODevice *d, int l);
    void writeData(QIODevice *d) const;

private:
    QList<QWspField> hdr;
    QByteArray ba;
};

class QTOPIAPHONE_EXPORT QWspMultipart
{
public:
    QWspMultipart();
    QWspMultipart( const QWspMultipart& mpart );
    ~QWspMultipart();

    QWspMultipart& operator=( const QWspMultipart& mpart );

    int count() const { return parts.count(); }
    void addPart(const QWspPart &);
    const QWspPart &part(int idx) const { return parts[idx]; }

private:
    QList<QWspPart> parts;
};

class QTOPIAPHONE_EXPORT QWspPush : public QWspPart
{
public:
    QWspPush();
    QWspPush( const QWspPush& push );
    ~QWspPush();

    QWspPush& operator=( const QWspPush& push );

    int identifier() const { return ident; }
    int pduType() const { return pdu; }

    void setIdentifier( int value ) { ident = value; }
    void setPduType( int value ) { pdu = value; }

    static QString quickContentType( const QByteArray& data );

private:
    int ident;
    int pdu;
};

class QTOPIAPHONE_EXPORT QWspPduDecoder : public QObject
{
    Q_OBJECT
public:
    explicit QWspPduDecoder(QIODevice *);
    ~QWspPduDecoder();

    quint8 peekOctet();
    quint8 decodeOctet();
    quint8 decodeUInt8();
    quint16 decodeUInt16();
    quint32 decodeUInt32();
    quint32 decodeUIntVar();
    quint8 decodeShortInteger();
    quint32 decodeLongInteger();
    quint32 decodeInteger();
    quint32 decodeLength();
    QString decodeTextString();
    QString decodeEncodedString();
    QString decodeTokenText();
    QString decodeVersion();
    QString decodeContentType();
    QWspField decodeField();
    QString decodeParameter();
    QWspMultipart decodeMultipart();
    void decodeContentTypeAndHeaders(QWspPart& part, quint32 hdrLen);
    QWspPart decodePart();
    QWspPush decodePush();

    void setHeaderCodec(QWspHeaderCodec *c) {
        if (c)
            headerCodec = c;
        else
            headerCodec = defaultCodec;
    }

    enum Status { OK, Unhandled, Eof, Fatal };
    Status status() const { return stat; }
    void setStatus(Status s) { stat = s; }

    QIODevice *device() { return dev; }

signals:
    void selectCodePage(quint8);

private:
    QString decodeCharset( const QString &encoded, quint32 mib);
    QString decodeTextBlock(int length);
private:
    QIODevice *dev;
    QWspHeaderCodec *headerCodec;
    QWspHeaderCodec *defaultCodec;
    Status stat;
};

class QTOPIAPHONE_EXPORT QWspPduEncoder : public QObject
{
    Q_OBJECT
public:
    explicit QWspPduEncoder(QIODevice *);
    ~QWspPduEncoder();

    void encodeOctet(quint8);
    void encodeUInt8(quint8);
    void encodeUInt16(quint16);
    void encodeUInt32(quint32);
    void encodeUIntVar(quint32);
    void encodeShortInteger(quint8);
    void encodeLongInteger(quint32);
    void encodeInteger(quint32);
    void encodeLength(quint32);
    void encodeTextString(const QString &str);
    void encodeEncodedString(const QString &str);
    void encodeVersion(const QString &);
    void encodeTokenText(const QString &str);
    void encodeContentType(const QString &str);
    void encodeField(const QWspField &);
    void encodeParameter(const QString &);
    void encodeMultipart(const QWspMultipart &);
    void encodePart(const QWspPart &);
    void encodePush(const QWspPush &);

    int longIntegerLength(quint32 d);
    int integerLength(quint32 d);

    void setHeaderCodec(QWspHeaderCodec *c) {
        if (c)
            headerCodec = c;
        else
            headerCodec = defaultCodec;
    }

//    void setCodePage(quint8);

private:
    QString unquoteString(const QString &str);

private:
    QIODevice *dev;
    QWspHeaderCodec *headerCodec;
    QWspHeaderCodec *defaultCodec;
};

#endif
