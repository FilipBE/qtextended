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

#ifndef EXIFCONTENTPLUGIN_H
#define EXIFCONTENTPLUGIN_H

#include <qcontentplugin.h>
#include <qtopiaglobal.h>
#include <QDataStream>
#include <QFile>
#include <QList>

class QTOPIA_PLUGIN_EXPORT ExifContentPlugin : public QObject, public QContentPlugin, public QContentPropertiesPlugin
{
    Q_OBJECT
    Q_INTERFACES(QContentPlugin QContentPropertiesPlugin)
public:
    ExifContentPlugin();
    ~ExifContentPlugin();

    virtual QStringList keys() const;
    virtual bool installContent( const QString &filePath, QContent *content );
    virtual bool updateContent( QContent *content );

    virtual QStringList mimeTypes() const;
    virtual QImage thumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode );
    virtual QContentPropertiesEngine *createPropertiesEngine( const QContent &content );
};


#endif
