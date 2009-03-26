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

#ifndef ATSMSCOMMANDS_H
#define ATSMSCOMMANDS_H

#include <QTelephony>
#include <QObject>

class AtCommands;
class QSMSSender;
class QSMSReader;
class QSMSMessage;

class AtSmsCommands : public QObject
{
    Q_OBJECT

public:
    AtSmsCommands(AtCommands * parent);
    ~AtSmsCommands();

public slots:
    void atcmgd( const QString& params );
    void atcmgf( const QString& params );
    void atcmgl( const QString& params );
    void atcmgr( const QString& params );
    void atcmgs( const QString& params );
    void atcmgw( const QString& params );
    void atcmms( const QString& params );
    void atcnmi( const QString& params );
    void atcpms( const QString& params );
    void atcres(  );
    void atcsas(  );
    void atcsca( const QString& params );
    void atcsdh( const QString& params );

private slots:
    void extraLine( const QString& line, bool cancel );
    void smsFetched( const QString & id, const QSMSMessage & m );
    void smsFinished( const QString& id, QTelephony::Result result );

private:
    AtCommands *atc;

    // sms related
    QString smsNumber;
    QString smsMessageId;
    bool sendingSms;
    QSMSSender *smsSender;
    int smsMessageReference;
    QSMSReader *smsReader;
    bool readingSms;
    int readingSmsCount;
    int wantedSmsIndex;
    bool writingSms;
    uint cmgw_address_type;
    QString cmgw_status;
};

#endif
