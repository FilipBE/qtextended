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

#ifndef QTOPIASENDVIA_H
#define QTOPIASENDVIA_H

class QByteArray;
class QString;
class QWidget;
class QContent;

#include <qtopiaglobal.h>

class QTOPIA_EXPORT QtopiaSendVia
{
public:
    static bool isDataSupported(const QString &metatype);
    static bool isFileSupported();

    static bool sendData(QWidget *parent, const QByteArray &data, const QString &mimetype);
    static bool sendFile(QWidget *parent, const QString &filename, const QString &mimetype,
                         const QString &description = QString(), bool autodelete = false);
    static bool sendFile(QWidget *parent, const QContent &content, bool autodelete = false);
};

#endif
