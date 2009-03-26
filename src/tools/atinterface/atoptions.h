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

#ifndef ATOPTIONS_H
#define ATOPTIONS_H

#include <qstring.h>
#include <qstringlist.h>
#include <QDateTime>

class QTextCodec;

class AtOptions
{
public:
    AtOptions( const QString& startupOptions = QString() );

    bool echo;              // ATE
    char backspace;         // ATS5
    char terminator;        // ATS3
    char response;          // ATS4

    bool suppressResults;   // ATQ
    bool verboseResults;    // ATV
    int extendedErrors;     // AT+CMEE

    QString charset;        // AT+CSCS
    QTextCodec *codec;

    bool cring;             // +CRING unsolicited notification state
    bool ccwa;              // +CCWA unsolicited notification state
    bool clip;              // +CLIP unsolicited notification state
    bool qcam;              // *QCAM unsolicited notification state
    int cind;               // +CMER indicator reporting mode (0-2)
    int cmer;               // +CMER indicator buffering mode (0-3)
    bool qsq;               // *QSQ signal quality notification state
    bool qbc;               // *QBC battery charge notification state
    int creg;               // +CREG unsolicited notification state
    bool cr;                // +CR unsolicited notification state
    bool colp;              // +COLP unsolicited notification state
    bool clae;              // +CLAE unsolicited notification state
    bool cssi;              // +CSSI unsolicited notification state
    bool cssu;              // +CSSU unsolicited notification state
    bool ccwv;              // +CCWV unsolicited notification state
    bool cccm;              // +CCCM unsolicited notification state
    QDateTime lastTimeCCCM; // last time of +CCCM unsolicited notification

    bool cusd;              // +CUSD mode
    uint cmod;              // +CMOD mode
    uint csns;              // +CSNS mode

    bool contextSet;        // +CGDCONT context has been set prior to dial
    QString apn;            // +CGDCONT Access Point Name

    int cbstSpeed;          // +CBST speed value
    int cbstName;           // +CBST name value
    int cbstCe;             // +CBST ce value

    uint cpls;              // +CPLS list value
    uint cpolFormat;        // +CPOL format value
    uint timeFormat;        // +CSTF value
    uint dateFormat;        // +CSDF primary value
    uint auxDateFormat;     // +CSDF auxillary value
    int mtDateTimeOffset;   // the "faked" time offset (seconds) from real time
    int mtTimeZoneSeconds;  // the "faked" time zone offset in seconds from UTC
    uint curr_call_mode;    // not used atm - if voice, ath disconn etc.

    bool csvm;              // +CSVM <mode> value
    bool ignore_ath;        // +ATH behaviour value
    bool ignore_drop_dtr;   // "Drop DTR" behaviour value

    int smsService;         // +CSMS value
    bool messageFormat;     // +CMGF value
    bool csdh;              // +CSDH value

    bool csgt;              // +CSGT mode value
    QString greetingText;   // the greeting text

    QString phoneStore;     // +CPBS value
    //QString phoneStorePw;   // +CPBS value

    QString startupOptions;
    QStringList startupOptionsList;

    void factoryDefaults();
    void load();
    void save();
    void setCharset( const QString& value );
    bool hasStartupOption( const QString& option );

    void clearDataOptions();

    QString nextString( const QString& buf, uint& posn );
};

#endif
