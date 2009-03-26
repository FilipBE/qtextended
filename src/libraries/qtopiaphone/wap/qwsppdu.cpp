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

#include <qwsppdu.h>
#include <qmimetype.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qiodevice.h>
#include <qtimezone.h>
#include <qbuffer.h>
#include <netinet/in.h>
#include <stdlib.h>

// Reference: WAP-230-WSP
//            Wireless Application Protocol
//            Wirelesss Session Protocol Specification
// See also: http://www.wapforum.org/wina/wsp-content-type.htm

static const char * const contentTypeAssignments[] = {
    "*/*",
    "text/*",
    "text/html",
    "text/plain",
    "text/x-hdml",
    "text/x-ttml",
    "text/x-vCalendar",
    "text/x-vCard",
    "text/vnd.wap.wml",
    "text/vnd.wap.wmlscript",
    "text/vnd.wap.channel",
    "Multipart/*",
    "Multipart/mixed",
    "Multipart/form-data",
    "Multipart/byteranges",
    "multipart/alternative",
    "application/*",
    "application/java-vm",
    "application/x-www-form-urlencoded",
    "application/x-hdmlc",
    "application/vnd.wap.wmlc",
    "application/vnd.wap.wmlscriptc",
    "application/vnd.wap.channelc",
    "application/vnd.wap.uaprof",
    "application/vnd.wap.wtls-ca-certificate",
    "application/vnd.wap.wtls-user-certificate",
    "application/x-x509-ca-cert",
    "application/x-x509-user-cert",
    "image/*",
    "image/gif",
    "image/jpeg",
    "image/tiff",
    "image/png",
    "image/vnd.wap.wbmp",
    "application/vnd.wap.multipart.*",
    "application/vnd.wap.multipart.mixed",
    "application/vnd.wap.multipart.form-data",
    "application/vnd.wap.multipart.byteranges",
    "application/vnd.wap.multipart.alternative",
    "application/xml",
    "text/xml",
    "application/vnd.wap.wbxml",
    "application/x-x968-cross-cert",
    "application/x-x968-ca-cert",
    "application/x-x968-user-cert",
    "text/vnd.wap.si",
    "application/vnd.wap.sic",
    "text/vnd.wap.sl",
    "application/vnd.wap.slc",
    "text/vnd.wap.co",
    "application/vnd.wap.coc",
    "application/vnd.wap.multipart.related",
    "application/vnd.wap.sia",
    "text/vnd.wap.connectivity-xml",
    "application/vnd.wap.connectivity-wbxml",
    "application/pkcs7-mime",
    "application/vnd.wap.hashed-certificate",
    "application/vnd.wap.signed-certificate",
    "application/vnd.wap.cert-response",
    "application/xhtml+xml",
    "application/wml+xml",
    "text/css",
    "application/vnd.wap.mms-message",
    "application/vnd.wap.rollover-certificate",
    "application/vnd.wap.locc+wbxml",
    "application/vnd.wap.loc+xml",
    "application/vnd.syncml.dm+wbxml",
    "application/vnd.syncml.dm+xml",
    "application/vnd.syncml.notification",
    "application/vnd.wap.xhtml+xml",
    "application/vnd.wv.csp.cir",
    "application/vnd.oma.dd+xml",
    "application/vnd.oma.drm.message",
    "application/vnd.oma.drm.content",
    "application/vnd.oma.drm.rights+xml",
    "application/vnd.oma.drm.rights+wbxml",
    0
};
#define numContentTypes (int)((sizeof(contentTypeAssignments) / \
                               sizeof(char *)) - 1)

struct CharsetAssignment {
    quint32 number;
    const char *charset;
};

static const CharsetAssignment charsetAssignments[] = {
    { 0x07EA, "big5" },
    { 0x03E8, "iso-10646-ucs-2" },
    { 0x04, "iso-8859-1" },
    { 0x05, "iso-8859-2" },
    { 0x06, "iso-8859-3" },
    { 0x07, "iso-8859-4" },
    { 0x08, "iso-8859-5" },
    { 0x09, "iso-8859-6" },
    { 0x0A, "iso-8859-7" },
    { 0x0B, "iso-8859-8" },
    { 0x0C, "iso-8859-9" },
    { 0x11, "shift_JIS" },
    { 0x03, "us-ascii" },
    { 0x6A, "utf-8" },
    { 0x00, "" }
};

QWspHeaderCodec::~QWspHeaderCodec()
{
}

class QWspDefaultCodec : public QWspHeaderCodec
{
public:
    QWspDefaultCodec() {}

    enum Type { Accept, AcceptLanguage, AcceptRanges, Age, Allow,
                Authorization, Connection, ContentBase, ContentEncoding,
                ContentLanguage, ContentLength, ContentLocation, ContentMD5,
                ContentType, Date, ETag, Expires, From, Host,
                IfModifiedSince, IfMatch, IfNoneMatch, IfRange,
                IfUnmodifiedSince, Location, LastModified, MaxForwards, Pragma,
                ProxyAuthenticate, ProxyAuthorization, Public, Range,
                Referer, RetryAfter, Server, TransferEncoding, Upgrade,
                UserAgent, Vary, Via, Warning, WWWAuthenticate,
                ContentDisposition, XWapApplicationId, XWapContentURI,
                XWapInitiatorURI, AcceptApplication, BearerIndication,
                PushFlag, Profile, ProfileDiff, ProfileWarning, Expect,
                TE, Trailer, AcceptCharset, AcceptEncoding, CacheControl,
                ContentRange, XWapTod, ContentID, SetCookie, Cookie,
                EncodingVersion
    };

    virtual QWspField decode(QWspPduDecoder &);
    virtual bool encode(QWspPduEncoder&, const QWspField&);
    virtual quint8 codePage() const { return 1; }

private:
    int fieldFromNumber(quint8 id);
    int numberFromField(const QString &str);
};

struct WAPHeaderField {
    quint8 number;
    int version;
    const char *name;
    QWspDefaultCodec::Type type;
};

static const WAPHeaderField headerFieldNameAssignments[] = {
    { 0x00, 11, "Accept", QWspDefaultCodec::Accept },
    { 0x01, 11, "Accept-Charset", QWspDefaultCodec::AcceptCharset },
    { 0x02, 11, "Accept-Encoding", QWspDefaultCodec::AcceptEncoding },
    { 0x03, 11, "Accept-Language", QWspDefaultCodec::AcceptLanguage },
    { 0x04, 11, "Accept-Ranges", QWspDefaultCodec::AcceptRanges },
    { 0x05, 11, "Age", QWspDefaultCodec::Age },
    { 0x06, 11, "Allow", QWspDefaultCodec::Allow },
    { 0x07, 11, "Authorization", QWspDefaultCodec::Authorization },
    { 0x08, 11, "Cache-Control", QWspDefaultCodec::CacheControl },
    { 0x09, 11, "Connection", QWspDefaultCodec::Connection },
    { 0x0A, 11, "Content-Base", QWspDefaultCodec::ContentBase },
    { 0x0B, 11, "Content-Encoding", QWspDefaultCodec::ContentEncoding },
    { 0x0C, 11, "Content-Language", QWspDefaultCodec::ContentLanguage },
    { 0x0D, 11, "Content-Length", QWspDefaultCodec::ContentLength },
    { 0x0E, 11, "Content-Location", QWspDefaultCodec::ContentLocation },
    { 0x0F, 11, "Content-MD5", QWspDefaultCodec::ContentMD5 },
    { 0x10, 11, "Content-Range", QWspDefaultCodec::ContentRange },
    { 0x11, 11, "Content-Type", QWspDefaultCodec::ContentType },
    { 0x12, 11, "Date", QWspDefaultCodec::Date },
    { 0x13, 11, "Etag", QWspDefaultCodec::ETag },
    { 0x14, 11, "Expires", QWspDefaultCodec::Expires },
    { 0x15, 11, "From", QWspDefaultCodec::From },
    { 0x16, 11, "Host", QWspDefaultCodec::Host },
    { 0x17, 11, "If-Modified-Since", QWspDefaultCodec::IfModifiedSince },
    { 0x18, 11, "If-Match", QWspDefaultCodec::IfMatch },
    { 0x19, 11, "If-None-Match", QWspDefaultCodec::IfNoneMatch },
    { 0x1A, 11, "If-Range", QWspDefaultCodec::IfRange },
    { 0x1B, 11, "If-Unmodified-Since", QWspDefaultCodec::IfUnmodifiedSince },
    { 0x1C, 11, "Location", QWspDefaultCodec::Location },
    { 0x1D, 11, "Last-Modified", QWspDefaultCodec::LastModified },
    { 0x1E, 11, "Max-Forwards", QWspDefaultCodec::MaxForwards },
    { 0x1F, 11, "Pragma", QWspDefaultCodec::Pragma },
    { 0x20, 11, "Proxy-Authenticate", QWspDefaultCodec::ProxyAuthenticate },
    { 0x21, 11, "Proxy-Authorization", QWspDefaultCodec::ProxyAuthorization },
    { 0x22, 11, "Public", QWspDefaultCodec::Public },
    { 0x23, 11, "Range", QWspDefaultCodec::Range },
    { 0x24, 11, "Referer", QWspDefaultCodec::Referer },
    { 0x25, 11, "Retry-After", QWspDefaultCodec::RetryAfter },
    { 0x26, 11, "Server", QWspDefaultCodec::Server },
    { 0x27, 11, "Transfer-Encoding", QWspDefaultCodec::TransferEncoding },
    { 0x28, 11, "Upgrade", QWspDefaultCodec::Upgrade },
    { 0x29, 11, "User-Agent", QWspDefaultCodec::UserAgent },
    { 0x2A, 11, "Vary", QWspDefaultCodec::Vary },
    { 0x2B, 11, "Via", QWspDefaultCodec::Via },
    { 0x2C, 11, "Warning", QWspDefaultCodec::Warning },
    { 0x2D, 11, "WWW-Authenticate", QWspDefaultCodec::WWWAuthenticate },
    { 0x2E, 11, "Content-Disposition", QWspDefaultCodec::ContentDisposition },
    { 0x2F, 12, "X-Wap-Application-Id", QWspDefaultCodec::XWapApplicationId },
    { 0x30, 12, "X-Wap-Content-URI", QWspDefaultCodec::XWapContentURI },
    { 0x31, 12, "X-Wap-Initiator-URI", QWspDefaultCodec::XWapInitiatorURI },
    { 0x32, 12, "Accept-Application", QWspDefaultCodec::AcceptApplication },
    { 0x33, 12, "Bearer-Indication", QWspDefaultCodec::BearerIndication },
    { 0x34, 12, "Push-Flag", QWspDefaultCodec::PushFlag },
    { 0x35, 12, "Profile", QWspDefaultCodec::Profile },
    { 0x36, 12, "Profile-Diff", QWspDefaultCodec::ProfileDiff },
    { 0x37, 12, "Profile-Warning", QWspDefaultCodec::ProfileWarning },
    { 0x38, 13, "Expect", QWspDefaultCodec::Expect },
    { 0x39, 13, "TE", QWspDefaultCodec::TE },
    { 0x3A, 13, "Trailer", QWspDefaultCodec::Trailer },
    { 0x3B, 13, "Accept-Charset", QWspDefaultCodec::AcceptCharset },
    { 0x3C, 13, "Accept-Encoding", QWspDefaultCodec::AcceptEncoding },
    { 0x3D, 13, "Cache-Control", QWspDefaultCodec::CacheControl },
    { 0x3E, 13, "Content-Range", QWspDefaultCodec::ContentRange },
    { 0x3F, 13, "X-Wap-Tod", QWspDefaultCodec::XWapTod },
    { 0x40, 13, "Content-ID", QWspDefaultCodec::ContentID },
    { 0x41, 13, "Set-Cookie", QWspDefaultCodec::SetCookie },
    { 0x42, 13, "Cookie", QWspDefaultCodec::Cookie },
    { 0x43, 13, "Encoding-Version", QWspDefaultCodec::EncodingVersion },
    { 0x80, 13, "", QWspDefaultCodec::Accept }
};

static QString token(QString str, QChar c1, QChar c2, int *index)
{
    int start, stop;
    start = str.indexOf(c1, *index, Qt::CaseInsensitive);
    if (start == -1)
        return QString();
    stop = str.indexOf(c2, ++start, Qt::CaseInsensitive);
    if (stop == -1)
        return QString();

    *index = stop + 1;
    return str.mid(start, stop - start);
}

//changing timezones are slow, cache info in this var
static int utcTimeZoneDiff = -1;

/*  Returns the difference between local time and GMT time in seconds   */
static int timeZoneDiff()
{
    if ( utcTimeZoneDiff == -1 ) {
        QDateTime gmt, current = QDateTime::currentDateTime();
        QString tz =
#ifndef PHONESIM
         QTimeZone::current().id();
#else
         getenv("TZ");
#endif
        if ( !tz.isEmpty() && (setenv("TZ", "GMT", true) == 0) ) {
            gmt = QDateTime::currentDateTime();
        } else return 0;

        QByteArray tzcstr = tz.toLatin1();
        setenv("TZ", tzcstr.constData(), true );

        utcTimeZoneDiff = gmt.secsTo( current );
    }

    return utcTimeZoneDiff;
}

static QString secsToUTC(int seconds)
{
    QString str;

    int h = seconds / 3600;
    int m = ( abs(seconds) - abs(h * 3600) ) / 60;
    str.sprintf("%+.2d%.2d", h, m );

    return str;
}

static int UTCToSecs(QString utc)
{
    if ( utc.length() != 5 )
        return 0;

    bool result;
    int h = utc.left(3).toInt(&result);
    if ( !result)
        return 0;

    int m = utc.right(2).toInt(&result);
    if ( !result)
        return 0;

    return (h*3600 + m*60);
}


/*!
  \class QWspDateTime
    \inpublicgroup QtTelephonyModule

  \brief The QWspDateTime class provides functions for converting between the WSP date time formats and standard date time formats.
  \ingroup time
  \ingroup telephony

  The QWspDateTime class provides functions for converting between the WSP
  date time formats and standard date time formats.
*/

/*!
    Parses the HTTP date format within \a in.

    \sa dateString()
*/
QDateTime QWspDateTime::parseDate(QString in)
{
    const QString Months("janfebmaraprmayjunjulaugsepoctnovdec");
    QDateTime dateTime;
    QString str, org;
    int month = -1, day = -1, year = -1;
    bool ok;
    int x, index;
    uint len;
    QString time, timeDiff;

    for (int z = 0; z < in.length(); z++) {
        if (in[(int)z] != ',') {
            org += in[(int)z];
        } else {
            org += " ";
        }
    }

    org = org.simplified();
    org += " ";

    index = org.indexOf(' ');
    str = org.left((uint) index);
    while ( str != QString() ) {
        len = str.length();
        index--;
        if ( (day == -1) && (len <= 2) ) {
            x = str.toInt(&ok);
            if ( ok )
                day = x;
        } else if ( (month == -1) && (len == 3) ) {
            x = Months.indexOf( str.toLower() );
            if ( x >= 0 )
                month = (x + 3) / 3;
        } else if ( (year == -1) && (len == 4) ) {
            x = str.toInt(&ok);
            if ( ok )
                year = x;
        } else if ( time.isEmpty() && len == 8 ) {      // time part: 14:22:22
            time = str;
        } else if ( timeDiff.isEmpty() && len == 5 ) {  // time localizer: +1000
            timeDiff = str;
        }

        str = token(org, ' ', ' ', &index);
    }

    if ( (day != -1) && (month != -1) && (year != -1) ) {
        dateTime.setDate( QDate(year, month, day) );

        if ( !time.isEmpty() ) {
            int h = time.left(2).toInt();
            int m = time.mid(3, 2).toInt();
            int senderLocal = UTCToSecs( timeDiff );

            //adjust sender local time to our local time
            int localDiff = timeZoneDiff() - senderLocal;

            //we add seconds after adding time, as it may change the date
            dateTime.setTime( QTime(h, m) );
            dateTime = dateTime.addSecs( localDiff);
        }
    }

    return dateTime;
}

/*!
    Convert \a d into the HTTP date time format.

    \sa parseDate()
*/
QString QWspDateTime::dateString(QDateTime d)
{
    QDate date = d.date();
    QTime time = d.time();
    QString str;
    QByteArray dayName = date.shortDayName(date.dayOfWeek()).toUtf8();
    QByteArray monthName = date.shortMonthName(date.month()).toUtf8();
    QByteArray utcOffs = secsToUTC(timeZoneDiff()).toUtf8();
    str.sprintf("%s, %.2d %s %d %.2d:%.2d:%.2d %s",
            dayName.constData(),
            date.day(),
            monthName.constData(),
            date.year(),
            time.hour(),
            time.minute(),
            time.second(),
            utcOffs.constData());

    return str;
}

/*!
    Convert the GMT time \a t into a QDateTime value.

    \sa toTime_t(), toGmtTime_t()
*/
QDateTime QWspDateTime::fromGmtTime_t(quint32 t)
{
    QDateTime dt;
    dt.setTime_t(t + timeZoneDiff());
    return dt;
}

/*!
    Convert \a dt into a seconds value in local time.

    \sa fromGmtTime_t(), toGmtTime_t()
*/
quint32 QWspDateTime::toTime_t(const QDateTime &dt)
{
    QDateTime st(QDate(1970, 1, 1));
    return st.secsTo(dt) - timeZoneDiff();
}

/*!
    Convert \a dt into a seconds value in GMT time.

    \sa fromGmtTime_t(), toTime_t()
*/
quint32 QWspDateTime::toGmtTime_t(const QDateTime &dt)
{
    QDateTime st(QDate(1970, 1, 1));
    return st.secsTo(dt) - 2*timeZoneDiff();
}

//===========================================================================

/*!
  \class QWspField
    \inpublicgroup QtTelephonyModule

  \brief The QWspField class provides an encapsulation of a field name and value.
  \ingroup telephony

  The QWspField class provides an encapsulation of a field name and value.
*/

/*!
    Construct an empty WSP field.
*/
QWspField::QWspField()
{
}

/*!
    Construct a copy of \a field.
*/
QWspField::QWspField( const QWspField& field )
{
    name = field.name;
    value = field.value;
}

/*!
    Destruct this WSP field.
*/
QWspField::~QWspField()
{
}

/*!
    Copy the contents of \a field into this object.
*/
QWspField& QWspField::operator=( const QWspField& field )
{
    if ( this != &field ) {
        name = field.name;
        value = field.value;
    }
    return *this;
}

//===========================================================================

/*!
  \class QWspPduDecoder
    \inpublicgroup QtTelephonyModule

  \brief The QWspPduDecoder class provides facilities for parsing WSP PDU's.
  \ingroup telephony

  The QWspPduDecoder class provides facilities for parsing WSP PDU's.
*/


/*!
    Construct a WSP PDU decoder to decode the data from \a d.
*/
QWspPduDecoder::QWspPduDecoder(QIODevice *d)
    : dev(d), stat(OK)
{
    headerCodec = defaultCodec = new QWspDefaultCodec;
}

/*!
    Destroy this WSP PDU decoder.
*/
QWspPduDecoder::~QWspPduDecoder()
{
}

/*!
    Peek at the next octet in the input data stream.
*/
quint8 QWspPduDecoder::peekOctet()
{
    char o;
    if ( dev->getChar(&o) ) {
        dev->ungetChar(o);
        return (quint8)o;
    } else {
        return (quint8)0;
    }
}

/*!
    Decode the next octet from the input data stream.
*/
quint8 QWspPduDecoder::decodeOctet()
{
    char o;
    if (dev->atEnd()) {
        stat = Eof;
        return (quint8)0;
    } else if ( dev->getChar(&o) ) {
        return (quint8)o;
    } else {
        stat = Eof;
        return (quint8)0;
    }
}

/*!
    Decode an unsigned 8-bit integer from the input data stream.
*/
quint8 QWspPduDecoder::decodeUInt8()
{
    return decodeOctet();
}

/*!
    Decode an unsigned 16-bit integer from the input data stream.
*/
quint16 QWspPduDecoder::decodeUInt16()
{
    quint16 d;
    d = quint16(decodeOctet()) << 8;
    d |= quint16(decodeOctet());

    return d;
}

/*!
    Decode an unsigned 32-bit integer from the input data stream.
*/
quint32 QWspPduDecoder::decodeUInt32()
{
    quint32 d;
    d = quint32(decodeOctet()) << 24;
    d |= quint32(decodeOctet()) << 16;
    d |= quint32(decodeOctet()) << 8;
    d |= quint32(decodeOctet());

    return d;
}

/*!
    Decode an unsigned 32-bit integer from the input data stream
    that is encoded with the variable-length WSP encoding.
*/
quint32 QWspPduDecoder::decodeUIntVar()
{
    unsigned char d[5];
    int count = 0;
    quint8 o = decodeOctet();
    while (o & 0x80 && count < 4) {
        d[count++] = o & 0x7F;
        o = decodeOctet();
    }
    d[count++] = o;

    quint32 v = 0;
    for (int i = 0; i < count; i++) {
        v |= (quint32)d[i] << ((count-i-1)*7);
    }

    return v;
}

/*!
    Decode a short integer value from the input data stream.
    Short integers are between 0 and 127, and have the high bit set
    to indicate "short".
*/
quint8 QWspPduDecoder::decodeShortInteger()
{
    return decodeUInt8() & 0x7F;
}

/*!
    Decode a long integer value from the input data stream.
    Long integers start with a byte indicating the length of
    the integer encoding, followed by that many bytes from
    most significant onwards.
*/
quint32 QWspPduDecoder::decodeLongInteger()
{
    quint8 len = decodeUInt8();    // short length
    quint32 v = 0;
    for (int i = 0; i < len && !dev->atEnd(); i++) {
        v |= (quint32)decodeOctet() << ((len-i-1)*8);
    }
    return v;
}

/*!
    Decode either a short integer or a long integer, depending
    upon whether the first byte has the high bit set (short)
    or unset (long).

    \sa decodeShortInteger(), decodeLongInteger()
*/
quint32 QWspPduDecoder::decodeInteger()
{
    if (peekOctet() & 0x80)
        return decodeShortInteger();
    else
        return decodeLongInteger();
}

/*!
    Decode a length value from the input data stream.  Leading byte
    values between 0 and 30 indicate an explicit length.  A leading
    byte value of 31 indicates that a variable-length integer follows.

    \sa decodeUIntVar()
*/
quint32 QWspPduDecoder::decodeLength()
{
    quint32 len = 0;
    quint8 o = peekOctet();
    if (o < 31) {
        len = (quint32)decodeOctet();
    } else if (o == 31) {
        decodeOctet();
        len = decodeUIntVar();
    }

    return len;
}

/*!
    Decode a text string from the input data stream.
*/
QString QWspPduDecoder::decodeTextString()
{
    QString str;
    quint8 o = decodeOctet();
    if (o == '\"')
        o = decodeOctet();
    while (o != 0 && !dev->atEnd()) {
        str += o;
        o = decodeOctet();
    }

    int l = str.length();
    if ((l > 0) && (str[l-1] == '\"'))
        str.chop(1);

    return str;
}

QString QWspPduDecoder::decodeTextBlock(int length)
{
    QString result;
    for(int i = 0; i < length; ++i)
        result += decodeOctet();
    return result;
}

/*!
    Decode an encoded string from the input data stream.
*/
QString QWspPduDecoder::decodeEncodedString()
{
    quint8 o = peekOctet();
    if (o >= 32 && o <= 127)
        return decodeTextString();

    quint32 len = decodeLength();
    QString encText;
    if(len)
    {
        quint32 mib = decodeInteger();
        quint32 miblen = mib > 127 ? 2 : 1;
        int textlen = len-miblen-1;
        if(peekOctet() == '\"') //ignore quote
        {
            decodeOctet();
            --textlen;
        }
        QString encText = decodeTextBlock(textlen);
        decodeOctet(); //ignore null terminator
        return decodeCharset(encText, mib);
    }
    return encText;
}

/*!
    Decode a token text value from the input data stream.
*/
QString QWspPduDecoder::decodeTokenText()
{
    return decodeTextString();
}

/*!
    Decode a version indicator from the input data stream.
*/
QString QWspPduDecoder::decodeVersion()
{
    QString version;
    quint8 octet = peekOctet();
    if (octet & 0x80) {
        int v = decodeShortInteger();
        version = QString::number((v & 0x70) >> 4);
        v &= 0x0F;
        if (v != 0x0F)
            version += "." + QString::number(v);
    } else {
        version = decodeTextString();
    }

    return version;
}

/*!
    Decode a content type value from the input data stream.
*/
QString QWspPduDecoder::decodeContentType()
{
    QString type;
    // Content-type-value = Constrained-Media | Content-general-form
    // Constrained-Media = Constrained-encoding
    // Constrained-encoding = Extension-Media | Short-integer
    // Extension-Media *Text End-of-string
    // Content-general-form = Value-length Media-type
    // Media-type = (Well-known-media | Extension-Media) *(Parameter)
    // Well-known-media = Integer-value
    quint8 o = peekOctet();
    if (o <= 31) {
        // Content-general-form
        quint32 len = decodeLength();
        int endByte = (int)(dev->pos()+len);
        // decode media type
        o = peekOctet();
        if (o & 0x80) {
            // Well-known-media
            quint8 enc = decodeInteger();
            if (enc < numContentTypes) {
                type = contentTypeAssignments[enc];
            } else {
                qWarning("Unknown content type: %d", enc);
                stat = Unhandled;
            }
        } else {
            // Extension-Media
            o = decodeOctet();
            while (o != 0) {
                type += o;
                o = decodeOctet();
            }
        }
        while (endByte > ((int)(dev->pos())) && !dev->atEnd()) {
            // read parameters
            QString p = decodeParameter();
            if (!p.isEmpty())
                type += "; " + p;
        }
    } else if (o & 0x80 ) {
        // Constrained-encoding = Short-integer
        quint8 enc = decodeShortInteger();
        if (enc < numContentTypes) {
            type = contentTypeAssignments[enc];
        } else {
            qWarning("Unknown content type: %d", enc);
            stat = Unhandled;
        }
    } else {
        // Constrained-encoding = Extension-Media
        o = decodeOctet();
        while (o != 0) {
            type += o;
            o = decodeOctet();
        }
    }

    return type;
}

/*!
    Decode a field declaration consisting of a name and a value
    from the input data stream.
*/
QWspField QWspPduDecoder::decodeField()
{
    quint8 octet = peekOctet();
    if (octet == 127 || (octet >= 1 && octet <= 31)) {
        decodeOctet();
        if (octet == 127) {
            // Shift-delimiter Page-identity
            octet = decodeOctet(); // == code page
        }
        if (octet == 1)
            setHeaderCodec(defaultCodec);
        else
            emit selectCodePage(octet);
        octet = peekOctet();
    }

    QWspField fld;
    if (octet & 0x80) {
        // Well-known-header
        fld = headerCodec->decode(*this);
    } else {
        // Application-header
        fld.name = decodeTokenText();
        fld.value = decodeTextString();
    }

    return fld;
}

/*!
    Decode a parameter value from the input data stream.
*/
QString QWspPduDecoder::decodeParameter()
{
    QString p;
    quint8 octet = peekOctet();

    if (octet <= 31 || octet & 0x80) {
        // Typed-parameter = Well-known-parameter-token Typed-value
        quint32 parm = decodeInteger();
        octet = peekOctet();
        switch (parm) {
            case 0x00: // Q
                {
                    p = "q=";
                    quint32 q = decodeUIntVar();
                    quint32 q1 = q - (q <= 100 ? 1 : 100);
                    QString v = "00" + QString::number(q1);
                    p += "0." + v.right(q <= 100 ? 2 : 3);
                }
                break;
            case 0x01: // charset
                p = "charset=";
                if (octet == 128) {
                    p += "\"*\"";
                } else {
                    quint32 c = decodeInteger();
                    int i = 0;
                    while (charsetAssignments[i].number) {
                        if (charsetAssignments[i].number == c)
                            p += charsetAssignments[i].charset;
                        i++;
                    }
                    if (!charsetAssignments[i].number)
                        p += "\"*\"";
                }
                break;
            case 0x02: // level
                p = "level=" + decodeVersion();
                break;
            case 0x03:
                p = "type=" + QString::number(decodeInteger());
                break;
            case 0x05:
                p = "name=\"" + decodeTextString() + '\"';
                break;
            case 0x06:
                p = "filename=\"" + decodeTextString() + '\"';
                break;
            case 0x07:
                p = "differences=";
                if (octet & 0x80)
                    p += QString::number((int)decodeShortInteger());
                else
                    p += '\"' + decodeTokenText() + '\"';
                break;
            case 0x08:
                p = "padding=" + QString::number((int)decodeShortInteger());
                break;
            case 0x09:
                p = "type=";
                if (octet & 0x80 ) {
                    // Constrained-encoding = Short-integer
                    quint8 enc = decodeShortInteger();
                    if (enc < numContentTypes)
                        p += contentTypeAssignments[enc];
                    else
                        qWarning("Unknown content type: %d", enc);
                } else {
                    // Constrained-encoding = Extension-Media
                    p += '\"' + decodeTextString() + '\"';
                }
                break;
            case 0x0A:
                p = "start=\"" + decodeTextString() + '\"';
                break;
            case 0x0B:
                p = "start-info=\"" + decodeTextString() + '\"';
                break;
            case 0x0C:
                p = "comment=\"" + decodeTextString() + '\"';
                break;
            case 0x0D:
                p = "domain=\"" + decodeTextString() + '\"';
                break;
            case 0x0E:
                {
                    p = "max-age=";
                    QDateTime dt = QWspDateTime::fromGmtTime_t(decodeInteger());
                    p += '\"' + QWspDateTime::dateString(dt) + '\"';
                }
                break;
            case 0x0F:
                p = "path=\"" + decodeTextString() + '\"';
                break;
            case 0x10:
                p = "secure";
                break;
            default:
                qWarning("Unknown parameter type: %d", parm);
                stat = Fatal;
                break;
        }
    } else {
        // Untyped-parameter
        p = decodeTokenText() + "=";
        octet = peekOctet();
        if (octet <= 31 || octet & 0x80) {
            p += decodeInteger();
        } else {
            // Extension-Media
            p += '\"';
            octet = decodeOctet();
            while (octet != 0 && !dev->atEnd()) {
                p += octet;
                octet = decodeOctet();
            }
            p += '\"';
        }
    }

    return p;
}

/*!
    Decode a multipart content block from the input data stream.
*/
QWspMultipart QWspPduDecoder::decodeMultipart()
{
    QWspMultipart mp;
    quint32 count = decodeUIntVar();
    for (unsigned int i = 0; i < count; i++) {
        mp.addPart(decodePart());
    }

    return mp;
}

/*!
    Decode the content type and headers from the input data stream.
    The \a hdrLen parameter indicates the number of bytes within
    the block that holds the content type and headers.  The \a part
    parameter indicates the object to place the content type and
    header information into.
*/
void QWspPduDecoder::decodeContentTypeAndHeaders(QWspPart& part, quint32 hdrLen)
{
    int curByte = (int)(dev->pos());
    int afterHeader = curByte + (int)hdrLen;
    QWspField field;
    field.name = "Content-Type";
    field.value = decodeContentType();
    part.addHeader(field);
    hdrLen -= ((int)(dev->pos())) - curByte;

    int dataByte = ((int)(dev->pos())) + hdrLen;
    while (((int)(dev->pos())) < dataByte && !dev->atEnd()) {
        field = decodeField();
        part.addHeader(field);
    }

    dev->seek(afterHeader);
}

/*!
    Decode a single WSP part from an input data stream.
*/
QWspPart QWspPduDecoder::decodePart()
{
    quint32 hdrLen = decodeUIntVar();
    quint32 dataLen = decodeUIntVar();

    QWspPart part;
    decodeContentTypeAndHeaders(part, hdrLen);
    part.readData(dev, dataLen);

    return part;
}

/*!
    Decode a WSP push datagram from an input data stream.
*/
QWspPush QWspPduDecoder::decodePush()
{
    QWspPush part;

    part.setIdentifier(decodeOctet());
    part.setPduType(decodeOctet());

    quint32 hdrLen = decodeUIntVar();

    decodeContentTypeAndHeaders(part, hdrLen);

    part.readData(dev, ((int)dev->size()) - ((int)(dev->pos())));

    return part;
}

/*!
    \fn void QWspPduDecoder::setHeaderCodec(QWspHeaderCodec *c)

    Set the header codec, that converts byte values into string names, to \a c.
*/

/*!
    \enum QWspPduDecoder::Status
    This enumeration defines the status of the input dats stream.

    \value OK The stream is OK.
    \value Unhandled The stream contains data that cannot be handled.
    \value Eof End of file was reached before the real end of the message.
    \value Fatal The stream contains data that caused a fatal parse error.
*/

/*!
    \fn QWspPduDecoder::Status QWspPduDecoder::status() const

    Get the current status of the input stream.
*/

/*!
    \fn void QWspPduDecoder::setStatus(Status s)

    Set the status of the input stream to \a s.
*/

/*!
    \fn QIODevice *QWspPduDecoder::device()

    Get the device that this WSP PDU decoder is reading from.
*/

/*!
    \fn void QWspPduDecoder::selectCodePage(quint8 page)

    Signal that is emitted when the WSP PDU decoder wants to change
    the header code \a page.  Slots that are connected to this signal
    should call setHeaderCodec() to select the correct codec.
*/

QString QWspPduDecoder::decodeCharset( const QString &encoded, quint32 mib)
{
    // Get the codec that is associated with the character set.
    QTextCodec *codec = QTextCodec::codecForMib( mib );
    if ( !codec )
        return encoded;

    // Convert the input body into a byte array.
    QByteArray bytes( encoded.length(), 0 );


    // network byte order for unicode
    if(mib == 1000)
    {
        for(int x = 1; x < encoded.length(); x+=2)
        {
            bytes[x-1] = (char)encoded[x].unicode();
            bytes[x] = (char)encoded[x-1].unicode();
        }
    }
    else
    {
        int posn = (int)encoded.length();
        while ( posn > 0 ) {
            --posn;
            bytes[posn] = (char)( encoded[posn].unicode() );
        }
    }

    // Convert the byte array into Unicode and return it.
    return codec->toUnicode( bytes );
}

//===========================================================================

/*!
  \class QWspPduEncoder
    \inpublicgroup QtTelephonyModule

  \brief The QWspPduEncoder class provides facilities for writing WSP PDU's.
  \ingroup telephony

  The QWspPduEncoder class provides facilities for writing WSP PDU's.
*/


/*!
    Create a WSP PDU encoder that writes the the output data stream \a d.
*/
QWspPduEncoder::QWspPduEncoder(QIODevice *d)
    : dev(d)
{
    headerCodec = defaultCodec = new QWspDefaultCodec;
}

/*!
    Destroy this WSP PDU encoder.
*/
QWspPduEncoder::~QWspPduEncoder()
{
}

/*!
    Encode an \a octet to the output data stream.
*/
void QWspPduEncoder::encodeOctet(quint8 octet)
{
    dev->putChar((char)octet);
}

/*!
    Encode an unsigned 8-bit \a octet to the output data stream.
*/
void QWspPduEncoder::encodeUInt8(quint8 octet)
{
    dev->putChar((char)octet);
}

/*!
    Encode an unsigned 16-bit value \a d to the output data stream.
*/
void QWspPduEncoder::encodeUInt16(quint16 d)
{
    dev->putChar((char)(d >> 8));
    dev->putChar((char)(d & 0x0f));
}

/*!
    Encode an unsigned 32-bit value \a d to the output data stream.
*/
void QWspPduEncoder::encodeUInt32(quint32 d)
{
    dev->putChar((char)((d >> 24) & 0x0f));
    dev->putChar((char)((d >> 16) & 0x0f));
    dev->putChar((char)((d >> 8) & 0x0f));
    dev->putChar((char)(d & 0x0f));
}

/*!
    Encode an unsigned variable length integer value \a iv
    to the output data stream.
*/
void QWspPduEncoder::encodeUIntVar(quint32 iv)
{
    unsigned char d[5];
    int count = 0;
    while (iv || !count) {
        d[count++] = iv & 0x7F;
        iv = iv >> 7;
    }

    while (count) {
        count--;
        int val = d[count];
        if (count)
            val |= 0x80;
        dev->putChar((char)val);
    }
}

/*!
    Encode a short integer value \a d to the output data stream.
*/
void QWspPduEncoder::encodeShortInteger(quint8 d)
{
    dev->putChar((char)(d | 0x80));
}

/*!
    Encode a long integer value \a iv to the output data stream.
*/
void QWspPduEncoder::encodeLongInteger(quint32 iv)
{
    unsigned char d[5];
    int count = 0;
    while (iv || !count) {
        d[count++] = iv & 0xFF;
        iv = iv >> 8;
    }

    encodeUInt8(count);
    while (count) {
        count--;
        dev->putChar((char)(d[count]));
    }
}

/*!
    Encode a short or long integer value \a d to the output data stream.
*/
void QWspPduEncoder::encodeInteger(quint32 d)
{
    if (d < 128)
        encodeShortInteger(d);
    else
        encodeLongInteger(d);
}

/*!
    Encode a length value \a d to the output data stream.
*/
void QWspPduEncoder::encodeLength(quint32 d)
{
    if (d <= 30) {
        encodeOctet(d);
    } else {
        encodeOctet(31);
        encodeUIntVar(d);
    }
}

/*!
    Encode a text string \a str to the output data stream.
*/
void QWspPduEncoder::encodeTextString(const QString &str)
{
    if ((uchar)str[0].toLatin1() >= 128)
        dev->putChar('\"');

    for (int i = 0; i < str.length(); i++)
        dev->putChar((char)(str[i].toLatin1()));

    if ((uchar)str[0].toLatin1() >= 128)
        dev->putChar('\"');

    dev->putChar(0);
}

/*!
    Encode an encoded string \a str to the output data stream.
*/
void QWspPduEncoder::encodeEncodedString(const QString &str)
{
    encodeTextString(str);
}

/*!
    Encode a token text string \a str to the output data stream.
*/
void QWspPduEncoder::encodeTokenText(const QString &str)
{
    encodeTextString(str);
}

/*!
    Encode a version \a value to the output data stream.
*/
void QWspPduEncoder::encodeVersion(const QString &value)
{
    int dot = value.indexOf(QChar('.'));
    int major = 0;
    int minor = 0;
    if (dot > 0) {
        major = value.left(dot).toInt();
        minor = value.mid(dot+1).toInt();
    } else {
        major = value.toInt();
    }
    if (major >= 1 && major <= 7 && minor <= 14){
        quint8 encVal;
        if (dot)
            encVal = (major << 4) | minor;
        else
            encVal = (major << 4) | 0x0F;
        encodeShortInteger(encVal);
    } else {
        encodeTextString(value);
    }
}

/*!
    Encode a \a field consisting of a name and value to the output data stream.
*/
void QWspPduEncoder::encodeField(const QWspField &field)
{
    if (!headerCodec->encode(*this, field)) {
        encodeTokenText(field.name);
        encodeTextString(field.value);
    }
}

/*!
    Encode a parameter value \a p to the output data stream.
*/
void QWspPduEncoder::encodeParameter(const QString &p)
{
    QString param = p;
    while (param.length() && param[0].isSpace())
        param = param.mid(1);
    while (param.length() && param[param.length()-1].isSpace())
        param.truncate(param.length()-1);
    if (param.indexOf("q=") == 0) {
        QString value = param.mid(2);
        int dot = value.indexOf(QChar('.'));
        if (dot >= 0)
            value = value.mid(dot+1);
        if (value.length() <= 2) {
            quint32 q = value.toUInt() * 100 + 1;
            encodeInteger(0x00);
            encodeUIntVar(q);
        } else if (value.length() == 3) {
            quint32 q = value.toUInt() * 1000 + 1;
            encodeInteger(0x00);
            encodeUIntVar(q);
        }
    } else if (param.indexOf("charset=") == 0) {
        encodeInteger(0x01);
        QString value = unquoteString(param.mid(8));
        if (value == "*") {
            encodeOctet(128);
        } else {
            int i = 0;
            while (charsetAssignments[i].number) {
                if (charsetAssignments[i].charset == value)
                    break;
                i++;
            }
            if (charsetAssignments[i].number)
                encodeInteger(charsetAssignments[i].number);
            else
                qWarning("Unknown charset");    //### unrecoverable
        }
    } else if (param.indexOf("level=") == 0) {
        encodeInteger(0x02);
        QString value = param.mid(6);
        int dot = value.indexOf(QChar('.'));
        int major = 0;
        int minor = 0;
        if (dot > 0) {
            major = value.left(dot).toInt();
            minor = value.mid(dot+1).toInt();
        } else {
            major = value.toInt();
        }
        if (major >= 1 && major <= 7 && minor <= 14){
            quint8 encVal;
            if (dot)
                encVal = (major << 4) | minor;
            else
                encVal = (major << 4) | 0x0F;
            encodeShortInteger(encVal);
        } else {
            encodeTextString(value);
        }
    } else if (param.indexOf("type=") == 0) {
        bool ok;
        quint32 d = param.mid(5).toUInt(&ok);
        if (ok) {
            encodeInteger(0x03);
            encodeInteger(d);
        } else {
            encodeInteger(0x09);
            QString value = unquoteString(param.mid(5));
            int idx = 0;
            while (contentTypeAssignments[idx]) {
                if (value == contentTypeAssignments[idx])
                    break;
                idx++;
            }

            if (contentTypeAssignments[idx]) {
                encodeShortInteger(idx);
            } else {
                encodeTextString(value);
            }
        }
    } else if (param.indexOf("name=") == 0) {
        encodeInteger(0x05);
        encodeTextString(unquoteString(param.mid(5)));
    } else if (param.indexOf("filename=") == 0) {
        encodeInteger(0x06);
        encodeTextString(unquoteString(param.mid(9)));
    } else if (param.indexOf("differences=") == 0) {
        encodeInteger(0x07);
        QString value = unquoteString(param.mid(12));
        bool ok;
        quint32 d = value.toUInt(&ok);
        if (ok) {
            encodeShortInteger(d);
        } else {
            encodeTokenText(value);
        }
    } else if (param.indexOf("padding=") == 0) {
        encodeInteger(0x08);
        encodeShortInteger(param.mid(7).toUInt());
    } else if (param.indexOf("start=") == 0) {
        encodeInteger(0x0A);
        encodeTextString(unquoteString(param.mid(6)));
    } else if (param.indexOf("start-info=") == 0) {
        encodeInteger(0x0B);
        encodeTextString(unquoteString(param.mid(11)));
    } else if (param.indexOf("comment=") == 0) {
        encodeInteger(0x0C);
        encodeTextString(unquoteString(param.mid(8)));
    } else if (param.indexOf("domain=") == 0) {
        encodeInteger(0x0D);
        encodeTextString(unquoteString(param.mid(7)));
    } else if (param.indexOf("max-age=") == 0) {
        encodeInteger(0x0E);
        QDateTime dt = QWspDateTime::parseDate(param.mid(8));
        encodeLongInteger(QWspDateTime::toGmtTime_t(dt));
    } else if (param.indexOf("path=") == 0) {
        encodeInteger(0x0F);
        encodeTextString(unquoteString(param.mid(5)));
    } else if (param == "secure") {
        encodeInteger(0x10);
    } else {
        int equal = param.indexOf('=');
        if (equal > 0) {
            encodeTokenText(param.left(equal));
            QString value = unquoteString(param.mid(equal+1));
            bool ok;
            quint32 d = value.toUInt(&ok);
            if (ok)
                encodeInteger(d);
            else
                encodeTextString(value);
        }
    }
}

/*!
    Encode a content type value \a str to the output data stream.
*/
void QWspPduEncoder::encodeContentType(const QString &str)
{
    // Content-type-value = Constrained-Media | Content-general-form
    // Constrained-Media = Constrained-encoding
    // Constrained-encoding = Extension-Media | Short-integer
    // Extension-Media *Text End-of-string
    // Content-general-form = Value-length Media-type
    // Media-type = (Well-known-media | Extension-Media) *(Parameter)
    // Well-known-media = Integer-value
    QStringList params = str.split(QChar(';'));

    int idx = 0;
    while (contentTypeAssignments[idx]) {
        if (params[0] == contentTypeAssignments[idx])
            break;
        idx++;
    }

    if (params.count() == 1) {
        // Constrained-Media
        if (contentTypeAssignments[idx]) {
            encodeShortInteger(idx);
        } else {
            encodeTextString(params[0]);
        }
    } else {
        // Content-general-form
        // Make a local encoding buffer so that we can find the length
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QWspPduEncoder bufEnc(&buffer);
        if (contentTypeAssignments[idx]) {
            // Well-known-media
            bufEnc.encodeInteger(idx);
        } else {
            // Extension-Media
            bufEnc.encodeTextString(params[0]);
        }
        // parameters
        for (int i = 1; i < params.count(); i++) {
            bufEnc.encodeParameter(params[i]);
        }

        // copy encBuf to dev.
        buffer.close();
        encodeLength(buffer.buffer().size());
        dev->write(buffer.buffer());
    }
}

/*!
    Encode a multipart content block \a mp to the output data stream.
*/
void QWspPduEncoder::encodeMultipart(const QWspMultipart &mp)
{
    encodeUIntVar(mp.count());
    for (int i = 0; i < mp.count(); i++) {
        encodePart(mp.part(i));
    }
}

/*!
    Encode a WSP \a part content block to the output data stream.
*/
void QWspPduEncoder::encodePart(const QWspPart &part)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QWspPduEncoder bufEnc(&buffer);

    const QWspField *f = part.header("Content-Type");
    if (f) {
        bufEnc.encodeContentType(f->value);
#ifndef PHONESIM
    } else {
        f = part.header("Content-Location");
        if (f) {
            QMimeType mime(f->value);
            bufEnc.encodeContentType(mime.id());
        } else {
            bufEnc.encodeContentType("*/*");    // last resort
        }
#endif
    }

    QList<QWspField>::ConstIterator it;
    for (it = part.headers().begin(); it != part.headers().end(); ++it) {
        if ((*it).name != "Content-Type")
            bufEnc.encodeField(*it);
    }

    quint32 hdrLen = buffer.buffer().size();

    encodeUIntVar(hdrLen);
    encodeUIntVar(part.data().size());
    buffer.close();

    dev->write(buffer.buffer());

    part.writeData(dev);
}

/*!
    Encode a WSP PDU PDU \a part to the output data stream.
*/
void QWspPduEncoder::encodePush(const QWspPush &part)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QWspPduEncoder bufEnc(&buffer);

    const QWspField *f = part.header("Content-Type");
    if (f) {
        bufEnc.encodeContentType(f->value);
#ifndef PHONESIM
    } else {
        f = part.header("Content-Location");
        if (f) {
            QMimeType mime(f->value);
            bufEnc.encodeContentType(mime.id());
        } else {
            bufEnc.encodeContentType("*/*");    // last resort
        }
#endif
    }

    QList<QWspField>::ConstIterator it;
    for (it = part.headers().begin(); it != part.headers().end(); ++it) {
        if ((*it).name != "Content-Type")
            bufEnc.encodeField(*it);
    }

    quint32 hdrLen = buffer.buffer().size();

    encodeOctet((quint8)(part.identifier()));
    encodeOctet((quint8)(part.pduType()));
    encodeUIntVar(hdrLen);
    buffer.close();

    dev->write(buffer.buffer());

    part.writeData(dev);
}

QString QWspPduEncoder::unquoteString(const QString &str)
{
    QString tmp(str);

    if ( tmp[0] == '\"' )
        tmp = tmp.right( tmp.length() - 1 );
    if ( tmp[(int)tmp.length() - 1] == '\"' )
        tmp = tmp.left( tmp.length() - 1 );

    return tmp;
}

/*!
    Get the number of bytes that will be needed to encode \a iv
    as a long integer.
*/
int QWspPduEncoder::longIntegerLength(quint32 iv)
{
    int count = 0;
    while (iv || !count) {
        iv = iv >> 8;
        count++;
    }

    return count+1;
}

/*!
    Get the number of bytes that will be needed to encode \a d
    as either a short or long integer.
*/
int QWspPduEncoder::integerLength(quint32 d)
{
    if (d < 128)
        return 1;
    else
        return longIntegerLength(d);
}

/*!
    \fn void QWspPduEncoder::setHeaderCodec(QWspHeaderCodec *c)

    Set the header codec, that converts string names into byte values, to \a c.
*/

//===========================================================================

/*!
  \class QWspPart
    \inpublicgroup QtTelephonyModule

  \brief The QWspPart class represents a single part within a WSP message.
  \ingroup telephony

  The QWspPart class represents a single part within a WSP message.
*/

/*!
    Construct an empty WSP part structure.
*/
QWspPart::QWspPart()
{
}

/*!
    Construct a WSP part structure that is a copy of \a part.
*/
QWspPart::QWspPart( const QWspPart& part )
{
    hdr = part.hdr;
    ba = part.ba;
}

/*!
    Destruct this WSP part.
*/
QWspPart::~QWspPart()
{
}

/*!
    Copy the contents of \a part into this object.
*/
QWspPart& QWspPart::operator=( const QWspPart& part )
{
    if ( this != &part ) {
        hdr = part.hdr;
        ba = part.ba;
    }
    return *this;
}

/*!
    \fn const QList<QWspField>& QWspPart::headers() const

    Return the list of headers associated with this WSP part.
*/

/*!
    Find the header called \a name and return its field block.
    Returns null if there is no such header.
*/
const QWspField *QWspPart::header(const QString &name) const
{
    QList<QWspField>::ConstIterator it;
    for (it = hdr.begin(); it != hdr.end(); ++it) {
        if ((*it).name == name)
            return &(*it);
    }

    return 0;
}

/*!
    Add header \a h to this WSP part.
*/
void QWspPart::addHeader(const QWspField &h)
{
    hdr.append(h);
}

/*!
    Add a header with the specified \a name and \a value to this WSP part.
*/
void QWspPart::addHeader(const QString &name, const QString &value)
{
    QWspField field;
    field.name = name;
    field.value = value;
    hdr.append(field);
}

/*!
    \fn const QByteArray& QWspPart::data() const

    Return the data body associated with this WSP part.
*/

/*!
    Set the data body of this WSP part to the \a l bytes from
    the array \a d.
*/
void QWspPart::setData(const char *d, int l)
{
    ba = QByteArray(d, l);
}

/*!
    Read \a l bytes from \a d and make them the body of this WSP part.
*/
void QWspPart::readData(QIODevice *d, int l)
{
    // Sanity-check the length, just in case something invalid was supplied.
    if ( l >= 0 && ( (d->isSequential() && l < 2097152) || (!d->isSequential() && l <= d->size() - d->pos()) ) ) {
        ba.resize(l);
        d->read(ba.data(), l);
    }
}

/*!
    Write the body of this WSP part to \a d.
*/
void QWspPart::writeData(QIODevice *d) const
{
    d->write(ba.data(), ba.size());
}

/*!
  \class QWspMultipart
    \inpublicgroup QtTelephonyModule

  \brief The QWspMultipart class represents a collection of parts from a WSP message.
  \ingroup telephony

  The QWspMultipart class represents a collection of parts from a WSP message.
*/

/*!
    Construct an empty WSP multipart structure.
*/
QWspMultipart::QWspMultipart()
{
}

/*!
    Construct a WSP multipart structure that is a copy of \a mpart.
*/
QWspMultipart::QWspMultipart( const QWspMultipart& mpart )
{
    parts = mpart.parts;
}

/*!
    Destruct this WSP multipart structure.
*/
QWspMultipart::~QWspMultipart()
{
}

/*!
    Copy the contents of \a mpart into this object.
*/
QWspMultipart& QWspMultipart::operator=( const QWspMultipart& mpart )
{
    if ( this != &mpart ) {
        parts = mpart.parts;
    }
    return *this;
}

/*!
    \fn int QWspMultipart::count() const

    Return the number of parts in this object.
*/

/*!
    Add part \a p to this object.
*/
void QWspMultipart::addPart(const QWspPart &p)
{
    parts.append(p);
}

/*!
    \fn const QWspPart & QWspMultipart::part(int idx) const

    Return the part at index \a idx within this object.
*/

/*!
  \class QWspPush
    \inpublicgroup QtTelephonyModule

  \brief The QWspPush class represents the contents of a WSP Push PDU.
  \ingroup telephony

  The QWspPush class represents the contents of a WSP Push PDU.
*/

/*!
    Construct an empty WSP Push PDU.
*/
QWspPush::QWspPush()
{
    ident = 0;
    pdu = 0x06;     // Unconfirmed push PDU type.
}

/*!
    Construct a WSP Push PDU that is a copy of \a push.
*/
QWspPush::QWspPush( const QWspPush& push )
    : QWspPart( push )
{
    ident = push.ident;
    pdu = push.pdu;
}

/*!
    Destruct this WSP Push PDU.
*/
QWspPush::~QWspPush()
{
}

/*!
    Copy the contents of \a push into this object.
*/
QWspPush& QWspPush::operator=( const QWspPush& push )
{
    QWspPart::operator=( push );
    if ( this != &push ) {
        ident = push.ident;
        pdu = push.pdu;
    }
    return *this;
}

/*!
    \fn int QWspPush::identifier() const

    Get the identifier associated with this PDU.
*/

/*!
    \fn int QWspPush::pduType() const

    Get the PDU type associated with this PDU.
*/

/*!
    \fn void QWspPush::setIdentifier( int value )

    Set the identifier associated with this PDU to \a value.
*/

/*!
    \fn void QWspPush::setPduType( int value )

    Set the PDU type associated with this PDU to \a value.
*/

/*!
    Decode the content type from the push PDU in \a data.
    This is quicker than using QWspPduDecoder::decodePush()
    and is suitable for quickly determining how to dispatch a
    push PDU.
*/
QString QWspPush::quickContentType( const QByteArray& data )
{
    uint posn = 0;
    uint size = data.size();
    int ch;
    QString type;

    // Skip the identifier.
    ++posn;

    // Check that the PDU type is 0x06 (indicating WSP-PUSH).
    if ( posn >= size || data[posn] != 0x06 )
        return QString();
    ++posn;

    // Skip the header length.
    while ( posn < size ) {
        ch = data[posn++];
        if ( (ch & 0x80) == 0 )
            break;
    }
    if ( posn >= size )
        return QString();

    // Decode the content type according to WAP-230.
    ch = data[posn] & 0xFF;
    if ( ch <= 31 ) {
        ++posn;
        if ( ch == 31 ) {
            while ( posn < size ) {
                ch = data[posn++];
                if ( (ch & 0x80) == 0 )
                    break;
            }
        }
        if ( posn >= size )
            return QString();
        ch = data[posn] & 0xFF;
        if ( (ch & 0x80) != 0 ) {
            ch &= 0x7F;
            if ( ch < numContentTypes )
                type = contentTypeAssignments[ch];
        } else {
            while ( ch != 0 ) {
                type += (QChar)ch;
                ++posn;
                if ( posn < size )
                    ch = data[posn] & 0xFF;
                else
                    ch = 0;
            }
        }
    } else if ( (ch & 0x80) != 0 ) {
        ch &= 0x7F;
        if ( ch < numContentTypes )
            type = contentTypeAssignments[ch];
    } else {
        while ( ch != 0 ) {
            type += (QChar)ch;
            ++posn;
            if ( posn < size )
                ch = data[posn] & 0xFF;
            else
                ch = 0;
        }
    }
    if ( type.isEmpty() )
        return QString();

    // Make sure that the content type does not contain
    // invalid characters, to protect the filesystem.
    for ( posn = 0; posn < (uint)(type.length()); ++posn ) {
        ch = (int)(type[posn].unicode());
        if ( ch >= 'A' && ch <= 'Z' )
            continue;
        if ( ch >= 'a' && ch <= 'z' )
            continue;
        if ( ch >= '0' && ch <= '9' )
            continue;
        if ( ch == '-' || ch == '.' || ch == '_' || ch == '+' )
            continue;
        if ( ch == '/' ) {
            // If the '/' is followed by '.', it may be an attempt
            // to do "xyz/..", which would be bad.
            if ( (posn + 1) < (uint)(type.length()) && type[posn + 1] == '.' )
                return QString();

            // Cannot use '/' at the start of a MIME type.
            if ( posn == 0 )
                return QString();

            // This '/' is OK to use.
            continue;
        }
        return QString();
    }
    return type;
}

//===========================================================================



QWspField QWspDefaultCodec::decode(QWspPduDecoder &dec)
{
    QWspField wapField;
    quint8 field = dec.decodeShortInteger();
    int idx = fieldFromNumber(field);
    if (idx >= 0) {
        const WAPHeaderField &fld = headerFieldNameAssignments[idx];
        wapField.name = fld.name;
        switch (fld.type) {
            case ContentBase:
            case ContentLocation:
            case ETag:
            case From:
            case Host:
            case IfMatch:
            case IfNoneMatch:
            case Location:
            case Referer:
            case Server:
            case Upgrade:
            case UserAgent:
            case Via:
            case XWapContentURI:
            case XWapInitiatorURI:
            case Profile:
            case ContentID:
                wapField.value = dec.decodeTextString();
                break;
            case Allow:
            case PushFlag:
                wapField.value = QString::number(dec.decodeShortInteger());
                break;
            case Date:
            case Expires:
            case IfModifiedSince:
            case IfUnmodifiedSince:
            case LastModified:
            case XWapTod:
                {
                    quint32 d = dec.decodeLongInteger();
                    QDateTime dt = QWspDateTime::fromGmtTime_t(d);
                    wapField.value = QWspDateTime::dateString(dt);
                }
                break;
            case Age:
            case ContentLength:
            case MaxForwards:
            case BearerIndication:
                wapField.value = QString::number(dec.decodeInteger());
                break;
            case ContentEncoding:
                {
                    if (dec.peekOctet() & 0x80) {
                        switch (dec.decodeOctet()) {
                            case 128:
                                wapField.value = "Gzip";
                                break;
                            case 129:
                                wapField.value = "Compress";
                                break;
                            case 130:
                                wapField.value = "Deflate";
                                break;
                            default:
                                qWarning("Unknown Content-encoding");
                                break;
                        }
                    } else {
                        wapField.value = dec.decodeTokenText();
                    }
                }
                break;
            case ContentLanguage:
                {
                    quint8 octet = dec.peekOctet();
                    if (octet == 128) {
                        dec.decodeOctet();
                        wapField.value = "*";
                    } else if (octet & 0x80) {
                        wapField.value = QString::number(dec.decodeInteger()); //### should decode
                    } else {
                        wapField.value = dec.decodeTokenText();
                    }
                }
                break;
            case ContentType:
                wapField.value = dec.decodeContentType();
                break;
            case XWapApplicationId:
                {
                    quint8 octet = dec.peekOctet();
                    if (octet & 0x80 || octet <= 31) {
                        wapField.value = QString::number(dec.decodeInteger());
                    } else {
                        wapField.value = dec.decodeTokenText();
                    }
                }
                break;
            case AcceptRanges:
                {
                    quint8 octet = dec.peekOctet();
                    if (octet == 128) {
                        wapField.value = "None";
                    } else if (octet == 129) {
                        wapField.value = "Bytes";
                    } else {
                        wapField.value = dec.decodeTokenText();
                    }
                }
                break;
            // ### None of these handled
            case Accept:
            case AcceptLanguage:
            case Authorization:
            case Connection:
            case ContentMD5:
            case IfRange:
            case Pragma:
            case ProxyAuthenticate:
            case ProxyAuthorization:
            case Public:
            case Range:
            case RetryAfter:
            case TransferEncoding:
            case Vary:
            case Warning:
            case WWWAuthenticate:
            case ContentDisposition:
            case AcceptApplication:
            case ProfileDiff:
            case ProfileWarning:
            case Expect:
            case TE:
            case Trailer:
            case AcceptCharset:
            case AcceptEncoding:
            case CacheControl:
            case ContentRange:
            case SetCookie:
            case Cookie:
            case EncodingVersion:
            default:
                {
                    qWarning("Unhandled WSP header: %s", fld.name);
                    quint8 octet = dec.peekOctet();
                    if (octet <= 31) {
                        quint32 len = dec.decodeLength();
                        while (len-- && !dec.device()->atEnd())
                            dec.decodeOctet();
                    } else if (octet & 0x80) {
                        dec.decodeOctet();
                    } else {
                        dec.decodeTokenText();
                    }
                    dec.setStatus(QWspPduDecoder::Unhandled);
                }
                break;
        }
    } else {
        qWarning("Cannot decode header field: %d", field);
    }

    return wapField;
}

bool QWspDefaultCodec::encode(QWspPduEncoder &enc, const QWspField &field)
{
    int idx = numberFromField(field.name);
    if (idx < 0)
        return false;

    const WAPHeaderField &fld = headerFieldNameAssignments[idx];
    if (fld.version > 12)   // ### We will only encode upto version 1.2
        return false;

    switch (fld.type) {
        case ContentBase:
        case ContentLocation:
        case ETag:
        case From:
        case Host:
        case IfMatch:
        case IfNoneMatch:
        case Location:
        case Referer:
        case Server:
        case Upgrade:
        case UserAgent:
        case Via:
        case XWapContentURI:
        case XWapInitiatorURI:
        case Profile:
        case ContentID:
            enc.encodeShortInteger(fld.number);
            enc.encodeTextString(field.value);
            break;
        case Allow:
        case PushFlag:
            enc.encodeShortInteger(fld.number);
            enc.encodeShortInteger(field.value.toUInt());
            break;
        case Date:
        case Expires:
        case IfModifiedSince:
        case IfUnmodifiedSince:
        case LastModified:
        case XWapTod:
            {
                enc.encodeShortInteger(fld.number);
                QDateTime dt = QWspDateTime::parseDate(field.value);
                enc.encodeLongInteger(QWspDateTime::toGmtTime_t(dt));
            }
            break;
        case Age:
        case ContentLength:
        case MaxForwards:
        case BearerIndication:
            enc.encodeShortInteger(fld.number);
            enc.encodeInteger(field.value.toUInt());
            break;
        case ContentEncoding:
            enc.encodeShortInteger(fld.number);
            if (field.value == "Gzip")
                enc.encodeOctet(128);
            else if (field.value == "Compress")
                enc.encodeOctet(129);
            else if (field.value == "Deflate")
                enc.encodeOctet(130);
            else
                enc.encodeTokenText(field.value);
            break;
        case ContentLanguage:
            //### Should encode
            enc.encodeShortInteger(fld.number);
            if (field.value == "*")
                enc.encodeOctet(128);
            else
                enc.encodeTokenText(field.value);
            break;
        case ContentType:
            enc.encodeShortInteger(fld.number);
            enc.encodeContentType(field.value);
            break;
        // ### None of these handled
        case Accept:
        case AcceptLanguage:
        case AcceptRanges:
        case Authorization:
        case Connection:
        case ContentMD5:
        case IfRange:
        case Pragma:
        case ProxyAuthenticate:
        case ProxyAuthorization:
        case Public:
        case Range:
        case RetryAfter:
        case TransferEncoding:
        case Vary:
        case Warning:
        case WWWAuthenticate:
        case ContentDisposition:
        case XWapApplicationId:
        case AcceptApplication:
        case ProfileDiff:
        case ProfileWarning:
        case Expect:
        case TE:
        case Trailer:
        case AcceptCharset:
        case AcceptEncoding:
        case CacheControl:
        case ContentRange:
        case SetCookie:
        case Cookie:
        case EncodingVersion:
        default:
            qWarning("Unhandled WSP header: %s", fld.name);
            return false;
    }

    return true;
}

int QWspDefaultCodec::fieldFromNumber(quint8 id)
{
    int i = 0;
    while (!(headerFieldNameAssignments[i].number & 0x80)) {
        if (headerFieldNameAssignments[i].number == id)
            return i;
        i++;
    }

    return -1;
}

int QWspDefaultCodec::numberFromField(const QString &str)
{
    int i = 0;
    while (!(headerFieldNameAssignments[i].number & 0x80)) {
        if (headerFieldNameAssignments[i].name == str)
            return i;
        i++;
    }

    return -1;
}

