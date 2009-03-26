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
#ifndef QSIMCOMMAND_H
#define QSIMCOMMAND_H


#include <qtopiaipcmarshal.h>
#include <qlist.h>
#include <qdatastream.h>

class QSimMenuItemPrivate;
class QSimCommandPrivate;


class QTOPIAPHONE_EXPORT QSimMenuItem
{
public:
    QSimMenuItem();
    QSimMenuItem( const QSimMenuItem& value );
    ~QSimMenuItem();

    uint identifier() const;
    void setIdentifier( uint value );

    QString label() const;
    void setLabel( const QString& value );

    QByteArray labelAttribute() const;
    void setLabelAttribute( const QByteArray& value );

    QString labelHtml() const;

    bool hasHelp() const;
    void setHasHelp( bool value );

    uint iconId() const;
    void setIconId( uint value );

    bool iconSelfExplanatory() const;
    void setIconSelfExplanatory( bool value );

    uint nextAction() const;
    void setNextAction( uint value );

    QSimMenuItem& operator=( const QSimMenuItem & );
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSimMenuItemPrivate *d;
};

class QTOPIAPHONE_EXPORT QSimCommand
{
public:
    QSimCommand();
    QSimCommand( const QSimCommand& value );
    ~QSimCommand();

    // Command types.
    enum Type
    {
        NoCommand                   = -1,
        Timeout                     = -2,
        Refresh                     = 0x01,
        MoreTime                    = 0x02,
        PollInterval                = 0x03,
        PollingOff                  = 0x04,
        SetupEventList              = 0x05,
        SetupCall                   = 0x10,
        SendSS                      = 0x11,
        SendUSSD                    = 0x12,
        SendSMS                     = 0x13,
        SendDTMF                    = 0x14,
        LaunchBrowser               = 0x15,
        PlayTone                    = 0x20,
        DisplayText                 = 0x21,
        GetInkey                    = 0x22,
        GetInput                    = 0x23,
        SelectItem                  = 0x24,
        SetupMenu                   = 0x25,
        ProvideLocalInformation     = 0x26,
        TimerManagement             = 0x27,
        SetupIdleModeText           = 0x28,
        PerformCardAPDU             = 0x30,
        PowerOnCard                 = 0x31,
        PowerOffCard                = 0x32,
        GetReaderStatus             = 0x33,
        RunATCommand                = 0x34,
        LanguageNotification        = 0x35,
        OpenChannel                 = 0x40,
        CloseChannel                = 0x41,
        ReceiveData                 = 0x42,
        SendData                    = 0x43,
        GetChannelStatus            = 0x44,
        ServiceSearch               = 0x45,
        GetServiceInformation       = 0x46,
        DeclareService              = 0x47,
        SetFrames                   = 0x50,
        GetFramesStatus             = 0x51,
        RetrieveMultimediaMessage   = 0x60,
        SubmitMultimediaMessage     = 0x61,
        DisplayMultimediaMessage    = 0x62,
        EndSession                  = 0x81,

        SetupMainMenu               = SetupMenu,
        SetupSubMenu                = SelectItem
    };

    // Disposition of other calls during a call setup.
    enum Disposition
    {
        IfNoOtherCalls      = 0,    // Setup if no other calls.
        PutOnHold           = 1,    // Put other calls on hold first.
        Disconnect          = 2     // Disconnect other calls first.
    };

    // Class of call to setup.
    enum CallClass
    {
        Voice               = 0,
        Data                = 1,
        Fax                 = 2
    };

    // Tones that can be played.
    enum Tone
    {
        ToneNone            = -1,
        ToneDial            = 0x01,
        ToneBusy            = 0x02,
        ToneCongestion      = 0x03,
        ToneRadioAck        = 0x04,
        ToneDropped         = 0x05,
        ToneError           = 0x06,
        ToneCallWaiting     = 0x07,
        ToneRinging         = 0x08,
        ToneGeneralBeep     = 0x10,
        TonePositiveBeep    = 0x11,
        ToneNegativeBeep    = 0x12
    };

    // The type of refresh to perform.
    enum RefreshType
    {
        InitAndFullFileChange   = 0,
        FileChange              = 1,
        InitAndFileChange       = 2,
        Initialization          = 3,
        Reset                   = 4,
        NaaApplicationReset     = 5,
        NaaSessionReset         = 6
    };

    // Event types.
    enum Event
    {
        NoEvent             = 0,
        IdleScreen          = 1,
        UserActivity        = 2,
        Both                = 3,
        Cancel              = 4
    };

    // Browser launch mode.
    enum BrowserLaunchMode
    {
        IfNotAlreadyLaunched    = 0,
        UseExisting             = 2,
        CloseExistingAndLaunch  = 3
    };

    // Menu presentation type.
    enum MenuPresentation
    {
        AnyPresentation                 = 0,
        DataValuesPresentation          = 1,
        NavigationOptionsPresentation   = 2
    };

    // Device identities for source and destination of commands.
    enum Device
    {
        Keypad              = 0x01,
        Display             = 0x02,
        Earpiece            = 0x03,
        CardReader0         = 0x10,
        CardReader1         = 0x11,
        CardReader2         = 0x12,
        CardReader3         = 0x13,
        CardReader4         = 0x14,
        CardReader5         = 0x15,
        CardReader6         = 0x16,
        CardReader7         = 0x17,
        Channel1            = 0x21,
        Channel2            = 0x22,
        Channel3            = 0x23,
        Channel4            = 0x24,
        Channel5            = 0x25,
        Channel6            = 0x26,
        Channel7            = 0x27,
        SIM                 = 0x81,
        ME                  = 0x82,
        Network             = 0x83
    };

    int commandNumber() const;
    void setCommandNumber( int value );

    Type type() const;
    void setType( QSimCommand::Type value );

    QSimCommand::Device sourceDevice() const;
    void setSourceDevice( QSimCommand::Device value );

    QSimCommand::Device destinationDevice() const;
    void setDestinationDevice( QSimCommand::Device value );

    bool hasHelp() const;
    void setHasHelp( bool value );

    QString text() const;
    void setText( const QString& value );

    QByteArray textAttribute() const;
    void setTextAttribute( const QByteArray& value );

    QString textHtml() const;

    bool suppressUserFeedback() const;
    void setSuppressUserFeedback( bool value );

    QString otherText() const;
    void setOtherText( const QString& value );

    QByteArray otherTextAttribute() const;
    void setOtherTextAttribute( const QByteArray& value );

    QString otherTextHtml() const;

    QString defaultText() const;
    void setDefaultText( const QString& value );

    bool highPriority() const;
    void setHighPriority( bool value );

    bool clearAfterDelay() const;
    void setClearAfterDelay( bool value );

    bool immediateResponse() const;
    void setImmediateResponse( bool value );

    bool ucs2Input() const;
    void setUcs2Input( bool value );

    bool packedInput() const;
    void setPackedInput( bool value );

    bool wantDigits() const;
    void setWantDigits( bool value );

    bool wantYesNo() const;
    void setWantYesNo( bool value );

    uint minimumLength() const;
    void setMinimumLength( uint value );

    uint maximumLength() const;
    void setMaximumLength( uint value );

    bool echo() const;
    void setEcho( bool value );

    QSimCommand::Disposition disposition() const;
    void setDisposition( QSimCommand::Disposition value );

    bool withRedial() const;
    void setWithRedial( bool value );

    QString number() const;
    void setNumber( const QString& value );

    QString subAddress() const;
    void setSubAddress( const QString& value );

    QSimCommand::CallClass callClass() const;
    void setCallClass( QSimCommand::CallClass value );

    QSimCommand::Tone tone() const;
    void setTone( QSimCommand::Tone value );

    uint toneTime() const;
    void setToneTime( uint value );

    uint duration() const;
    void setDuration( uint value );

    bool softKeysPreferred() const;
    void setSoftKeysPreferred( bool value );

    QSimCommand::MenuPresentation menuPresentation() const;
    void setMenuPresentation( QSimCommand::MenuPresentation value );

    QString title() const;
    void setTitle( const QString& value );

    QByteArray titleAttribute() const;
    void setTitleAttribute( const QByteArray& value );

    QString titleHtml() const;

    uint defaultItem() const;
    void setDefaultItem( uint value );

    QList<QSimMenuItem> menuItems() const;
    void setMenuItems( const QList<QSimMenuItem>& value );

    QSimCommand::RefreshType refreshType() const;
    void setRefreshType( QSimCommand::RefreshType value );

    QSimCommand::Event events() const;
    void setEvents( QSimCommand::Event value );

    QSimCommand::BrowserLaunchMode browserLaunchMode() const;
    void setBrowserLaunchMode( QSimCommand::BrowserLaunchMode value );

    QString url() const;
    void setUrl( const QString& value );

    uint iconId() const;
    void setIconId( uint value );

    bool iconSelfExplanatory() const;
    void setIconSelfExplanatory( bool value );

    uint otherIconId() const;
    void setOtherIconId( uint value );

    bool otherIconSelfExplanatory() const;
    void setOtherIconSelfExplanatory( bool value );

    bool smsPacking() const;
    void setSmsPacking( bool value );

    int qualifier() const;
    void setQualifier( int value );

    QByteArray extensionData() const;
    void setExtensionData( QByteArray value );

    QByteArray extensionField( int tag ) const;
    void addExtensionField( int tag, const QByteArray& value );

    enum ToPduOptions
    {
        NoPduOptions        = 0x00,
        NoBerWrapper        = 0x01,
        PackedStrings       = 0x02,
        UCS2Strings         = 0x04,
        EncodeEmptyStrings  = 0x08
    };

    static QSimCommand fromPdu( const QByteArray& pdu );
    QByteArray toPdu( QSimCommand::ToPduOptions options = NoPduOptions ) const;

    QSimCommand& operator=( const QSimCommand & );
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSimCommandPrivate *d;

    QSimCommandPrivate *dwrite();
};

Q_DECLARE_USER_METATYPE(QSimMenuItem)
Q_DECLARE_USER_METATYPE_ENUM(QSimCommand::Type)
Q_DECLARE_USER_METATYPE(QSimCommand)

#endif
