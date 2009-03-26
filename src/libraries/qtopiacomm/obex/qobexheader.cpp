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
#include "qobexheader_p.h"
#include "qobexauthenticationchallenge_p.h"
#include "qobexauthentication_p.h"

#include <QList>
#include <QDataStream>
#include <QDebug>

#include <openobex/obex.h>
#include <netinet/in.h>

#include <typeinfo>


// the format for the Time header as defined in spec (IrOBEX 1.3, section 2.2.5)
// (but this is slightly modified to accommadate for how QDateTime likes the
// format string)
// Note the 'T' is the separator between time & date.
static const char UTC_TIME_POSTFIX = 'Z';
static const QString LOCAL_TIME_FORMAT = "yyyyMMddThhmmss";
static const QString UTC_TIME_FORMAT = LOCAL_TIME_FORMAT + UTC_TIME_POSTFIX;


QObexHeaderPrivate::QObexHeaderPrivate()
{
}

QObexHeaderPrivate::~QObexHeaderPrivate()
{
}

void QObexHeaderPrivate::setValue(int headerId, const QVariant &data)
{
    if (!m_hash.contains(headerId))
        m_ids.append(headerId);
    m_hash[headerId] = data;
}

QVariant QObexHeaderPrivate::value(int headerId) const
{
    return m_hash.value(headerId, QVariant());
}

bool QObexHeaderPrivate::remove(int headerId)
{
    if (headerId == QObexHeader::AuthChallenge && m_hash.contains(headerId))
        m_challengeNonce.clear();

    if (m_hash.contains(headerId)) {
        int index = m_ids.indexOf(headerId);
        Q_ASSERT(index != -1);
        m_ids.removeAt(index);
        m_hash.remove(headerId);
        return true;
    }
    return false;
}

bool QObexHeaderPrivate::contains(int headerId) const
{
    return m_hash.contains(headerId);
}

void QObexHeaderPrivate::clear()
{
    m_ids.clear();
    m_hash.clear();
    m_challengeNonce.clear();
}

QList<int> QObexHeaderPrivate::keys() const
{
    return m_ids;
}

int QObexHeaderPrivate::size() const
{
    return m_hash.size();
}

/*!
    \internal
    Takes the date & time stored in \a timeString (containing the date/time
    in ISO 8601 format) and stores it in \a dateTime.
*/
void QObexHeaderPrivate::dateTimeFromString(QDateTime &dateTime, const QString &timeString)
{
    if (timeString.isEmpty())
        return;

    if (timeString[timeString.size()-1] == UTC_TIME_POSTFIX) {
        dateTime = QDateTime::fromString(timeString, UTC_TIME_FORMAT);
        if (dateTime.isValid())
            dateTime.setTimeSpec(Qt::UTC);
    } else {
        dateTime = QDateTime::fromString(timeString, LOCAL_TIME_FORMAT);
        if (dateTime.isValid())
            dateTime.setTimeSpec(Qt::LocalTime);
    }
}

/*!
    \internal
    Sets \a timeString to contain the date & time from \a dateTime, in
    ISO 8601 format.
*/
void QObexHeaderPrivate::stringFromDateTime(QString &timeString, const QDateTime &dateTime)
{
    if (dateTime.isValid()) {
        timeString = dateTime.toString(dateTime.timeSpec() == Qt::UTC?
                UTC_TIME_FORMAT : LOCAL_TIME_FORMAT);
    }
}


/*!
    \internal
    Converts a unicode string from network order to host horder.
 */
static inline void convertNBOQString(QString &str)
{
    // Convert from network order to host order ...
    for (int i = 0; i < str.length(); i++)
        str[i] = QChar(ntohs(str[i].unicode()));
}

/*!
    \internal
    Converts a unicode string from host order to network order.
 */
static inline void convertHBOQString(QString &str)
{
    // Convert unicode string to network order
    for(int i = 0; i < str.length(); i++)
        str[i] = QChar(htons(str[i].unicode()));
}

/*!
    \internal
    Stores \a string into \a bytes as OBEX unicode data.

    The data will be stored in OBEX unicode format (UTF-16, network
    order, two null terminator bytes).
 */
void QObexHeaderPrivate::unicodeBytesFromString(QByteArray &bytes, const QString &string)
{
    if (string.isEmpty())
        return;

    // Shouldn't need to add null terminators ourselves because unicode()
    // should return null-terminated strings.
    QString uc = string;
    convertHBOQString(uc);

    int str_size = (uc.length() * 2) + 2;   // add 2 for extra null termintor bytes
    bytes.resize(str_size);
    memcpy(bytes.data(), reinterpret_cast<const uchar *>(uc.unicode()), str_size);
}

/*!
    \internal
    Takes the OBEX unicode data stored in \a data and puts it into
    \a string.

    The \a data is expected to be in OBEX unicode format (UTF-16, network
    order, two null terminator bytes).
 */
void QObexHeaderPrivate::stringFromUnicodeBytes(QString &string, const uchar *data, uint size)
{
    // size must be >= 2 because a utf-16 unicode char is 2 bytes
    if (size < 2)
        return;

    // The returned string does not include the null terminator (otherwise
    // if you compare the string to one without a null terminator it will
    // fail).

    const QChar *str = reinterpret_cast<const QChar *>(data);
    uint strsize = size >> 1;

    // OBEX unicode strings should end with two null terminators, but check
    // it just in case.
    if (data[size-1] == uchar('\0') && data[size-2] == uchar('\0'))
        strsize--;

    string.setUnicode(str, strsize);
    convertNBOQString(string);
}


/*!
    \internal
    Extracts all the headers from \a obj and inserts them into \a header.

    Remember you have to call OBEX_ObjectReParseHeaders() on \a obj if you
    want to read the headers again after calling this function.

    Returns whether the headers were read successfully.
 */
bool QObexHeaderPrivate::readOpenObexHeaders(QObexHeader &header, obex_t* handle, obex_object_t *obj)
{
    if (!handle || !obj)
        return false;

    uchar hi;
    obex_headerdata_t hv;
    unsigned int hv_size;

    while (OBEX_ObjectGetNextHeader(handle, obj, &hi, &hv, &hv_size))   {
        /*
        For each case, check whether the value is empty (e.g. name might
        be empty) otherwise it'll blow up...
        */

        // Special treatment for reading null-terminated byte stream
        if (hi == OBEX_HDR_TYPE) {
            if (hv_size == 0) {
                header.setValue(hi, QLatin1String("")); // empty, not null
            } else {
                // Remove the null terminator from the received string; otherwise,
                // if the string is compared to one without a null terminator
                // it will fail.
                if (hv_size > 0 && (hv.bs[hv_size - 1] == '\0')) {
                    // Null terminated, crop the string before the null
                    header.setValue(hi, QString::fromAscii(
                            reinterpret_cast<const char *>(hv.bs), hv_size-1));
                } else {
                    // Not null terminated
                    header.setValue(hi, QString::fromAscii(
                            reinterpret_cast<const char *>(hv.bs), hv_size));
                }
            }
            // skip to next header!
            continue;
        }

        switch (hi & QObexHeaderPrivate::HeaderEncodingMask) {
            case QObexHeaderPrivate::HeaderUnicodeEncoding:
                if (hv_size == 0) {
                    header.setValue(hi, QString("")); // empty, not null
                } else {
                    QString value;
                    QObexHeaderPrivate::stringFromUnicodeBytes(value, hv.bs, hv_size);
                    header.setValue(hi, value);
                }
                break;

            case QObexHeaderPrivate::HeaderByteSequenceEncoding:
                if (hv_size == 0) {
                    header.setValue(hi, QByteArray("")); // empty, not null
                } else {
                    header.setValue(hi, QByteArray(
                            reinterpret_cast<const char *>(hv.bs), hv_size));
                }
                break;

            case QObexHeaderPrivate::HeaderByteEncoding:
                // Must use qVariantFromValue() or else the value may be
                // added as zero in the variant.
                header.setValue(hi,
                                qVariantFromValue(static_cast<quint8>(hv.bq1)));
                break;

            case QObexHeaderPrivate::HeaderIntEncoding:
                header.setValue(hi,
                                qVariantFromValue(static_cast<quint32>(hv.bq4)));
                break;

            default:
                return false;
        }
    }

    return true;
}

/*!
    \internal
    Takes all the headers contained in \a header and adds them as headers into
    \a obj. If \a fitOnePacket is \c true, the OBEX_FL_FIT_ONE_PACKET flag
    is used.

    Returns whether the headers were written successfully.
 */
bool QObexHeaderPrivate::writeOpenObexHeaders(obex_t* handle, obex_object_t *obj, bool fitOnePacket, const QObexHeader &header)
{
    if (!handle || !obj)
        return false;

    int flags = ( fitOnePacket ? OBEX_FL_FIT_ONE_PACKET : 0 );
    obex_headerdata_t hv;

    // Handle Target & Connection Id headers separately:
    // Target must be the first header in operation, and Connection Id
    // must be 1st header in a request (spec 2.2.7 & 2.2.11).
    // This doesn't conflict cos they shouldn't both be in the same request
    // (and for responses it doesn't say)

    if (header.contains(QObexHeader::ConnectionId)) {
        hv.bq4 = header.connectionId();
        if (OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_CONNECTION, hv, 4,
                                flags) < 0) {
            return false;
        }
    }
    if (header.contains(QObexHeader::Target)) {
        QByteArray bytes = header.target();
        hv.bs = reinterpret_cast<const uchar *>(bytes.constData());
        if (OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_TARGET, hv, bytes.size(),
                             flags) < 0) {
            return false;
        }
    }

    // now go through all the other headers
    QList<int> headerIds = header.headerIds();

    for (int i=0; i<headerIds.size(); i++) {
        int headerId = headerIds[i];

        // skip the previously handled headers
        if (headerId == QObexHeader::ConnectionId ||
                headerId == QObexHeader::Target) {
            continue;
        }

        switch (headerId & QObexHeaderPrivate::HeaderEncodingMask) {

            case QObexHeaderPrivate::HeaderUnicodeEncoding:
            {
                QByteArray bytes;
                QObexHeaderPrivate::unicodeBytesFromString(bytes,
                        header.value(headerId).toString());
                hv.bs = reinterpret_cast<const uchar *>(bytes.constData());

                if (OBEX_ObjectAddHeader(handle, obj, headerId, hv,
                            bytes.size(), flags) < 0) {
                    return false;
                }
                break;
            }

            case QObexHeaderPrivate::HeaderByteSequenceEncoding:
            {
                if (headerId == QObexHeader::Type) {
                    // Handle Type header separately because it must be null terminated
                    // ascii. Use +1 when adding header to add a null terminator byte.
                    QByteArray bytes = header.type().toAscii();
                    hv.bs = reinterpret_cast<const uint8_t*>(bytes.constData());
                    if (OBEX_ObjectAddHeader(handle, obj, OBEX_HDR_TYPE, hv,
                                        bytes.size() + 1, flags) < 0) {
                        return false;
                    }

                    // Add PalmOS-style application id header
                    if (!bytes.isEmpty() && header.type().compare("text/x-vCalendar",
                                Qt::CaseInsensitive) == 0) {
                        hv.bq4 = 0x64617465;
                        if (OBEX_ObjectAddHeader(handle, obj, 0xcf, hv, 4, flags) < 0) {
                            return false;
                        }
                    }
                } else {
                    QByteArray bytes = header.value(headerId).toByteArray();
                    hv.bs = reinterpret_cast<const uchar *>(bytes.constData());
                    if (OBEX_ObjectAddHeader(handle, obj, headerId, hv, bytes.size(),
                                        flags) < 0) {
                        return false;
                    }
                }
                break;
            }

            case QObexHeaderPrivate::HeaderByteEncoding:
            {
                QVariant v = header.value(headerId);
                if (v.canConvert<quint8>()) {
                    hv.bq1 = v.value<quint8>();
                    if (OBEX_ObjectAddHeader(handle, obj, headerId, hv, 1,
                                         flags) < 0) {
                        return false;
                    }
                } else {
                    // this shouldn't happen since setValue() does a check
                    return false;
                }
                break;
            }

            case QObexHeaderPrivate::HeaderIntEncoding:
            {
                QVariant v = header.value(headerId);
                if (v.canConvert<quint32>()) {
                    hv.bq4 = v.value<quint32>();
                    if (OBEX_ObjectAddHeader(handle, obj, headerId, hv, 4,
                                         flags) < 0) {
                        return false;
                    }
                } else {
                    // this shouldn't happen since setValue() does a check
                    return false;
                }
                break;
            }

            default:
                return false;
        }
    }

    return true;
}


bool QObexHeaderPrivate::requestShouldFitOnePacket(QObex::Request request)
{
    switch (request)
    {
        case QObex::Connect:
            return true;
        case QObex::Disconnect:
            return true;
        case QObex::Put:
            return false;
        case QObex::PutDelete:
            return false;
        case QObex::Get:
            return false;
        case QObex::SetPath:
            return true;

        case QObex::NoRequest:
            break;
    }
    return true;
}

/*!
    \internal
    Returns a string description of the given header id.
 */
QString QObexHeaderPrivate::headerIdToString(int headerId)
{
    switch (headerId) {
        case QObexHeader::Count:
            return "Count";
        case QObexHeader::Name:
            return "Name";
        case QObexHeader::Type:
            return "Type";
        case QObexHeader::Length:
            return "Length";
        case QObexHeader::Time:
            return "Time";
        case QObexHeader::Description:
            return "Description";
        case QObexHeader::Target:
            return "Target";
        case QObexHeader::Http:
            return "Http";
        case QObexHeader::Who:
            return "Who";
        case QObexHeader::ConnectionId:
            return "ConnectionId";
        case QObexHeader::AppParameters:
            return "AppParameters";
        case QObexHeader::AuthChallenge:
            return "AuthChallenge";
        case QObexHeader::AuthResponse:
            return "AuthResponse";
        case QObexHeader::CreatorId:
            return "CreatorId";
        case QObexHeader::WanUuid:
            return "WanUuid";
        case QObexHeader::ObjectClass:
            return "ObjectClass";
        case QObexHeader::SessionParameters:
            return "SessionParameters";
        case QObexHeader::SessionSequenceNumber:
            return "SessionSequenceNumber";
        default:
            QString s;
            s.sprintf("<Unknown Header 0x%02x>", headerId);
            return s;
    }
}



//=========================================================


/*!
    \class QObexHeader
    \inpublicgroup QtBaseModule

    \brief The QObexHeader class contains header information for an OBEX request or response.

    The QObexHeader class provides a container for a set of headers for an
    OBEX request or response.

    Convenience methods are provided to set and get header values for
    all header identifiers that are defined in the IrOBEX specification; for
    example, setName(), name(), setType(), type().

    Additionally, setValue() and value() can be used to set and retrieve
    custom header values as QVariant values.

    \ingroup qtopiaobex
    \sa QObexClientSession, QObexServerSession
 */

/*!
    \enum QObexHeader::HeaderId
    This enum defines the Header Identifiers that can be added to a
    QObexHeader. Note the range 0x30 to 0x3F can be used for user-defined
    headers.

    \value Count The number of objects in the operation.
    \value Name The name of the object.
    \value Type The type of the object.
    \value Length The size of the object.
    \value Time A date/time stamp in ISO 8601 format.
    \value Description A text description of the object.
    \value Target The service that is the target of the operation.
    \value Http A HTTP 1.x header.
    \value Who Identifies the OBEX application.
    \value ConnectionId An identifier used for OBEX connection multiplexing.
    \value AppParameters Used for extended application request and response information.
    \value AuthChallenge An authentication digest-challenge.
    \value AuthResponse An authentication digest-response.
    \value CreatorId Indicates the creator of an object.
    \value WanUuid Uniquely identifies a network client (OBEX server).
    \value ObjectClass Details the object class and properties.
    \value SessionParameters Parameters used in session commands and responses.
    \value SessionSequenceNumber Sequence number used in each OBEX packet for reliability.
    \value MaximumHeaderId The maximum value for a Header Identifer, as defined in the OBEX specification.
*/


/*!
    Constructs an empty QObexHeader.
 */
QObexHeader::QObexHeader()
    : m_data(new QObexHeaderPrivate)
{
}

/*!
    Constructs a QObexHeader with the contents from \a other.
*/
QObexHeader::QObexHeader(const QObexHeader &other)
    : m_data(new QObexHeaderPrivate)
{
    operator=(other);
}

/*!
    Destroys the header.
 */
QObexHeader::~QObexHeader()
{
    delete m_data;
}

/*!
    Assigns the contents of \a other to this object and returns a reference
    to this header.
*/
QObexHeader &QObexHeader::operator=(const QObexHeader &other)
{
    if (this == &other)
        return *this;

    m_data->m_ids = other.m_data->m_ids;
    m_data->m_hash = other.m_data->m_hash;
    m_data->m_challengeNonce = other.m_data->m_challengeNonce;

    return *this;
}

/*!
    Returns \c true if \a other contains the same values as this header;
    otherwise returns \c false.

    \sa operator!=()
*/
bool QObexHeader::operator==(const QObexHeader &other) const
{
    if (this == &other)
        return true;

    // The header ordering is not checked - only the header values. As long
    // as the values are the same, we will consider the headers to be the same.
    return (m_data->m_hash == other.m_data->m_hash);
}

/*!
    \fn bool QObexHeader::operator!=(const QObexHeader &other) const
    Returns \c true if \a other is \i not equal to this header;
    otherwise returns \c false.

    \sa operator==()
*/

/*!
    Returns true if this set of headers has a header entry with the header
    identifier \a headerId; otherwise returns false.

    \sa headerIds()
 */
bool QObexHeader::contains(int headerId) const
{
    return m_data->contains(headerId);
}

/*!
    Returns a list of the header identifiers in this set of headers.

    \sa contains()
 */
QList<int> QObexHeader::headerIds() const
{
    return m_data->keys();
}

/*!
    Returns the number of entries in the OBEX header.
 */
int QObexHeader::size() const
{
    return m_data->size();
}

/*!
    Sets the value of the OBEX header field \c Count to \a count.

    This value indicates the number of objects involved in the OBEX operation.

    \sa count()
 */
void QObexHeader::setCount(quint32 count)
{
    setValue(Count, qVariantFromValue(count));
}

/*!
    Returns the value of the OBEX header field \c Count.

    Returns 0 if the \c Count header is not present.

    \sa setCount()
 */
quint32 QObexHeader::count() const
{
    if (!contains(Count))
        return 0;
    QVariant c = value(Count);
    if (c.canConvert<quint32>())
        return c.value<quint32>();

    return 0;
}

/*!
    Sets the value of the OBEX header field \c Name to \a name.

    If an object is being sent to another device, this value should generally
    be the filename of the object.

    An empty \a name string is used to specify particular behaviors for some
    \c GET and \c SETPATH operations. For \c GET, an empty string means that
    the "default" object should be retrieved. For \c SETPATH, an empty string
    means the path should be reset to the "default" folder.

    \sa name()
 */
void QObexHeader::setName(const QString &name)
{
    setValue(Name, QVariant(name));
}

/*!
    Returns the value of the OBEX header field \c Name.

    Returns an empty string if the \c Name header is not present.

    \sa setName()
 */
QString QObexHeader::name() const
{
    if (!contains(Name))
        return QString();
    return value(Name).toString();
}

/*!
    Sets the value of the OBEX header field \c Type to \a type.

    This header corresponds to the content-type header in HTTP. The \a type
    value should be a standard ASCII mime type string, e.g. "text/plain",
    "image/jpeg".

    \sa type()
 */
void QObexHeader::setType(const QString &type)
{
    setValue(Type, QVariant(type));
}

/*!
    Returns the value of the OBEX header field \c Type.

    Returns an empty string if the \c Type header is not present.

    \sa setType()
 */
QString QObexHeader::type() const
{
    if (!contains(Type))
        return QString();
    return value(Type).toString();
}

/*!
    Sets the value of the OBEX header field \c Length to \a length.

    If the length of the object to be transferred is known in advance, this
    header should be used.

    \sa length()
 */
void QObexHeader::setLength(quint32 length)
{
    setValue(Length, qVariantFromValue(length));
}

/*!
    Returns the value of the OBEX header field \c Length.

    Returns 0 if the \c Length header is not present.

    \sa setLength()
 */
quint32 QObexHeader::length() const
{
    if (!contains(Length))
        return 0;
    QVariant len = value(Length);
    if (len.canConvert<quint32>())
        return len.value<quint32>();

    return 0;
}

/*!
    If \a dateTime is a valid QDateTime object, sets the value of the OBEX 
    header field \c Time to \a dateTime.

    \sa time()
 */
void QObexHeader::setTime(const QDateTime &dateTime)
{
    if (!dateTime.isValid()) 
        return;
    QString s;
    QObexHeaderPrivate::stringFromDateTime(s, dateTime);
    setValue(Time, s);
}

/*!
    Returns the value of the OBEX header field \c Time.

    Returns an invalid QDateTime object if the \c Time header is not present,
    or if the header value does not conform to the ISO 8601 format.

    \sa setTime()
 */
QDateTime QObexHeader::time() const
{
    if (!contains(Time))
        return QDateTime();

    QDateTime dateTime;
    QObexHeaderPrivate::dateTimeFromString(dateTime, value(Time).toString());
    return dateTime;
}

/*!
    Sets the value of the OBEX header field \c Description to \a description.

    This value can be used to provide additional information about the
    operation.

    \sa description()
 */
void QObexHeader::setDescription(const QString &description)
{
    setValue(Description, QVariant(description));
}

/*!
    Returns the value of the OBEX header field \c Description.

    Returns an empty string if the \c Description header is not present.

    \sa setDescription()
 */
QString QObexHeader::description() const
{
    if (!contains(Description))
        return QString();
    return value(Description).toString();
}

/*!
    Sets the value of the OBEX header field \c Target to \a target.

    This value should identify the intended target of an OBEX operation.

    \bold {Note:} If this header is to be sent in an OBEX request, you should not
    also add a \c {Connection Id} value, as it is illegal to send a \c Target
    header and a \c {Connection Id} header in the same \i request (as stated
    in the OBEX specification).

    \sa target()
 */
void QObexHeader::setTarget(const QByteArray &target)
{
    setValue(Target, QVariant(target));
}

/*!
    Returns the value of the OBEX header field \c Target.

    Returns an empty byte array if the \c Target header is not present.

    \sa setTarget()
 */
QByteArray QObexHeader::target() const
{
    if (!contains(Target))
        return QByteArray();
    return value(Target).toByteArray();
}

/*!
    Sets the value of the OBEX header field \c HTTP to \a http.

    This value should be a byte sequence containing a HTTP 1.x header.

    \sa http()
 */
void QObexHeader::setHttp(const QByteArray &http)
{
    setValue(Http, QVariant(http));
}

/*!
    Returns the value of the OBEX header field \c HTTP.

    Returns an empty byte array if the \c HTTP header is not present.

    \sa setHttp()
 */
QByteArray QObexHeader::http() const
{
    if (!contains(Http))
        return QByteArray();
    return value(Http).toByteArray();
}

/*!
    Sets the value of the OBEX header field \c Who to \a who.

    This value can be used to allow peer applications to identify each other.

    \sa who()
 */
void QObexHeader::setWho(const QByteArray &who)
{
    setValue(Who, QVariant(who));
}

/*!
    Returns the value of the OBEX header field \c Who.

    Returns an empty byte array if the \c Who header is not present.

    \sa setWho()
 */
QByteArray QObexHeader::who() const
{
    if (!contains(Who))
        return QByteArray();
    return value(Who).toByteArray();
}

/*!
    Sets the value of the OBEX header field \c {Connection Id} to
    \a connectionId.

    This value identifies the OBEX connection to which an OBEX request
    belongs, in order to differentiate between multiple clients on the same
    connection.

    \bold {Note:} If this header is to be sent in an OBEX request, you should not
    also add a \c Target value, as it is illegal to send a \c Target
    header and a \c {Connection Id} header in the same \i request (as stated
    in the OBEX specification).

    \sa connectionId(), QObexClientSession::connectionId()
 */
void QObexHeader::setConnectionId(quint32 connectionId)
{
    setValue(ConnectionId, qVariantFromValue(connectionId));
}

/*!
    Returns the value of the OBEX header field \c {Connection Id}.

    Returns 0 if the \c {Connection Id} header is not present.

    \sa setConnectionId()
 */
quint32 QObexHeader::connectionId() const
{
    if (!contains(ConnectionId))
        return 0;
    QVariant headerId = value(ConnectionId);
    if (headerId.canConvert<quint32>())
        return headerId.value<quint32>();

    return 0;
}

/*!
    Sets the value of the OBEX header field \c {Application Parameters}
    to \a params.

    \sa appParameters()
 */
void QObexHeader::setAppParameters(const QByteArray &params)
{
    setValue(AppParameters, QVariant(params));
}

/*!
    Returns the value of the OBEX header field
    \c {Application Parameters}.

    Returns an empty byte array if the \c {Application Parameters} header is
    not present.

    \sa setAppParameters()
 */
QByteArray QObexHeader::appParameters() const
{
    if (!contains(AppParameters))
        return QByteArray();
    return value(AppParameters).toByteArray();
}

/*!
    Sets the value of the OBEX header field \c {Creator ID}
    to \a creatorId.

    \sa creatorId()
 */
void QObexHeader::setCreatorId(quint32 creatorId)
{
    setValue(CreatorId, qVariantFromValue(creatorId));
}

/*!
    Returns the value of the OBEX header field \c {Connection Id}.

    Returns 0 if the \c {Creator ID} header is not present.

    \sa setCreatorId()
 */
quint32 QObexHeader::creatorId() const
{
    if (!contains(CreatorId))
        return 0;
    QVariant headerId = value(CreatorId);
    if (headerId.canConvert<quint32>())
        return headerId.value<quint32>();

    return 0;
}

/*!
    Sets the value of the OBEX header field \c {WAN UUID}
    to \a uuid.

    \sa wanUuid()
 */
void QObexHeader::setWanUuid(QUuid uuid)
{
    //setValue(WanUuid, QVariant(uuid.toString()));
    QByteArray bytes;
    QDataStream dataStream(&bytes, QIODevice::WriteOnly);
    dataStream << uuid;
    setValue(WanUuid, bytes);
}

/*!
    Returns the value of the OBEX header field \c {WAN UUID}.

    Returns a null UUID if the \c {WAN UUID} header is not present.

    \sa setWanUuid()
 */
QUuid QObexHeader::wanUuid() const
{
    if (!contains(WanUuid))
        return QUuid();
    //return QUuid(value(WanUuid).toString());

    QByteArray bytes = value(WanUuid).toByteArray();
    if (bytes.isEmpty())
        return QUuid();

    QUuid uuid;
    QDataStream stream(bytes);
    stream >> uuid;
    return uuid;
}

/*!
    Sets the value of the OBEX header field \c {Object Class}
    to \a objectClass.

    \sa objectClass()
 */
void QObexHeader::setObjectClass(const QByteArray &objectClass)
{
    setValue(ObjectClass, QVariant(objectClass));
}

/*!
    Returns the value of the OBEX header field \c {Object Class}.

    Returns an empty byte array if the \c {Object Class} header is not present.

    \sa setObjectClass()
 */
QByteArray QObexHeader::objectClass() const
{
    if (!contains(ObjectClass))
        return QByteArray();
    return value(ObjectClass).toByteArray();
}

/*!
    Sets the value of the OBEX header field \c {Session-Parameters}
    to \a params.

    \sa sessionParameters()
 */
void QObexHeader::setSessionParameters(const QByteArray &params)
{
    setValue(SessionParameters, QVariant(params));
}

/*!
    Returns the value of the OBEX header field \c {Session-Parameters}.

    Returns an empty byte array if the \c {Session-Parameters} header is not
    present.

    \sa setSessionParameters()
 */
QByteArray QObexHeader::sessionParameters() const
{
    if (!contains(SessionParameters))
        return QByteArray();
    return value(SessionParameters).toByteArray();
}

/*!
    Sets the value of the OBEX header field
    \c {Session-Sequence-Number} to \a num.

    \sa sessionSequenceNumber()
 */
void QObexHeader::setSessionSequenceNumber(quint8 num)
{
    setValue(SessionSequenceNumber, qVariantFromValue(num));
}

/*!
    Returns the value of the OBEX header field
    \c {Session-Sequence-Number}.

    Returns 0 if the \c {Session-Sequence-Number} header is not present.

    \sa setSessionSequenceNumber()
 */
quint8 QObexHeader::sessionSequenceNumber() const
{
    if (!contains(SessionSequenceNumber))
        return 0;
    QVariant num = value(SessionSequenceNumber);
    if (num.canConvert<quint8>())
        return num.value<quint8>();

    return 0;
}

/*!
    Sets the value of the OBEX header field \c {Authentication Challenge}
    to an authentication challenge generated using the given \a options and
    \a realm.

    The \a realm is a string that can be displayed to users to inform
    them of the username and password to be used for authentication. The
    supported encodings for the \a realm are ASCII, ISO-8859-1 and Unicode.

    The options and realm fields are optional when creating a challenge.
*/
void QObexHeader::setAuthenticationChallenge(QObex::AuthChallengeOptions options,
                                             const QString &realm)
{
    QByteArray nonce;
    QObexAuth::generateNonce(nonce);
    m_data->m_challengeNonce = nonce;    // so obex client/server can use it for validation

    QByteArray bytes;
    QObexAuthenticationChallengePrivate::writeRawChallenge(nonce, options, realm, bytes);
    m_data->setValue(QObexHeader::AuthChallenge, QVariant(bytes));
}

/*!
    Sets the value of the header entry with identifier \a headerId to the
    value in \a variant.

    The \a variant must be able to be converted into one of the following
    types:
    \list
    \o QString
    \o QByteArray
    \o \c quint8
    \o \c quint32
    \endlist

    The header encoding of \a headerId will be used to determine which of
    the data types should be used for conversion.

    Returns true if the value was set for the header. Otherwise, returns
    \c false (for example, if the value in \a variant cannot be converted to
    the data type specified by \a headerId).

    \sa value()
 */
bool QObexHeader::setValue(int headerId, const QVariant &variant)
{
    // well, we don't really extract the data here, we just want to
    // check it *can* be converted later on in writeOpenObexHeader
    switch (headerId & QObexHeaderPrivate::HeaderEncodingMask) {
        case QObexHeaderPrivate::HeaderIntEncoding:
            if (!variant.canConvert<quint32>())
                return false;
            break;
        case QObexHeaderPrivate::HeaderByteEncoding:
            if (!variant.canConvert<quint8>())
                return false;
            break;
        case QObexHeaderPrivate::HeaderByteSequenceEncoding:
            if (!variant.canConvert(QVariant::ByteArray))
                return false;
            break;
        case QObexHeaderPrivate::HeaderUnicodeEncoding:
            if (!variant.canConvert(QVariant::String))
                return false;
            break;
        default:
            return false;
    }

    // go ahead and set it
    m_data->setValue(headerId, variant);
    return true;
}

/*!
    Returns the value of the header entry with identifier \a headerId.

    Returns an invalid variant if no entry exists for \a headerId.

    \sa setValue()
 */
QVariant QObexHeader::value(int headerId) const
{
    return m_data->value(headerId);
}

/*!
    Removes the header entry with identifier \a headerId.

    Returns true if the value was removed, or returns false if no entry exists
    for the identifier \a headerId.
 */
bool QObexHeader::remove(int headerId)
{
    return m_data->remove(headerId);
}

/*!
    Removes all header entries.
*/
void QObexHeader::clear()
{
    m_data->clear();
}

/*!
    Returns a QDateTime that represents the date/time specified by
    \a dateTimeString, which must be in the \c Time header value format
    defined in the OBEX specification. That is, it must be in the
    format YYYYMMDDTHHMMSS (for local times) or YYYYMMDDTHHMMSSZ (for
    UTC time). The letter 'T' separates the date from the time.

    Returns an invalid QDateTime if \a dateTimeString could not be parsed.

    \sa stringFromDateTime()
*/
QDateTime QObexHeader::dateTimeFromString(const QString &dateTimeString)
{
    QDateTime dateTime;
    QObexHeaderPrivate::dateTimeFromString(dateTime, dateTimeString);
    return dateTime;
}

/*!
    Returns a date/time string that represents \a dateTime in the format
    defined for the \c Time header value in the OBEX specification.

    The \a dateTime \l {QDateTime::timeSpec()}{timeSpec()} value is used to
    determine the time zone information. That is, the string will be in
    the format YYYYMMDDTHHMMSS if \l {QDateTime::timeSpec()}{timeSpec()}
    is Qt::LocalTime, or the format YYYYMMDDTHHMMSSZ if it is Qt::UTC.

    Returns an empty string if \a dateTime is invalid.

    \sa dateTimeFromString()
*/
QString QObexHeader::stringFromDateTime(const QDateTime &dateTime)
{
    QString s;
    QObexHeaderPrivate::stringFromDateTime(s, dateTime);
    return s;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QObexHeader &header)
{
    dbg.nospace() << "QObexHeader({";
    QList<int> ids = header.headerIds();
    for (int i=0; i<ids.size(); i++) {
        dbg.nospace() << QObexHeaderPrivate::headerIdToString(ids[i]);
        dbg.nospace() << ": ";
        dbg.nospace() << header.value(ids[i]).toString();
        if (i < ids.size()-1)
            dbg.nospace() << ", ";
    }
    dbg.nospace() << " })";
    return dbg.space();
}
#endif
