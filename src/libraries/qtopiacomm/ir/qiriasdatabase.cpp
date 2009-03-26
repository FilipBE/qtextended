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

#include "qirnamespace_p.h"
#include <qiriasdatabase.h>

#if defined(Q_OS_LINUX)
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/irda.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <errno.h>
#endif

#if defined(Q_OS_WIN32)
#include <winsock2.h>
#define _WIN32_WINDOWS
#include <AF_IrDA.h>
#endif

#include <stdio.h>

#include <QStringList>
#include <QTextCodec>
#include <QVariant>

/*!
    \class QIrIasDatabase

    \brief The QIrIasDatabase class provides access to the local IAS Database.

    IAS stands for Information Access Service.  The IAS database provides a way
    for applications to register information about the infrared services they
    provide.

    QIrIasDatabase class provides access to the local IAS Database, and gives
    the ability to add, query and delete attributes.  This class also allows
    to modify the advertised major classes this device supports.  These are
    generally referred to as service hints.

    \ingroup qtopiair
 */

/*!
    Sets a new attribute into the IAS Database returning true if successful; otherwise returning false.  The \a attr parameter
    holds the attribute to be set.  Attr can be of the following types:

    \list
        \o \bold{QString} - Represents a String IAS attribute.
        \o \bold{uint} - Represents an Integer IAS attribute.
        \o \bold{QByteArray} - Represents an octet sequence IAS attribute.
    \endlist

    Any other type will be treated as invalid.  The \a className parameter
    holds the class name of the IAS attribute.  The \a attribName parameter
    holds the attribute name of the attribute.

    NOTE: Under linux this function requires administrator privileges.

    \sa attribute(), removeAttribute()
 */
bool QIrIasDatabase::setAttribute(const QString &className,
                                  const QString &attribName,
                                  const QVariant &attr)
{
#if defined(Q_OS_LINUX) || defined (Q_OS_WIN32)
    if (className.size() > IAS_MAX_CLASSNAME)
        return false;

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return false;

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return false;

#if defined(Q_OS_LINUX)
    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());
#else
    IAS_SET entry;
    strncpy(entry.irdaClassName, className.toAscii().constData(), IAS_MAX_CLASSNAME);
    strncpy(entry.irdaAttribName, attribName.toAscii().constData(), IAS_MAX_ATTRIBNAME);
#endif

    switch (attr.type()) {
        case QVariant::String:
        {
            QByteArray value;
            value = attr.value<QString>().toAscii();

#if defined(Q_OS_LINUX)
            // Linux only supports the ASCII charset right now
            entry.irda_attrib_type = IAS_STRING;
            entry.attribute.irda_attrib_string.charset = CS_ASCII;

            int len = value.length() < IAS_MAX_STRING ?
                      value.length() : IAS_MAX_STRING;
                      entry.attribute.irda_attrib_string.len = len;
            strncpy(reinterpret_cast<char *>(entry.attribute.irda_attrib_string.string),
                    value.constData(), IAS_MAX_STRING);
#else
            entry.irdaAttribType = IAS_ATTRIB_STR;
            entry.irdaAttribute.irdaAttribUsrStr.CharSet = LmCharSetASCII;
            int len = value.length() < IAS_MAX_USER_STRING ?
                      value.length() : IAS_MAX_USER_STRING;
            entry.irdaAttribute.irdaAttribUsrStr.Len = len;
            strncpy(reinterpret_cast<char *>(entry.irdaAttribute.irdaAttribUsrStr.UsrStr),
                    value.constData(), IAS_MAX_USER_STRING);
#endif
            break;
        }
        case QVariant::UInt:
        {
#if defined(Q_OS_LINUX)
            entry.irda_attrib_type = IAS_INTEGER;
            entry.attribute.irda_attrib_int = attr.value<uint>();
#else
            entry.irdaAttribType = IAS_ATTRIB_INT;
            entry.irdaAttribute.irdaAttribInt = attr.value<uint>();
#endif
            break;
        }
        case QVariant::ByteArray:
        {
#if defined(Q_OS_LINUX)
            entry.irda_attrib_type = IAS_OCT_SEQ;
            int len = attr.value<QByteArray>().length() < IAS_MAX_OCTET_STRING ?
                    attr.value<QByteArray>().length() : IAS_MAX_OCTET_STRING;
            entry.attribute.irda_attrib_octet_seq.len = len;
            strncpy(reinterpret_cast<char *>(entry.attribute.irda_attrib_octet_seq.octet_seq),
                    attr.value<QByteArray>().constData(), IAS_MAX_OCTET_STRING);
#else
            entry.irdaAttribType = IAS_ATTRIB_OCTETSEQ;
            int len = attr.value<QByteArray>().length() < IAS_MAX_OCTET_STRING ?
                      attr.value<QByteArray>().length() : IAS_MAX_OCTET_STRING;
            entry.irdaAttribute.irdaAttribOctetSeq.Len = len;
            strncpy(reinterpret_cast<char *>(entry.irdaAttribute.irdaAttribOctetSeq.OctetSeq),
                    attr.value<QByteArray>().constData(), IAS_MAX_OCTET_STRING);
#endif
            break;
        }
        default:
            return false;
    }

    int status = setsockopt(fd, SOL_IRLMP, IRLMP_IAS_SET, reinterpret_cast<char *>(&entry), sizeof(entry));

#if defined(Q_OS_LINUX)
    close(fd);
#else
    ::closesocket(fd);
#endif

    return status == 0;
#else
    return false;
#endif
}

/*!
    Returns the value of an attribute from the local IAS Database.
    The \a className parameter specifies the class name of the attribute.
    The \a attribName parameter specifies the attribute name.  Returns an
    invalid QVariant in case of failure, and valid otherwise.
    The value of QVariant can be one of:
    \list
        \o \bold{QString} - Represents a String IAS attribute.
        \o \bold{uint} - Represents an Integer IAS attribute.
        \o \bold{QByteArray} - Represents an octet sequence IAS attribute.
    \endlist

    NOTE: Under linux this function requires administrator privileges.  Under
    windows this function has no effect and always returns an invalid value.

    \sa setAttribute(), removeAttribute()
 */
QVariant QIrIasDatabase::attribute(const QString &className, const QString &attribName)
{
#if defined(Q_OS_LINUX)
    if (className.size() > IAS_MAX_CLASSNAME)
        return QVariant();

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return QVariant();

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return QVariant();

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());

    socklen_t len = sizeof(entry);

    int status = getsockopt(fd, SOL_IRLMP, IRLMP_IAS_GET, &entry, &len);
    close(fd);

    if (status != 0)
        return QVariant();

    return convert_ias_entry(entry);
#else
    Q_UNUSED(attribName)
    Q_UNUSED(className)
    return QVariant();
#endif
}

/*!
    Attempts to remove the attribute from the local IAS Database.
    The \a className parameter specifies the class name of the attribute.
    The \a attribName parameter specifies the attribute name.
    Returns true if the attribute could be removed, false otherwise.

    NOTE: Under linux this function requires administrator privileges.  Under Windows
    this function has no effect and always returns false

    \sa setAttribute(), attribute()
*/
bool QIrIasDatabase::removeAttribute(const QString &className,
                                     const QString &attribName)
{
#if defined(Q_OS_LINUX)
    if (className.size() > IAS_MAX_CLASSNAME)
        return false;

    if (attribName.size() > IAS_MAX_ATTRIBNAME)
        return false;

    int fd = socket(AF_IRDA, SOCK_STREAM, 0);
    if ( fd == -1 )
        return false;

    struct irda_ias_set entry;
    strcpy(entry.irda_class_name, className.toAscii().constData());
    strcpy(entry.irda_attrib_name, attribName.toAscii().constData());

    int status = setsockopt(fd, SOL_IRLMP, IRLMP_IAS_DEL, &entry, sizeof(entry));
    close(fd);

    return status == 0;
#else
    Q_UNUSED(attribName)
    Q_UNUSED(className)
    return false;
#endif
}
