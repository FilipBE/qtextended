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

#include <qdialoptions.h>
#include <qmap.h>

/*!
    \class QDialOptions
    \inpublicgroup QtTelephonyModule

    \brief The QDialOptions class provides an encapsulation of options for dialing an outgoing call.
    \ingroup telephony

    Dial options are used with QPhoneCall::dial() when initiating an outgoing call.
    The following example demonstrates dialing a voice call:

    \code
        QPhoneCallManager mgr;
        QPhoneCall call = mgr.create( "Voice" );
        QDialOptions dialOptions;
        dialOptions.setNumber( "1234567" );
        call.dial( dialOptions );
    \endcode

    \sa QPhoneCall::dial(), QPhoneCall
*/
class QDialOptionsPrivate
{
public:
    QDialOptionsPrivate()
    {
        callerId = QDialOptions::DefaultCallerId;
        speed = -1;
        gsmSpeed = -1;
        bearer = QDialOptions::DataCircuitAsyncUDI;
        transparentMode = QDialOptions::NonTransparent;
        programName = "/usr/sbin/pppd";
        demand = false;
        useIpModule = false;
        contextId = 0;
        closedUserGroup = false;
    }

    QString number;
    QDialOptions::CallerId callerId;
    QUniqueId contact;
    int speed;
    int gsmSpeed;
    QDialOptions::Bearer bearer;
    QDialOptions::TransparentMode transparentMode;
    QMap< QString, QVariant > extensionOptions;
    QString programName;
    QStringList args;
    QString connectScript;
    QString disconnectScript;
    bool demand;
    bool useIpModule;
    bool closedUserGroup;
    QString apn;
    int contextId;
    QString pdpType;

    void copy( const QDialOptionsPrivate *d )
    {
        number = d->number;
        callerId = d->callerId;
        contact = d->contact;
        speed = d->speed;
        gsmSpeed = d->gsmSpeed;
        bearer = d->bearer;
        transparentMode = d->transparentMode;
        extensionOptions = d->extensionOptions;
        programName = d->programName;
        args = d->args;
        connectScript = d->connectScript;
        disconnectScript = d->disconnectScript;
        demand = d->demand;
        useIpModule = d->useIpModule;
        apn = d->apn;
        contextId = d->contextId;
        pdpType = d->pdpType;
        closedUserGroup = d->closedUserGroup;
    }

    template <typename Stream> void read( Stream &stream )
    {
        int value;
        stream >> number;
        stream >> value;
        callerId = (QDialOptions::CallerId)value;
        stream >> contact;
        stream >> speed;
        stream >> gsmSpeed;
        stream >> value;
        bearer = (QDialOptions::Bearer)value;
        stream >> value;
        transparentMode = (QDialOptions::TransparentMode)value;
        stream >> extensionOptions;
        stream >> programName;
        stream >> args;
        stream >> connectScript;
        stream >> disconnectScript;
        stream >> value;
        demand = ( value != 0 );
        stream >> value;
        useIpModule = ( value != 0 );
        stream >> apn;
        stream >> contextId;
        stream >> pdpType;
        stream >> value;
        closedUserGroup = ( value != 0 );
    }

    template <typename Stream> void write( Stream &stream ) const
    {
        stream << number;
        stream << (int)callerId;
        stream << contact;
        stream << speed;
        stream << gsmSpeed;
        stream << (int)bearer;
        stream << (int)transparentMode;
        stream << extensionOptions;
        stream << programName;
        stream << args;
        stream << connectScript;
        stream << disconnectScript;
        stream << (int)demand;
        stream << (int)useIpModule;
        stream << apn;
        stream << contextId;
        stream << pdpType;
        stream << (int)closedUserGroup;
    }
};

/*!
    Creates a dial options object with default values.
*/
QDialOptions::QDialOptions()
{
    d = new QDialOptionsPrivate();
}

/*!
    Makes a copy of \a other.
*/
QDialOptions::QDialOptions( const QDialOptions& other )
{
    d = new QDialOptionsPrivate();
    d->copy( other.d );
}

/*!
    Deletes this dial options object.
*/
QDialOptions::~QDialOptions()
{
    delete d;
}

/*!
    Makes a copy of \a other.
*/
QDialOptions& QDialOptions::operator=( const QDialOptions& other )
{
    if ( &other != this )
        d->copy( other.d );
    return *this;
}

/*!
    Returns the phone number to be dialed.  The default is the empty string.

    \sa setNumber()
*/
QString QDialOptions::number() const
{
    return d->number;
}

/*!
    Sets the phone number to be dialed to \a value.

    \sa number()
*/
void QDialOptions::setNumber( const QString& value )
{
    d->number = value;
}

/*!
    \enum QDialOptions::CallerId
    This enum defines how caller ID information should be transmitted
    during a dial.

    \value DefaultCallerId Use the default configuration.
    \value SendCallerId Always send caller ID, even if normally disabled.
    \value SuppressCallerId Never send caller ID, even if normally enabled.
*/

/*!
    Returns the current caller-ID mode to use for this call.
    The default is \c DefaultCallerId.

    \sa setCallerId()
*/
QDialOptions::CallerId QDialOptions::callerId() const
{
    return d->callerId;
}

/*!
    Sets the caller-ID mode to use for this call to \a value.

    \sa callerId()
*/
void QDialOptions::setCallerId( QDialOptions::CallerId value )
{
    d->callerId = value;
}

/*!
    Returns true if closed user group information should be used for
    this call, even if it is normally not used by default for calls; otherwise returns false.
    The default value is false.

    \sa setClosedUserGroup()
*/
bool QDialOptions::closedUserGroup() const
{
    return d->closedUserGroup;
}

/*!
    Sets the closed user group mode to \a value.

    \sa closedUserGroup()
*/
void QDialOptions::setClosedUserGroup( bool value )
{
    d->closedUserGroup = value;
}

/*!
    Returns the unique contact id associated with the call.
    The default is a null contact id.

    \sa setContact()
*/
QUniqueId QDialOptions::contact() const
{
    return d->contact;
}

/*!
    Sets the unique contact id associated with the call to \a value.

    \sa contact()
*/
void QDialOptions::setContact( const QUniqueId& value )
{
    d->contact = value;
}

/*!
    Returns the explicit bit rate speed.  The default value of -1 indicates
    that the system should choose the best speed for the requested service.

    \sa setSpeed()
*/
int QDialOptions::speed() const
{
    return d->speed;
}

/*!
    Sets the explicit bit rate speed to \a value.  For example, setting
    this to 9600 will select a bearer capable of 9600 bps.

    \sa speed()
*/
void QDialOptions::setSpeed( int value )
{
    d->speed = value;
}

/*!
    Returns the explicit 3GPP TS 27.007 bearer speed value.  The default value
    of -1 indicates that the system will use a bearer speed value compatible
    with the explicit bit rate specified by speed().

    \sa setGsmSpeed()
*/
int QDialOptions::gsmSpeed() const
{
    return d->gsmSpeed;
}

/*!
    Sets the explicit 3GPP TS 27.007 bearer speed to \a value.  The argument
    should be one of values from 3GPP TS 27.007's documentation of the
    \c{AT+CBST} command.  If the value is set to -1, then the system
    will use a bearer speed value compatible with the explicit
    bit rate specified by speed().

    This function should only be used when the caller is absolutely certain
    which bearer speed value they want, and they are certain that the
    underlying modem supports that value.  The setSpeed() function is a
    better interface because it can determine the appropriate low-level
    bearer speed value from the capabilities of the modem.

    \sa gsmSpeed()
*/
void QDialOptions::setGsmSpeed( int value )
{
    d->gsmSpeed = value;
}

/*!
    \enum QDialOptions::Bearer
    This enumeration defines the bearer type to use for data traffic.
    The values are defined by 3GPP TS 27.007.

    \value DataCircuitAsyncUDI Data circuit asynchronous (UDI or 3.1 kHz modem)
    \value DataCircuitSyncUDI Data circuit synchronous (UDI or 3.1 kHz modem)
    \value PadAccessUDI PAD access (asynchronous) (UDI)
    \value PacketAccessUDI Packet access (synchronous) (UDI)
    \value DataCircuitAsyncRDI Data circuit asynchronous (RDI)
    \value DataCircuitSyncRDI Data circuit synchronous (RDI)
    \value PadAccessRDI PAD access (asynchronous) (RDI)
    \value PacketAccessRDI Packet access (synchronous) (RDI)
*/

/*!
    Returns the bearer type to use for data traffic.  The default is
    \c DataCircuitAsyncUDI.

    \sa setBearer()
*/
QDialOptions::Bearer QDialOptions::bearer() const
{
    return d->bearer;
}

/*!
    Sets the bearer type to use for data traffic to \a value.

    \sa bearer()
*/
void QDialOptions::setBearer( QDialOptions::Bearer value )
{
    d->bearer = value;
}

/*!
    \enum QDialOptions::TransparentMode
    This enumeration defines the data transparency modes for data traffic.
    The values are defined by 3GPP TS 27.007.

    \value Transparent Transparent data traffic.
    \value NonTransparent Non-transparent data traffic.
    \value TransparentPreferred Both, transparent preferred.
    \value NonTransparentPreferred Both, non-transparent preferred.
*/

/*!
    Returns the data traffic transparency mode.  The default value is
    \c NonTransparent.

    \sa setTransparentMode()
*/
QDialOptions::TransparentMode QDialOptions::transparentMode() const
{
    return d->transparentMode;
}

/*!
    Sets the data traffic transparency mode to \a value.

    \sa transparentMode()
*/
void QDialOptions::setTransparentMode( QDialOptions::TransparentMode value )
{
    d->transparentMode = value;
}

/*!
    Returns the value of the extension option called \a name.  If the
    option is not present, \a def will be returned as the default value.

    \sa setExtensionOption()
*/
QVariant QDialOptions::extensionOption
        ( const QString& name, const QVariant& def ) const
{
    QMap< QString, QVariant >::ConstIterator it;
    it = d->extensionOptions.find( name );
    if ( it != d->extensionOptions.end() )
        return it.value();
    else
        return def;
}

/*!
    Sets the extension option \a name to \a value.  Extension options are
    used to set dial features beyond those defined by the GSM standards.
    Non-GSM telephony implementations might need such extension options.

    The options are passed to the telephony implementation, which will
    obey the options it understands and ignore the rest.  If the telephony
    implementation expects an option to be present, and it isn't, then the
    implementation should choose a reasonable default based on the type
    of call being made.

    \sa extensionOption()
*/
void QDialOptions::setExtensionOption
        ( const QString& name, const QVariant& value )
{
    d->extensionOptions[ name ] = value;
}

/*!
    Returns true if the call will use a IP module such as \c pppd to
    manage the data within the call; otherwise returns false.

    \sa setUseIpModule()
*/
bool QDialOptions::useIpModule() const
{
    return d->useIpModule;
}

/*!
    Sets the IP module flag to \a value.  If \a value is true, then
    the ipProgramName(), ipArgs(), ipConnectScript(), ipDisconnectScript(),
    and ipDemandDialing() functions should be used to determine how to
    launch the IP module.

    \sa useIpModule()
*/
void QDialOptions::setUseIpModule( bool value )
{
    d->useIpModule = value;
}

/*!
    Returns the full pathname of the pppd binary to be executed.
    The default value is \c{/usr/sbin/pppd}.

    \sa setIpProgramName()
*/
QString QDialOptions::ipProgramName() const
{
    return d->programName;
}

/*!
    Sets the full pathname of the pppd binary to \a value.

    \sa ipProgramName()
*/
void QDialOptions::setIpProgramName( const QString& value )
{
    d->programName = value;
}

/*!
    Returns the list of command-line options to be passed to pppd.

    \sa setIpArgs()
*/
QStringList QDialOptions::ipArgs() const
{
    return d->args;
}

/*!
    Sets the list of command-line options to be passed to pppd to \a value.

    The list should not include the tty device name or port speed
    that normally appear at the start of the pppd command-line.  These
    options will be added automatically by the system.

    The list should also not include the \c{connect} or \c{disconnect}
    options, as they will be set based on the information in
    connectScript() and disconnectScript().

    \sa ipArgs()
*/
void QDialOptions::setIpArgs( const QStringList& value )
{
    d->args = value;
}

/*!
    Returns the filename for the connect script.

    \sa setIpConnectScript()
*/
QString QDialOptions::ipConnectScript() const
{
    return d->connectScript;
}

/*!
    Sets the \a filename for the connect script.  The contents of the
    file must be formatted according to the chat(1) manual page.

    \sa ipConnectScript()
*/
void QDialOptions::setIpConnectScript( const QString& filename )
{
    d->connectScript = filename;
}

/*!
    Returns the filename for the disconnect script.

    \sa setIpDisconnectScript()
*/
QString QDialOptions::ipDisconnectScript() const
{
    return d->disconnectScript;
}

/*!
    Sets the \a filename for the disconnect script.  The contents of the
    file must be formatted according to the chat(1) manual page.

    \sa ipDisconnectScript()
*/
void QDialOptions::setIpDisconnectScript( const QString& filename )
{
    d->disconnectScript = filename;
}

/*!
    Returns true if the demand-dialing flag is set; otherwise returns false.
    If the demand-dialing flag is set to true, then it
    indicates that pppd should be started, but the dial process should
    not happen yet.  The dial will happen only when pppd detects TCP/IP
    network activity.

    If the demand-dialing flag is set to false, then it indicates
    that pppd should be started and the dial process should be initiated
    immediately.

    \sa setIpDemandDialing()
*/
bool QDialOptions::ipDemandDialing() const
{
    return d->demand;
}

/*!
    Sets the demand-dialing flag to \a value.

    \sa ipDemandDialing()
*/
void QDialOptions::setIpDemandDialing( bool value )
{
    d->demand = value;
}


/*!
    Returns the context identifier for GPRS sessions.  Returns zero if
    the session is not GPRS.

    \sa setContextId()
*/
int QDialOptions::contextId() const
{
    return d->contextId;
}

/*!
    Sets the context identifier for GPRS sessions to \a value.

    \sa contextId()
*/
void QDialOptions::setContextId( int value )
{
    d->contextId = value;
}

/*!
    Returns the PDP type for GPRS sessions.

    \sa setPdpType()
*/
QString QDialOptions::pdpType() const
{
    return d->pdpType;
}

/*!
    Sets the PDP type for GPRS sessions to \a value.  The contextId()
    must be non-zero for the type to be used when setting up a GPRS session.

    \sa pdpType()
*/
void QDialOptions::setPdpType( const QString& value )
{
    d->pdpType = value;
}

/*!
    Returns the access point name for GPRS sessions.

    \sa setApn()
*/
QString QDialOptions::apn() const
{
    return d->apn;
}

/*!
    Sets the access point for GPRS sessions to \a value.  The contextId()
    must be non-zero for the APN to be used when setting up a GPRS session.

    \sa apn()
*/
void QDialOptions::setApn( const QString& value )
{
    d->apn = value;
}


// User code

/*!
    \internal
    \fn void QDialOptions::serialize(Stream &stream) const
*/
template <typename Stream> void QDialOptions::serialize(Stream &stream) const
{
    d->write(stream);
}

/*!
    \internal
    \fn void QDialOptions::deserialize(Stream &stream)
*/
template <typename Stream> void QDialOptions::deserialize(Stream &stream)
{
    d->read(stream);
}

Q_IMPLEMENT_USER_METATYPE(QDialOptions)
