/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

/****************************************************************************
**
** Definition of QInputContextPlugin class
**
** Copyright (C) 2003-2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Nokia Corporation and/or its subsidiary(-ies) under their own
** license. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
****************************************************************************/

#ifndef QINPUTCONTEXTPLUGIN_H
#define QINPUTCONTEXTPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_IM) && !defined(QT_NO_LIBRARY)

class QInputContext;
class QInputContextPluginPrivate;

struct Q_GUI_EXPORT QInputContextFactoryInterface : public QFactoryInterface
{
    virtual QInputContext *create( const QString &key ) = 0;
    virtual QStringList languages( const QString &key ) = 0;
    virtual QString displayName( const QString &key ) = 0;
    virtual QString description( const QString &key ) = 0;
};

#define QInputContextFactoryInterface_iid "com.trolltech.Qt.QInputContextFactoryInterface"
Q_DECLARE_INTERFACE(QInputContextFactoryInterface, QInputContextFactoryInterface_iid)

class Q_GUI_EXPORT QInputContextPlugin : public QObject, public QInputContextFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QInputContextFactoryInterface:QFactoryInterface)
public:
    explicit QInputContextPlugin(QObject *parent = 0);
    ~QInputContextPlugin();

    virtual QStringList keys() const = 0;
    virtual QInputContext *create( const QString &key ) = 0;
    virtual QStringList languages( const QString &key ) = 0;
    virtual QString displayName( const QString &key ) = 0;
    virtual QString description( const QString &key ) = 0;
};

#endif // QT_NO_IM

QT_END_NAMESPACE

QT_END_HEADER

#endif // QINPUTCONTEXTPLUGIN_H
