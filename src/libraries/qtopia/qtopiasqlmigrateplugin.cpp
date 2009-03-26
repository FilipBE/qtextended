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
#include <qtopiasqlmigrateplugin_p.h>

/*!
    \class QtopiaSqlMigratePlugin
    \inpublicgroup QtBaseModule
    \brief The QtopiaSqlMigratePlugin class provides an interface for a database maintenance agent.
    \internal

    To minimize ongoing overhead and simplify replacement the database migration functionality of
    QtopiaSql is implemented in a plug-in which can be dynamically loaded as required and
    subsequently unloaded again when no longer needed.
*/

/*!
    Destroys a database migration plug-in.
*/
QtopiaSqlMigratePlugin::~QtopiaSqlMigratePlugin()
{
}

/*!
    \fn QtopiaSqlMigratePlugin::migrate(QSqlDatabase *database, bool system)

    Ensures a \a database has the latest schema.

    If the database is a \a system database all tables will be migrated, otherwise only content
    tables will be migrated.


    This will also attempt to discover and repair any corruption in the database.  In the case that
    the database is corrupted beyond repair a backup will be created and a replacement created.

    Returns true if the schema is up to date and false if the database is unusable for any reason.

    \sa QtopiaSql::attachDB()
*/
