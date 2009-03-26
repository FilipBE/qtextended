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

#include "qdecorationplugin_qws.h"
#include "qdecoration_qws.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDecorationPlugin
    \ingroup qws
    \ingroup plugins

    \brief The QDecorationPlugin class is an abstract base class for
    window decoration plugins in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    \l{Qt for Embedded Linux} provides three ready-made decoration styles: \c
    Default, \c Styled and \c Windows. Custom decorations can be
    implemented by subclassing the QDecoration class and creating a
    decoration plugin.

    A decoration plugin can be created by subclassing
    QDecorationPlugin and implementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, the default implementation of the
    QDecorationFactory class will automatically detect the plugin and
    load the driver into the application at run-time. See \l{How to
    Create Qt Plugins} for details.

    To actually apply a decoration, use the
    QApplication::qwsSetDecoration() function.

    \sa QDecoration, QDecorationFactory
*/

/*!
    \fn QStringList QDecorationPlugin::keys() const

    Returns the list of valid keys, i.e., the decorations supported by
    this plugin.

    \sa create()
*/

/*!
    \fn QDecoration *QDecorationPlugin::create(const QString &key)

    Creates a decoration matching the given \a key. Note that keys are
    case-insensitive.

    \sa keys()
*/

/*!
    Constructs a decoration plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QDecorationPlugin::QDecorationPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the decoration plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QDecorationPlugin::~QDecorationPlugin()
{
}

QT_END_NAMESPACE
