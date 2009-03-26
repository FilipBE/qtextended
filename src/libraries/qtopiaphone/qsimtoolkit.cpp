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

#include <qsimtoolkit.h>
#include <qtopianamespace.h>

/*!
    \class QSimToolkit
    \inpublicgroup QtTelephonyModule

    \brief The QSimToolkit class provides an interface to SIM toolkit applications.

    This class is used by a host program (usually \c simapp) to access
    the SIM toolkit facilities defined by 3GPP TS 11.14.

    The SIM toolkit application sends a stream of QSimCommand objects
    to the host program by way of the command() signal.
    The host program then performs the designated command and responds
    by calling one of the methods in the QSimToolkit class.

    For example, if the SIM toolkit sends a \c SetupMenu command
    to the host program, the program displays the indicated menu
    to the user.  When the user selects an item, the host program
    calls sendEnvelope() to inform the SIM toolkit of the choice.  The
    SIM toolkit will then send another QSimCommand object for the
    next phase of the application.

    Most commands use sendResponse() to send the client's response to the
    command, except \c SetupMenu which uses sendEnvelope().

    Some commands are automatically acknowledged by the modem or modem
    vendor plugin and do not need an explicit client acknowledgement
    with sendResponse().  The emitCommandAndRespond() function documents
    the commands that should be automatically acknowledged.

    \ingroup telephony
    \sa QSimCommand, QSimMenuItem
*/

/*!
    Construct a new SIM toolkit handling object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports SIM toolkit.  If there is more than one
    service that supports SIM toolkit, the caller should enumerate
    them with QCommServiceManager::supports() and create separate
    QSimToolkit objects for each.

    \sa QCommServiceManager::supports()
*/
QSimToolkit::QSimToolkit( const QString& service, QObject *parent,
                          QCommInterface::Mode mode )
    : QCommInterface( "QSimToolkit", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this SIM toolkit handling object.
*/
QSimToolkit::~QSimToolkit()
{
}


/*!
    Begin using the SIM toolkit facilities and request the main menu.

    This is the first method that should be called by a host program
    that wishes to access the SIM toolkit.

    In response, this class will emit a command() signal with the main menu,
    or a beginFailed() signal if SIM toolkit facilities are not available.

    \sa end(), command(), beginFailed()
*/
void QSimToolkit::begin()
{
    invoke( SLOT(begin()) );
}


/*!
    End using the SIM toolkit facilities.

    \sa begin()
*/
void QSimToolkit::end()
{
    invoke( SLOT(end()) );
}


/*!
    Sends a \c{TERMINAL RESPONSE} \a resp for the last SIM toolkit command that was
    received via command().  The command must be set on the response with
    QSimTerminalResponse::setCommand() so that the SIM knows which command is
    being responded to.

    \sa sendEnvelope(), command(), QSimTerminalResponse::setCommand()
*/
void QSimToolkit::sendResponse( const QSimTerminalResponse& resp )
{
    invoke( SLOT(sendResponse(QSimTerminalResponse)), qVariantFromValue( resp ) );
}


/*!
    Sends an \c{ENVELOPE} \a env to the SIM toolkit application.  This is typically
    used for selecting items from the main menu.

    \sa sendResponse(), command()
*/
void QSimToolkit::sendEnvelope( const QSimEnvelope& env )
{
    invoke( SLOT(sendEnvelope(QSimEnvelope)), qVariantFromValue( env ) );
}


/*!
    Indicate to the SIM toolkit application that an \a item on the
    main menu has been selected.

    Applies to: \c SetupMenu.

    This function is deprecated.  Use sendEnvelope() to generate a QSimEnvelope::MenuSelection
    envelope instead.

    \sa sendEnvelope()
*/
void QSimToolkit::mainMenuSelected( uint item )
{
    QSimEnvelope env;
    env.setType( QSimEnvelope::MenuSelection );
    env.setSourceDevice( QSimCommand::Keypad );
    env.setMenuItem( item );
    sendEnvelope( env );
}


/*!
    The user has cleared the text display after a \c DisplayText
    command that did not have QSimCommand::clearAfterDelay() set.

    Applies to: \c DisplayText.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::Success
    response instead.

    \sa sendResponse()
*/
void QSimToolkit::clearText()
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::DisplayText );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::Success );
    resp.setCommand( cmd );
    sendResponse( resp );
}


/*!
    Sends a string \a value in response to a \c GetInkey command.

    Applies to: \c GetInkey.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::Success
    response instead.

    \sa sendResponse(), input()
*/
void QSimToolkit::key( const QString& value )
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetInkey );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::Success );
    resp.setCommand( cmd );
    resp.setText( value );
    sendResponse( resp );
}


/*!
    Sends a string \a value in response to a \c GetInput command.

    Applies to: \c GetInput.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::Success
    response instead.

    \sa key()
*/
void QSimToolkit::input( const QString& value )
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::GetInput );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::Success );
    resp.setCommand( cmd );
    resp.setText( value );
    sendResponse( resp );
}


/*!
    Allow a call setup request to proceed.

    Applies to: \c SetupCall.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::Success
    response instead.

    \sa denyCallSetup()
*/
void QSimToolkit::allowCallSetup()
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupCall );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::Success );
    resp.setCommand( cmd );
    sendResponse( resp );
}


/*!
    The user denied a call setup request.

    Applies to: \c SetupCall.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::UserDidNotAccept
    response instead.

    \sa allowCallSetup()
*/
void QSimToolkit::denyCallSetup()
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::SetupCall );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::UserDidNotAccept );
    resp.setCommand( cmd );
    sendResponse( resp );
}


/*!
    Indicate to the SIM toolkit application that an \a item on a
    sub menu has been selected.

    Applies to: \c SelectItem.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::Success
    response instead.

    \sa subMenuExited()
*/
void QSimToolkit::subMenuSelected( uint item )
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::SelectItem );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::Success );
    resp.setCommand( cmd );
    resp.setMenuItem( item );
    sendResponse( resp );
}


/*!
    Indicate to the SIM toolkit application that the user has
    chosen to exit from the current sub menu.

    Applies to: \c SelectItem.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::BackwardMove
    response instead.

    \sa subMenuSelected()
*/
void QSimToolkit::subMenuExited()
{
    QSimCommand cmd;
    cmd.setType( QSimCommand::SelectItem );

    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::BackwardMove );
    resp.setCommand( cmd );
    sendResponse( resp );
}


/*!
    Indicate to the SIM toolkit application that the system has become idle
    and the SIM had previously requested \c IdleScreen events.

    Applies to: \c SetupEventList.

    This function is deprecated.  Use sendEnvelope() to generate an idle screen
    event instead.

    \sa userActivity()
*/
void QSimToolkit::idleScreen()
{
    QSimEnvelope env;
    env.setType( QSimEnvelope::EventDownload );
    env.setEvent( QSimEnvelope::IdleScreenAvailable );
    sendEnvelope( env );
}


/*!
    Indicate to the SIM toolkit application that user activity has occurred
    and the SIM had previously requested \c UserActivity events.

    Applies to: \c SetupEventList.

    This function is deprecated.  Use sendEnvelope() to generate a user activity
    event instead.

    \sa idleScreen()
*/
void QSimToolkit::userActivity()
{
    QSimEnvelope env;
    env.setType( QSimEnvelope::EventDownload );
    env.setEvent( QSimEnvelope::UserActivity );
    sendEnvelope( env );
}


/*!
    End the current session, upon the user's request.  The SIM returns
    to the main menu.

    Applies to: \c GetInkey, \c GetInput, \c SelectItem.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::SessionTerminated
    response instead.
*/
void QSimToolkit::endSession()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::SessionTerminated );
    sendResponse( resp );
}


/*!
    Move backward within the current session.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::BackwardMove
    response instead.
*/
void QSimToolkit::moveBackward()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::BackwardMove );
    sendResponse( resp );
}


/*!
    Tell the SIM that the previous request was beyond the
    capability of the ME to perform.  i.e. the ME does not
    have the functionality at all.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::BeyondMECapabilities
    response instead.
*/
void QSimToolkit::cannotProcess()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::BeyondMECapabilities );
    sendResponse( resp );
}


/*!
    Tell the SIM that the ME cannot process the previous
    request at this time, but may be able to later.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::MEUnableToProcess
    response instead.
*/
void QSimToolkit::temporarilyUnavailable()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::MEUnableToProcess );
    sendResponse( resp );
}


/*!
    Tell the SIM that no response has been received from the
    user within a reasonable period of time.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::NoResponseFromUser
    response instead.
*/
void QSimToolkit::noResponseFromUser()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::NoResponseFromUser );
    sendResponse( resp );
}


/*!
    Tell the SIM that the current session was aborted by
    the user.  The SIM returns to the main menu.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() to generate a QSimTerminalResponse::SessionTerminated
    response instead.
*/
void QSimToolkit::aborted()
{
    // Don't know what command this should apply to.
    QSimTerminalResponse resp;
    resp.setResult( QSimTerminalResponse::SessionTerminated );
    sendResponse( resp );
}


/*!
    Request help for a particular type of \a command or menu \a item.

    Applies to: \c SetupMenu, \c GetInkey, \c GetInput, \c SelectItem.

    This function is deprecated because it cannot properly match commands
    and responses.  Use sendResponse() or sendEnvelope() to generate the appropriate
    help request instead.
*/
void QSimToolkit::help( QSimCommand::Type command, uint item )
{
    if ( command == QSimCommand::SetupMenu ) {
        QSimEnvelope env;
        env.setType( QSimEnvelope::MenuSelection );
        env.setSourceDevice( QSimCommand::Keypad );
        env.setMenuItem( item );
        env.setRequestHelp( true );
        sendEnvelope( env );
    } else {
        QSimCommand cmd;
        cmd.setType( command );
        QSimTerminalResponse resp;
        resp.setResult( QSimTerminalResponse::HelpInformationRequested );
        resp.setMenuItem( item );
        sendResponse( resp );
    }
}


/*!
    \fn void QSimToolkit::command(const QSimCommand& command)

    Signal that is emitted when the application running in the SIM
    wishes to send a \a command to the host program.

    Usually server-side implementations will use emitCommandAndRespond()
    instead of directly emitting command(), because there are some commands
    that need an immediate response.

    \sa QSimCommand, emitCommandAndRespond()
*/

/*!
    \fn void QSimToolkit::beginFailed()

    Signal that is emitted if begin() fails because SIM
    toolkit functionality was not available.
*/

/*!
    \fn void QSimToolkit::controlEvent( const QSimControlEvent& event )

    Signal that is emitted when a SIM control \a event arrives.

    \sa QSimControlEvent
*/

/*!
    Emits the command() signal, passing \a command as its argument.
    Then, if the commmand needs an immediate response to the SIM toolkit
    application, then sendResponse() is called.

    The following commands need an immediate response by emitCommandAndRespond(),
    but the commands themselves are processed by client applications:

    \table
        \row \o \c SetupMenu \o Acknowledges receipt of the command.  The actual menu
                                selection is performed using sendEnvelope(),
                                not sendResponse().
        \row \o \c SetupIdleModeText \o Acknowledges receipt of the command.
    \endtable

    The following commands are expected to be handled internally by the modem,
    or the modem vendor plugin.  Qt Extended will simply acknowledge receipt of the
    command with sendResponse() and then passes the command up to the client applications.
    Most of these can be safely ignored by client applications, or the application
    only needs to display a text string and/or an icon until such time as a
    new command becomes available.

    \table
        \row \o \c MoreTime
        \row \o \c PollingOff
        \row \o \c PollInterval
        \row \o \c Refresh
        \row \o \c SendSMS
        \row \o \c SendSS
        \row \o \c SendUSSD
        \row \o \c SendDTMF
        \row \o \c PerformCardAPDU
        \row \o \c PowerOnCard
        \row \o \c PowerOffCard
        \row \o \c GetReaderStatus
        \row \o \c ProvideLocalInformation
        \row \o \c TimerManagement
        \row \o \c SetupEventList
        \row \o \c RunATCommand
        \row \o \c LanguageNotification
    \endtable

    If the modem does not handle one or more of the above commands internally,
    the modem vendor plugin is responsible implementing the command and for
    composing the correct terminal response.  If the modem vendor plugin does this,
    it should bypass emitCommandAndRespond() and emit the command() signal directly.

    For example, if the modem does not handle the \c SendSMS command internally,
    the modem vendor plugin is responsible for formatting the correct SMS message
    and transmitting it over the network.  The client application only needs to
    handle text display while the message is being sent.

    In the case of \c ProvideLocalInformation, if the language setting is being
    requested, the current Qt Extended language code is returned.

    \sa command(), sendResponse()
*/
void QSimToolkit::emitCommandAndRespond( const QSimCommand& command )
{
    emit this->command( command );

    bool needImmediate = false;
    QSimTerminalResponse resp;

    switch ( command.type() ) {

        case QSimCommand::SetupMenu:
        case QSimCommand::SetupIdleModeText:
        case QSimCommand::MoreTime:
        case QSimCommand::PollingOff:
        case QSimCommand::PollInterval:
        case QSimCommand::Refresh:
        case QSimCommand::SendSMS:
        case QSimCommand::SendSS:
        case QSimCommand::SendUSSD:
        case QSimCommand::SendDTMF:
        case QSimCommand::PerformCardAPDU:
        case QSimCommand::PowerOnCard:
        case QSimCommand::PowerOffCard:
        case QSimCommand::GetReaderStatus:
        case QSimCommand::TimerManagement:
        case QSimCommand::SetupEventList:
        case QSimCommand::RunATCommand:
        case QSimCommand::LanguageNotification:
        {
            needImmediate = true;
        }
        break;

        case QSimCommand::ProvideLocalInformation:
        {
            needImmediate = true;
            if ( command.qualifier() == 0x04 ) {
                // Send the current language back to the SIM.
                QStringList languages = Qtopia::languageList();
                QString lang;
                if ( languages.size() >= 1 && languages[0].length() >= 2 ) {
                    lang = languages[0].left(2);
                } else {
                    // Something wrong with the language list - assume English.
                    lang = "en";
                }
                resp.addExtensionField( 0xAD, lang.toLatin1() );
            }
        }
        break;

        default: break;

    }
    if ( needImmediate ) {
        resp.setCommand( command );
        resp.setResult( QSimTerminalResponse::Success );
        if ( command.type() == QSimCommand::PollInterval )
            resp.setDuration( command.duration() );
        sendResponse( resp );
    }
}
