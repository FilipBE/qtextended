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

#ifndef RADIOBANDDUMMY_H
#define RADIOBANDDUMMY_H

#include "radioband.h"
#include <QList>

class RadioBandDummy : public RadioBand
{
    Q_OBJECT
public:
    RadioBandDummy( const QString& name, RadioBand::Frequency low,
                    RadioBand::Frequency high, bool isXm,
                    QList<RadioBand::Frequency>& stations,
                    QObject *parent = 0 );
    ~RadioBandDummy();

    static void createBands( QList<RadioBand *>& bands, QObject *parent = 0 );

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
    bool _active;
    bool _muted;
    RadioBand::Frequency _frequency;
    QList<RadioBand::Frequency> stations;
    static int _volume;
};

#endif
