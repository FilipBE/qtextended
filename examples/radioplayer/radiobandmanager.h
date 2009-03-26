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

#ifndef RADIOBANDMANAGER_H
#define RADIOBANDMANAGER_H

#include "radioband.h"
#include <QList>

class RadioBandManagerPrivate;

class RadioBandManager : public QObject
{
    Q_OBJECT
public:
    RadioBandManager( QObject *parent = 0 );
    ~RadioBandManager();

    QList<RadioBand *> bands() const;

    int band() const;
    RadioBand::Frequency frequency() const;
    QString bandName() const;
    bool isValid() const { return ( band() >= 0 ); }

    bool muted() const;
    bool stereo() const;
    bool signal() const;
    bool rds() const;
    bool signalDetectable() const;
    int volume() const;
    bool speakerActive() const;
    bool speakerPresent() const;

    bool scanning() const;

    RadioBand *currentBand() const;
    RadioBand *bandFromName( const QString& name ) const;

    static QString formatFrequency( RadioBand::Frequency frequency );
    static bool frequencyIsChannelNumber( RadioBand::Frequency frequency );

public slots:
    void setBand( int value );
    bool setFrequency( RadioBand::Frequency value );
    bool setFrequency( const QString& band, RadioBand::Frequency value );
    void scanForward();
    void scanBackward();
    void scanAll();
    void stopScan();
    void stepForward();
    void stepBackward();
    void setMuted( bool value );
    void adjustVolume( int diff );
    void setSpeakerActive( bool value );

signals:
    void scanProgress( RadioBand::Frequency frequency, int band );
    void scanFoundStation( RadioBand::Frequency frequency, int band );
    void scanStarted();
    void scanStopped();
    void signalChanged();

private:
    void startScan( bool forward, bool all );
    void nextScan();

private slots:
    void scanTimeout();
    void signalTimeout();
    void startSignalCheck( bool firstAfterFreqChange = true );
    void backoffSignalCheck();

private:
    RadioBandManagerPrivate *d;

    int bandFromFrequency( RadioBand::Frequency frequency ) const;
};

#endif
