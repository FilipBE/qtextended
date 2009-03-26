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

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>
#include "qsdpxmlparser_p.h"

#include <QString>
#include <QUrl>

static bool sdp_compare(const QVariant &lhs, const QVariant &rhs)
{
    if (lhs.userType() != rhs.userType())
        return false;

    if ((lhs.type() != QVariant::UserType) && (lhs != rhs))
        return false;

    if (lhs.userType() == qMetaTypeId<quint128>()) {
        if (lhs.value<quint128>() != rhs.value<quint128>())
            return false;
    }
    else if (lhs.userType() == qMetaTypeId<qint128>()) {
        if (lhs.value<qint128>() != rhs.value<qint128>())
            return false;
    }
    else if (lhs.userType() == qMetaTypeId<QBluetoothSdpUuid>()) {
        if (lhs.value<QBluetoothSdpUuid>() != rhs.value<QBluetoothSdpUuid>())
            return false;
    }
    else if (lhs.userType() == qMetaTypeId<QBluetoothSdpSequence>()) {
        if (lhs.value<QBluetoothSdpSequence>() != rhs.value<QBluetoothSdpSequence>())
            return false;
    }
    else if (lhs.userType() == qMetaTypeId<QBluetoothSdpAlternative>()) {
        if (lhs.value<QBluetoothSdpAlternative>() != rhs.value<QBluetoothSdpAlternative>())
            return false;
    }

    return true;
}

static bool sdp_compare(const QList<QVariant> &lhs, const QList<QVariant> &rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (int i = 0; i < lhs.size(); i++) {
        if (!sdp_compare(lhs[i], rhs[i]))
            return false;
    }

    return true;
}

/*!
    \class QBluetoothSdpSequence
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothSdpSequence class is a convenience wrapper used in QBluetoothSdpRecord.

    This class is a convenience class introduced for the purpose
    of distinguishing attributes that represent SDP Sequences
    from attributes that represent SDP Alternates.  Both SDP
    Alternate and Sequence types can be represented as
    sequences of \c QVariant objects.  The QBluetoothSdpSequence and
    QBluetoothSdpAlternative were introduced to make it easy to
    distinguish between these types.

    \ingroup qtopiabluetooth
    \sa QBluetoothSdpAlternative
*/

/*!
    Comparison operator.  Returns true if the current object is equal
    to \a other.
*/
bool QBluetoothSdpSequence::operator==(const QBluetoothSdpSequence &other) const
{
    return sdp_compare(*this, other);
}

/*!
    \fn bool QBluetoothSdpSequence::operator!=(const QBluetoothSdpSequence &other) const

    Comparison operator.  Returns true if the current object is
    not equal to \a other.
*/

/*!
    \class QBluetoothSdpAlternative
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothSdpAlternative class is a convenience wrapper used in QBluetoothSdpRecord.

    This class is a convenience class introduced for the
    purpose of distinguishing attributes that represent SDP
    Sequences from attributes that represent SDP Alternates.
    Both SDP Alternate and Sequence types can be represented
    as sequences of \c QVariant objects.  The QBluetoothSdpSequence
    and QBluetoothSdpAlternative were introduced to make it easy to
    distinguish between these types.

    \ingroup qtopiabluetooth
    \sa QBluetoothSdpSequence
 */

/*!
    Comparison operator.  Returns true if the current object is equal
    to \a other.
*/
bool QBluetoothSdpAlternative::operator==(const QBluetoothSdpAlternative &other) const
{
    return sdp_compare(*this, other);
}

/*!
    \fn bool QBluetoothSdpAlternative::operator!=(const QBluetoothSdpAlternative &other) const

    Comparison operator.  Returns true if the current object is
    not equal to \a other.
*/

/*!
    \class QBluetoothSdpRecord
    \inpublicgroup QtBluetoothModule

    \brief The QBluetoothSdpRecord class represents a bluetooth SDP record.

    Each Bluetooth record is composed of zero or more attributes.  Each
    attribute contains exactly one value.  To group several values, sequences
    or alternatives are used.  Each attribute has a unique 16 bit identifier
    associated with it.  The mapping between SDP basic types and the types
    used by the QBluetoothSdpRecord implementation are given below:

    \table
        \header
            \o SDP Type
            \o Qt Type
        \row
            \o UINT8
            \o quint8
        \row
            \o INT8
            \o qint8
        \row
            \o UINT16
            \o quint16
        \row
            \o INT16
            \o qint16
        \row
            \o UINT32
            \o quint32
        \row
            \o INT32
            \o qint32
        \row
            \o UINT64
            \o quint64
        \row
            \o INT64
            \o qint64
        \row
            \o UINT128
            \o quint128
        \row
            \o INT128
            \o qint128
        \row
            \o BOOL
            \o bool
        \row
            \o Alternate
            \o QBluetoothSdpAlternative
        \row
            \o Sequence
            \o QBluetoothSdpSequence
        \row
            \o UUID16, UUID32, UUID128
            \o QBluetoothSdpUuid
        \row
            \o URL
            \o QUrl
        \row
            \o TEXT
            \o QString
        \row
            \o TEXT (binary data)
            \o QByteArray
        \endtable

    The attributes are stored as QVariants.

    For example, the following code retrieves the value of the \c ServiceClassIDList attribute:
    \code
       QBluetoothSdpRecord record = ...
       quint16 UUID_SERVICE_CLASS_ID_LIST = 0x0001;     // as defined in Bluetooth specification
       QBluetoothSdpSequence serviceClassIdList = record.attribute(UUID_SERVICE_CLASS_ID_LIST).value<QBluetoothSdpSequence>();
       for (int i=0; i<serviceClassIdList.count(); i++)
            qDebug() << "Service class ID:" << serviceClassIdList[i];
    \endcode

    Typically a QBluetoothSdpRecord is populated by:
    \list
        \o Creating an empty SDP record and adding attributes using \l QBluetoothSdpRecord::addAttribute(), or
        \o Loading all record data using the static methods \l QBluetoothSdpRecord::fromData() or \l QBluetoothSdpRecord::fromDevice()
    \endlist

    \ingroup qtopiabluetooth
    \sa QVariant, QBluetoothSdpSequence, QBluetoothSdpAlternative
 */

static const int ServiceClassIDList = 0x0001;
static const int ProtocolDescriptorList = 0x0004;

static QBluetoothSdpUuid OBEXObjectPushUUID(static_cast<quint16>(0x1105));
static QBluetoothSdpUuid DirectPrintingUUID(static_cast<quint16>(0x1118));
static QBluetoothSdpUuid RFCOMMProtocol(static_cast<quint16>(0x0003));
static QBluetoothSdpUuid DialupNetworkingUUID(static_cast<quint16>(0x1103));

static const quint16 SVCNAME_PRIMARY =  0x0100;
static const quint16 SVCDESC_PRIMARY =  0x0101;
static const quint16 PROVNAME_PRIMARY = 0x0102;
static const quint16 DOC_URL = 0x000a;
static const quint16 CLNT_EXEC_URL = 0x000b;
static const quint16 ICON_URL = 0x000c;
static const quint16 SERVICE_ID = 0x0003;
static const quint16 GROUP_ID = 0x0200;
static const quint16 BROWSE_GRP_LIST = 0x0005;
static const quint16 RECORD_HANDLE = 0x0000;

/*!
    Construct a new empty SDP Service record.
 */
QBluetoothSdpRecord::QBluetoothSdpRecord()
{

}

/*!
    Deconstruct a SDP Service record.
 */
QBluetoothSdpRecord::~QBluetoothSdpRecord()
{

}

/*!
    Returns \c true if this SDP service record has no attributes.
*/
bool QBluetoothSdpRecord::isNull() const
{
    return m_attrs.isEmpty();
}

/*!
    Construct a SDP service record, copying contents from \a other.
*/
QBluetoothSdpRecord::QBluetoothSdpRecord(const QBluetoothSdpRecord &other)
{
    operator=(other);
}

/*!
    Assign the contents of \a other to the current SDP service record.
 */
QBluetoothSdpRecord &QBluetoothSdpRecord::operator=(const QBluetoothSdpRecord &other)
{
    if (this == &other)
        return *this;

    m_attrs = other.m_attrs;
    return *this;
}

/*!
    Returns whether \a other is equal to this SDP service record.
 */
bool QBluetoothSdpRecord::operator==(const QBluetoothSdpRecord &other) const
{
    if (m_attrs.size() != other.m_attrs.size())
        return false;

    QMap<quint16, QVariant>::const_iterator it1 = m_attrs.begin();
    QMap<quint16, QVariant>::const_iterator it2 = other.m_attrs.begin();

    while (it1 != m_attrs.end()) {
        if (!(sdp_compare(it1.value(), it2.value())) || qMapLessThanKey(it1.key(), it2.key()) || qMapLessThanKey(it2.key(), it1.key()))
            return false;
        ++it2;
        ++it1;
    }
    return true;

    return other.m_attrs == m_attrs;
}

/*!
    Returns the service name attribute.

    \sa setServiceName()
*/
QString QBluetoothSdpRecord::serviceName() const
{
    return m_attrs.value(SVCNAME_PRIMARY).value<QString>();
}

/*!
    Sets the service name attribute to \a serviceName.

    \sa serviceName()
*/
void QBluetoothSdpRecord::setServiceName(const QString &serviceName)
{
    m_attrs.insert(SVCNAME_PRIMARY, QVariant::fromValue(serviceName));
}

/*!
    Returns the service description attribute.

    \sa setServiceDescription()
*/
QString QBluetoothSdpRecord::serviceDescription() const
{
    return m_attrs.value(SVCDESC_PRIMARY).value<QString>();
}

/*!
    Sets the service description attribute to \a serviceDesc.

    \sa serviceDescription()
*/
void QBluetoothSdpRecord::setServiceDescription(const QString &serviceDesc)
{
    m_attrs.insert(SVCDESC_PRIMARY, QVariant::fromValue(serviceDesc));
}

/*!
    Returns the provider name attribute.

    \sa setProviderName()
*/
QString QBluetoothSdpRecord::providerName() const
{
    return m_attrs.value(PROVNAME_PRIMARY).value<QString>();
}

/*!
    Sets the provider name attribute to \a providerName

    \sa providerName()
*/
void QBluetoothSdpRecord::setProviderName(const QString &providerName)
{
    m_attrs.insert(PROVNAME_PRIMARY, QVariant::fromValue(providerName));
}

/*!
    Returns the Doc URL attribute.

    \sa setDocUrl()
*/
QUrl QBluetoothSdpRecord::docUrl() const
{
    return m_attrs.value(DOC_URL).value<QUrl>();
}

/*!
    Sets the Doc URL attribute to \a docUrl.

    \sa docUrl()
*/
void QBluetoothSdpRecord::setDocUrl(const QUrl &docUrl)
{
    m_attrs.insert(DOC_URL, QVariant::fromValue(docUrl));
}

/*!
    Returns the Exec URL attribute.

    \sa setExecUrl()
 */
QUrl QBluetoothSdpRecord::execUrl() const
{
    return m_attrs.value(CLNT_EXEC_URL).value<QUrl>();
}

/*!
    Sets the Exec URL attribute to \a execUrl.

    \sa execUrl()
 */
void QBluetoothSdpRecord::setExecUrl(const QUrl &execUrl)
{
    m_attrs.insert(CLNT_EXEC_URL, QVariant::fromValue(execUrl));
}

/*!
    Returns the Icon URL attribute.

    \sa setIconUrl()
 */
QUrl QBluetoothSdpRecord::iconUrl() const
{
    return m_attrs.value(ICON_URL).value<QUrl>();
}

/*!
    Sets the Icon URL attribute to \a iconUrl.

    \sa iconUrl()
 */
void QBluetoothSdpRecord::setIconUrl(const QUrl &iconUrl)
{
    m_attrs.insert(ICON_URL, QVariant::fromValue(iconUrl));
}

/*!
    Returns the ServiceID attribute - default value 0x0000.  Whilst not mandatory the service on the SDP Server can be
    uniquely identified using this uuid.

    \sa setId()
*/
QBluetoothSdpUuid QBluetoothSdpRecord::id() const
{
    return m_attrs.value(SERVICE_ID).value<QBluetoothSdpUuid>();
}

/*!
    Sets the ServiceID attribute to \a id.  The \a id argument should be unique
    identifier of the service.

    \sa id()
*/
void QBluetoothSdpRecord::setId(const QBluetoothSdpUuid &id)
{
    m_attrs.insert(SERVICE_ID, QVariant::fromValue(id));
}

/*!
    Returns the group id attribute.

    \sa setGroup()
*/
QBluetoothSdpUuid QBluetoothSdpRecord::group() const
{
    return m_attrs.value(GROUP_ID).value<QBluetoothSdpUuid>();
}

/*!
   Sets the GroupID attribute to \a group.  All services which belong to
   a Group Service Class will require this attribute.  All other services
   can be a part of one or more groups.  This is set through the
   browse group list attribute.

    \sa group()
*/
void QBluetoothSdpRecord::setGroup(const QBluetoothSdpUuid &group)
{
    m_attrs.insert(GROUP_ID, QVariant::fromValue(group));
}

/*!
    Returns a list of unique identifiers of all browse groups this service is
    a part of.

    \sa setBrowseGroups()
*/
QList<QBluetoothSdpUuid> QBluetoothSdpRecord::browseGroups() const
{
    QList<QBluetoothSdpUuid> ret;

    QVariant browseAttr = m_attrs.value(BROWSE_GRP_LIST);
    if (!browseAttr.canConvert<QBluetoothSdpSequence>())
        return ret;

    QBluetoothSdpSequence list = browseAttr.value<QBluetoothSdpSequence>();

    foreach (QVariant attr, list) {
        if (!browseAttr.canConvert<QBluetoothSdpUuid>()) {
            qWarning("browseGroup list contains non-UUID elements");
            return QList<QBluetoothSdpUuid>();
        }

        ret.push_back(attr.value<QBluetoothSdpUuid>());
    }

    return ret;
}

/*!
    Sets a list of unique identifiers of all browse groups this service is
    a part of to \a groups.

    \sa browseGroups()
*/
void QBluetoothSdpRecord::setBrowseGroups(const QList<QBluetoothSdpUuid> &groups)
{
    QBluetoothSdpSequence list;

    foreach (QBluetoothSdpUuid group, groups) {
        QVariant attr = QVariant::fromValue(group);
        list.push_back(attr);
    }

    m_attrs.insert(BROWSE_GRP_LIST, QVariant::fromValue(list));
}

/*!
    Returns a server specific record handle.

    \sa setRecordHandle()
*/
quint32 QBluetoothSdpRecord::recordHandle() const
{
    return m_attrs.value(RECORD_HANDLE).value<quint32>();
}

/*!
    Sets a server specific record handle to \a handle.

    \sa recordHandle()
*/
void QBluetoothSdpRecord::setRecordHandle(quint32 handle)
{
    m_attrs.insert(RECORD_HANDLE, QVariant::fromValue(handle));
}

/*!
   Returns a list of all attribute identifiers this service contains.

    \sa addAttribute(), removeAttribute(), attribute(), clearAttributes()
*/
QList<quint16> QBluetoothSdpRecord::attributeIds() const
{
    return m_attrs.keys();
}

/*!
    Tries to add an attribute \a attr with id \a id to the service.
    Returns false if the attribute already exists.

    \sa attributeIds(), removeAttribute(), attribute(), clearAttributes()
*/
bool QBluetoothSdpRecord::addAttribute(quint16 id, const QVariant &attr)
{
    if (m_attrs.contains(id))
        return false;

    m_attrs.insert(id, attr);

    return true;
}

/*!
    Removes the attribute with the specified id \a id from the service record.
    Returns true on success.  If the attribute is not found,
    nothing is done and false is returned.

    \sa attributeIds(), addAttribute(), attribute(), clearAttributes()
*/
bool QBluetoothSdpRecord::removeAttribute(quint16 id)
{
    return m_attrs.remove(id) > 0;
}

/*!
    Returns the attribute with id \a id from the service.  If the
    attribute is not
    found, a null QSDPAttribute is returned.  For extra error information,
    you can pass in the \a ok flag, which specifies whether an error occurred,
    or an actual NULL attribute was returned.

    \sa attributeIds(), addAttribute(), removeAttribute(), clearAttributes()
*/
QVariant QBluetoothSdpRecord::attribute(quint16 id, bool *ok) const
{
    QVariant ret;

    bool retOk = false;

    if (m_attrs.contains(id)) {
        retOk = true;
        ret = m_attrs.value(id);
    }

    if (ok)
        *ok = retOk;

    return ret;
}

/*!
    Clears all attributes.

    \sa attributeIds(), addAttribute(), removeAttribute(), attribute()
*/
void QBluetoothSdpRecord::clearAttributes()
{
    m_attrs.clear();
}

/*!
    This method can be used to find out whether a SDP record
    is an implementation of a particular service class,
    given by \a serviceUuid parameter.

    This method returns true if the service class matches,
    false otherwise.
 */
bool QBluetoothSdpRecord::isInstance(const QBluetoothSdpUuid &serviceUuid) const
{
    QVariant var = attribute(ServiceClassIDList);
    if (!var.canConvert<QBluetoothSdpSequence>())
        return false;

    QBluetoothSdpSequence attrList = var.value<QBluetoothSdpSequence>();
    foreach (QVariant seqAttr, attrList) {
        if (seqAttr.canConvert<QBluetoothSdpUuid>()) {
            QBluetoothSdpUuid uuid = seqAttr.value<QBluetoothSdpUuid>();
            if (uuid == serviceUuid)
                return true;
        }
    }

    return false;
}

/*!
    This method can be used to find out whether a SDP record
    is an implementation of a particular service class,
    given by \a profile parameter.

    This method returns true if the service class matches,
    false otherwise.
 */
bool QBluetoothSdpRecord::isInstance(QBluetooth::SDPProfile profile) const
{
    return isInstance(QBluetoothSdpUuid::fromProfile(profile));
}

/*!
    For a family of services that work over the RFCOMM protocol,
    this method returns the RFCOMM channel the service is
    running on.  The \a service parameter specifies the
    service record to search.

    Returns the channel number on success, -1 if no channel
    number was found.
 */
int QBluetoothSdpRecord::rfcommChannel(const QBluetoothSdpRecord &service)
{
    QVariant var = service.attribute(ProtocolDescriptorList);
    if (!var.canConvert<QBluetoothSdpSequence>())
        return -1;

    QBluetoothSdpSequence attrList = var.value<QBluetoothSdpSequence>();
    foreach (QVariant i, attrList) {
        if (i.canConvert<QBluetoothSdpSequence>()) {
            QBluetoothSdpSequence seq = i.value<QBluetoothSdpSequence>();
            if ((seq[0].value<QBluetoothSdpUuid>() == RFCOMMProtocol) &&
                (seq.size() >= 2)) {
                if (!seq[1].canConvert<quint8>())
                    return -1;
                return seq[1].value<quint8>();
            }
        }
    }

    return -1;
}

/*!
    Returns a SDP service record generated from the contents of \a device.

    Returns a null service record if the contents of \a device cannot be
    parsed.
*/
QBluetoothSdpRecord QBluetoothSdpRecord::fromDevice(QIODevice *device)
{
    QSdpXmlParser parser;
    if (!parser.parseRecord(device))
        return QBluetoothSdpRecord();
    return parser.record();
}

/*!
    Returns a SDP service record generated from the contents of \a data.

    Returns a null service record if the contents of \a data cannot be
    parsed.
*/
QBluetoothSdpRecord QBluetoothSdpRecord::fromData(const QByteArray &data)
{
    QSdpXmlParser parser;
    if (!parser.parseRecord(data))
        return QBluetoothSdpRecord();
    return parser.record();
}
