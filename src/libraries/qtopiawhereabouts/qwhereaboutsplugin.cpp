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

#include "qwhereaboutsplugin.h"

/*!
    \class QWhereaboutsPlugin
    \inpublicgroup QtLocationModule
    \ingroup whereabouts
    \brief The QWhereaboutsPlugin class defines an interface for writing a whereabouts plugin.

    Whereabouts plugins can be written to retrieve location information from
    arbitrary data sources.

    To create a Whereabouts plugin, subclass QWhereaboutsPlugin and implement
    create(), and use the QTOPIA_EXPORT_PLUGIN macro to export your plugin.

    See \l{Location Services} for a detailed example of creating and using a
    Whereabouts plugin.
*/

/*!
    Creates a whereabouts plugin with the parent object \a parent.
*/
QWhereaboutsPlugin::QWhereaboutsPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the plugin.

    Note that Qt Extended destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QWhereaboutsPlugin::~QWhereaboutsPlugin()
{
}

/*!
    \fn QWhereabouts *QWhereaboutsPlugin::create(const QString &source)

    Creates and returns a QWhereabouts object that will read position data
    using the specified \a source.

    The \a source value may be empty to specify that a default source
    should be used, if possible.
*/
