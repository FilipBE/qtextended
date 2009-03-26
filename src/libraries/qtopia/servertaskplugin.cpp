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

#include "servertaskplugin.h"

/*!
  \class ServerTaskPlugin
    \inpublicgroup QtBaseModule

  \brief The ServerTaskPlugin class provides an abstract base class for all server task plug-ins.
 
  \ingroup plugins
  \ingroup QtopiaServer

  The plug-in must override name(). The Qt Extended network 
  plug-ins can be found under \c{QPEDIR/src/server/plugins}. Note that plugin based server tasks
  can only ever be referenced by name (also see QtopiaServerApplication::qtopiaTask()).

  For more information on how to develop plug-in based server tasks refer to the \l {Tutorial: Writing server task plugin}{Server task plug-in tutorial}.

  \sa ServerTaskFactoryIface
  */

/*!
  Constructs a ServerTaskPlugin instance with the given \a parent.
*/
ServerTaskPlugin::ServerTaskPlugin(QObject* parent)
    :QObject(parent)
{
}

/*!
  Destroys the ServerTaskPlugin instance.
*/
ServerTaskPlugin::~ServerTaskPlugin()
{
}

/*!
  \class ServerTaskFactoryIface
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer

  \brief The ServerTaskFactoryIface class defines the interface to server task plug-ins.

  The ServerTaskFactoryIface class defines the interface to server task plug-ins. Plug-ins will
  typically inherit from ServerTaskPlugin rather than this class.

  \sa ServerTaskPlugin, QtopiaServerApplication
*/

/*!
  \fn QByteArray ServerTaskFactoryIface::name() const

  Returns the name of the server task. The name is used to identify the task and may be used
  in \c QPEDIR/etc/Task.cfg to determine the startup priority of this task.
*/


/*!
  \fn QObject* ServerTaskFactoryIface::initTask(void* createArg = 0) const;

  Initializes and returns the new server task object.
  \a createArg is currently an unused parameter which may be used by future versions of Qtopia.
*/

/*!
  \fn bool ServerTaskFactoryIface::demand() const = 0;

  Returns true if the task should be started on demand, otherwise false which implies preemptive startup.
*/

/*!
  \internal
  */
ServerTaskFactoryIface::~ServerTaskFactoryIface()
{
}
