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

#ifndef QICONENGINEPLUGIN_H
#define QICONENGINEPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QIconEngine;
class QIconEngineV2;

struct Q_GUI_EXPORT QIconEngineFactoryInterface : public QFactoryInterface
{
    virtual QIconEngine *create(const QString &filename) = 0;
};

#define QIconEngineFactoryInterface_iid \
    "com.trolltech.Qt.QIconEngineFactoryInterface"
Q_DECLARE_INTERFACE(QIconEngineFactoryInterface, QIconEngineFactoryInterface_iid)

class Q_GUI_EXPORT QIconEnginePlugin : public QObject, public QIconEngineFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QIconEngineFactoryInterface:QFactoryInterface)
public:
    QIconEnginePlugin(QObject *parent = 0);
    ~QIconEnginePlugin();

    virtual QStringList keys() const = 0;
    virtual QIconEngine *create(const QString &filename) = 0;
};

// ### Qt 5: remove version 2
struct Q_GUI_EXPORT QIconEngineFactoryInterfaceV2 : public QFactoryInterface
{
    virtual QIconEngineV2 *create(const QString &filename = QString()) = 0;
};

#define QIconEngineFactoryInterfaceV2_iid \
    "com.trolltech.Qt.QIconEngineFactoryInterfaceV2"
Q_DECLARE_INTERFACE(QIconEngineFactoryInterfaceV2, QIconEngineFactoryInterfaceV2_iid)

class Q_GUI_EXPORT QIconEnginePluginV2 : public QObject, public QIconEngineFactoryInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(QIconEngineFactoryInterfaceV2:QFactoryInterface)
public:
    QIconEnginePluginV2(QObject *parent = 0);
    ~QIconEnginePluginV2();

    virtual QStringList keys() const = 0;
    virtual QIconEngineV2 *create(const QString &filename = QString()) = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QICONENGINEPLUGIN_H
