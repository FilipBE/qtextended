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

#include <qsimcommand.h>
#include <qatutils.h>
#include <qsmsmessage.h>
#include <qgsmcodec.h>
#include <qvarlengtharray.h>

static QString textToHtml(const QString& text, const QByteArray& attrs);

class QSimMenuItemPrivate
{
public:
    QSimMenuItemPrivate()
    {
        identifier = 0;
        iconId = 0;
        hasHelp = false;
        iconSelfExplanatory = false;
        nextAction = 0;
    }

    uint identifier;
    QString label;
    QByteArray labelAttribute;
    uint iconId;
    bool hasHelp;
    bool iconSelfExplanatory;
    uint nextAction;
};


class QSimCommandPrivate
{
public:
    QSimCommandPrivate()
    {
        ref = 1;
        commandNumber = 1;
        type = QSimCommand::NoCommand;
        sourceDevice = QSimCommand::SIM;
        destinationDevice = QSimCommand::ME;
        flags = 0;
        minimumLength = 0;
        maximumLength = 255;
        callClass = QSimCommand::Voice;
        tone = QSimCommand::ToneNone;
        duration = 0;
        defaultItem = 0;
        iconId = 0;
        otherIconId = 0;
        device = -1;
        qualifier = 0;
    }
    QSimCommandPrivate( QSimCommandPrivate *other )
    {
        ref = 1;
        commandNumber = other->commandNumber;
        type = other->type;
        sourceDevice = other->sourceDevice;
        destinationDevice = other->destinationDevice;
        flags = other->flags;
        text = other->text;
        textAttribute = other->textAttribute;
        otherText = other->otherText;
        otherTextAttribute = other->otherTextAttribute;
        defaultText = other->defaultText;
        minimumLength = other->minimumLength;
        maximumLength = other->maximumLength;
        number = other->number;
        subAddress = other->subAddress;
        callClass = other->callClass;
        tone = other->tone;
        duration = other->duration;
        title = other->title;
        titleAttribute = other->titleAttribute;
        defaultItem = other->defaultItem;
        menuItems = other->menuItems;
        url = other->url;
        iconId = other->iconId;
        otherIconId = other->otherIconId;
        device = other->device;
        qualifier = other->qualifier;
        extensionData = other->extensionData;
    }

    template <typename Stream> int readInt( Stream &stream )
    {
        int value;
        stream >> value;
        return value;
    }

    template <typename Stream>  void read( Stream &stream )
    {
        commandNumber = readInt( stream );
        type = (QSimCommand::Type)readInt( stream );
        sourceDevice = (QSimCommand::Device)readInt( stream );
        destinationDevice = (QSimCommand::Device)readInt( stream );
        flags = readInt( stream );
        stream >> text;
        stream >> textAttribute;
        stream >> otherText;
        stream >> otherTextAttribute;
        stream >> defaultText;
        stream >> minimumLength;
        stream >> maximumLength;
        stream >> number;
        stream >> subAddress;
        callClass = (QSimCommand::CallClass)readInt( stream );
        tone = (QSimCommand::Tone)readInt( stream );
        stream >> duration;
        stream >> title;
        stream >> titleAttribute;
        stream >> defaultItem;
        stream >> menuItems;
        stream >> url;
        stream >> iconId;
        stream >> otherIconId;
        stream >> device;
        stream >> qualifier;
        stream >> extensionData;
    }

    template <typename Stream> void write( Stream &stream )
    {
        stream << commandNumber;
        stream << (int)type;
        stream << (int)sourceDevice;
        stream << (int)destinationDevice;
        stream << flags;
        stream << text;
        stream << textAttribute;
        stream << otherText;
        stream << otherTextAttribute;
        stream << defaultText;
        stream << minimumLength;
        stream << maximumLength;
        stream << number;
        stream << subAddress;
        stream << (int)callClass;
        stream << (int)tone;
        stream << duration;
        stream << title;
        stream << titleAttribute;
        stream << defaultItem;
        stream << menuItems;
        stream << url;
        stream << iconId;
        stream << otherIconId;
        stream << device;
        stream << qualifier;
        stream << extensionData;
    }

    bool flag( int bit ) const
    {
        return ( ( flags & bit ) != 0 );
    }

    void setFlag( int bit, bool value )
    {
        if ( value )
            flags |= bit;
        else
            flags &= ~bit;
    }

    bool qualifierBit( int bit ) const
    {
        return ( ( qualifier & bit ) != 0 );
    }

    void setQualifierBit( int bit, bool value )
    {
        if ( value )
            qualifier |= bit;
        else
            qualifier &= ~bit;
    }

    #define SC_ImmediateResponse    (1<<0)
    #define SC_IconSelfExplanatory  (1<<1)
    #define SC_IconSelfExplanatory2 (1<<2)
    #define SC_SuppressUserFeedback (1<<3)

    QAtomicInt ref;
    int commandNumber;
    QSimCommand::Type type;
    QSimCommand::Device sourceDevice;
    QSimCommand::Device destinationDevice;
    int flags;
    QString text;
    QByteArray textAttribute;
    QString otherText;
    QByteArray otherTextAttribute;
    QString defaultText;
    uint minimumLength;
    uint maximumLength;
    QString number;
    QString subAddress;
    QSimCommand::CallClass callClass;
    QSimCommand::Tone tone;
    uint duration;
    QString title;
    QByteArray titleAttribute;
    uint defaultItem;
    QList<QSimMenuItem> menuItems;
    QString url;
    uint iconId;
    uint otherIconId;
    int device;
    int qualifier;
    QByteArray extensionData;

};


/*!
    \class QSimMenuItem
    \inpublicgroup QtTelephonyModule

    \brief The QSimMenuItem class provides information about a menu item within a SIM toolkit application

    \ingroup telephony
    \sa QSimCommand, QSimToolkit
*/

/*!
    Construct a new menu item with default parameters.
*/
QSimMenuItem::QSimMenuItem()
{
    d = new QSimMenuItemPrivate();
}


/*!
    Construct a new menu item as a copy of \a value.
*/
QSimMenuItem::QSimMenuItem( const QSimMenuItem& value )
{
    d = new QSimMenuItemPrivate();
    d->identifier = value.d->identifier;
    d->label = value.d->label;
    d->labelAttribute = value.d->labelAttribute;
    d->iconId = value.d->iconId;
    d->hasHelp = value.d->hasHelp;
    d->iconSelfExplanatory = value.d->iconSelfExplanatory;
    d->nextAction = value.d->nextAction;
}


/*!
    Destruct a menu item.
*/
QSimMenuItem::~QSimMenuItem()
{
    delete d;
}


/*!
    Returns the menu item's numeric identifier.

    \sa setIdentifier()
*/
uint QSimMenuItem::identifier() const
{
    return d->identifier;
}


/*!
    Sets the menu item's numeric identifier to \a value.

    \sa identifier()
*/
void QSimMenuItem::setIdentifier( uint value )
{
    d->identifier = value;
}


/*!
    Returns the menu item's label.

    \sa setLabel()
*/
QString QSimMenuItem::label() const
{
    return d->label;
}


/*!
    Sets the menu item's label to \a value.

    \sa label()
*/
void QSimMenuItem::setLabel( const QString& value )
{
    d->label = value;
}


/*!
    Returns the menu item's text attributes for formatting label().
    Returns an empty QByteArray if the label() is not formatted.

    \sa setLabelAttribute(), label(), labelHtml()
    \since 4.4
*/
QByteArray QSimMenuItem::labelAttribute() const
{
    return d->labelAttribute;
}


/*!
    Sets the menu item's text attributes for formatting label() to \a value.
    If \a value is an empty QByteArray, then label() is not formatted.

    \sa labelAttribute(), label(), labelHtml()
    \since 4.4
*/
void QSimMenuItem::setLabelAttribute( const QByteArray& value )
{
    d->labelAttribute = value;
}


/*!
    Returns the menu item's label() as a HTML string.  If the menu
    item has labelAttribute(), then they will be used to format
    the label() appropriately.

    \sa label(), labelAttribute()
    \since 4.4
*/
QString QSimMenuItem::labelHtml() const
{
    return textToHtml( d->label, d->labelAttribute );
}


/*!
    Returns true if this menu item has help available; otherwise returns false.

    \sa setHasHelp()
*/
bool QSimMenuItem::hasHelp() const
{
    return d->hasHelp;
}


/*!
    Sets the flag that indicates if this menu item has help
    available to \a value.

    \sa hasHelp()
*/
void QSimMenuItem::setHasHelp( bool value )
{
    d->hasHelp = value;
}


/*!
    Returns the icon identifier associated with this menu item.
    Returns zero if there is no icon.

    \sa setIconId()
*/
uint QSimMenuItem::iconId() const
{
    return d->iconId;
}


/*!
    Sets the icon identifier associated with this menu item to \a value.

    \sa iconId()
*/
void QSimMenuItem::setIconId( uint value )
{
    d->iconId = value;
}


/*!
    Returns true if the icon specified by iconId() is self-explanatory
    without the display of label().  If this function returns false,
    then label() should be displayed next to the icon.  If iconId()
    returns zero, then iconSelfExplanatory() should be ignored.

    \sa setIconSelfExplanatory()
*/
bool QSimMenuItem::iconSelfExplanatory() const
{
    return d->iconSelfExplanatory;
}


/*!
    Sets the self-explanatory flag to \a value.

    \sa iconSelfExplanatory()
*/
void QSimMenuItem::setIconSelfExplanatory( bool value )
{
    d->iconSelfExplanatory = value;
}


/*!
    Returns the next action indicator for this menu item.  Next action indicators
    have the same values as command types from the QSimCommand::Type list.
    The default value is zero, indicating no next action indicator for this menu item.

    \sa setNextAction()
*/
uint QSimMenuItem::nextAction() const
{
    return d->nextAction;
}


/*!
    Sets the next action indicator for this menu item to \a value.  Next action
    indicators have the same values as command types from the QSimCommand::Type list.
    The value zero indicates that there is no next action indicator for this menu item.

    \sa nextAction()
*/
void QSimMenuItem::setNextAction( uint value )
{
    d->nextAction = value;
}


/*!
    Make a copy of \a value.
*/
QSimMenuItem& QSimMenuItem::operator=( const QSimMenuItem & value )
{
    d->identifier = value.d->identifier;
    d->label = value.d->label;
    d->labelAttribute = value.d->labelAttribute;
    d->iconId = value.d->iconId;
    d->hasHelp = value.d->hasHelp;
    d->iconSelfExplanatory = value.d->iconSelfExplanatory;
    d->nextAction = value.d->nextAction;
    return *this;
}


/*!
    \fn void QSimMenuItem::deserialize(Stream &value)

    \internal

    Deserializes the QSimMenuItem instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimMenuItem::deserialize(Stream &stream)
{
    stream >> d->identifier;
    stream >> d->label;
    stream >> d->labelAttribute;
    stream >> d->iconId;
    int value;
    stream >> value;
    d->hasHelp = (value != 0);
    stream >> value;
    d->iconSelfExplanatory = (value != 0);
    stream >> d->nextAction;
}

/*!
    \fn void QSimMenuItem::serialize(Stream &value) const

    \internal

    Serializes the QSimMenuItem instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimMenuItem::serialize(Stream &stream) const
{
    stream << d->identifier;
    stream << d->label;
    stream << d->labelAttribute;
    stream << d->iconId;
    stream << (int)(d->hasHelp);
    stream << (int)(d->iconSelfExplanatory);
    stream << d->nextAction;
}

/*!
    \class QSimCommand
    \inpublicgroup QtTelephonyModule

    \brief The QSimCommand class specifies the details of a SIM toolkit command message.

    Applications that run within a SIM send commands to the host program
    to interact with the user.  These commands might entail choosing an
    item from a menu, asking if it is OK to dial a phone number, asking
    for a line of input, displaying text messages, etc.

    The QSimCommand class encapsulates a single SIM toolkit command,
    containing all of the information about it.  QSimCommand objects
    are delivered to the host program by way of the QSimToolkit::command() signal.

    In Qtopia, the host program is \c simapp.

    \ingroup telephony
    \sa QSimToolkit, QSimMenuItem, QSimTerminalResponse
*/

/*!
    \enum QSimCommand::Type
    This enum defines the type of a SIM toolkit message within a
    QSimCommand object.

    \value NoCommand No command type currently set.
    \value Timeout Indicate that an operation has timed out.
    \value Refresh Notification of a SIM refresh.
    \value MoreTime The SIM toolkit application is asking for more time.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value PollInterval The SIM toolkit application is setting the polling interval
           for new commands.  This command type is handled automatically by the
           modem or the modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value PollingOff This SIM toolkit application is asking for polling to be turned off.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value SetupEventList Set up the list of events to be reported to the
           SIM toolkit application.
    \value SetupCall Set up a phone call to a specific number.
    \value SendSS Notification that the application is sending a SS message.
    \value SendSMS Notification that the application is sending a SMS message.
    \value SendUSSD Notification that the application is sending a USSD message.
    \value SendDTMF Notification that the application is sending a DTMF tone.
    \value LaunchBrowser Launch a Web browser on a URL.
    \value PlayTone Play a tone to the user.
    \value DisplayText Display a text message.
    \value GetInkey Get a single key of input.
    \value GetInput Get a line of input.
    \value SelectItem Process a submenu within a SIM toolkit application.
    \value SetupMenu Process the main menu of a SIM toolkit application.
    \value ProvideLocalInformation The SIM toolkit application is asking for local
           information to be supplied.  This command type is handled automatically
           by the modem or the modem vendor plugin, so client applications will
           not see this command type via QSimToolkit::command().
    \value TimerManagement The SIM toolkit application is activating or deactivating
           timers.  This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value SetupIdleModeText Set the text to be displayed when the phone is idle.
    \value PerformCardAPDU Send an APDU to the additional card reader.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value PowerOnCard Power on an additional card reader.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value PowerOffCard Power off an additional card reader.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value GetReaderStatus Get the status of an additional card reader.
           This command type is handled automatically by the modem or the
           modem vendor plugin, so client applications will not see this
           command type via QSimToolkit::command().
    \value RunATCommand Run an AT command against the modem
    \value LanguageNotification Inform the user about the language the SIM
           toolkit application will be displaying messages in.
    \value OpenChannel Open a data channel.
    \value CloseChannel Close a data channel.
    \value ReceiveData Receive data on a data channel.
    \value SendData Send data on a data channel.
    \value GetChannelStatus Get the current status of a data channel.
    \value ServiceSearch Search for the availability of a service in the
           environment of the terminal.  Since Qtopia 4.4.  Compliant
           with ETSI TS 102 223.
    \value GetServiceInformation Get the complete service record related
           to a service.  Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value DeclareService Declare a service that is provided by the SIM.
           Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value SetFrames Divide the screen into multiple, scrollable rectangular
           regions in order to present multiple information at once.
           Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value GetFramesStatus Get the status on the frames supported by the SIM.
           Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value RetrieveMultimediaMessage Retrieve a multimedia message from
           the network.  Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value SubmitMultimediaMessage Submit a multimedia message to the network.
           Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value DisplayMultimediaMessage Display a multimedia message.
           Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value EndSession Indicate that the SIM toolkit session has ended
    \value SetupMainMenu Alias for SetupMenu, provided for backwards compatiblity.
    \value SetupSubMenu Alias for SelectItem, provided for backwards compatiblity.
*/

/*!
    \enum QSimCommand::Disposition
    This enum defines the disposition of other calls when a \c SetupCall
    command is executed by the SIM toolkit application.

    \value IfNoOtherCalls Perform the setup only if there are no other calls.
    \value PutOnHold Put other calls on hold when the new call is setup.
    \value Disconnect Disconnect other calls when the new call is setup.
*/

/*!
    \enum QSimCommand::CallClass
    This enum defines the class of call that is being created by a
    \c SetupCall command.

    \value Voice Voice call
    \value Data Data call
    \value Fax Fax call
*/

/*!
    \enum QSimCommand::Tone
    This enum defines the tones that can be played by a \c PlayTone command.

    \value ToneNone No tone specified.
    \value ToneDial Currently dialing
    \value ToneBusy Called party is busy
    \value ToneCongestion Network is congested
    \value ToneRadioAck Radio acknowledged
    \value ToneDropped Connection has been dropped
    \value ToneError An error occurred
    \value ToneCallWaiting An incoming call is waiting
    \value ToneRinging Ring to indicate and incoming call
    \value ToneGeneralBeep A general beep tone
    \value TonePositiveBeep A beep indicating a positive outcome
    \value ToneNegativeBeep A beep indicating a negative outcome

*/

/*!
    \enum QSimCommand::RefreshType
    This enum defines the type of refresh that was performed by the SIM.

    \value InitAndFullFileChange SIM initialization and full file
           change notification.
    \value FileChange File change notification.
    \value InitAndFileChange SIM initialization and file change notification.
    \value Initialization SIM initialization.
    \value Reset SIM reset.
    \value NaaApplicationReset NAA application reset.  This is only applicable
           for a 3G platform.  Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
    \value NaaSessionReset NAA session reset.  This is only applicable
           for a 3G platform.  Since Qtopia 4.4.  Compliant with ETSI TS 102 223.
*/

/*!
    \enum QSimCommand::Event
    This enum defines the types of events that the SIM expects to be reported.

    \value NoEvent No event type specified.
    \value IdleScreen Report an event when the system is idle.
    \value UserActivity Report an event when there is user activity.
    \value Both Report for both system idle and user activity.
    \value Cancel Cancel event reporting.
*/

/*!
    \enum QSimCommand::BrowserLaunchMode
    This enum defines the launch mode to use when displaying web pages in the browser.

    \value IfNotAlreadyLaunched Launch browser, if not already launched.
    \value UseExisting Use the existing browser, but not if it is running
           a secured session.
    \value CloseExistingAndLaunch Close the existing browser session
           and launch a new browser session.
*/

/*!
    \enum QSimCommand::MenuPresentation
    This enum defines the type of presentation to use for SIM toolkit menus.

    \value AnyPresentation The user interface may use any presentation it wishes.
    \value DataValuesPresentation The user interface should present the menu as
           a choice of several data values.
    \value NavigationOptionsPresentation The user interface should present the menu
           as a choice of several navigation options.
*/

/*!
    \enum QSimCommand::Device
    This enum defines the source or destination device for a SIM command, according
    to 3GPP TS 11.14, section 12.7.
    
    \value Keypad Keypad device
    \value Display Display device
    \value Earpiece Earpiece device
    \value CardReader0 Additional card reader 0
    \value CardReader1 Additional card reader 1
    \value CardReader2 Additional card reader 2
    \value CardReader3 Additional card reader 3
    \value CardReader4 Additional card reader 4
    \value CardReader5 Additional card reader 5
    \value CardReader6 Additional card reader 6
    \value CardReader7 Additional card reader 7
    \value Channel1 Data channel 1
    \value Channel2 Data channel 2
    \value Channel3 Data channel 3
    \value Channel4 Data channel 4
    \value Channel5 Data channel 5
    \value Channel6 Data channel 6
    \value Channel7 Data channel 7
    \value SIM SIM device
    \value ME ME (mobile equipment) device
    \value Network Source or destination is the network
*/

/*!
    Construct a new SIM toolkit command object with default parameters.
*/
QSimCommand::QSimCommand()
{
    d = new QSimCommandPrivate();
}


/*!
    Construct a new SIM toolkit command object as a copy of \a value.
*/
QSimCommand::QSimCommand( const QSimCommand& value )
{
    d = value.d;
    d->ref.ref();
}


/*!
    Destruct a SIM toolkit command object.
*/
QSimCommand::~QSimCommand()
{
    if ( !d->ref.deref() )
        delete d;
}


/*!
    Returns the SIM command number.  The default value is 1.  The command number
    may be something other than 1 if more than one SIM command is being processed
    simultaneously, but this is a fairly rare occurence.  The main use for
    command numbers is to match commands and responses.

    Applies to: all commands

    \sa setCommandNumber()
*/
int QSimCommand::commandNumber() const
{
    return d->commandNumber;
}


/*!
    Sets the SIM command number to \a value.  The usual value for the command
    number is 1.  The command number may be set to something other than 1 if more
    than one SIM command is being processed simultaneously, but this is a fairly
    rare occurence.  The main use for command numbers is to match commands and responses.

    Applies to: all commands

    \sa commandNumber()
*/
void QSimCommand::setCommandNumber( int value )
{
    dwrite()->commandNumber = value;
}


/*!
    Returns the type of this command.  The default value is \c NoCommand.

    Applies to: all commands.

    \sa setType()
*/
QSimCommand::Type QSimCommand::type() const
{
    return d->type;
}


/*!
    Sets the type of this command to \a value.

    Applies to: all commands.

    \sa type()
*/
void QSimCommand::setType( QSimCommand::Type value )
{
    dwrite()->type = value;
}


/*!
    Returns the source device that originated the command.  The default value is \c SIM.

    Applies to: all commands.

    \sa setSourceDevice()
*/
QSimCommand::Device QSimCommand::sourceDevice() const
{
    return d->sourceDevice;
}


/*!
    Sets the source device that originated the command to \a value.  Usually this
    is \c SIM.

    Applies to: all commands.

    \sa sourceDevice()
*/
void QSimCommand::setSourceDevice( QSimCommand::Device value )
{
    dwrite()->sourceDevice = value;
}


/*!
    Returns the destination device for the command.  The default value is \c ME.

    Applies to: all commands.

    \sa setDestinationDevice()
*/
QSimCommand::Device QSimCommand::destinationDevice() const
{
    return d->destinationDevice;
}


/*!
    Sets the destination device for the command to \a value.

    Applies to: all commands.

    \sa destinationDevice()
*/
void QSimCommand::setDestinationDevice( QSimCommand::Device value )
{
    dwrite()->destinationDevice = value;
}


/*!
    Returns true if there is help available for this command; otherwise returns false.
    The help flag is the most significant bit of the qualifier() byte.

    Applies to: \c SetupMenu, \c GetInkey, \c GetInput, \c SelectItem.

    \sa setHasHelp()
*/
bool QSimCommand::hasHelp() const
{
    if ( d->type == QSimCommand::GetInkey ||
         d->type == QSimCommand::GetInput ||
         d->type == QSimCommand::SelectItem ||
         d->type == QSimCommand::SetupMenu )
        return d->qualifierBit( 0x80 );
    else
        return false;
}


/*!
    Sets the flag that determines if there is help available
    for this command to \a value.  The help flag is the most significant bit
    of the qualifier() byte.

    Applies to: \c SetupMenu, \c GetInkey, \c GetInput, \c SelectItem.

    \sa hasHelp()
*/
void QSimCommand::setHasHelp( bool value )
{
    if ( d->type == QSimCommand::GetInkey ||
         d->type == QSimCommand::GetInput ||
         d->type == QSimCommand::SelectItem ||
         d->type == QSimCommand::SetupMenu )
        dwrite()->setQualifierBit( 0x80, value );
}


/*!
    Returns the text to be displayed as either a message, a prompt, or an SMS.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c SendSS, \c SendSMS,
    \c SendUSSD, \c PlayTone, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage.

    \sa setText()
*/
QString QSimCommand::text() const
{
    return d->text;
}


/*!
    Sets the text to be displayed by this command to \a value.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c SendSS, \c SendSMS,
    \c SendUSSD, \c PlayTone, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage.

    \sa text()
*/
void QSimCommand::setText( const QString& value )
{
    dwrite()->text = value;
}


/*!
    Returns the text attributes to be applied to text() for formatting.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c SendSS, \c SendSMS,
    \c SendUSSD, \c PlayTone, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage.

    \sa setTextAttribute(), text()
    \since 4.4
*/
QByteArray QSimCommand::textAttribute() const
{
    return d->textAttribute;
}


/*!
    Sets the text attributes to be applied to text() for formatting to \a value.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c SendSS, \c SendSMS,
    \c SendUSSD, \c PlayTone, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage.

    \sa textAttribute(), text(), textHtml()
    \since 4.4
*/
void QSimCommand::setTextAttribute( const QByteArray& value )
{
    dwrite()->textAttribute = value;
}


/*!
    Returns text() formatted as HTML according to the attributes
    in textAttribute().

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c SendSS, \c SendSMS,
    \c SendUSSD, \c PlayTone, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage.

    \sa text(), textAttribute()
    \since 4.4
*/
QString QSimCommand::textHtml() const
{
    return textToHtml( d->text, d->textAttribute );
}


/*!
    Returns true if user feedback should be suppressed.  The default value is false.

    This option controls what should happen when text() is an empty string for
    the \c SendSS, \c SendSMS, \c SendUSSD, and \c PlayTone commands.  If
    suppressUserFeedback() returns true, then the command should be performed
    without any notification to the user.  If suppressUserFeedback() is false,
    then generic feedback should be provided to the user to indicate that a
    command is being performed.

    Applies to: \c SendSS, \c SendSMS, \c SendUSSD, \c PlayTone.

    \sa setSuppressUserFeedback(), text()
*/
bool QSimCommand::suppressUserFeedback() const
{
    return d->flag( SC_SuppressUserFeedback );
}


/*!
    Sets the user feedback suppression flag to \a value.

    This option controls what should happen when text() is an empty string for
    the \c SendSS, \c SendSMS, \c SendUSSD, and \c PlayTone commands.  If
    \a value is true, then the command should be performed without any notification
    to the user.  If \a value is false, then generic feedback should be provided to
    the user to indicate that a command is being performed.

    Applies to: \c SendSS, \c SendSMS, \c SendUSSD, \c PlayTone.

    \sa suppressUserFeedback(), text()
*/
void QSimCommand::setSuppressUserFeedback( bool value )
{
    dwrite()->setFlag( SC_SuppressUserFeedback, value );
}


/*!
    Returns the other text to be displayed as a message.  This is typically used
    with \c SetupCall commands to specify the text to be displayed during the call setup
    phase, when text() is used for the user confirmation phase.

    Applies to: \c SetupCall

    \sa setOtherText()
*/
QString QSimCommand::otherText() const
{
    return d->otherText;
}


/*!
    Sets the other text to be displayed to \a value.  This is typically used
    with \c SetupCall commands to specify the text to be displayed during the call setup
    phase, when text() is used for the user confirmation phase.

    Applies to: \c SetupCall

    \sa otherText()
*/
void QSimCommand::setOtherText( const QString& value )
{
    dwrite()->otherText = value;
}


/*!
    Returns the text attributes to use to format otherText().

    Applies to: \c SetupCall

    \sa setOtherTextAttribute(), otherText(), otherTextHtml()
    \since 4.4
*/
QByteArray QSimCommand::otherTextAttribute() const
{
    return d->otherTextAttribute;
}


/*!
    Sets the text attributes to use to format otherText() to \a value.

    Applies to: \c SetupCall

    \sa otherTextAttribute(), otherText(), otherTextHtml()
    \since 4.4
*/
void QSimCommand::setOtherTextAttribute( const QByteArray& value )
{
    dwrite()->otherTextAttribute = value;
}


/*!
    Returns otherText() formatted as HTML according to the attributes
    in otherTextAttribute().

    Applies to: \c SetupCall

    \sa otherText(), otherTextAttribute()
    \since 4.4
*/
QString QSimCommand::otherTextHtml() const
{
    return textToHtml( d->otherText, d->otherTextAttribute );
}


/*!
    Returns the default text for \c GetInput commands.

    Applies to: \c GetInput

    \sa setDefaultText()
*/
QString QSimCommand::defaultText() const
{
    return d->defaultText;
}


/*!
    Sets the default text for \c GetInput commands to \a value.

    Applies to: \c GetInput

    \sa defaultText()
*/
void QSimCommand::setDefaultText( const QString& value )
{
    dwrite()->defaultText = value;
}


/*!
    Returns true if a DisplayText command should attempt to display
    the text as high priority (true) or normal priority (false).
    The default value is false.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage.

    \sa setHighPriority()
*/
bool QSimCommand::highPriority() const
{
    if ( d->type == QSimCommand::DisplayText ||
         d->type == QSimCommand::DisplayMultimediaMessage)
        return d->qualifierBit( 0x01 );
    else
        return false;
}


/*!
    Sets the high priority text display flag to \a value.  If type() is not \c DisplayText
    or \c DisplayMultimediaMessage, then the request is ignored.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage.

    \sa highPriority()
*/
void QSimCommand::setHighPriority( bool value )
{
    if ( d->type == QSimCommand::DisplayText ||
         d->type == QSimCommand::DisplayMultimediaMessage)
        dwrite()->setQualifierBit( 0x01, value );
}


/*!
    Returns true if the text should be automatically cleared after
    a small delay (usually 3 seconds); otherwise returns false.  If this is false, then
    the client must call QSimToolkit::clearText() to move on to
    the next SIM application state.  The default value is true.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage.

    Note: in Qtopia 4.3 and newer, the default value is true.  If Qtopia 4.2 and older,
    the default value was false.  The change was due to the introduction of the
    qualifier() value.

    \sa setClearAfterDelay()
*/
bool QSimCommand::clearAfterDelay() const
{
    if ( d->type == QSimCommand::DisplayText ||
         d->type == QSimCommand::DisplayMultimediaMessage)
        return !d->qualifierBit( 0x80 );
    else
        return true;
}


/*!
    Sets the clear after delay flag for text display to \a value.  If type() is not
    \c DisplayText or \c DisplayMultimediaMessage, the request is ignored.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage.

    \sa clearAfterDelay()
*/
void QSimCommand::setClearAfterDelay( bool value )
{
    if ( d->type == QSimCommand::DisplayText ||
         d->type == QSimCommand::DisplayMultimediaMessage)
        dwrite()->setQualifierBit( 0x80, !value );
}


/*!
    Returns true if a \c DisplayText or \c DisplayMultimediaMessage command
    should result in an immediate response to the SIM without asking the
    user to confirm the text first; or false if the user should confirm,
    or the response should be sent after the usual delay if clearAfterDelay()
    is set.  The default value is false.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage

    \sa setImmediateResponse(), clearAfterDelay()
*/
bool QSimCommand::immediateResponse() const
{
    return d->flag( SC_ImmediateResponse );
}


/*!
    Sets the immediate response flag to \a value.  If \a value is true, then the
    \c DisplayText or \c DisplayMultimediaMessage command should result in an
    immediate response to the SIM without asking the user to confirm the text first.
    If \a value is false, the user should confirm, or the response should be sent
    after the usual delay if clearAfterDelay() is set.

    Applies to: \c DisplayText, \c DisplayMultimediaMessage

    \sa immediateResponse(), clearAfterDelay()
*/
void QSimCommand::setImmediateResponse( bool value )
{
    dwrite()->setFlag( SC_ImmediateResponse, value );
}


/*!
    Returns true if the \c GetInkey and \c GetInput commands should return UCS2
    input strings to the SIM; false if the commands should return strings in the
    default SMS alphabet instead.  The default value is false.

    This setting is ignored if either wantYesNo() returns true, and this setting
    always overrides packedInput().

    Applies to: \c GetInkey, \c GetInput

    \sa setUcs2Input(), wantYesNo(), packedInput()
*/
bool QSimCommand::ucs2Input() const
{
    if ( d->type == QSimCommand::GetInkey || d->type == QSimCommand::GetInput )
        return d->qualifierBit( 0x02 );
    else
        return false;
}


/*!
    Sets the UCS2 input flag to \a value.  If \a value is true, then the
    \c GetInkey and \c GetInput commands should send UCS2 input strings to the SIM.
    If \a value is false, then the commands should send strings in the
    default SMS alphabet instead.

    This setting is ignored if either wantYesNo() returns true, and this setting
    always overrides packedInput().

    \sa ucs2Input(), wantYesNo(), packedInput()
*/
void QSimCommand::setUcs2Input( bool value )
{
    if ( d->type == QSimCommand::GetInkey || d->type == QSimCommand::GetInput )
        dwrite()->setQualifierBit( 0x02, value );
}


/*!
    Returns true if the \c GetInput command should return packed
    7-bit input strings to the SIM; false if the commands should return strings in the
    unpacked 8-bit alphabet instead.  The default value is false.

    This setting is ignored if either wantYesNo() or ucs2Input() returns true.

    Applies to: \c GetInput

    \sa setPackedInput(), wantYesNo(), ucs2Input()
*/
bool QSimCommand::packedInput() const
{
    if ( d->type == QSimCommand::GetInput )
        return d->qualifierBit( 0x08 );
    else
        return false;
}


/*!
    Sets the packed input flag to \a value.  If \a value is true, then the
    \c GetInput command should send packed 7-bit input strings to the SIM.
    If \a value is false, then the commands should send strings in the
    unpacked 8-bit alphabet instead.

    This setting is ignored if either wantYesNo() or ucs2Input() returns true,
    of if type() is not \c GetInput.

    Applies to: \c GetInput

    \sa packedInput(), wantYesNo(), ucs2Input()
*/
void QSimCommand::setPackedInput( bool value )
{
    if ( d->type == QSimCommand::GetInput )
        dwrite()->setQualifierBit( 0x08, value );
}


/*!
    Returns true if \c GetInkey or \c GetInput wants input that consists
    only of digits (true), or any character combination (false).
    The allowable digits are 0-9, #, *, and +.  The default value is true.

    Applies to: \c GetInkey, \c GetInput.

    Note: in Qtopia 4.3 and newer, the default value is true.  If Qtopia 4.2 and older,
    the default value was false.  The change was due to the introduction of the
    qualifier() value.

    \sa setWantDigits()
*/
bool QSimCommand::wantDigits() const
{
    if ( d->type == QSimCommand::GetInkey || d->type == QSimCommand::GetInput )
        return !d->qualifierBit( 0x01 );
    else
        return true;
}


/*!
    Sets the flag that determines if \c GetInkey or \c GetInput wants
    input that consists only of digits (true), or any character
    combination (false), to \a value.

    Applies to: \c GetInkey, \c GetInput.

    \sa wantDigits()
*/
void QSimCommand::setWantDigits( bool value )
{
    if ( d->type == QSimCommand::GetInkey || d->type == QSimCommand::GetInput )
        dwrite()->setQualifierBit( 0x01, !value );
}


/*!
    Returns true if \c GetInkey wants input that consists of a \c Yes or
    \c No answer.  The default value is false.

    Applies to: \c GetInkey

    \sa setWantYesNo()
*/
bool QSimCommand::wantYesNo() const
{
    if ( d->type == QSimCommand::GetInkey )
        return d->qualifierBit( 0x04 );
    else
        return false;
}


/*!
    Sets the flag that determines if \c GetInkey wants input that
    consists of a \c Yes or \c No answer, to \a value.

    Applies to: \c GetInkey

    \sa wantYesNo()
*/
void QSimCommand::setWantYesNo( bool value )
{
    if ( d->type == QSimCommand::GetInkey )
        dwrite()->setQualifierBit( 0x04, value );
}


/*!
    Returns the minimum text length for input.  The default value is 0.

    Applies to: \c GetInput.

    \sa setMinimumLength()
*/
uint QSimCommand::minimumLength() const
{
    return d->minimumLength;
}


/*!
    Sets the minimum text length for input to \a value.

    Applies to: \c GetInput.

    \sa minimumLength()
*/
void QSimCommand::setMinimumLength( uint value )
{
    dwrite()->minimumLength = value;
}


/*!
    Returns the maximum text length for input.  The default value is 255, which
    indicates that the text is unlimited in length.

    Applies to: \c GetInput.

    \sa setMaximumLength()
*/
uint QSimCommand::maximumLength() const
{
    return d->maximumLength;
}


/*!
    Sets the maximum text length for input to \a value.

    Applies to: \c GetInput.

    \sa maximumLength()
*/
void QSimCommand::setMaximumLength( uint value )
{
    dwrite()->maximumLength = value;
}


/*!
    Returns true if input should be echoed; otherwise returns false.
    The default value is true.

    Applies to: \c GetInput.

    Note: in Qtopia 4.3 and newer, the default value is true.  If Qtopia 4.2 and older,
    the default value was false.  The change was due to the introduction of the
    qualifier() value.

    \sa setEcho()
*/
bool QSimCommand::echo() const
{
    if ( d->type == QSimCommand::GetInput )
        return !d->qualifierBit( 0x04 );
    else
        return true;
}


/*!
    Sets the flag that determines if input should be echoed to \a value.

    Applies to: \c GetInput.

    \sa echo()
*/
void QSimCommand::setEcho( bool value )
{
    if ( d->type == QSimCommand::GetInput )
        dwrite()->setQualifierBit( 0x04, !value );
}


/*!
    Returns the disposition of other calls when a call setup occurs.
    The default value is \c IfNoOtherCalls.

    Applies to: \c SetupCall.

    \sa setDisposition()
*/
QSimCommand::Disposition QSimCommand::disposition() const
{
    if ( d->type == QSimCommand::SetupCall )
        return (QSimCommand::Disposition)(d->qualifier >> 1);
    else
        return IfNoOtherCalls;
}


/*!
    Sets the disposition of other calls when a call setup occurs to \a value.

    Applies to: \c SetupCall.

    \sa disposition()
*/
void QSimCommand::setDisposition( QSimCommand::Disposition value )
{
    if ( d->type == QSimCommand::SetupCall )
        dwrite()->qualifier = (((int)value) << 1) | (d->qualifier & 0x01);
}


/*!
    Returns true if with-redial modifier is set for call setup disposition();
    otherwise returns false.  The default value is false.

    Applies to: \c SetupCall.

    \sa setWithRedial()
*/
bool QSimCommand::withRedial() const
{
    if ( d->type == QSimCommand::SetupCall )
        return d->qualifierBit( 0x01 );
    else
        return false;
}

/*!
    Sets the with-redial modifier for call setup disposition to \a value.

    Applies to: \c SetupCall.

    \sa withRedial()
*/
void QSimCommand::setWithRedial( bool value )
{
    if ( d->type == QSimCommand::SetupCall )
        dwrite()->setQualifierBit( 0x01, value );
}

/*!
    Returns the phone number for \c SetupCall, address for \c SendSMS, the
    supplementary service string for \c SendSS, the unstructured supplementary
    service string for \c SendUSSD, or the sequence of DTMF digits for \c SendDTMF.

    Applies to: \c SetupCall, \c SendSMS, \c SendSS, \c SendUSSD, \c SendDTMF.

    \sa setNumber()
*/
QString QSimCommand::number() const
{
    return d->number;
}


/*!
    Sets the phone number for \c SetupCall, address for \c SendSMS, the
    supplementary service string for \c SendSS, unstructured supplementary service
    string for \c SendUSSD, to \a value, or the sequence of DTMF digits for \c SendDTMF.

    Applies to: \c SetupCall, \c SendSMS, \c SendSS, \c SendUSSD, \c SendDTMF.

    \sa number()
*/
void QSimCommand::setNumber( const QString& value )
{
    dwrite()->number = value;
}


/*!
    Returns the sub-address (e.g. extension) for a call setup.

    Applies to: \c SetupCall.

    \sa setSubAddress()
*/
QString QSimCommand::subAddress() const
{
    return d->subAddress;
}


/*!
    Sets the sub-address (e.g. extension) for a call setup to \a value.

    Applies to: \c SetupCall.

    \sa subAddress()
*/
void QSimCommand::setSubAddress( const QString& value )
{
    dwrite()->subAddress = value;
}


/*!
    Returns the class of call to be setup (Voice, Data, or Fax).
    The default value is \c Voice.

    Applies to: \c SetupCall.

    \sa setCallClass()
*/
QSimCommand::CallClass QSimCommand::callClass() const
{
    return d->callClass;
}


/*!
    Sets the class of call to be setup (Voice, Data, or Fax) to \a value.

    Applies to: \c SetupCall.

    \sa callClass()
*/
void QSimCommand::setCallClass( QSimCommand::CallClass value )
{
    dwrite()->callClass = value;
}


/*!
    Returns the tone to be played by a \c PlayTone command.  The default
    value is \c ToneNone.

    Applies to: \c PlayTone

    \sa setTone(), duration()
*/
QSimCommand::Tone QSimCommand::tone() const
{
    return d->tone;
}


/*!
    Sets the tone to be played by a \c PlayTone command to \a value.

    Applies to: \c PlayTone

    \sa tone(), duration()
*/
void QSimCommand::setTone( QSimCommand::Tone value )
{
    dwrite()->tone = value;
}


/*!
    Returns the number of milliseconds to play a tone.  The default value is zero,
    indicating that the default duration for the tone should be used.

    Applies to: \c PlayTone.

    Note: this function is obsoleted by duration(), and will return the same value
    as duration().

    \sa duration(), setToneTime(), tone()
*/
uint QSimCommand::toneTime() const
{
    return d->duration;
}


/*!
    Sets the number of milliseconds to play a tone to \a value.

    Applies to: \c PlayTone.

    Note: this function is obsoleted by setDuration().

    \sa setDuration(), toneTime(), tone()
*/
void QSimCommand::setToneTime( uint value )
{
    dwrite()->duration = value;
}


/*!
    Returns the number of milliseconds to play a tone or the poll interval.
    The default value is zero, indicating that the default duration should be used.

    Applies to: \c PlayTone, \c PollInterval.

    \sa setDuration(), tone()
*/
uint QSimCommand::duration() const
{
    return d->duration;
}

/*!
    Sets the duration of a poll interval or to play a tone to \a value milliseconds.

    Applies to: \c PlayTone, \c PollInterval.

    \sa duration(), tone()
*/
void QSimCommand::setDuration( uint value )
{
    dwrite()->duration = value;
}

/*!
    Returns true if icons for menu items should be displayed on the device's soft keys
    if the number of icons is less than or equal to the number of soft keys.  If there are
    more icons than soft keys, then regular menu selection should be used.  Returns false
    for regular menu selection.  The default value is false.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa setSoftKeysPreferred(), menuPresentation()
*/
bool QSimCommand::softKeysPreferred() const
{
    if ( d->type == QSimCommand::SetupMenu )
        return d->qualifierBit( 0x01 );
    else if ( d->type == QSimCommand::SelectItem )
        return d->qualifierBit( 0x04 );
    else
        return false;
}

/*!
    Sets the soft key preferred option to \a value.  If \a value is true then the
    icons for menu items should be displayed on the device's soft keys if the number
    of icons is less than or equal to the number of soft keys.  If there are more
    icons than soft keys, then regular menu selection should be used.  If \a value
    is false, then regular menu selection is always used.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa softKeysPreferred(), menuPresentation()
*/
void QSimCommand::setSoftKeysPreferred( bool value )
{
    if ( d->type == QSimCommand::SetupMenu )
        dwrite()->setQualifierBit( 0x01, value );
    else if ( d->type == QSimCommand::SelectItem )
        dwrite()->setQualifierBit( 0x04, value );
}

/*!
    Returns the menu presentation type to use for \c SelectItem commands.
    The default value is \c AnyPresentation.

    Applies to: \c SelectItem.

    \sa setMenuPresentation(), softKeysPreferred()
*/
QSimCommand::MenuPresentation QSimCommand::menuPresentation() const
{
    if ( d->type == QSimCommand::SelectItem && d->qualifierBit( 0x01 ) != 0 ) {
        if ( d->qualifierBit( 0x02 ) != 0 )
            return NavigationOptionsPresentation;
        else
            return DataValuesPresentation;
    } else {
        return AnyPresentation;
    }
}

/*!
    Sets the menu presentation type to use for \c SelectItem commands
    to \a value.

    Applies to: \c SelectItem.

    \sa menuPresentation(), softKeysPreferred()
*/
void QSimCommand::setMenuPresentation( QSimCommand::MenuPresentation value )
{
    if ( d->type == QSimCommand::SelectItem ) {
        if ( value == NavigationOptionsPresentation ) {
            dwrite()->setQualifierBit( 0x01, true );
            dwrite()->setQualifierBit( 0x02, true );
        } else if ( value == DataValuesPresentation ) {
            dwrite()->setQualifierBit( 0x01, true );
            dwrite()->setQualifierBit( 0x02, false );
        } else {
            dwrite()->setQualifierBit( 0x01, false );
            dwrite()->setQualifierBit( 0x02, false );
        }
    }
}

/*!
    Returns the title to be displayed on a menu.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa setTitle()
*/
QString QSimCommand::title() const
{
    return d->title;
}


/*!
    Sets the title to be displayed on a menu to \a value.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa title()
*/
void QSimCommand::setTitle( const QString& value )
{
    dwrite()->title = value;
}


/*!
    Returns the text attribute to use to format title().

    Applies to: \c SetupMenu, \c SelectItem.

    \sa setTitleAttribute(), title(), titleHtml()
    \since 4.4
*/
QByteArray QSimCommand::titleAttribute() const
{
    return d->titleAttribute;
}


/*!
    Sets the text attribute to use to format title() to \a value.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa titleAttribute(), title(), titleHtml()
    \since 4.4
*/
void QSimCommand::setTitleAttribute( const QByteArray& value )
{
    dwrite()->titleAttribute = value;
}


/*!
    Returns title() formatted as HTML according to the attributes
    in titleAttribute().

    Applies to: \c SetupMenu, \c SelectItem.

    \sa title(), titleAttribute()
    \since 4.4
*/
QString QSimCommand::titleHtml() const
{
    return textToHtml( d->title, d->titleAttribute );
}


/*!
    Returns the index of the default item in the menu, or zero if no default.

    Applies to: \c SelectItem.

    \sa setDefaultItem()
*/
uint QSimCommand::defaultItem() const
{
    return d->defaultItem;
}


/*!
    Sets the index of the default item in the menu, or zero if no default,
    to \a value.

    Applies to: \c SelectItem.

    \sa defaultItem()
*/
void QSimCommand::setDefaultItem( uint value )
{
    dwrite()->defaultItem = value;
}


/*!
    Returns the list of menu items in the menu.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa setMenuItems()
*/
QList<QSimMenuItem> QSimCommand::menuItems() const
{
    return d->menuItems;
}


/*!
    Sets the list of menu items in the menu to \a value.

    Applies to: \c SetupMenu, \c SelectItem.

    \sa menuItems()
*/
void QSimCommand::setMenuItems( const QList<QSimMenuItem>& value )
{
    dwrite()->menuItems = value;
}


/*!
    Returns the type of refresh to that was performed by the SIM.
    The default value is InitAndFullFileChange.

    Applies to: \c Refresh.

    \sa setRefreshType()
*/
QSimCommand::RefreshType QSimCommand::refreshType() const
{
    if ( d->type == QSimCommand::Refresh )
        return (QSimCommand::RefreshType)(d->qualifier);
    else
        return InitAndFullFileChange;
}


/*!
    Sets the type of refresh to that was performed by the SIM to \a value.
    The request is ignored if type() is not \c Refresh.

    Applies to: \c Refresh.

    \sa refreshType()
*/
void QSimCommand::setRefreshType( QSimCommand::RefreshType value )
{
    if ( d->type == QSimCommand::Refresh )
        setQualifier( (int)value );
}


/*!
    Returns the events to be reported.

    Applies to: \c SetupEventList.

    Note: this function can only return information about \c IdleScreen and \c UserActivity
    events.  To access information about all SIM toolkit events in the command,
    use extensionField() with a tag value of 0x99.

    \sa setEvents()
*/
QSimCommand::Event QSimCommand::events() const
{
    if ( d->type != QSimCommand::SetupEventList )
        return NoEvent;
    if ( d->extensionData.isEmpty() )
        return NoEvent;
    QByteArray evdata = extensionField( 0x99 );
    if ( evdata.isEmpty() ) {
        return Cancel;      // Field is present, but empty, which means "Cancel".
    } else if ( evdata.contains( (char)0x05 ) ) {
        if ( evdata.contains( (char)0x04 ) )
            return Both;
        else
            return IdleScreen;
    } else if ( evdata.contains( (char)0x04 ) ) {
        return UserActivity;
    } else {
        return NoEvent;
    }
}


/*!
    Sets the events to be reported to \a value.

    Applies to: \c SetupEventList.

    Note: this function can only set information about \c IdleScreen and \c UserActivity
    events.  To set information about other SIM toolkit events, use setExtensionField()
    with a tag value of 0x99.

    \sa events()
*/
void QSimCommand::setEvents( QSimCommand::Event value )
{
    if ( d->type == QSimCommand::SetupEventList ) {
        dwrite()->extensionData = QByteArray();
        switch ( value ) {

            case NoEvent:       break;

            case IdleScreen:
            {
                d->extensionData += (char)0x99;
                d->extensionData += (char)0x01;
                d->extensionData += (char)0x05;
            }
            break;

            case UserActivity:
            {
                d->extensionData += (char)0x99;
                d->extensionData += (char)0x01;
                d->extensionData += (char)0x04;
            }
            break;

            case Both:
            {
                d->extensionData += (char)0x99;
                d->extensionData += (char)0x02;
                d->extensionData += (char)0x04;
                d->extensionData += (char)0x05;
            }
            break;

            case Cancel:
            {
                d->extensionData += (char)0x99;
                d->extensionData += (char)0x00;
            }
            break;
        }
    }
}


/*!
    Returns the browser launch mode.  The default value is IfNotAlreadyLaunched.

    Applies to: \c LaunchBrowser.

    \sa setBrowserLaunchMode()
*/
QSimCommand::BrowserLaunchMode QSimCommand::browserLaunchMode() const
{
    if ( d->type == QSimCommand::LaunchBrowser )
        return (QSimCommand::BrowserLaunchMode)(d->qualifier);
    else
        return IfNotAlreadyLaunched;
}

/*!
    Sets the browser launch mode to \a value.

    Applies to: \c LaunchBrowser.

    \sa browserLaunchMode()
*/
void QSimCommand::setBrowserLaunchMode( QSimCommand::BrowserLaunchMode value )
{
    if ( d->type == QSimCommand::LaunchBrowser )
        setQualifier( (int)value );
}

/*!
    Returns the URL to launch in the browser.

    Applies to: \c LaunchBrowser.

    \sa setUrl()
*/
QString QSimCommand::url() const
{
    return d->url;
}

/*!
    Sets the URL to launch in the browser to \a value.

    Applies to: \c LaunchBrowser.

    \sa url()
*/
void QSimCommand::setUrl( const QString& value )
{
    dwrite()->url = value;
}

/*!
    Returns the icon identifier associated with this command.
    Returns zero if there is no icon.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c PlayTone, \c SelectItem,
    \c SendSMS, \c SendSS, \c SetupCall, \c SetupMenu, \c LaunchBrowser,
    \c SetupIdleModeText, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage

    \sa setIconId()
*/
uint QSimCommand::iconId() const
{
    return d->iconId;
}

/*!
    Sets the icon identifier associated with this menu item to \a value.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c PlayTone, \c SelectItem,
    \c SendSMS, \c SendSS, \c SetupCall, \c SetupMenu, \c LaunchBrowser,
    \c SetupIdleModeText, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage

    \sa iconId()
*/
void QSimCommand::setIconId( uint value )
{
    dwrite()->iconId = value;
}

/*!
    Returns true if the icon specified by iconId() is self-explanatory
    without the display of text().  If this function returns false,
    then text() should be displayed next to the icon.  If iconId()
    returns zero, then iconSelfExplanatory() should be ignored.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c PlayTone, \c SelectItem,
    \c SendSMS, \c SendSS, \c SetupCall, \c SetupMenu, \c LaunchBrowser,
    \c SetupIdleModeText, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage

    \sa setIconSelfExplanatory()
*/
bool QSimCommand::iconSelfExplanatory() const
{
    return d->flag( SC_IconSelfExplanatory );
}

/*!
    Sets the self-explanatory flag to \a value.

    Applies to: \c DisplayText, \c GetInkey, \c GetInput, \c PlayTone, \c SelectItem,
    \c SendSMS, \c SendSS, \c SetupCall, \c SetupMenu, \c LaunchBrowser,
    \c SetupIdleModeText, \c ServiceSearch, \c GetServiceInformation,
    \c RetrieveMultimediaMessage, \c SubmitMultimediaMessage

    \sa iconSelfExplanatory()
*/
void QSimCommand::setIconSelfExplanatory( bool value )
{
    dwrite()->setFlag( SC_IconSelfExplanatory, value );
}

/*!
    Returns the other icon identifier associated with this command.
    Returns zero if there is no icon.  This is typically used
    with \c SetupCall commands to specify the icon to be displayed during the call setup
    phase, when iconId() is used for the user confirmation phase.

    Applies to: \c SetupCall

    \sa setOtherIconId()
*/
uint QSimCommand::otherIconId() const
{
    return d->otherIconId;
}


/*!
    Sets the other icon identifier associated with this menu item to \a value.
    This is typically used with \c SetupCall commands to specify the icon to be
    displayed during the call setup phase, when iconId() is used for the user
    confirmation phase.

    Applies to: \c SetupCall

    \sa otherIconId()
*/
void QSimCommand::setOtherIconId( uint value )
{
    dwrite()->otherIconId = value;
}


/*!
    Returns true if the icon specified by otherIconId() is self-explanatory
    without the display of otherText().  If this function returns false,
    then otherText() should be displayed next to the icon.  If otherIconId()
    returns zero, then otherIconSelfExplanatory() should be ignored.

    Applies to: \c SetupCall

    \sa setOtherIconSelfExplanatory()
*/
bool QSimCommand::otherIconSelfExplanatory() const
{
    return d->flag( SC_IconSelfExplanatory2 );
}


/*!
    Sets the self-explanatory flag for the icon specified by otherIconId to \a value.

    Applies to: \c SetupCall

    \sa otherIconSelfExplanatory()
*/
void QSimCommand::setOtherIconSelfExplanatory( bool value )
{
    dwrite()->setFlag( SC_IconSelfExplanatory2, value );
}


/*!
    Returns true if SMS packing should be performed when sending an SMS message
    for the \c SendSMS command; or false if packing is not required.  The default
    value is false.

    Many modems or modem vendor plugins will consume this value and remove it
    from the SIM command before it reaches the client application layer.  Client
    applications should not rely upon this value being present.

    Applies to: \c SendSMS

    \sa setSmsPacking()
*/
bool QSimCommand::smsPacking() const
{
    if ( d->type == QSimCommand::SendSMS )
        return d->qualifierBit( 0x01 );
    else
        return false;
}

/*!
    Sets the SMS packing flag to \a value.  If \a value is true, then SMS packing
    should be performed when sending an SMS message for the \c SendSMS command.
    If \a value is false, then no packing is required.

    Many modems or modem vendor plugins will consume this value and remove it
    from the SIM command before it reaches the client application layer.  Client
    applications should not rely upon this value being present.

    Applies to: \c SendSMS

    \sa smsPacking()
*/
void QSimCommand::setSmsPacking( bool value )
{
    if ( d->type == QSimCommand::SendSMS )
        dwrite()->setQualifierBit( 0x01, value );
}

/*!
    Copy the QSimCommand object \a value.
*/
QSimCommand& QSimCommand::operator=( const QSimCommand & value )
{
    if ( d == value.d )
        return *this;

    if ( !d->ref.deref() )
        delete d;

    d = value.d;
    d->ref.ref();

    return *this;
}

// Read BER tag and length information from a QByteArray.
// We also need to export this to qsimterminalresponse.cpp and qsimenvelope.cpp.
void _qtopiaphone_readBer( const QByteArray& binary, uint& posn, uint& tag, uint& length )
{
    if ( posn < (uint)binary.size() ) {
        tag = (uint)(binary[posn] & 0xFF);
        ++posn;
    } else {
        tag = (uint)(-1);
    }
    if ( posn < (uint)binary.size() ) {
        length = (uint)(binary[posn] & 0xFF);
        ++posn;
        if ( length == 0x81 ) {
            // Two-byte length value.
            if ( posn < (uint)binary.size() ) {
                length = (uint)(binary[posn] & 0xFF);
                ++posn;
            } else {
                length = 0;
            }
        } else if ( length == 0x82 ) {
            // Three-byte length value.
            if ( ( posn + 1 ) < (uint)binary.size() ) {
                length = (((uint)(binary[posn] & 0xFF)) << 8) |
                         (uint)(binary[posn + 1] & 0xFF);
                posn += 2;
            } else {
                length = 0;
                posn = (uint)binary.size();
            }
        }
    } else {
        length = 0;
    }
}
#define readBer _qtopiaphone_readBer

// Decode an EFADN string from a BER field.  GSM 11.11, section 10.5.1.
// Exported to qsimcontrolevent.cpp.
QString _qtopiaphone_decodeEFADN( const QByteArray& binary, uint posn, uint length )
{
    uint num, page, ch;
    QString temp;
    QTextCodec *codec = QAtUtils::codec( "gsm" );
    if ( length == 0 )
        return QString("");
    else if ( binary[posn] == (char)0x80 ) {
        // UCS-2 coding.
        ++posn;
        --length;
        while ( length >= 2 ) {
            ch = (((uint)((binary[posn]) & 0xFF)) << 8) |
                  ((uint)(binary[posn + 1] & 0xFF));
            if ( ch != 0xFFFF )
                temp += (QChar)ch;
            posn += 2;
            length -= 2;
        }
        return temp;
    } else if ( binary[posn] == (char)0x81 ) {
        // 8-bit half page index coding.
        if ( length < 3 )
            return QString("");
        num = ((uint)(binary[posn + 1] & 0xFF));
        page = ((uint)(binary[posn + 2] & 0xFF)) << 7;
        posn += 3;
        length -= 3;
    } else if ( binary[posn] == (char)0x82 ) {
        // 16-bit half page index coding.
        if ( length < 4 )
            return QString("");
        num = ((uint)(binary[posn + 1] & 0xFF));
        page = (((uint)(binary[posn + 2] & 0xFF)) << 8) |
                ((uint)(binary[posn + 3] & 0xFF));
        posn += 4;
        length -= 4;
    } else {
        // 7-bit GSM default alphabet coding.
        while ( length > 0 && binary[posn + length - 1] == (char)0xFF ) {
            // Strip 0xFF bytes from the end of the string first.
            --length;
        }
        return codec->toUnicode( binary.data() + posn, (int)length );
    }
    while ( num > 0 && length > 0 ) {
        ch = ((uint)(binary[posn] & 0xFF));
        if ( ch < 0x80 ) {
            temp += QGsmCodec::singleToUnicode( (char)ch );
        } else {
            temp += (QChar)( page + (ch & 0x7F) );
        }
        ++posn;
        --length;
        --num;
    }
    return temp;
}
#define decodeEFADN _qtopiaphone_decodeEFADN

// Decode an EFADN number from a BER field.  GSM 11.11, section 10.5.1.
static QString decodeEFADNNumber( const QByteArray& binary,
                                  uint posn, uint length )
{
    static char const digits[] = "0123456789*#pDE";
    QString temp;
    uint ch;
    while ( length > 0 ) {
        ch = ((uint)(binary[posn] & 0xFF));
        if ( ( ch & 0x0F ) == 0x0F )
            break;
        temp += (QChar)( digits[ch & 0x0F] );
        if ( ( ch & 0xF0 ) == 0xF0 )
            break;
        temp += (QChar)( digits[(ch >> 4) & 0x0F] );
        ++posn;
        --length;
    }
    return temp;
}

// Write an EFADN number to a BER field.
static void writeEFADNNumber( QByteArray& data, const QString& number, int tag = 0x86,
                              int localTag = 129 )
{
    QString num = QAtUtils::stripNumber( number );
    if ( tag == 0xAC ) {
        // DTMF strings don't have a type prefix.
        data += (char)tag;
        data += (char)((num.length() + 1) / 2);
    } else if ( num.startsWith( QChar('+') ) ) {
        num = num.mid(1);
        data += (char)tag;
        data += (char)(1 + ((num.length() + 1) / 2));
        data += (char)145;
    } else {
        data += (char)tag;
        data += (char)(1 + ((num.length() + 1) / 2));
        data += (char)localTag;
    }
    int byte = 0;
    int nibble = 0;
    for ( int index = 0; index < num.length(); ++index ) {
        int ch = num[index].unicode();
        switch ( ch ) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':   ch -= '0'; break;

            case '*':           ch = 10; break;
            case '#':           ch = 11; break;
            case 'p': case 'P': case ',':
                                ch = 12; break;
            case 'D': case 'd': ch = 13; break;
            case 'E': case 'e': ch = 14; break;

            default:            ch = 10; break;
        }
        if ( nibble ) {
            data += (char)(byte | (ch << 4));
            nibble = 0;
        } else {
            byte = ch;
            nibble = 1;
        }
    }
    if ( nibble ) {
        data += (char)(byte | 0xF0);
    }
}

// Decode a coded string, GSM 11.14, section 12.15.
QString _qtopiaphone_decodeCodedString( const QByteArray& binary, uint posn, uint length )
{
    QString temp;
    uint ch, byte, bitCount;
    if ( length == 0 )
        return QString("");
    ch = (uint)(binary[posn++] & 0x0C);
    --length;
    if ( ch == QSMS_DefaultAlphabet ) {
        ch = 0;
        bitCount = 0;
        while ( length > 0 ) {
            byte = binary[posn++];
            --length;
            for ( int i = 0; i < 8; i++ ) {
                // test the bit and shift it down again, as i doesn't mark
                // where we are in the current char, but bitCount does
                ch |= ( ( byte & (1 << i) ) >> i) << bitCount;
                bitCount++;
                if ( bitCount == 7 ) {
                    bitCount = 0;
                    temp += QGsmCodec::singleToUnicode( (char)ch );
                    ch = 0;
                }
            }
        }
        if ( bitCount == 0 && temp.length() > 0 &&
             temp[temp.length() - 1] == (char)0x0D ) {
            // CR used as a padding character in the last byte - strip it.
            temp = temp.left( temp.length() - 1 );
        }
        return temp;
    } else if ( ch == QSMS_UCS2Alphabet ) {
        while ( length >= 2 ) {
            ch = (((uint)((binary[posn]) & 0xFF)) << 8) |
                  ((uint)(binary[posn + 1] & 0xFF));
            temp += (QChar)ch;
            posn += 2;
            length -= 2;
        }
        return temp;
    } else {
        return decodeEFADN( binary, posn, length );
    }
}
#define decodeCodedString _qtopiaphone_decodeCodedString

// Decode a subaddress, GSM 11.14, section 12.3.  The actual encoding
// is specified in GSM 24.008, Annex E.
static QString decodeSubAddress( const QByteArray& pdu, uint posn, uint length )
{
    // Verify that this looks like a BCD-encoded IA5 NSAP subaddress.
    if ( length < 2 || pdu[posn] != (char)0x80 || pdu[posn + 1] != (char)0x50 )
        return QString();
    QString result;
    posn += 2;
    length -= 2;
    while ( length > 0 ) {
        int bcd = (pdu[posn++] & 0xFF);
        result += QChar( (bcd >> 4) * 10 + (bcd & 0x0F) + 32 );
        --length;
    }
    return result;
}

/*!
    Decode a SIM command from its raw binary \a pdu form, as according
    to 3GPP TS 11.14.  The binary data is assumed to be in BER form,
    starting with the command tag.  If the data starts with a
    "Proactive SIM" BER command wrapper, it will be skipped.
    Returns the decoded SIM command.

    \sa toPdu()
*/
QSimCommand QSimCommand::fromPdu( const QByteArray& pdu )
{
    QSimCommand sc;
    uint posn = 0;
    uint newPosn;
    uint tag, length;
    QList<QSimMenuItem> items;
    bool seenText = false;
    bool seenTextAttribute = false;
    bool seenIcon = false;

    // Read the first BER blob, which should be the command details.
    readBer( pdu, posn, tag, length );
    if ( tag == 0xD0 ) {
        // There appears to be a "Proactive SIM" wrapper on the front
        // of the command details.  Skip over it.
        readBer( pdu, posn, tag, length );
    }
    if ( (tag & 0x7F) != 0x01 ) {
        // This doesn't appear to be a SIM command at all.
        return sc;
    }

    // Process the contents of the SIM command, tag by tag.
    for (;;) {
        if ( ( posn + length ) > (uint)pdu.size() ) {
            break;
        }
        newPosn = posn + length;
        switch ( tag & 0x7F ) {

            case 0x01:
            {
                // Main command details blob, GSM 11.14, section 12.6.
                if ( length < 3 )
                    break;
                sc.setCommandNumber( pdu[posn] & 0xFF );
                sc.setType( (QSimCommand::Type)(pdu[posn + 1] & 0xFF) );
                sc.setQualifier( pdu[posn + 2] & 0xFF );
            }
            break;

            case 0x02:
            {
                // Device identities, GSM 11.14, section 12.7.
                if ( length >= 2 ) {
                    sc.setSourceDevice( (QSimCommand::Device)( pdu[posn] & 0xFF ) );
                    sc.setDestinationDevice( (QSimCommand::Device)( pdu[posn + 1] & 0xFF ) );
                }
            }
            break;

            case 0x04:
            {
                // Duration for a PlayTone or PollInterval command, GSM 11.14, section 12.8.
                if ( sc.type() == QSimCommand::OpenChannel ) {
                    // This is actually part of the network setup parameters,
                    // so add it to the extension data.
                    sc.addExtensionField( tag, pdu.mid( posn, length ) );
                    break;
                }
                if ( length < 2 )
                    break;
                uint multiplier = 60000;            // Minutes.
                if ( pdu[posn] == 0x01 )         // Seconds.
                    multiplier = 1000;
                else if ( pdu[posn] == 0x02 )    // Tenths of a second.
                    multiplier = 100;
                sc.setDuration( multiplier * ( pdu[posn + 1] & 0xFF ) );
            }
            break;

            case 0x05:
            {
                // Title to display with a menu, or alpha identifier for other commands.
                // GSM 11.14, section 12.2.
                if ( sc.type() == SetupMenu || sc.type() == SelectItem ) {
                    sc.setTitle( decodeEFADN( pdu, posn, length ) );
                } else if ( seenText ) {
                    sc.setOtherText( decodeEFADN( pdu, posn, length ) );
                } else {
                    sc.setText( decodeEFADN( pdu, posn, length ) );
                    seenText = true;
                    if ( length == 0 )
                        sc.setSuppressUserFeedback( true );
                }
            }
            break;

            case 0x06:
            case 0x09:
            {
                // Address for SetupCall and SendSMS, GSM 11.14, section 12.1.
                // SS string for SendSS command, GSM 11.14, section 12.14.
                if ( length < 1 )
                    break;
                if ( pdu[posn] == (char)145 ) {
                    sc.setNumber
                        ( "+" + decodeEFADNNumber
                                    ( pdu, posn + 1, length - 1 ) );
                } else {
                    sc.setNumber( decodeEFADNNumber
                                    ( pdu, posn + 1, length - 1 ) );
                }
            }
            break;

            case 0x08:
            {
                // Sub-address for SetupCall, GSM 11.14, section 12.3.
                sc.setSubAddress( decodeSubAddress( pdu, posn, length ) );
            }
            break;

            case 0x0D:
            {
                // Text string for command, GSM 11.14, section 12.15.
                if ( sc.type() == QSimCommand::LaunchBrowser ||
                     sc.type() == QSimCommand::OpenChannel ) {
                    // This is actually part of the network setup parameters,
                    // so add it to the extension data.
                    sc.addExtensionField( tag, pdu.mid( posn, length ) );
                } else {
                    sc.setText( decodeCodedString( pdu, posn, length ) );
                }
            }
            break;

            case 0x0A:
            {
                // USSD string, GSM 11.14, section 12.17.
                sc.setNumber( decodeCodedString( pdu, posn, length ) );
            }
            break;

            case 0x11:
            {
                // Response length, GSM 11.14, section 12.11.
                if ( length >= 2 ) {
                    sc.setMinimumLength( pdu[posn] & 0xFF );
                    sc.setMaximumLength( pdu[posn + 1] & 0xFF );
                }
            }
            break;

            case 0x17:
            {
                // Default text string for GET INPUT command, GSM 11.14, section 12.23.
                sc.setDefaultText( decodeCodedString( pdu, posn, length ) );
            }
            break;

            case 0x18:
            {
                // Next action indicator list, GSM 11.14, section 12.24.
                int index = 0;
                while ( length > 0 && index < items.size() ) {
                    uint action = (uint)(pdu[posn++] & 0xFF);
                    items[index].setNextAction( action );
                    --length;
                    ++index;
                }
            }
            break;

            case 0x0E:
            {
                // Tone code for a PlayTone command, GSM 11.14, section 12.16.
                if ( length > 0 )
                    sc.setTone( (QSimCommand::Tone)(pdu[posn] & 0xFF) );
            }
            break;

            case 0x0F:
            {
                // Menu item for command, GSM 11.14 section, 12.9.
                if ( !length )
                    break;      // Zero-length items indicate empty menu.
                QSimMenuItem item;
                if ( length > 0 ) {
                    item.setIdentifier( (uint)(pdu[posn] & 0xFF) );
                    ++posn;
                    --length;
                }
                item.setLabel( decodeEFADN( pdu, posn, length ) );
                item.setHasHelp( sc.hasHelp() );
                items.append( item );
            }
            break;

            case 0x10:
            {
                // Default menu item identifier, GSM 11.14, section 12.10.
                if ( length > 0 )
                    sc.setDefaultItem( (uint)(pdu[posn] & 0xFF) );
            }
            break;

            case 0x1E:
            {
                // Icon identifier, GSM 11.14, section 12.31.
                if ( length >= 2 ) {
                    if ( seenIcon ) {
                        sc.setOtherIconSelfExplanatory( (pdu[posn] & 0x01) == 0 );
                        sc.setOtherIconId( (uint)(pdu[posn + 1] & 0xFF) );
                    } else {
                        sc.setIconSelfExplanatory( (pdu[posn] & 0x01) == 0 );
                        sc.setIconId( (uint)(pdu[posn + 1] & 0xFF) );
                        seenIcon = true;
                    }
                }
            }
            break;

            case 0x1F:
            {
                // Item icon identifier list, GSM 11.14, section 12.32.
                int index = 0;
                bool selfExplanatory = true;
                if ( length > 0 ) {
                    if ( (pdu[posn] & 0x01) != 0 )
                        selfExplanatory = false;
                    ++posn;
                    --length;
                }
                while ( length > 0 && index < items.size() ) {
                    uint iconId = (uint)(pdu[posn++] & 0xFF);
                    items[index].setIconId( iconId );
                    items[index].setIconSelfExplanatory( selfExplanatory );
                    --length;
                    ++index;
                }
            }
            break;

            case 0x2B:
            {
                // Immediate response block.
                sc.setImmediateResponse(true);
            }
            break;

            case 0x2C:
            {
                // DTMF digits for SendDTMF command, GSM 11.14, section 12.44.
                sc.setNumber( decodeEFADNNumber( pdu, posn, length ) );
            }
            break;

            case 0x31:
            {
                // URL for LaunchBrowser command, GSM 11.14, section 12.48.
                sc.setUrl( decodeEFADN( pdu, posn, length ) );
            }
            break;

            case 0x50:
            {
                // Text attribute, ETSI TS 102 223, section 8.72.
                if ( sc.type() == SetupMenu || sc.type() == SelectItem ) {
                    sc.setTitleAttribute( pdu.mid( posn, length ) );
                } else if ( seenTextAttribute ) {
                    sc.setOtherTextAttribute( pdu.mid( posn, length ) );
                } else {
                    sc.setTextAttribute( pdu.mid( posn, length ) );
                    seenTextAttribute = true;
                }
            }
            break;

            case 0x51:
            {
                // Item text attribute list, ETSI TS 102 223, section 8.73.
                int index = 0;
                while ( length >= 4 && index < items.size() ) {
                    items[index].setLabelAttribute( pdu.mid( posn, 4 ) );
                    length -= 4;
                    posn += 4;
                    ++index;
                }
            }
            break;

            default:
            {
                // Don't know what this is, so add it as an extension field.
                sc.addExtensionField( tag, pdu.mid( posn, length ) );
            }
            break;
        }
        posn = newPosn;
        if ( posn >= (uint)pdu.size() ) {
            break;
        }
        readBer( pdu, posn, tag, length );
    }

    sc.setMenuItems( items );
    return sc;
}

// Write a BER length value.  Exported to qsimterminalresponse.cpp and qsimenvelope.cpp.
void _qtopiaphone_writeBerLength( QByteArray& binary, int length )
{
    int size = binary.size();
    if ( length < 128 ) {
        binary.resize( size + 1 );
        binary[size] = (char)length;
    } else if ( length < 256 ) {
        binary.resize( size + 2 );
        binary[size] = (char)0x81;
        binary[size + 1] = (char)length;
    } else {
        binary.resize( size + 3 );
        binary[size] = (char)0x82;
        binary[size + 1] = (char)( length >> 8 );
        binary[size + 2] = (char)length;
    }
}
#define writeBerLength _qtopiaphone_writeBerLength

// Write a string, encoded in the best possible data coding scheme.
// We also need to export this to qsimterminalresponse.cpp.
void _qtopiaphone_writeTextString( QByteArray& binary, const QString& str,
                                   QSimCommand::ToPduOptions options, int tag = 0x8D );
void _qtopiaphone_writeTextString( QByteArray& binary, const QString& str,
                                   QSimCommand::ToPduOptions options, int tag )
{
    if ( str.isEmpty() && ( options & QSimCommand::EncodeEmptyStrings ) == 0 ) {
        // Special form for zero-length strings.
        binary += (char)tag;
        binary += (char)0x00;
        return;
    }
    int schemeMask = ((tag & 0xFF00) >> 8);     // For USSD string output.
    if ( ( options & QSimCommand::UCS2Strings ) == 0 ) {
        QTextCodec *gsm = QAtUtils::codec( "gsm-noloss" );
        if ( gsm->canEncode( str ) ) {
            if ( ( options & QSimCommand::PackedStrings ) != 0 ) {
                // Use the packed 7-bit GSM alphabet to encode the string.
                QByteArray packed;
                int bitCount = 0;
                int bits = 0;
                for ( int u = 0; u < str.length(); u++ ) {
                    unsigned short c = QGsmCodec::twoByteFromUnicode( str[u].unicode() );
                    if ( c >= 256 ) {
                        // Encode a two-byte sequence.
                        for ( int i = 0; i < 7; i++ ) {
                            if ( bitCount == 8 ) {
                                bitCount = 0;
                                packed += (char)bits;
                                bits = 0;
                            }
                            if ( ((1 << (i+8)) & c) != 0 )
                                bits |= (1 << bitCount);
                            ++bitCount;
                        }
                    }
                    for ( int i = 0; i < 7; i++ ) {
                        if ( bitCount == 8 ) {
                            bitCount = 0;
                            packed += (char)bits;
                            bits = 0;
                        }
                        if ( ((1 << i) & c) != 0 )
                            bits |= (1 << bitCount);
                        ++bitCount;
                    }
                }
                if ( bitCount != 0 ) {
                    packed += (char)bits;
                }
                binary += (char)tag;
                writeBerLength( binary, packed.length() + 1 );
                binary += (char)(QSMS_DefaultAlphabet | schemeMask);
                binary += packed;
            } else {
                // Use the unpacked 8-bit GSM alphabet to encode the string.
                QByteArray unpacked = gsm->fromUnicode( str );
                binary += (char)tag;
                writeBerLength( binary, unpacked.length() + 1 );
                binary += (char)(QSMS_8BitAlphabet | schemeMask);
                binary += unpacked;
            }
            return;
        }
    }

    // If we get here, we must use UCS-2 to encode the text string.
    QByteArray ucs2;
    const ushort *data = str.utf16();
    int len = str.length();
    while ( len-- > 0 ) {
        ushort ch = *data++;
        ucs2 += (char)(ch >> 8);
        ucs2 += (char)ch;
    }
    binary += (char)tag;
    writeBerLength( binary, ucs2.length() + 1 );
    binary += (char)(QSMS_UCS2Alphabet | schemeMask);
    binary += ucs2;
}
#define writeTextString _qtopiaphone_writeTextString

// Write an EFADN string as a BER field.  GSM 11.11, section 10.5.1.
// Exported to qsimcontrolevent.cpp.
void _qtopiaphone_writeEFADN( QByteArray& binary, const QString& str,
                              QSimCommand::ToPduOptions options, int tag = 0x85 );
void _qtopiaphone_writeEFADN( QByteArray& binary, const QString& str,
                              QSimCommand::ToPduOptions options, int tag )
{
    // Determine if we can encode the string in the unpacked GSM alphabet encoding.
    if ( ( options & QSimCommand::UCS2Strings ) == 0 ) {
        QTextCodec *gsm = QAtUtils::codec( "gsm-noloss" );
        if ( gsm->canEncode( str ) ) {
            QByteArray unpacked = gsm->fromUnicode( str );
            if ( tag != -1 ) {
                binary += (char)tag;
                writeBerLength( binary, unpacked.length() );
            }
            binary += unpacked;
            return;
        }
    }

    // If we get here, we must use UCS-2 to encode the text string.
    QByteArray ucs2;
    const ushort *data = str.utf16();
    int len = str.length();
    while ( len-- > 0 ) {
        ushort ch = *data++;
        ucs2 += (char)(ch >> 8);
        ucs2 += (char)ch;
    }
    if ( tag != -1 ) {
        binary += (char)tag;
        writeBerLength( binary, ucs2.length() + 1 );
    }
    binary += (char)0x80;
    binary += ucs2;
}
#define writeEFADN _qtopiaphone_writeEFADN

// Write icon details.
static void writeIcon( QByteArray& binary, uint iconId, bool selfExplanatory, bool mandatory )
{
    if ( iconId ) {
        binary += (char)( mandatory ? 0x9E : 0x1E );
        binary += (char)0x02;
        binary += (char)( selfExplanatory ? 0 : 1 );
        binary += (char)iconId;
    }
}

// Write duration information.  Needs to be exported to qsimterminalresponse.cpp.
void _qtopiaphone_writeDuration( QByteArray& data, uint time )
{
    if ( time != 0 ) {
        data += (char)0x84;
        data += (char)0x02;
        if ( ( time % 1000 ) != 0 && time <= 25500 ) {
            // Encode in tenths of a second.
            data += (char)0x02;
            data += (char)(time / 100);
        } else if ( ( time % 60000 ) != 0 && time <= 255000 ) {
            // Encode in seconds.
            data += (char)0x01;
            data += (char)(time / 1000);
        } else if ( time <= 255 * 6000 ) {
            // Encode in minutes.
            data += (char)0x00;
            data += (char)(time / 60000);
        } else {
            // Encode maximum time value.
            data += (char)0x00;
            data += (char)0xFF;
        }
    }
}
#define writeDuration _qtopiaphone_writeDuration

// Write a subaddress, GSM 11.14, section 12.3.  The actual encoding
// is specified in GSM 24.008, Annex E.
static void writeSubAddress( QByteArray& data, const QString& value )
{
    data += (char)0x88;
    writeBerLength( data, value.length() + 2 );
    data += (char)0x80;     // NSAP address type
    data += (char)0x50;     // NSAP address is BCD-encoded IA5 characters
    foreach ( QChar ch, value ) {
        int val = ch.unicode() - 32;
        if ( val > 127 )
            val = 0;
        data += (char)(((val / 10) << 4) + (val % 10));
    }
}

// Extract an extension field named "tag" from "extData" and write it to "data".
// The field will be removed from "extData".  Returns false if no such field.
// Exported to qsimcontrolevent.cpp.
bool _qtopiaphone_extractAndWriteExtField( QByteArray& data, QByteArray& extData, int tag )
{
    uint startposn;
    uint posn = 0;
    uint currentTag, length;
    while ( posn < (uint)( extData.length() ) ) {
        startposn = posn;
        readBer( extData, posn, currentTag, length );
        if ( ( currentTag & 0x7F ) == (uint)( tag & 0x7F ) ) {
            data += extData.mid( startposn, (posn + length) - startposn );
            extData = extData.left( startposn ) + extData.mid( posn + length );
            return true;
        }
        posn += length;
    }
    return false;
}
#define extractAndWriteExtField _qtopiaphone_extractAndWriteExtField

// Write a text attribute field.
static void writeTextAttribute( QByteArray& data, const QByteArray& attr )
{
    if ( !attr.isEmpty() ) {
        data += (char)0xD0;
        writeBerLength( data, attr.length() );
        data += attr;
    }
}

/*!
    \enum QSimCommand::ToPduOptions
    This enum defines additional options to use when encoding SIM commands with QSimCommand::toPdu().

    \value NoPduOptions No additional options.
    \value NoBerWrapper Do not include the outermost BER wrapper around the command parameters.
    \value PackedStrings Encode text strings in the packed 7-bit GSM alphabet instead
                         of the unpacked 8-bit GSM alphabet.  If a text string cannot
                         be encoded with the GSM alphabet, UCS-2 will be used.
    \value UCS2Strings Encode text strings in the UCS-2 alphabet, even if it could
                       otherwise be encoded in the packed 7-bit or unpacked 8-bit
                       GSM alphabet.
    \value EncodeEmptyStrings Encode empty strings in the actual character set, instead
                       of using the shorthand encoding.
*/

/*!
    Encode a SIM command into its raw binary PDU form, as described
    in 3GPP TS 11.14.  The resulting PDU form may be modified by the
    supplied \a options.

    \sa fromPdu()
*/
QByteArray QSimCommand::toPdu( QSimCommand::ToPduOptions options ) const
{
    QByteArray data;

    // Output the command details.
    QSimCommand::Type cmd = type();
    if ( cmd == NoCommand || cmd == Timeout )
        return QByteArray();
    int qualifier = d->qualifier;
    if ( cmd == GetInkey && wantYesNo() ) {
        // Make sure the "wantDigits" bit is unset if using Yes/No.
        qualifier &= ~0x01;
    }
    data += (char)0x81;
    data += (char)0x03;
    data += (char)commandNumber();
    data += (char)cmd;
    data += (char)qualifier;

    // Output the device identities (SIM to Display/Keypad/Earpiece/etc).
    data += (char)0x82;
    data += (char)0x02;
    data += (char)sourceDevice();
    data += (char)destinationDevice();

    // If user feedback suppression is on, then encode empty strings.
    if ( suppressUserFeedback() )
        options = (QSimCommand::ToPduOptions)( options | EncodeEmptyStrings );

    // Add the command parameters.
    QByteArray extData = extensionData();
    switch ( type() ) {
        case DisplayText:
        {
            writeTextString( data, text(), options );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            if ( immediateResponse() ) {
                data += (char)0xAB;
                data += (char)0x00;
            }
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case GetInkey:
        {
            writeTextString( data, text(), options );
            writeIcon( data, iconId(), iconSelfExplanatory(), false );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case GetInput:
        {
            writeTextString( data, text(), options );
            if ( minimumLength() != 0 || maximumLength() != 255 ) {
                data += (char)0x91;
                data += (char)0x02;
                data += (char)minimumLength();
                data += (char)maximumLength();
            }
            if ( !defaultText().isEmpty() ) {
                writeTextString( data, defaultText(), QSimCommand::NoPduOptions, 0x17 );
            }
            writeIcon( data, iconId(), iconSelfExplanatory(), false );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case PlayTone:
        {
            if ( !text().isEmpty() )
                writeEFADN( data, text(), options );
            if ( tone() != ToneNone ) {
                data += (char)0x8E;
                data += (char)0x01;
                data += (char)tone();
            }
            writeDuration( data, duration() );
            writeIcon( data, iconId(), iconSelfExplanatory(), false );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case PollInterval:
        {
            writeDuration( data, duration() );
        }
        break;

        case SetupMenu:
        case SelectItem:
        {
            if ( type() == SetupMenu || !title().isEmpty() )
                writeEFADN( data, title(), options );
            bool hasIcons = false;
            int attrLen = 0;
            bool hasNextActions = false;
            bool selfExplanatory = false;
            if ( menuItems().size() == 0 ) {
                data += (char)0x8F;
                data += (char)0x00;
            }
            foreach ( QSimMenuItem item, menuItems() ) {
                QByteArray contents;
                contents += (char)item.identifier();
                writeEFADN( contents, item.label(), options, -1 );
                data += (char)0x8F;
                writeBerLength( data, contents.size() );
                data += contents;
                if ( item.iconId() != 0 )
                    hasIcons = true;
                if ( item.nextAction() != 0 )
                    hasNextActions = true;
                if ( item.iconSelfExplanatory() )
                    selfExplanatory = true;
                attrLen += item.labelAttribute().size();
            }
            if ( hasNextActions ) {
                data += (char)0x18;
                data += (char)menuItems().size();
                foreach ( QSimMenuItem item, menuItems() )
                    data += (char)item.nextAction();
            }
            if ( type() == SelectItem && defaultItem() != 0 ) {
                data += (char)0x90;
                data += (char)0x01;
                data += (char)defaultItem();
            }
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            if ( hasIcons ) {
                data += (char)0x9F;
                data += (char)(menuItems().size() + 1);
                data += (char)(selfExplanatory ? 0x00 : 0x01);
                foreach ( QSimMenuItem item, menuItems() )
                    data += (char)item.iconId();
            }
            writeTextAttribute( data, titleAttribute() );
            if ( attrLen != 0 ) {
                data += (char)0xD1;
                writeBerLength( data, attrLen );
                foreach ( QSimMenuItem item, menuItems() ) {
                    data += item.labelAttribute();
                }
            }
        }
        break;

        case SendSMS:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options );
            if ( !number().isEmpty() )
                writeEFADNNumber( data, number() );
            extractAndWriteExtField( data, extData, 0x8B );
            writeIcon( data, iconId(), iconSelfExplanatory(), iconSelfExplanatory() );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case SendSS:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options );
            writeEFADNNumber( data, number(), 0x89, 255 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case SendUSSD:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), QSimCommand::NoPduOptions );
            if ( ( options & QSimCommand::PackedStrings ) != 0 )
                writeTextString( data, number(), options, 0xF08A );
            else
                writeTextString( data, number(), options, 0x408A );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case SetupCall:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), QSimCommand::NoPduOptions );
            writeEFADNNumber( data, number() );
            extractAndWriteExtField( data, extData, 0x87 );
            if ( !subAddress().isEmpty() )
                writeSubAddress( data, subAddress() );
            writeDuration( data, duration() );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            if ( !otherText().isEmpty() )
                writeEFADN( data, otherText(), QSimCommand::NoPduOptions );
            writeIcon( data, otherIconId(), otherIconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
            writeTextAttribute( data, otherTextAttribute() );
        }
        break;

        case SetupIdleModeText:
        {
            writeTextString( data, text(), options );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case RunATCommand:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), QSimCommand::NoPduOptions );
            extractAndWriteExtField( data, extData, 0xA8 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case SendDTMF:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options );
            writeEFADNNumber( data, number(), 0xAC );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case LaunchBrowser:
        {
            extractAndWriteExtField( data, extData, 0x30 ); // Browser id
            writeEFADN( data, url(), QSimCommand::NoPduOptions, 0x31 );
            data += extData;        // Add all network setup parameters before the text.
            extData = QByteArray();
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), false );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case OpenChannel:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            if ( !number().isEmpty() )
                writeEFADNNumber( data, number() );
            if ( !subAddress().isEmpty() )
                writeSubAddress( data, subAddress() );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case CloseChannel:
        case ReceiveData:
        case SendData:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case ServiceSearch:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            extractAndWriteExtField( data, extData, 0xC3 ); // Service search
            extractAndWriteExtField( data, extData, 0xC2 ); // Device filter
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case GetServiceInformation:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            extractAndWriteExtField( data, extData, 0xC4 ); // Attribute information
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case RetrieveMultimediaMessage:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            extractAndWriteExtField( data, extData, 0xEA ); // Multimedia message reference
            extractAndWriteExtField( data, extData, 0x92 ); // MMS reception file
            extractAndWriteExtField( data, extData, 0xEE ); // Content identifier
            extractAndWriteExtField( data, extData, 0xEB ); // Message identifier
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case SubmitMultimediaMessage:
        {
            if ( !text().isEmpty() || ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
                writeEFADN( data, text(), options, 0x05 );
            writeIcon( data, iconId(), iconSelfExplanatory(), true );
            extractAndWriteExtField( data, extData, 0x92 ); // MMS submission file
            extractAndWriteExtField( data, extData, 0xEB ); // Message identifier
            writeTextAttribute( data, textAttribute() );
        }
        break;

        case DisplayMultimediaMessage:
        {
            extractAndWriteExtField( data, extData, 0x92 ); // MMS submission file
            extractAndWriteExtField( data, extData, 0xEB ); // Message identifier
            if ( immediateResponse() ) {
                data += (char)0xAB;
                data += (char)0x00;
            }
        }
        break;

        default: break;
    }

    // Add any remaining extension data that is specified.
    data += extData;

    // Add the "Proactive SIM" BER command wrapper if required.
    if ( ( options & NoBerWrapper ) == 0 ) {
        QByteArray ber;
        ber += (char)0xD0;
        writeBerLength( ber, data.size() );
        ber += data;
        return ber;
    } else {
        return data;
    }
}

/*!
    Returns the qualifier byte from the header of the SIM command.  The qualifier byte
    contains flags that modify the behavior of the command from its default.  The default
    value is zero.

    Other functions such as hasHelp(), wantDigits(), highPriority(), etc provide a
    more convenient method to access the bits within the qualifier byte.

    Applies to: all commands.

    \sa setQualifier()
*/
int QSimCommand::qualifier() const
{
    return d->qualifier;
}

/*!
    Sets the qualifier byte in the header of the SIM command to \a value.
    The qualifier byte contains flags that modify the behavior of the command
    from its default.

    Other functions such as setHasHelp(), setWantDigits(), setHighPriority(), etc provide a
    more convenient method to set the bits within the qualifier byte.

    Applies to: all commands.

    \sa qualifier()
*/
void QSimCommand::setQualifier( int value )
{
    d->qualifier = value;
}

/*!
    Returns the extension data for this SIM command.  The extension data consists of
    zero or more BER tag-length-value fields for fields that are not otherwise handled
    by some other method in this class.

    Applies to: all commands.

    \sa setExtensionData(), extensionField()
*/
QByteArray QSimCommand::extensionData() const
{
    return d->extensionData;
}

/*!
    Sets the extension data for this SIM command to \a value.  The extension
    data consists of zero or more BER tag-length-value fields for fields that are
    not otherwise handled by some other method in this class.

    Applies to: all commands.

    \sa extensionData(), addExtensionField()
*/
void QSimCommand::setExtensionData( QByteArray value )
{
    d->extensionData = value;
}

/*!
    Returns the contents of an extension field.  The \a tag is an 8-bit value,
    from 3GPP TS 11.14, that specifies the particular field the caller is
    interested in.  The most significant bit of \a tag is ignored when searching
    for the field, as it contains the "must comprehend" status from 3GPP TS 11.14,
    and is not important for locating the desired field.

    This is a simpler method than extensionData() for accessing values within
    the extension data.  As an example, the following code extracts the
    "SMS TPDU" information from a \c SendSMS SIM command:

    \code
    QSimCommand cmd;
    ...
    QByteArray tpdu = cmd.extensionField(0x8B);
    \endcode

    Applies to: all commands.

    \sa addExtensionField()
*/
QByteArray QSimCommand::extensionField( int tag ) const
{
    uint posn = 0;
    uint currentTag, length;
    while ( posn < (uint)( d->extensionData.length() ) ) {
        readBer( d->extensionData, posn, currentTag, length );
        if ( ( currentTag & 0x7F ) == (uint)( tag & 0x7F ) )
            return d->extensionData.mid( posn, length );
        posn += length;
    }
    return QByteArray();
}

/*!
    Adds an extension field to the data from extensionData().  The field will have
    the specified \a tag and contents given by \a value.

    Applies to: all commands.

    \sa extensionField()
*/
void QSimCommand::addExtensionField( int tag, const QByteArray& value )
{
    d->extensionData += (char)tag;
    writeBerLength( d->extensionData, value.size() );
    d->extensionData += value;
}

/*!
    \fn void QSimCommand::deserialize(Stream &value)

    \internal

    Deserializes the QSimCommand instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimCommand::deserialize(Stream &stream)
{
    dwrite()->read( stream );
}


/*!
    \fn void QSimCommand::serialize(Stream &value) const

    \internal

    Serializes the QSimCommand instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QSimCommand::serialize(Stream &stream) const
{
    d->write( stream );
}

// Get a writable copy of the shared data storage.
QSimCommandPrivate *QSimCommand::dwrite()
{
    // If we are the only user of the private object, return it as-is.
    if ( d->ref == 1 )
        return d;

    // Create a new private object and copy the current contents into it.
    QSimCommandPrivate *newd = new QSimCommandPrivate(d);
    if ( !d->ref.deref() )
        delete d;
    d = newd;
    return newd;
}

#define EMS_ALIGNMENT       0x03
#define EMS_LEFT            0x00
#define EMS_CENTER          0x01
#define EMS_RIGHT           0x02
#define EMS_NO_ALIGN        0x03
#define EMS_FONT_SIZE       0x0C
#define EMS_FONT_LARGE      0x04
#define EMS_FONT_SMALL      0x08
#define EMS_BOLD            0x10
#define EMS_ITALIC          0x20
#define EMS_UNDERLINE       0x40
#define EMS_STRIKE          0x80
#define EMS_NO_FORMAT       ((-1 << 8) | EMS_NO_ALIGN)

// Map an EMS color code into a HTML color value.
static QString mapEmsColor(int color)
{
    static const char * const colors[16] = {
        "#000000",      // Black
        "#808080",      // Dark Grey
        "#800000",      // Dark Red
        "#808000",      // Dark Yellow
        "#008000",      // Dark Green
        "#008080",      // Dark Cyan
        "#000080",      // Dark Blue
        "#800080",      // Dark Magenta
        "#C0C0C0",      // Grey
        "#FFFFFF",      // White
        "#FF0000",      // Bright Red
        "#FFFF00",      // Bright Yellow
        "#00FF00",      // Bright Green
        "#00FFFF",      // Bright Cyan
        "#0000FF",      // Bright Blue
        "#FF00FF"       // Bright Magenta
    };
    return QString::fromLatin1(colors[color & 0x0F]);
}

// Convert a text string into HTML, using the specified EMS attributes.
static QString textToHtml(const QString& text, const QByteArray& attrs)
{
    if (text.isEmpty())
        return text;

    // Scan the attribute array and build up the formatting
    // flags for every character in the text string.
    int len = text.length();
    QVarLengthArray<int> formats(len);
    for (int posn = 0; posn < len; ++posn) {
        formats[posn] = EMS_NO_FORMAT;
    }
    int bgcolor = -1;
    for (int index = 0; (index + 3) <= attrs.size(); index += 4) {
        // Extract the next formatting block.
        int start = attrs[index] & 0xFF;
        int end;
        if (attrs[index + 1])
            end = start + (attrs[index + 1] & 0xFF);
        else
            end = len;
        int format = attrs[index + 2] & 0xFF;
        int colors;
        if ((index + 4) <= attrs.size()) {    // This byte is optional.
            colors = attrs[index + 3] & 0xFF;
            if (bgcolor == -1)
                bgcolor = ((colors >> 4) & 0x0F);
        } else {
            colors = -1;
        }

        // Apply the formatting to all of the positions that need it.
        while (start < end && start < len) {
            formats[start] = (colors << 8) | format;
            ++start;
        }
    }

    // Convert the text string according to the instructions.
    QString result;
    int posn = 0;
    int format = EMS_NO_FORMAT;
    int newFormat;
    if (bgcolor != -1) {
        // Set the overall background color for the entire page based
        // on the background color for the first attribute block.
        // If there are no attribute blocks, or only one attribute block
        // with three bytes, then leave the background color as the
        // system default.
        result += QLatin1String("<body bgcolor=\"");
        result += mapEmsColor(bgcolor);
        result += QLatin1String("\">");
    }
    while (posn < len) {
        // Copy characters that have the same format as the previous one.
        while (posn < len && formats[posn] == format) {
            QChar ch = text[posn];
            if (ch.unicode() == '\n') {
                result += QLatin1String("<br>");
            } else if (ch.unicode() == '\r') {
                result += QLatin1String("<br>");
                if ((posn + 1) < len && text[posn + 1] == QChar('\n'))
                    ++posn;
            } else if (ch.unicode() == '<') {
                result += QLatin1String("&lt;");
            } else if (ch.unicode() == '>') {
                result += QLatin1String("&gt;");
            } else if (ch.unicode() == '&') {
                result += QLatin1String("&amp;");
            } else {
                result += ch;
            }
            ++posn;
        }

        // Adjust the format to match the new requirements.
        if (posn < len)
            newFormat = formats[posn];
        else
            newFormat = EMS_NO_FORMAT;
        int oldColor = (format >> 8);
        int newColor = (newFormat >> 8);
        if ((format & EMS_STRIKE) != 0)
            result += QLatin1String("</s>");
        if ((format & EMS_UNDERLINE) != 0)
            result += QLatin1String("</u>");
        if ((format & EMS_ITALIC) != 0)
            result += QLatin1String("</i>");
        if ((format & EMS_BOLD) != 0)
            result += QLatin1String("</b>");
        if ((format & EMS_FONT_SIZE) == EMS_FONT_SMALL)
            result += QLatin1String("</small>");
        else if ((format & EMS_FONT_SIZE) == EMS_FONT_LARGE)
            result += QLatin1String("</big>");
        if (oldColor != -1)
            result += QLatin1String("</font>");
        if ((format & EMS_ALIGNMENT) != (newFormat & EMS_ALIGNMENT)) {
            if ((format & EMS_ALIGNMENT) == EMS_LEFT)
                result += QLatin1String("</div>");
            else if ((format & EMS_ALIGNMENT) == EMS_RIGHT)
                result += QLatin1String("</div>");
            else if ((format & EMS_ALIGNMENT) == EMS_CENTER)
                result += QLatin1String("</center>");
            if ((newFormat & EMS_ALIGNMENT) == EMS_LEFT)
                result += QLatin1String("<div align=\"left\">");
            else if ((newFormat & EMS_ALIGNMENT) == EMS_RIGHT)
                result += QLatin1String("<div align=\"right\">");
            else if ((newFormat & EMS_ALIGNMENT) == EMS_CENTER)
                result += QLatin1String("<center>");
        }
        if (newColor != -1) {
            result += QLatin1String("<font");
            int back = ((newColor >> 4) & 0x0F);
            int fore = (newColor & 0x0F);
            if (back != bgcolor) {
                result += QLatin1String(" style=\"background-color: ");
                result += mapEmsColor(back);
                result += QLatin1String("\"");
            }
            result += QLatin1String(" color=\"");
            result += mapEmsColor(fore);
            result += QLatin1String("\">");
        }
        if ((newFormat & EMS_FONT_SIZE) == EMS_FONT_SMALL)
            result += QLatin1String("<small>");
        else if ((newFormat & EMS_FONT_SIZE) == EMS_FONT_LARGE)
            result += QLatin1String("<big>");
        if ((newFormat & EMS_BOLD) != 0)
            result += QLatin1String("<b>");
        if ((newFormat & EMS_ITALIC) != 0)
            result += QLatin1String("<i>");
        if ((newFormat & EMS_UNDERLINE) != 0)
            result += QLatin1String("<u>");
        if ((newFormat & EMS_STRIKE) != 0)
            result += QLatin1String("<s>");

        // Switch to the new format.
        format = newFormat;
    }
    if (bgcolor != -1) {
        // Terminate the outer background color declaration.
        result += QLatin1String("</body>");
    }

    return result;
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QSimCommand::Type)
Q_IMPLEMENT_USER_METATYPE(QSimCommand)
Q_IMPLEMENT_USER_METATYPE(QSimMenuItem)
