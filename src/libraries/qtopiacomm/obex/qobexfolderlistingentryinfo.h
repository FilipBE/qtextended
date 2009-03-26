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

#ifndef QOBEXFOLDERLISTINGENTRYINFO_H
#define QOBEXFOLDERLISTINGENTRYINFO_H

#include <qglobal.h>
#include <qobexglobal.h>
#include <QString>
#include <QMap>

class QObexFolderListingEntryInfoPrivate;
class QDateTime;

typedef QMap<QString,QString> ExtensionAttributes;

class QTOPIAOBEX_EXPORT QObexFolderListingEntryInfo
{
public:
    enum Permission {
        ReadOwner = 0400,
        WriteOwner = 0200,
        DeleteOwner = 0100,
        ReadGroup = 0040,
        WriteGroup = 0020,
        DeleteGroup = 0010,
        ReadOther = 0004,
        WriteOther = 0002,
        DeleteOther = 0001
    };

    Q_DECLARE_FLAGS(Permissions, Permission)

    QObexFolderListingEntryInfo();
    ~QObexFolderListingEntryInfo();
    QObexFolderListingEntryInfo(const QObexFolderListingEntryInfo &other);

    QObexFolderListingEntryInfo &operator=(const QObexFolderListingEntryInfo &other);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    qint64 size() const;
    void setSize(qint64 size);

    QDateTime lastModified() const;
    void setLastModified(const QDateTime &lastModified);

    QDateTime created() const;
    void setCreated(const QDateTime &timeCreated);

    QDateTime lastRead() const;
    void setLastRead(const QDateTime &lastRead);

    bool isFolder() const;
    void setFolder(bool b);

    bool isFile() const;
    void setFile(bool b);

    bool isParent() const;
    void setParent(bool b);

    void setPermissions(Permissions p);
    Permissions permissions() const;

    QString owner() const;
    void setOwner(const QString &owner);

    QString group() const;
    void setGroup(const QString &group);

    QString type() const;
    void setType(const QString &mimetype);

    QString description() const;
    void setDescription(const QString &description);

    QMap<QString,QString> extensionAttributes() const;
    void setExtensionAttributes(const QMap<QString,QString> &attributes);

    bool operator==(const QObexFolderListingEntryInfo &info) const;
    inline bool operator!=(const QObexFolderListingEntryInfo &info) const
    { return !operator==(info); }

    static QObexFolderListingEntryInfo
            createFolder(const QString &name, qint64 size,
                         const QDateTime &lastModified, const QDateTime &lastRead,
                         const QDateTime &timeCreated,
                         QObexFolderListingEntryInfo::Permissions permissions,
                         const QString &owner, const QString &group,
                         const QString &description = QString(),
                         const ExtensionAttributes &extensionAttributes =
                                 ExtensionAttributes());

    static QObexFolderListingEntryInfo createParent();

    static QObexFolderListingEntryInfo
            createFile(const QString &name, qint64 size,
                       const QDateTime &lastModified, const QDateTime &lastRead,
                       const QDateTime &timeCreated,
                       QObexFolderListingEntryInfo::Permissions permissions,
                       const QString &owner, const QString &group,
                       const QString &mimetype, const QString &description = QString(),
                       const ExtensionAttributes &extensionAttributes =
                               ExtensionAttributes());

private:
    QObexFolderListingEntryInfoPrivate *m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QObexFolderListingEntryInfo::Permissions)

#endif
