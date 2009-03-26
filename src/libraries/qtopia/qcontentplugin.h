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

#ifndef QCONTENTPLUGIN_H
#define QCONTENTPLUGIN_H

#include <qcontent.h>
#include <qtopiaglobal.h>

class QContentPropertiesEngine;

class QTOPIA_EXPORT QContentPlugin
{
public:
    virtual ~QContentPlugin();

    virtual QStringList keys() const = 0;

    virtual bool installContent( const QString &path, QContent *content ) = 0;
    virtual bool updateContent( QContent *content );

    static void preloadPlugins();
};

class QTOPIA_EXPORT QContentPropertiesPlugin
{
public:
    virtual ~QContentPropertiesPlugin();

    virtual QStringList mimeTypes() const = 0;

    virtual QImage thumbnail( const QContent &content, const QSize &size, Qt::AspectRatioMode mode ) = 0;
    virtual QContentPropertiesEngine *createPropertiesEngine( const QContent &content ) = 0;
};

Q_DECLARE_INTERFACE( QContentPlugin, "com.trolltech.Qtopia.QContentPlugin/1.0" );
Q_DECLARE_INTERFACE( QContentPropertiesPlugin, "com.trolltech.Qtopia.QContentPropertiesPlugin/1.0" );

#endif
