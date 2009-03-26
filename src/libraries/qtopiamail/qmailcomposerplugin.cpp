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

#include "qmailcomposerplugin.h"

#include <QApplication>
#include <qtopialog.h>

/*!
    \class QMailComposerPluginInterface
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailComposerPluginInterface class defines the interface to plug-ins that provide mail message composers.
    \ingroup messaginglibrary

    The QMailComposerPluginInterface class defines the interface to mail message composer plug-ins.  Plug-ins will
    typically inherit from QMailComposerPlugin rather than this class.

    \sa QMailComposerPlugin, QMailComposerInterface, QMailComposerFactory
*/

/*!
    \fn QMailComposerInterface* QMailComposerPluginInterface::create( QWidget* parent )

    Creates an instance of the message composer provided by this plug-in, setting the returned object to
    have the parent widget \a parent.
*/

/*!
    \class QMailComposerPlugin
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

    \brief The QMailComposerPlugin class defines a base class for implementing mail message composer plug-ins.
    \ingroup messaginglibrary

    The QMailComposerPlugin class provides a base class for plug-in classes that provide mail message composing
    functionality.  Classes that inherit QMailComposerPlugin need to provide overrides of the
    \l {QMailComposerPlugin::keys()}{keys()} and \l {QMailComposerPlugin::create()}{create()} member functions.

    \sa QMailComposerPluginInterface, QMailComposerInterface
*/

/*!
    Create a mail message viewer plug-in instance.
*/
QMailComposerPlugin::QMailComposerPlugin()
{
}

/*!
    Destructs the QMailComposerPlugin object.
*/
QMailComposerPlugin::~QMailComposerPlugin()
{
}
/*!
    Returns the list of interfaces implemented by this plug-in.
*/
QStringList QMailComposerPlugin::keys() const
{
    QStringList list;
    return list << "QMailComposerPluginInterface";
}

