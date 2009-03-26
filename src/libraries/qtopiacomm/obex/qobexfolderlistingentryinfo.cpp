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

#include "qobexfolderlistingentryinfo.h"

#include <QString>
#include <QDateTime>

struct QObexFolderListingEntryInfoPrivate
{
    QObexFolderListingEntryInfoPrivate();

    bool isDir;
    bool isFile;
    bool isParent;

    QString name;
    qint64 size;
    QDateTime modified;
    QDateTime created;
    QDateTime accessed;
    QObexFolderListingEntryInfo::Permissions permissions;
    QString owner;
    QString group;
    QString type;
    QString description;
    QMap<QString,QString> extendedAttributes;
};

#define CHECK_ALLOC() \
    if (!m_data) \
        m_data = new QObexFolderListingEntryInfoPrivate();

QObexFolderListingEntryInfoPrivate::QObexFolderListingEntryInfoPrivate() :
        size(0), permissions(0)
{
}

/*!
    \class QObexFolderListingEntryInfo
    \inpublicgroup QtBaseModule


    \brief The QObexFolderListingEntryInfo class provides information about an item in an OBEX folder listing object.

    OBEX specification provides a standard way for OBEX peers to
    exchange file and folder information.  The OBEX specification
    defines a standard XML file format for exchanging these listing
    objects and is called Folder-Listing.

    The information provided by the folder listing format can include
    the name(), size(), lastModified(), created(), lastRead(),
    permissions(), group(), owner(), mimetype(), description()
    attributes about each particular item in the listing.  The
    QObexFolderListingEntryInfo object stores each folider listing
    item's attributes.

    You can create your own QObexFolderListingEntryInfo objects by passing
    in all the relevant information in the constructor, and you can
    modify a QObexFolderListingEntryInfo; for each getter mentioned above
    there is an equivalent setter. Note that setting values does not
    affect the underlying resource that the QObexFolderListingEntryInfo
    provides information about.

    \ingroup qtopiaobex
*/

/*!
    \enum QObexFolderListingEntryInfo::Permission

    This enum is used by the permissions() function to report the
    permissions of a file or directory.

    \value ReadOwner The file is readable by the owner of the file.
    \value WriteOwner The file is writable by the owner of the file.
    \value DeleteOwner The file is deletable by the owner of the file.
    \value ReadGroup The file is readable by the group.
    \value WriteGroup The file is writable by the group.
    \value DeleteGroup The file is deletable by the group.
    \value ReadOther The file is readable by anyone.
    \value WriteOther The file is writable by anyone.
    \value DeleteOther The file is deletable by anyone.
*/

/*!
    Constructs an invalid folder listing info object.

    \sa isValid()
*/
QObexFolderListingEntryInfo::QObexFolderListingEntryInfo()
    : m_data(0)
{
}

/*!
    Destructor
*/
QObexFolderListingEntryInfo::~QObexFolderListingEntryInfo()
{
    if (m_data)
        delete m_data;
}

/*!
    Constructs a new copy of a folder listing info from \a other.

    \sa isValid()
*/
QObexFolderListingEntryInfo::QObexFolderListingEntryInfo(const QObexFolderListingEntryInfo &other)
    : m_data(0)
{
    operator=(other);
}

/*!
    Assigns the contents of \a other to the current object.
*/
QObexFolderListingEntryInfo &QObexFolderListingEntryInfo::operator=(const QObexFolderListingEntryInfo &other)
{
    if (other.m_data) {
        if (!m_data)
            m_data = new QObexFolderListingEntryInfoPrivate();

        m_data->isDir = other.m_data->isDir;
        m_data->isParent = other.m_data->isParent;
        m_data->isFile = other.m_data->isFile;
        m_data->name = other.m_data->name;
        m_data->size = other.m_data->size;
        m_data->modified = other.m_data->modified;
        m_data->created = other.m_data->created;
        m_data->accessed = other.m_data->accessed;
        m_data->permissions = other.m_data->permissions;
        m_data->owner = other.m_data->owner;
        m_data->group = other.m_data->group;
        m_data->type = other.m_data->type;
        m_data->description = other.m_data->description;
        m_data->extendedAttributes = other.m_data->extendedAttributes;
    }
    else {
        delete m_data;
        m_data = NULL;
    }

    return *this;
}

/*!
    Returns true if the folder listing info is valid and false otherwise.  If the
    object is invalid, any value returned should not be relied upon.
*/
bool QObexFolderListingEntryInfo::isValid() const
{
    return m_data != NULL;
}

/*!
    Returns the name of the file or folder referred to by the
    folder listing info.  If the name was not provided or the
    folder listing entry is invalid, returns an empty string.

    \sa setName()
*/
QString QObexFolderListingEntryInfo::name() const
{
    if (!m_data)
        return QString();

    return m_data->name;
}

/*!
    Sets the name of the folder listing info to \a name.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa name()
*/
void QObexFolderListingEntryInfo::setName(const QString &name)
{
    CHECK_ALLOC()
    m_data->name = name;
}

/*!
    Returns the size in bytes of the file or folder referred to by the
    folder listing info.  According to the OBEX standard, the size might
    not be accurate (e.g. only an estimate) and should not be relied upon
    to be exact.  If no size information was provided, a 0 is returned.

    \sa setSize()
*/
qint64 QObexFolderListingEntryInfo::size() const
{
    if (!m_data)
        return 0;

    return m_data->size;
}

/*!
    Sets the size in bytes of the folder listing info to \a size.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa size()
*/
void QObexFolderListingEntryInfo::setSize(qint64 size)
{
    CHECK_ALLOC()
    m_data->size = size;
}

/*!
    Returns the last modification time of the file or folder referred to by the
    folder listing info.  If this information was not provided, an invalid
    QDateTime object is returned.

    \sa setLastModified()
*/
QDateTime QObexFolderListingEntryInfo::lastModified() const
{
    if (!m_data)
        return QDateTime();

    return m_data->modified;
}

/*!
    Sets the last modified time of the folder listing info to \a lastModified.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa lastModified()
*/
void QObexFolderListingEntryInfo::setLastModified(const QDateTime &lastModified)
{
    CHECK_ALLOC()
    m_data->modified = lastModified;
}

/*!
    Returns the creation time of the file or folder referred to by the
    folder listing info.  If this information was not provided, an invalid
    QDateTime object is returned.

    \sa setCreated()
*/
QDateTime QObexFolderListingEntryInfo::created() const
{
    if (!m_data)
        return QDateTime();

    return m_data->created;
}

/*!
    Sets the creation time of the folder listing info to \a timeCreated.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa created()
*/
void QObexFolderListingEntryInfo::setCreated(const QDateTime &timeCreated)
{
    CHECK_ALLOC()
    m_data->created = timeCreated;
}

/*!
    Returns the time the file or folder referred to by the
    folder listing info has been last accessed.  If this information
    was not provided, an invalid QDateTime object is returned.

    \sa setLastRead()
 */
QDateTime QObexFolderListingEntryInfo::lastRead() const
{
    if (!m_data)
        return QDateTime();

    return m_data->accessed;
}

/*!
    Sets the last access time of the folder listing info to \a lastRead.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa lastRead()
 */
void QObexFolderListingEntryInfo::setLastRead(const QDateTime &lastRead)
{
    CHECK_ALLOC()
    m_data->accessed = lastRead;
}

/*!
    Returns true if the folder listing info refers to a directory.

    \sa isFile(), isParent(), setFolder()
*/
bool QObexFolderListingEntryInfo::isFolder() const
{
    if (!m_data)
        return false;

    return m_data->isDir;
}

/*!
    If \a b is true, then the folder listing info is set to refer to a directory.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa isFolder()
 */
void QObexFolderListingEntryInfo::setFolder(bool b)
{
    CHECK_ALLOC()
    m_data->isDir = b;
}

/*!
    Returns true if the folder listing info refers to a file.

    \sa isFolder(), isParent(), setFile()
*/
bool QObexFolderListingEntryInfo::isFile() const
{
    if (!m_data)
        return false;

    return m_data->isFile;
}

/*!
    If \a b is true, then the folder listing info is set to refer to a file.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa isFile()
 */
void QObexFolderListingEntryInfo::setFile(bool b)
{
    CHECK_ALLOC()
    m_data->isFile = b;
}

/*!
    Returns true if the folder listing info refers to a parent directory.

    \sa isFolder()
 */
bool QObexFolderListingEntryInfo::isParent() const
{
    if (!m_data)
        return false;

    return m_data->isParent;
}

/*!
    If \a b is true, then the folder listing info is set to refer to a parent
    directory.  Parent directories should have no attributes associated with them.
    E.g. name, size, etc, information should not be provided.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa isParent()
*/
void QObexFolderListingEntryInfo::setParent(bool b)
{
    CHECK_ALLOC()
    m_data->isParent = b;
}

/*!
    Returns the permissions for the folder listing info.

    \sa setPermissions()
*/
QObexFolderListingEntryInfo::Permissions QObexFolderListingEntryInfo::permissions() const
{
    if (!m_data)
        return 0;

    return m_data->permissions;
}

/*!
    Sets the last permissions of the folder listing info to \a permissions.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa permissions()
*/
void QObexFolderListingEntryInfo::setPermissions(QObexFolderListingEntryInfo::Permissions permissions)
{
    CHECK_ALLOC()
    m_data->permissions = permissions;
}

/*!
    Returns the owner of the file or folder referred to by the
    folder listing info.  If this information was not provided, an empty
    string is returned.

    \sa setOwner()
*/
QString QObexFolderListingEntryInfo::owner() const
{
    if (!m_data)
        return QString();

    return m_data->owner;
}

/*!
    Sets the owner of the folder listing info to \a owner.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa owner()
 */
void QObexFolderListingEntryInfo::setOwner(const QString &owner)
{
    CHECK_ALLOC()
    m_data->owner = owner;
}

/*!
    Returns the group of the file or folder referred to by the
    folder listing info.  If this information was not provided, an empty
    string is returned.

    \sa setGroup()
 */
QString QObexFolderListingEntryInfo::group() const
{
    if (!m_data)
        return QString();

    return m_data->group;
}

/*!
    Sets the group of the folder listing info to \a group.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa group()
 */
void QObexFolderListingEntryInfo::setGroup(const QString &group)
{
    CHECK_ALLOC()
    m_data->group = group;
}

/*!
    Returns the mimetype of the file or folder referred to by the
    folder listing info.  If this information was not provided, an empty
    string is returned.

    \sa setType()
 */
QString QObexFolderListingEntryInfo::type() const
{
    if (!m_data)
        return QString();

    return m_data->type;
}

/*!
    Sets the mimetype of the folder listing info to \a mimetype.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa type()
 */
void QObexFolderListingEntryInfo::setType(const QString &mimetype)
{
    CHECK_ALLOC()
    m_data->type = mimetype;
}

/*!
    Returns the description of the file or folder referred to by the
    folder listing info.  If this information was not provided, an empty
    string is returned.

    \sa setDescription()
 */
QString QObexFolderListingEntryInfo::description() const
{
    if (!m_data)
        return QString();

    return m_data->description;
}

/*!
    Sets the description of the folder listing info to \a description.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa description()
 */
void QObexFolderListingEntryInfo::setDescription(const QString &description)
{
    CHECK_ALLOC()
    m_data->description = description;
}

/*!
    Returns the extended attributes of the file or folder referred to by the
    folder listing info.  Some implementations can report attributes which
    are not specified by the standard, cuch as folder location (e.g. phone memory,
    flash card, etc).  These attributes are reported as key-value pairs.

    If this information was not provided, an empty map is returned.

    \sa setExtensionAttributes()
*/
QMap<QString,QString> QObexFolderListingEntryInfo::extensionAttributes() const
{
    if (!m_data)
        return QMap<QString,QString>();

    return m_data->extendedAttributes;
}

/*!
    Sets the extended attributes of the folder listing info to \a attributes.

    If you call this function for an invalid folder listing info, this function
    turns it into a valid one.

    \sa extensionAttributes()
*/
void QObexFolderListingEntryInfo::setExtensionAttributes(const QMap<QString,QString> &attributes)
{
    CHECK_ALLOC()
    m_data->extendedAttributes = attributes;
}

/*!
    \fn bool QObexFolderListingEntryInfo::operator!=(const QObexFolderListingEntryInfo &info) const

    Comparison operator.  Returns true if the current object is not
    equal to \a info.
*/

/*!
    Comparison operator.  Returns true if the current object is equal to \a info.
*/
bool QObexFolderListingEntryInfo::operator==(const QObexFolderListingEntryInfo &info) const
{
    if (!m_data)
        return info.m_data == 0;

    if (!info.m_data)
        return false;

    return (m_data->isDir == info.m_data->isDir) &&
            (m_data->isParent == info.m_data->isParent) &&
            (m_data->isFile == info.m_data->isFile) &&
            (m_data->name == info.m_data->name) &&
            (m_data->size == info.m_data->size) &&
            (m_data->modified == info.m_data->modified) &&
            (m_data->created == info.m_data->created) &&
            (m_data->accessed == info.m_data->accessed) &&
            (m_data->permissions == info.m_data->permissions) &&
            (m_data->owner == info.m_data->owner) &&
            (m_data->group == info.m_data->group) &&
            (m_data->type == info.m_data->type) &&
            (m_data->description == info.m_data->description) &&
            (m_data->extendedAttributes == info.m_data->extendedAttributes);
}

/*!
    Creates a Folder Listing entry that represents a parent directory.
    OBEX Folders can have parent directories.  These directories do not have
    any attributes associated with them.

    \sa createFile(), createFolder()
*/
QObexFolderListingEntryInfo QObexFolderListingEntryInfo::createParent()
{
    QObexFolderListingEntryInfo info;
    info.m_data = new QObexFolderListingEntryInfoPrivate();
    info.m_data->isDir = true;
    info.m_data->isParent = true;
    info.m_data->isFile = false;

    return info;
}

/*!
    Creates a Folder Listing entry that represents a file with \a name,
    \a size.  The file was last modified on \a lastModified, last accessed on
    \a lastAccessed and created on \a timeCreated.  The \a permissions specify
    file permissions, and \a owner, \a group and \a mimetype specify the owner,
    group and mimetype of the file. The \a description refers to the
    optional description of the object.  The \a extensionAttributes parameter
    refers to non-standard attributes that might be used by various implementations.

    \sa createFolder(), createParent()
*/
QObexFolderListingEntryInfo QObexFolderListingEntryInfo::createFile(const QString &name,
        qint64 size,
        const QDateTime &lastModified, const QDateTime &lastAccessed,
        const QDateTime &timeCreated,
        QObexFolderListingEntryInfo::Permissions permissions,
        const QString &owner, const QString &group,
        const QString &mimetype, const QString &description,
        const ExtensionAttributes &extensionAttributes)
{
    QObexFolderListingEntryInfo info;
    info.m_data = new QObexFolderListingEntryInfoPrivate();
    info.m_data->isDir = false;
    info.m_data->isParent = false;
    info.m_data->isFile = true;
    info.m_data->name = name;
    info.m_data->size = size;
    info.m_data->modified = lastModified;
    info.m_data->created = timeCreated;
    info.m_data->accessed = lastAccessed;
    info.m_data->permissions = permissions;
    info.m_data->owner = owner;
    info.m_data->group = group;
    info.m_data->type = mimetype;
    info.m_data->description = description;
    info.m_data->extendedAttributes = extensionAttributes;

    return info;
}

/*!
    Creates a Folder Listing entry that represents a folder with \a name,
    \a size.  The folder was last modified on \a lastModified, last accessed on
    \a lastAccessed and created on \a timeCreated.  The \a permissions specify
    file permissions.  \a owner and \a group specify the owner and group
    the folder belongs to, respectively.  The \a description refers to the
    optional description of the object.  The \a extensionAttributes parameter
    refers to non-standard attributes that might be used by various implementations.

    \sa createFile(), createParent()
 */
QObexFolderListingEntryInfo QObexFolderListingEntryInfo::createFolder(const QString &name,
        qint64 size,
        const QDateTime &lastModified, const QDateTime &lastAccessed,
        const QDateTime &timeCreated,
        QObexFolderListingEntryInfo::Permissions permissions,
        const QString &owner, const QString &group,
        const QString &description,
        const ExtensionAttributes &extensionAttributes)
{
    QObexFolderListingEntryInfo info;
    info.m_data = new QObexFolderListingEntryInfoPrivate();
    info.m_data->isDir = true;
    info.m_data->isParent = false;
    info.m_data->isFile = false;
    info.m_data->name = name;
    info.m_data->size = size;
    info.m_data->modified = lastModified;
    info.m_data->created = timeCreated;
    info.m_data->accessed = lastAccessed;
    info.m_data->permissions = permissions;
    info.m_data->owner = owner;
    info.m_data->group = group;
    info.m_data->description = description;
    info.m_data->extendedAttributes = extensionAttributes;

    return info;
}
