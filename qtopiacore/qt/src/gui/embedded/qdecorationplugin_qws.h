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
** http://www.gnu.org/copyleft/gpl.html.
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

#ifndef QDECORATIONPLUGIN_QWS_H
#define QDECORATIONPLUGIN_QWS_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QDecoration;

struct Q_GUI_EXPORT QDecorationFactoryInterface : public QFactoryInterface
{
    virtual QDecoration *create(const QString &key) = 0;
};

#define QDecorationFactoryInterface_iid "com.trolltech.Qt.QDecorationFactoryInterface"
Q_DECLARE_INTERFACE(QDecorationFactoryInterface, QDecorationFactoryInterface_iid)

class Q_GUI_EXPORT QDecorationPlugin : public QObject, public QDecorationFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QDecorationFactoryInterface:QFactoryInterface)
        public:
    explicit QDecorationPlugin(QObject *parent = 0);
    ~QDecorationPlugin();

    virtual QStringList keys() const = 0;
    virtual QDecoration *create(const QString &key) = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECORATIONPLUGIN_QWS_H
