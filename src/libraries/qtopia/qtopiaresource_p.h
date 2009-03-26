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

#ifndef QTOPIARESOURCE_P_H
#define QTOPIARESOURCE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qabstractfileengine.h>
#include <qtopiaglobal.h>
#include <QMap>

class QFileResourceFileEngineHandler : QAbstractFileEngineHandler
{
public:
    QFileResourceFileEngineHandler();
    virtual ~QFileResourceFileEngineHandler();

    void setIconPath(const QStringList& p);
    virtual QAbstractFileEngine *create(const QString &path) const;

    static int fontHeightToIconSize( const int fontHeight );

private:
    void appendSearchDirs(QList<QByteArray> &dirs, const QString& dir, const char *subdir) const;

    QAbstractFileEngine *findArchivedResourceFile(const QString &path) const;
    QAbstractFileEngine *findArchivedImage(const QString &path) const;
    QAbstractFileEngine *findArchivedIcon(const QString &path) const;
    QString loadArchive(const QString &) const;
    mutable QMap<QString, QString> m_registeredArchives;

    QString findDiskResourceFile(const QString &path) const;
    QString findDiskSound(const QString &path) const;
    QString findDiskImage(const QString &path, const QString& _subdir) const;

    mutable QList<QByteArray> imagedirs;
    mutable QList<QByteArray> sounddirs;
    mutable QList<QByteArray> iconpath;
};

#endif
