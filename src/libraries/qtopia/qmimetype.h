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
#ifndef QMIMETYPE_H
#define QMIMETYPE_H

#include <qtopiaglobal.h>
#include <qstringlist.h>
#include <qicon.h>
#include <qlist.h>
#include <QMutex>

#include <qdrmcontent.h>
#include <qcontentset.h>

class QStringList;
class QMimeTypeData;

class QTOPIA_EXPORT QMimeType
{
public:
    QMimeType();
    explicit QMimeType( const QString& ext_or_id );
    explicit QMimeType( const QContent& );
    QMimeType( const QMimeType &other );

    QMimeType &operator =( const QMimeType &other );

    bool operator ==( const QMimeType &other ) const;

    enum IconType
    {
        Default,
        DrmValid,
        DrmInvalid
    };

    QString id() const;
    QString description() const;
    QIcon icon( IconType iconType = Default ) const;

    QString extension() const;
    QStringList extensions() const;
    QContentList applications() const;
    QContent application() const;

    QDrmRights::Permission permission() const;
    QList<QDrmRights::Permission> permissions() const;

    static void updateApplications();

    bool isNull() const;

    static QMimeType fromId( const QString &mimeId );
    static QMimeType fromExtension( const QString &extension );
    static QMimeType fromFileName( const QString &fileName );

    static void addAssociation(const QString& mimeType, const QString& application, const QString& icon, QDrmRights::Permission permission);
    static void removeAssociation(const QString& mimeType, const QString& application);
    static QContentList applicationsFor(const QContent&);
    static QContentList applicationsFor(const QMimeType&);
    static QContent defaultApplicationFor(const QContent&);
    static QContent defaultApplicationFor(const QMimeType&);
    static void setDefaultApplicationFor(const QString&, const QContent&);

private:
    static void clear();
    static void loadExtensions();
    static void loadExtensions(const QString&);
    void init( const QString& ext_or_id );
    static QMimeTypeData data(const QString& id);
    QString mimeId;
    static QMutex staticsGuardMutex;

    friend class QMimeTypeData;
    friend class QtopiaApplication;
};

#endif
