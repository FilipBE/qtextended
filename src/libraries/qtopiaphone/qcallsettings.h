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

#ifndef QCALLSETTINGS_H
#define QCALLSETTINGS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallSettings : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(CallerIdRestriction CallerIdRestrictionStatus SmsTransport)
    Q_ENUMS(PresentationStatus)
public:
    explicit QCallSettings( const QString& service = QString::null,
                            QObject *parent = 0,
                            QCommInterface::Mode mode = Client );
    ~QCallSettings();

    enum CallerIdRestriction
    {
        Subscription,
        Invoked,
        Suppressed
    };

    enum CallerIdRestrictionStatus
    {
        NotProvisioned,
        Permanent,
        Unknown,
        TemporaryRestricted,
        TemporaryAllowed
    };

    enum SmsTransport
    {
        SmsTransportPD,
        SmsTransportCS,
        SmsTransportPDPreferred,
        SmsTransportCSPreferred,
        SmsTransportUnavailable
    };

    enum PresentationStatus
    {
        PresentationNotProvisioned,
        PresentationProvisioned,
        PresentationUnknown
    };

public slots:
    virtual void requestCallWaiting();
    virtual void setCallWaiting( bool enable, QTelephony::CallClass cls );
    virtual void requestCallerIdRestriction();
    virtual void setCallerIdRestriction
        ( QCallSettings::CallerIdRestriction clir );
    virtual void requestSmsTransport();
    virtual void setSmsTransport( QCallSettings::SmsTransport transport );
    virtual void requestCallerIdPresentation();
    virtual void requestConnectedIdPresentation();

signals:
    void callWaiting( QTelephony::CallClass cls );
    void setCallWaitingResult( QTelephony::Result result );
    void callerIdRestriction( QCallSettings::CallerIdRestriction clir,
                              QCallSettings::CallerIdRestrictionStatus status );
    void setCallerIdRestrictionResult( QTelephony::Result result );
    void smsTransport( QCallSettings::SmsTransport transport );
    void setSmsTransportResult( QTelephony::Result result );
    void callerIdPresentation( QCallSettings::PresentationStatus status );
    void connectedIdPresentation( QCallSettings::PresentationStatus status );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::CallerIdRestriction)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::CallerIdRestrictionStatus)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::SmsTransport)
Q_DECLARE_USER_METATYPE_ENUM(QCallSettings::PresentationStatus)

#endif
