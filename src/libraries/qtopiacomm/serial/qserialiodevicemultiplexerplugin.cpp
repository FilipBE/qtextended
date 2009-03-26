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

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>

/*!
    \class QSerialIODeviceMultiplexerPluginInterface
    \inpublicgroup QtBaseModule

    \brief The QSerialIODeviceMultiplexerPluginInterface class defines the interface to multiplexing plugins.
    \ingroup io
    \ingroup telephony::serial

    Multiplexer plug-ins will typically inherit from QSerialIODeviceMultiplexerPlugin
    rather than this class.  This class defines the abstract interface for plug-ins,
    whereas QSerialIODeviceMultiplexerPlugin implements most of the functionality for a
    plug-in, except for the parts that are specific to the plug-in in question.

    See the \l{Tutorial: Writing a Multiplexer Plug-in} for more information
    on how to write a multiplexer plug-in.

    \sa QSerialIODeviceMultiplexerPlugin, QSerialIODeviceMultiplexer
*/

/*!
    \fn bool QSerialIODeviceMultiplexerPluginInterface::forceGsm0710Stop()

    Returns true if the caller should force an existing GSM 07.10 session
    to stop before calling detect().  The default implementation returns
    false.  This is typcially needed for plugins that issue the AT+CMUX
    command in their detect() function.

    \sa detect(), create()
*/

/*!
    \fn bool QSerialIODeviceMultiplexerPluginInterface::detect( QSerialIODevice *device )

    Issue commands to detect if this plug-in can provide multiplexing for
    \a device.  Returns true if multiplexing is possible, or false if not.
    The plug-in can use QSerialIODeviceMultiplexer::chat() to simplify the
    issuing of commands.

    \sa create(), QSerialIODeviceMultiplexer::chat()
*/

/*!
    \fn QSerialIODeviceMultiplexer *QSerialIODeviceMultiplexerPluginInterface::create( QSerialIODevice *device )

    Create a multiplexer handler around \a device.  This will be called
    only if detect() returns true.

    \sa forceGsm0710Stop(), detect()
*/

/*!
    \class QSerialIODeviceMultiplexerPlugin
    \inpublicgroup QtBaseModule

    \brief The QSerialIODeviceMultiplexerPlugin class defines a base class for implementing multiplexing plugins.
    \ingroup io
    \ingroup telephony::serial
    \ingroup plugins

    Multiplexer plug-ins should override at least detect() and create().

    See the \l{Tutorial: Writing a Multiplexer Plug-in} for more information
    on how to write a multiplexer plug-in.

    \sa QSerialIODeviceMultiplexerPluginInterface, QSerialIODeviceMultiplexer
*/

/*!
    Create a multiplexer plug-in instance and attach it to \a parent.
*/
QSerialIODeviceMultiplexerPlugin::QSerialIODeviceMultiplexerPlugin
            ( QObject *parent )
    : QObject( parent )
{
}

/*!
    Destruct a multiplexer plug-in instance.
*/
QSerialIODeviceMultiplexerPlugin::~QSerialIODeviceMultiplexerPlugin()
{
}

/*!
    Returns the list of keys for this plugin.
*/
QStringList QSerialIODeviceMultiplexerPlugin::keys() const
{
    QStringList list;
    return list << "QSerialIODeviceMultiplexerPluginInterface";
}

/*!
    Returns true if the caller should force an existing GSM 07.10 session
    to stop before calling detect().  The default implementation returns
    false.  This is typcially needed for plugins that issue the AT+CMUX
    command in their detect() function.

    \sa detect(), create()
*/
bool QSerialIODeviceMultiplexerPlugin::forceGsm0710Stop()
{
    return false;
}

/*!
    Issue commands to detect if this plug-in can provide multiplexing for
    \a device.  Returns true if multiplexing is possible, or false if not.
    The plug-in can use QSerialIODeviceMultiplexer::chat() to simplify the
    issuing of commands.

    \sa create(), QSerialIODeviceMultiplexer::chat()
*/
bool QSerialIODeviceMultiplexerPlugin::detect( QSerialIODevice * )
{
    return false;
}

/*!
    Create a multiplexer handler around \a device.  This will be called
    only if detect() returns true.

    \sa forceGsm0710Stop(), detect()
*/
QSerialIODeviceMultiplexer *QSerialIODeviceMultiplexerPlugin::create
            ( QSerialIODevice * )
{
    return 0;
}
