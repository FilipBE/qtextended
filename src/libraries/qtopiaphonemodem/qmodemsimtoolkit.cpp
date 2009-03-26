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

#include <qmodemsimtoolkit.h>
#include <qmodemservice.h>
#include <qsmsmessage.h>
#include <qgsmcodec.h>
#include <qatutils.h>
#include <qatresultparser.h>
#include <qtopialog.h>

/*!
    \class QModemSimToolkit
    \inpublicgroup QtCellModule

    \brief The QModemSimToolkit class provides access to SIM toolkit functionality for AT-based modems.
    \ingroup telephony::modem

    QModemSimToolkit implements the QSimToolkit telephony interface.  Client
    applications should use QSimToolkit instead of this class to
    access the modem's SIM toolkit facilities.

    There are no 3GPP standard for AT commands to access the SIM toolkit.
    This class will fail begin() requests, indicating that there is no SIM
    toolkit in the system.  Modem vendor plug-ins should inherit this class
    and implement SIM toolkit with their own proprietary commands.

    Default implementations of sendResponse() and sendEnvelope() are provided which
    send the binary formats from 3GPP TS 11.14 to the SIM using the \c{AT+CSIM}
    command.  While this may work for some modems, most modem vendor plugins
    will need to override sendResponse() and sendEnvelope() and use their own commands.

    The fetchCommand() function provides support for modems that use \c{AT+CSIM}
    to fetch proactive SIM commands.  However, it is still necessary for the modem
    vendor plugin to detect when a command is available and then call fetchCommand().

    \sa QSimToolkit
*/

class QModemSimToolkitPrivate
{
public:
    QModemSimToolkitPrivate( QModemService *service )
    {
        this->service = service;
        this->initActive = false;
    }

    QModemService *service;
    bool initActive;
};

/*!
    Create a modem SIM toolkit handler for \a service.
*/
QModemSimToolkit::QModemSimToolkit( QModemService *service )
    : QSimToolkit( service->service(), service, QCommInterface::Server )
{
    d = new QModemSimToolkitPrivate( service );
}

/*!
    Destroy this modem SIM toolkit handler.
*/
QModemSimToolkit::~QModemSimToolkit()
{
    delete d;
}

/*!
    Return the QModemService that this handler is associated with.
*/
QModemService *QModemSimToolkit::service() const
{
    return d->service;
}

/*!
    Initialize the SIM toolkit functionality.  Once initialization is
    complete, the modem vendor object must call initializationDone().

    If the initialization will take some time, and will not be complete
    before initialize() returns, the implementation should call
    initializationStarted().

    The default implementation calls initializationDone().
*/
void QModemSimToolkit::initialize()
{
    if ( d->initActive )
        initializationDone();
    else
        d->service->stkInitDone();
}

/*!
    \reimp

    This default implementation emits beginFailed().  It should be
    overridden in modem vendor plug-ins.
*/
void QModemSimToolkit::begin()
{
    emit beginFailed();
}

/*!
    \reimp
*/
void QModemSimToolkit::end()
{
    // Nothing to do here.
}

/*!
    Sends a \c{TERMINAL RESPONSE} \a resp for the last SIM toolkit command that was
    received via command().  The command must be set on the response with
    QSimTerminalResponse::setCommand() so that the SIM knows which command is
    being responded to.

    The implementation in QModemSimToolkit sends the response to the SIM using the
    \c{AT+CSIM} command.  This will need to be overridden by the modem vendor plugin
    if the modem uses some other mechanism for sending terminal responses to the SIM.

    \sa sendEnvelope(), command(), QSimTerminalResponse::setCommand()
*/
void QModemSimToolkit::sendResponse( const QSimTerminalResponse& resp )
{
    QByteArray cmd;
    QByteArray pdu = resp.toPdu();
    cmd += (char)0xA0;
    cmd += (char)0x14;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)pdu.size();
    cmd += pdu;
    service()->chat( "AT+CSIM=" + QString::number( cmd.size() * 2 ) + "," + 
                     QAtUtils::toHex( cmd ) );
}

/*!
    Sends an \c{ENVELOPE} \a env to the SIM toolkit application.  This is typically
    used for selecting items from the main menu.

    The implementation in QModemSimToolkit sends the envelope to the SIM using the
    \c{AT+CSIM} command.  This will need to be overridden by the modem vendor plugin
    if the modem uses some other mechanism for sending envelopes to the SIM.

    \sa sendResponse(), command()
*/
void QModemSimToolkit::sendEnvelope( const QSimEnvelope& env )
{
    QByteArray cmd;
    QByteArray pdu = env.toPdu();
    cmd += (char)0xA0;
    cmd += (char)0xC2;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)pdu.size();
    cmd += pdu;
    service()->chat( "AT+CSIM=" + QString::number( cmd.size() * 2 ) + "," + 
                     QAtUtils::toHex( cmd ) );
}

/*!
    Indicate that SIM toolkit initialization has started.

    \sa initializationDone()
*/
void QModemSimToolkit::initializationStarted()
{
    d->initActive = true;
}

/*!
    Indicate that SIM toolkit initialization has finished.

    \sa initializationStarted()
*/
void QModemSimToolkit::initializationDone()
{
    if ( d->initActive ) {
        d->initActive = false;
        d->service->post( "pinquery" );
    }
}

/*!
    Directs QModemSimToolkit to fetch the current proactive SIM command with
    the \c{AT+CSIM} command.  The \a size indicates the size of the command
    that is waiting to be fetched.  Once the command is fetched, this calls
    emitCommandAndRespond().

    This function is intended for modem vendor plugins that use \c{AT+CSIM}
    to fetch proactive SIM commands.  This function is typically called when
    the modem vendor plugin receives an unsolicited notification indicating
    that a new command is available.

    If the modem uses some command other than \c{AT+CSIM} to fetch the
    current proactive SIM command, this function should be ignored.  Instead,
    the modem vendor plugin should fetch the command using the modem's
    proprietry commands and then call emitCommandAndRespond().

    \sa emitCommandAndRespond()
*/
void QModemSimToolkit::fetchCommand( int size )
{
    QByteArray cmd;
    cmd += (char)0xA0;
    cmd += (char)0x12;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)size;
    service()->chat( "AT+CSIM=" + QString::number( cmd.size() * 2 ) + "," + 
                     QAtUtils::toHex( cmd ), this, SLOT(fetch(bool,QAtResult)) );
}

void QModemSimToolkit::fetch( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CSIM:" ) ) {
        QString line = parser.line();
        uint posn = 0;
        QAtUtils::parseNumber( line, posn );    // Skip length.
        QString data;
        if ( ((int)posn) < line.length() && line[posn] == ',' )
            data = line.mid( posn + 1 );
        else
            data = line.mid( posn );
        if ( data.contains( QChar('"') ) )
            data = data.remove( QChar('"') );
        if ( data.length() <= 4 ) {
            // Need at least sw1 and sw2 on the end of the command.
            return;
        }
        QByteArray pdu = QAtUtils::fromHex( data.left( data.length() - 4 ) );
        if ( !pdu.isEmpty() )
            emitCommandAndRespond( QSimCommand::fromPdu( pdu ) );
    }
}
