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

#include <QString>
#include <QXmlAttributes>

#include <QDateTime>

#include "qobexfolderlisting_p.h"


static const QString LOCAL_TIME_FORMAT = "yyyyMMddThhmmss";
static const QString UTC_TIME_FORMAT = LOCAL_TIME_FORMAT + 'Z';

QObexFolderListingHandler::QObexFolderListingHandler()
{
    m_valid_elem = false;
}

inline QDateTime parseTime(const QString &time)
{
    if (time.isEmpty())
        return QDateTime();

    return QDateTime::fromString(time,
                                 ((time[time.size()-1] == 'Z') ?
                                         UTC_TIME_FORMAT : LOCAL_TIME_FORMAT));
}

inline QObexFolderListingEntryInfo::Permissions parsePermissions(const QString &perm,
        QObexFolderListingEntryInfo::Permission read,
        QObexFolderListingEntryInfo::Permission write,
        QObexFolderListingEntryInfo::Permission remove)
{
    QObexFolderListingEntryInfo::Permissions permissions;

    for (int i = 0; i < perm.length(); i++) {
        if ((perm[i] == 'R') || (perm[i] == 'r'))
            permissions |= read;
        if ((perm[i] == 'W') || (perm[i] == 'w'))
            permissions |= write;
        if ((perm[i] == 'D') || (perm[i] == 'd'))
            permissions |= remove;
    }

    return permissions;
}

bool QObexFolderListingHandler::startElement(const QString &,
                                             const QString &,
                                             const QString &qName,
                                             const QXmlAttributes &attributes)
{
    if ((qName == "folder") || (qName == "file")) {
        QString name;
        qint64 size = 0;
        QDateTime modified;
        QDateTime accessed;
        QDateTime created;
        QString owner;
        QString group;
        QObexFolderListingEntryInfo::Permissions permissions = 0;
        QString mimetype;
        QMap<QString,QString> extended;

        for (int i = 0; i < attributes.count(); i++) {
            if (attributes.qName(i) == QLatin1String("name")) {
                name = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("size")) {
                bool ok = false;
                size = attributes.value(i).toLongLong(&ok, 10);
                if (!ok) {
                    size = 0;
                }
            }
            else if (attributes.qName(i) == QLatin1String("modified")) {
                modified = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("created")) {
                created = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("accessed")) {
                accessed = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("user-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadOwner,
                        QObexFolderListingEntryInfo::WriteOwner,
                        QObexFolderListingEntryInfo::DeleteOwner);
            }
            else if (attributes.qName(i) == QLatin1String("group-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadGroup,
                        QObexFolderListingEntryInfo::WriteGroup,
                        QObexFolderListingEntryInfo::DeleteGroup);
            }
            else if (attributes.qName(i) == QLatin1String("other-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadOther,
                        QObexFolderListingEntryInfo::WriteOther,
                        QObexFolderListingEntryInfo::DeleteOther);
            }
            else if (attributes.qName(i) == QLatin1String("group")) {
                group = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("owner")) {
                owner = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("type")) {
                mimetype = attributes.value(i);
            }
            else {
                extended.insert(attributes.qName(i), attributes.value(i));
            }
        }

        if (qName == "folder")
            m_info = QObexFolderListingEntryInfo::createFolder(name, size, modified,
                    accessed, created, permissions,
                    owner, group, QString(), extended);
        else
            m_info = QObexFolderListingEntryInfo::createFile(name, size, modified,
                    accessed, created, permissions,
                    owner, group, mimetype, QString(), extended);

        m_valid_elem = true;

        return true;
    }

    if (qName == "parent-folder") {
        m_info = QObexFolderListingEntryInfo::createParent();
        return true;
    }

    if (qName == "folder-listing") {
        return true;
    }

    // Just ignore unknown elements

    return true;
}

bool QObexFolderListingHandler::endElement(const QString &,
                                           const QString &,
                                           const QString &qName)
{
    if (qName == "file" || qName == "folder" || qName == "parent-folder") {
        m_valid_elem = false;
        emit info(m_info);
        return true;
    }

    if (qName == "folder-listing")
        return true;

    // Just ignore unknown elements

    return true;
}

bool QObexFolderListingHandler::characters(const QString &ch)
{
    if (m_valid_elem)
        m_info.setDescription(ch.trimmed());

    return true;
}

bool QObexFolderListingHandler::fatalError(const QXmlParseException &/*err*/)
{
    return false;
}

bool QObexFolderListingHandler::error(const QXmlParseException &/*err*/)
{
    return false;
}

bool QObexFolderListingHandler::warning(const QXmlParseException &/*err*/)
{
    return false;
}

