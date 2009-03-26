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

#ifndef RADIOBANDV4L_H
#define RADIOBANDV4L_H

#include "radioband.h"
#include <QList>

class RadioBandV4LVolume;

class RadioBandV4L : public RadioBand
{
    Q_OBJECT
private:
    RadioBandV4L( int fd, const QByteArray& deviceName, int tuner,
                  bool v4l2, bool tunerLow, bool canClose,
                  const QString& name, RadioBand::Frequency low,
                  RadioBand::Frequency high,
                  QObject *parent = 0 );

public:
    ~RadioBandV4L();

    static bool createBands( QList<RadioBand *>& bands, QObject *parent = 0 );

    bool active() const;
    bool muted() const;
    RadioBand::Frequency frequency() const;
    bool signal() const;
    bool stereo() const;
    bool rds() const;
    bool signalDetectable() const;
    int volume() const;

public slots:
    void setActive( bool value );
    void setMuted( bool value );
    void setFrequency( RadioBand::Frequency value );
    void adjustVolume( int diff );

private:
    int _fd;
    int tuner;
    bool v4l2;
    bool tunerLow;
    bool canClose;
    bool _active;
    bool _muted;
    bool _signalDetectable;
    bool muteViaVideo;
    RadioBand::Frequency _frequency;
    QByteArray deviceName;
    RadioBandV4LVolume *volumeControl;

    void updateMute();
    int fd() const;
    void shutdownfd();
};

class RadioBandV4LVolume : public QObject
{
    Q_OBJECT
public:
    RadioBandV4LVolume( QObject *parent = 0 );
    ~RadioBandV4LVolume();

    int volume() const;
    void adjustVolume( int diff );

private:
    int savedVolume;
};

#endif
