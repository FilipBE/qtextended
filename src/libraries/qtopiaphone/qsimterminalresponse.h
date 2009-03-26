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
#ifndef QSIMTERMINALRESPONSE_H
#define QSIMTERMINALRESPONSE_H

#include <qsimcommand.h>

class QSimTerminalResponsePrivate;

class QTOPIAPHONE_EXPORT QSimTerminalResponse
{
public:
    QSimTerminalResponse();
    QSimTerminalResponse( const QSimTerminalResponse& value );
    ~QSimTerminalResponse();

    enum Result
    {
        Success                             = 0x00,
        PartialComprehension                = 0x01,
        MissingInformation                  = 0x02,
        RefreshPerformed                    = 0x03,
        IconNotDisplayed                    = 0x04,
        ModifiedCallControl                 = 0x05,
        LimitedService                      = 0x06,
        WithModification                    = 0x07,
        SessionTerminated                   = 0x10,
        BackwardMove                        = 0x11,
        NoResponseFromUser                  = 0x12,
        HelpInformationRequested            = 0x13,
        UssdOrSsTerminatedByUser            = 0x14,

        MEUnableToProcess                   = 0x20,
        NetworkUnableToProcess              = 0x21,
        UserDidNotAccept                    = 0x22,
        UserClearedDownCall                 = 0x23,
        ActionInContradictionWithTimer      = 0x24,
        TemporaryCallControlProblem         = 0x25,
        LaunchBrowserError                  = 0x26,

        BeyondMECapabilities                = 0x30,
        TypeNotUnderstood                   = 0x31,
        DataNotUnderstood                   = 0x32,
        NumberNotUnderstood                 = 0x33,
        SsReturnError                       = 0x34,
        SmsRpError                          = 0x35,
        RequiredValuesMissing               = 0x36,
        UssdReturnError                     = 0x37,
        MultipleCardError                   = 0x38,
        PermanentCallControlProblem         = 0x39,
        BearerIndependentProtocolProblem    = 0x3A
    };

    enum Cause
    {
        // Common causes.
        NoSpecificCause                     = 0x00,

        // MEUnableToProcess causes.
        ScreenIsBusy                        = 0x01,
        BusyOnCall                          = 0x02,
        BusyOnSsTransaction                 = 0x03,
        NoService                           = 0x04,
        AccessControlClassBar               = 0x05,
        RadioResourceNotGranted             = 0x06,
        NotInSpeechCall                     = 0x07,
        BusyOnUssdTransaction               = 0x08,
        BusyOnDtmf                          = 0x09,

        // PermanentCallControlProblem causes.
        ActionNotAllowed                    = 0x01,
        TypeOfRequestHasChanged             = 0x02,

        // MultipleCardError causes.
        CardReaderRemovedOrNotPresent       = 0x01,
        CardRemovedOrNotPresent             = 0x02,
        CardReaderBusy                      = 0x03,
        CardPoweredOff                      = 0x04,
        CAPDUFormatError                    = 0x05,
        MuteCard                            = 0x06,
        TransmissionError                   = 0x07,
        ProtocolNotSupported                = 0x08,
        SpecifiedReaderNotValid             = 0x09,

        // LaunchBrowserError causes.
        BearerUnavailable                   = 0x01,
        BrowserUnavailable                  = 0x02,
        UnableToReadProvisioningData        = 0x03,

        // BearerIndependentProtocolProblem causes.
        NoChannelAvailable                  = 0x01,
        ChannelClosed                       = 0x02,
        ChannelIdentifierNotValid           = 0x03,
        RequestedBufferSizeNotAvailable     = 0x04,
        SecurityError                       = 0x05,
        RequestedTransportNotAvailable      = 0x06
    };

    QSimCommand command() const;
    void setCommand( const QSimCommand& value );

    QByteArray commandPdu() const;
    void setCommandPdu( const QByteArray& value );

    QSimCommand::Device sourceDevice() const;
    void setSourceDevice( QSimCommand::Device value );

    QSimCommand::Device destinationDevice() const;
    void setDestinationDevice( QSimCommand::Device value );

    QSimTerminalResponse::Result result() const;
    void setResult( QSimTerminalResponse::Result value );

    QSimTerminalResponse::Cause cause() const;
    void setCause( QSimTerminalResponse::Cause value );

    QByteArray causeData() const;
    void setCauseData( const QByteArray& value );

    QString text() const;
    void setText( const QString& value );

    uint duration() const;
    void setDuration( uint value );

    uint menuItem() const;
    void setMenuItem( uint value );

    int dataCodingScheme() const;
    void setDataCodingScheme( int value );

    QByteArray extensionData() const;
    void setExtensionData( QByteArray value );

    QByteArray extensionField( int tag ) const;
    void addExtensionField( int tag, const QByteArray& value );

    static QSimTerminalResponse fromPdu( const QByteArray& pdu );
    QByteArray toPdu() const;

    QSimTerminalResponse& operator=( const QSimTerminalResponse & );
    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSimTerminalResponsePrivate *d;
};

Q_DECLARE_USER_METATYPE(QSimTerminalResponse)

#endif
