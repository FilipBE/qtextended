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

#include "qmailviewerplugin.h"

#include <QApplication>
#include <qtopialog.h>

/*!
    \class QMailViewerPluginInterface
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailViewerPluginInterface class defines the interface to plug-ins that provide mail message viewers.
    \ingroup messaginglibrary

    The QMailViewerPluginInterface class defines the interface to mail message viewer plug-ins.  Plug-ins will 
    typically inherit from QMailViewerPlugin rather than this class.

    \sa QMailViewerPlugin, QMailViewerInterface, QMailViewerFactory
*/

/*!
    \fn QString QMailViewerPluginInterface::key() const

    Returns a string identifying this plug-in.
*/

/*!
    \fn bool QMailViewerPluginInterface::isSupported(QMailMessage::ContentType type, QMailViewerFactory::PresentationType pres) const

    Returns true if the viewer provided by this plug-in can display a mail message with \a type content using
    the presentation type \a pres; otherwise returns false.
*/

/*!
    \fn QMailViewerInterface* QMailViewerPluginInterface::create(QWidget* parent)

    Creates an instance of the message viewer class provided by the plug-in, setting the returned object to 
    have the parent widget \a parent.
*/

/*!
    \class QMailViewerPlugin
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailViewerPlugin class defines a base class for implementing mail message viewer plug-ins.
    \ingroup messaginglibrary

    The QMailViewerPlugin class provides a base class for plug-in classes that provide mail message viewing
    functionality.  Classes that inherit QMailViewerPlugin need to provide overrides of the
    \l {QMailViewerPlugin::key()}{key()}, \l {QMailViewerPlugin::isSupported()}{isSupported()} and 
    \l {QMailViewerPlugin::create()}{create()} member functions.

    \sa QMailViewerPluginInterface, QMailViewerInterface
*/

/*!
    Create a mail message viewer plug-in instance.
*/
QMailViewerPlugin::QMailViewerPlugin()
{
}

/*!
    Destructs the QMailViewerPlugin object.
*/
QMailViewerPlugin::~QMailViewerPlugin()
{
}

/*!
    Returns the list of interfaces implemented by this plug-in.
*/
QStringList QMailViewerPlugin::keys() const
{
    QStringList list;
    return list << "QMailViewerPluginInterface";
}

