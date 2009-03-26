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
#ifndef QFSCONTENTENGINE_P_H
#define QFSCONTENTENGINE_P_H

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

#include "qcontentengine_p.h"

class QFSContentEngine : public QContentEngine
{
public:
    QFSContentEngine();
    virtual ~QFSContentEngine();

    virtual QContentEngine *copyTo( const QString &newPath );
    virtual bool moveTo( const QString &newPath );
    virtual bool rename(const QString &name);

    virtual bool execute( const QStringList &arguments ) const;

    virtual QIODevice *open( QIODevice::OpenMode mode );

    virtual QContentEngine *createCopy() const;

    virtual bool isOutOfDate() const;

    virtual bool remove();

protected:
    QFSContentEngine( const QString &engineType );

    static QString newFilePath(const QString &name, const QString &extension, const QDir &directory);
    static QDir determineDirectory(const QMimeType &mimeType, const QString &location);
    static QString determineBaseName(const QString &name, const QString &extension);
    static QString determineExtension(const QString &name, const QMimeType &type);

    virtual qint64 querySize();

    virtual bool queryValidity();
};

#endif
