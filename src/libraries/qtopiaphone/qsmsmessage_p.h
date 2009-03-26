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

#ifndef QSMSMESSAGE_P_H
#define QSMSMESSAGE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qsmsmessage.h>
#include <qcbsmessage.h>

// Values are their bit reprensentation (defined in GSM-03.40 Section 9)
// bits 6 5 4 ( shl 4 )
enum SMSAddressType {
    SMS_Address_Unknown         = 0,
    SMS_Address_International   = 1,
    SMS_Address_National        = 2,
    SMS_Address_NetworkSpecific = 3,
    SMS_Address_SubscriberNmb   = 4,
    SMS_Address_AlphaNumeric    = 5,
    SMS_Address_Abbreviated     = 6,
    SMS_Address_Reserved        = 7
};

// Values are their bit reprensentation (defined in GSM-03.40 Section 9)
// bits 3 2 1 0 ( shl 0 )
enum SMSNumbering {
    SMS_NumberId_Unknown            = 0,
    SMS_Phone               = 1
    // .. 6 more defined though.
};

// TP-MTI (2 bits)
enum SMSMessageType {
    SMS_Deliver             = 0,
    SMS_DeliverReport       = 0,
    SMS_StatusReport        = 2,
    SMS_Command             = 2,
    SMS_Submit              = 1,
    SMS_SubmitReport        = 1,
    SMS_Reserved            = 3
};

// TP_VPF (2 bits);
enum SMSValidityFormat {
    SMS_VF_NoPresent        = 0,
    SMS_VF_Relative         = 2,
    SMS_VF_Enhanced         = 1,
    SMS_VF_Absolute         = 3
};

// User data header kinds from GSM 03.40 and GSM 23.040.
enum SMSHeaderKind {
    SMS_HK_Concat_8Bit                  = 0x00,
    SMS_HK_Special_Msg_Indic            = 0x01,
    SMS_HK_AppPort_8Bit                 = 0x04,
    SMS_HK_AppPort_16Bit                = 0x05,
    SMS_HK_Control_Parameters           = 0x06,
    SMS_HK_UDH_Source_Indic             = 0x07,
    SMS_HK_Concat_16Bit                 = 0x08,
    SMS_HK_WCMP                         = 0x09,
    SMS_HK_Text_Formatting              = 0x0A,
    SMS_HK_Predefined_Sound             = 0x0B,
    SMS_HK_User_Defined_Sound           = 0x0C,
    SMS_HK_Predefined_Animation         = 0x0D,
    SMS_HK_Large_Animation              = 0x0E,
    SMS_HK_Small_Animation              = 0x0F,
    SMS_HK_Large_Picture                = 0x10,
    SMS_HK_Small_Picture                = 0x11,
    SMS_HK_Variable_Picture             = 0x12,
    SMS_HK_User_Prompt_Indic            = 0x13,
    SMS_HK_Extended_Object              = 0x14,
    SMS_HK_Reused_Extended_Object       = 0x15,
    SMS_HK_Compression_Control          = 0x16,
    SMS_HK_Object_Dist_Indic            = 0x17,
    SMS_HK_Standard_WVG_Object          = 0x18,
    SMS_HK_Char_Size_WVG_Object         = 0x19,
    SMS_HK_Data_Request_Command         = 0x1A,
    SMS_HK_RFC_822_Header               = 0x20,
    SMS_HK_Hyperlink_Format_Element     = 0x21,
    SMS_HK_Reply_Address_Element        = 0x22
};

class QPDUMessage
{
public:
    QPDUMessage();
    explicit QPDUMessage(const QByteArray &);
    ~QPDUMessage();
    QPDUMessage(const QPDUMessage &);

    QByteArray toByteArray() const { return mBuffer; }

    void setBit(int b, bool on);
    void setBits(int offset, int len, int val);
    void commitBits();
    void appendOctet(uchar c) { mBuffer += (char)c; }

    bool bit(int b);
    unsigned char bits(int offset, int len);
    unsigned char getOctet();
    unsigned char peekOctet() const;
    void skipOctet();
    void abort() { mPosn = mBuffer.size(); }

    QByteArray getOctets( uint len );

    void setAddress(const QString &, bool SCAddress);
    QString address(bool SCAddress);

    uint addressLength() const;

    void setTimeStamp(const QDateTime &);
    QDateTime timeStamp();

    void setUserData(const QString &txt, QSMSDataCodingScheme scheme, QTextCodec *codec, const QByteArray& headers, bool implicitLength = false);
    QString userData(QSMSDataCodingScheme scheme, QTextCodec *codec, QByteArray *& headers, bool hasHeaders, bool implicitLength = false);

    SMSMessageType messageType();

    void reset() { mPosn = 0; }

    bool needOctets( uint num ) const
        { return ((uint)(mBuffer.size() - mPosn) >= num); }

protected:
    QByteArray mBuffer;
    int mPosn;
    char mBits;
};


class QSMSSubmitMessage: public QPDUMessage
{
public:
    explicit QSMSSubmitMessage(const QSMSMessage &m, bool isDeliver=false);
};

class QSMSDeliverMessage: public QPDUMessage
{
public:
    explicit QSMSDeliverMessage(const QByteArray &pdu);

    QSMSMessage unpack(QTextCodec *codec=0);
};

class QCBSDeliverMessage: public QPDUMessage
{
public:
    QCBSDeliverMessage();
    explicit QCBSDeliverMessage(const QByteArray &pdu);

    QCBSMessage unpack(QTextCodec *codec=0);
    void pack(const QCBSMessage &m, QSMSDataCodingScheme scheme);
};

#endif
