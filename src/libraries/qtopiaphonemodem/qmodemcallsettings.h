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

#ifndef QMODEMCALLSETTINGS_H
#define QMODEMCALLSETTINGS_H

#include <qcallsettings.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCallSettings : public QCallSettings
{
    Q_OBJECT
public:
    explicit QModemCallSettings( QModemService *service );
    ~QModemCallSettings();

public slots:
    void requestCallWaiting();
    void setCallWaiting( bool enable, QTelephony::CallClass cls );
    void requestCallerIdRestriction();
    void setCallerIdRestriction( QCallSettings::CallerIdRestriction clir );
    void requestSmsTransport();
    void setSmsTransport( QCallSettings::SmsTransport transport );
    void requestCallerIdPresentation();
    void requestConnectedIdPresentation();

private slots:
    void ccwaRequest( bool ok, const QAtResult& result );
    void ccwaSet( bool ok, const QAtResult& result );
    void clirRequest( bool ok, const QAtResult& result );
    void clirSet( bool ok, const QAtResult& result );
    void cgsmsRequest( bool ok, const QAtResult& result );
    void cgsmsSet( bool ok, const QAtResult& result );
    void clipRequest( bool ok, const QAtResult& result );
    void colpRequest( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif
