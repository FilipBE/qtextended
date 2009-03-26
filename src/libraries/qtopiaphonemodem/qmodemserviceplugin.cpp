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

#include <qmodemserviceplugin.h>

/*!
    \class QModemServicePluginInterface
    \inpublicgroup QtCellModule

    \brief The QModemServicePluginInterface class defines the interface to vendor-specific modem plug-ins.
    \ingroup telephony::modem

    Plug-ins will typically inherit from QModemServicePlugin rather than this class.

    \sa QModemServicePlugin, QModemService
*/

/*!
    \fn bool QModemServicePluginInterface::supports( const QString& manufacturer )

    Returns true if this plug-in supports the given \a manufacturer string; otherwise returns false.
    The caller obtains the string by issuing \c{AT+CGMI} to the modem.
    If the plug-in supports \a manufacturer, the caller will then call create().
    This function will typically contain code such as the following:

    \code
        return manufacturer.contains( "VENDORID" );
    \endcode

    where \c VENDORID is a vendor-specific identifier that will appear in
    the output of \c{AT+CGMI} only for that vendor's modems.

    \sa create()
*/

/*!
    \fn QModemService *QModemServicePluginInterface::create( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )

    Create a vendor-specific modem service handler called \a service and
    attach it to \a parent.  The \a mux parameter specifies the serial device
    multiplexer to use for accessing the modem.

    \sa supports()
*/

/*!
    \class QModemServicePlugin
    \inpublicgroup QtCellModule

    \brief The QModemServicePlugin class defines a base class for implementing vendor-specific modem plug-ins.
    \ingroup telephony::modem
    \ingroup plugins

    The plug-in should override supports() and create().

    See \l{GSM Modem Integration} for more information on writing modem vendor plug-ins.

    \sa QModemServicePluginInterface, QModemService
*/

/*!
    Create a vendor-specific modem plug-in instance and attach it to \a parent.
*/
QModemServicePlugin::QModemServicePlugin( QObject* parent )
    : QObject( parent )
{
}

/*!
    Destruct a vendor-specific modem plug-in instance.
*/
QModemServicePlugin::~QModemServicePlugin()
{
}

/*!
    Returns the list of keys for this plug-in.
*/
QStringList QModemServicePlugin::keys() const
{
    QStringList list;
    return list << "QModemServicePluginInterface";
}

/*!
    Returns true if this plug-in supports the given \a manufacturer string.
    The caller obtains the string by issuing \c{AT+CGMI} to the modem.
    If the plug-in supports \a manufacturer, the caller will then call create().
    This function will typically contain code such as the following:

    \code
        return manufacturer.contains( "VENDORID" );
    \endcode

    where \c VENDORID is a vendor-specific identifier that will appear in
    the output of \c{AT+CGMI} only for that vendor's modems.

    The default implementation returns false.

    \sa create()
*/
bool QModemServicePlugin::supports( const QString& manufacturer )
{
    Q_UNUSED(manufacturer);
    return false;
}

/*!
    Create a vendor-specific modem service handler called \a service and
    attach it to \a parent.  The \a mux parameter specifies the serial device
    multiplexer to use for accessing the modem.

    The default implementation returns null.

    \sa supports()
*/
QModemService *QModemServicePlugin::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    Q_UNUSED(service);
    Q_UNUSED(mux);
    Q_UNUSED(parent);
    return 0;
}
