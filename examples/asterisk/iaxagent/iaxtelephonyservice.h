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

#ifndef IAXTELEPHONYSERVICE_H
#define IAXTELEPHONYSERVICE_H

#include <qobject.h>
#include <qtopiaabstractservice.h>
#include <qtelephonyservice.h>

class QAudioInput;
class QAudioOutput;
class QTimer;
class IaxNetworkRegistration;

class IaxTelephonyService : public QTelephonyService
{
    Q_OBJECT
public:
    IaxTelephonyService( const QString& service, QObject *parent = 0 );
    ~IaxTelephonyService();

    void initialize();

    // Callbacks that interface to the iaxclient library.
    bool startAudio();
    void stopAudio();
    int writeAudio( const void *samples, int numSamples );
    int readAudio( void *samples, int numSamples );
    bool processEvent( struct iaxc_event_struct *e );

public slots:
    void serviceIaxClient();
    void updateRegistrationConfig();
    void updateCallerIdConfig();

private slots:
    void retryAudioOpen();

private:
    bool initialized;
    QAudioInput *audioInput;
    QAudioOutput *audioOutput;
    QTimer *timer;
    int retryCount;
    int interval;
    QTimer *iaxTimer;

    IaxNetworkRegistration *netReg() const;
};

class IaxTelephonyServiceQCop : public QtopiaAbstractService
{
    Q_OBJECT

public:
    IaxTelephonyServiceQCop( QObject *parent = 0 );
    ~IaxTelephonyServiceQCop();

public slots:
    void start();
    void stop();

private:
    IaxTelephonyService *service;
};

#endif
