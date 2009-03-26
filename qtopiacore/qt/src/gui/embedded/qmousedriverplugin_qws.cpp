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

#include "qmousedriverplugin_qws.h"

#ifndef QT_NO_LIBRARY

#include "qmouse_qws.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMouseDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QMouseDriverPlugin class is an abstract base class for
    mouse driver plugins in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    \l{Qt for Embedded Linux} provides ready-made drivers for several mouse
    protocols, see the \l{Qt for Embedded Linux Pointer Handling}{pointer
    handling} documentation for details. Custom mouse drivers can be
    implemented by subclassing the QWSMouseHandler class and creating
    a mouse driver plugin.

    A mouse driver plugin can be created by subclassing
    QMouseDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, The default implementation of the
    QMouseDriverFactory class will automatically detect the plugin and
    load the driver into the server application at run-time. See \l
    {How to Create Qt Plugins} for details.

    \sa QWSMouseHandler, QMouseDriverFactory
*/

/*!
    \fn QStringList QMouseDriverPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    mouse drivers supported by this plugin.

    \l{Qt for Embedded Linux} provides ready-made drivers for several mouse
    protocols, see the \l {Qt for Embedded Linux Pointer Handling}{pointer
    handling} documentation for details.

    \sa create()
*/

/*!
    Constructs a mouse driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QMouseDriverPlugin::QMouseDriverPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the mouse driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QMouseDriverPlugin::~QMouseDriverPlugin()
{
}

/*!
    \fn QScreen* QMouseDriverPlugin::create(const QString &key, const QString& device)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a device parameters. Note that
    keys are case-insensitive.

    \sa keys()
*/

QT_END_NAMESPACE

#endif // QT_NO_LIBRARY
