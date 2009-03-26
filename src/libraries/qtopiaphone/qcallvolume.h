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

#ifndef QCALLVOLUME_H
#define QCALLVOLUME_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QCallVolume : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(int speakerVolume READ speakerVolume WRITE setSpeakerVolume);
    Q_PROPERTY(int minimumSpeakerVolume READ minimumSpeakerVolume);
    Q_PROPERTY(int maximumSpeakerVolume READ maximumSpeakerVolume);
    Q_PROPERTY(int microphoneVolume READ microphoneVolume WRITE setMicrophoneVolume);
    Q_PROPERTY(int minimumMicrophoneVolume READ minimumMicrophoneVolume);
    Q_PROPERTY(int maximumMicrophoneVolume READ maximumMicrophoneVolume);
public:
    explicit QCallVolume( const QString& service = QString(),
                          QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QCallVolume();

    int speakerVolume() const;
    int minimumSpeakerVolume() const;
    int maximumSpeakerVolume() const;

    int microphoneVolume() const;
    int minimumMicrophoneVolume() const;
    int maximumMicrophoneVolume() const;

signals:
    void speakerVolumeChanged(int volume);
    void microphoneVolumeChanged(int volume);

public slots:
    virtual void setSpeakerVolume( int volume );
    virtual void setMicrophoneVolume( int volume );
};

#endif
