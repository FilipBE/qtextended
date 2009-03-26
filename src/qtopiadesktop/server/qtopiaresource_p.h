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

#include <qabstractfileengine.h>

class QFileResourceFileEngineHandler : QAbstractFileEngineHandler
{
public:
    QFileResourceFileEngineHandler();
    virtual ~QFileResourceFileEngineHandler();

    void setIconPath(const QStringList& p);
    virtual QAbstractFileEngine *create(const QString &path) const;

    static int fontHeightToIconSize( const int fontHeight );

private:
    void appendSearchDirs(QStringList& dirs, const QString& dir, const QString& subdir) const;
    QString findResourceFile(const QString &path) const;

    mutable QStringList imagedirs;
    mutable QStringList sounddirs;
    mutable QStringList iconpath;
};

#endif
