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

#ifndef RADIOBAND_H
#define RADIOBAND_H

#include <QObject>

class RadioBandPrivate;

class RadioBand : public QObject
{
    Q_OBJECT
public:
    typedef qint64 Frequency;

    RadioBand( const QString& name, QObject *parent = 0 );
    RadioBand( const QString& name, RadioBand::Frequency low,
               RadioBand::Frequency high, QObject *parent = 0 );
    ~RadioBand();

    static QString standardBandForFrequency( RadioBand::Frequency freq );

    virtual bool active() const = 0;
    virtual bool muted() const = 0;
    virtual RadioBand::Frequency frequency() const = 0;
    virtual bool signal() const = 0;
    virtual bool stereo() const = 0;
    virtual bool rds() const = 0;
    virtual bool signalDetectable() const = 0;
    virtual int volume() const = 0;
    virtual bool speakerActive() const;
    virtual bool speakerPresent() const;

    QString name() const;
    RadioBand::Frequency lowFrequency() const;
    RadioBand::Frequency highFrequency() const;
    RadioBand::Frequency scanStep() const;
    RadioBand::Frequency scanOffStationStep() const;
    int scanWaitTime() const;
    bool frequencyIsChannelNumber() const;

public slots:
    virtual void setActive( bool value ) = 0;
    virtual void setMuted( bool value ) = 0;
    virtual void setFrequency( RadioBand::Frequency value ) = 0;
    virtual void adjustVolume( int diff ) = 0;
    virtual void setSpeakerActive( bool value );

protected:
    void setLowFrequency( RadioBand::Frequency value );
    void setHighFrequency( RadioBand::Frequency value );
    void setScanStep( RadioBand::Frequency value );
    void setScanOffStationStep( RadioBand::Frequency value );
    void setScanWaitTime( int value );
    void setFrequencyIsChannelNumber( bool value );

private:
    RadioBandPrivate *d;
};

#endif
