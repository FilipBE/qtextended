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

#ifndef QAUDIOSTATE_H
#define QAUDIOSTATE_H

#include <qaudionamespace.h>
#include <QObject>

class QAudioStateInfo;

class QTOPIAAUDIO_EXPORT QAudioState : public QObject
{
    Q_OBJECT

public:
    explicit QAudioState(QObject *parent = 0);
    virtual ~QAudioState();

    virtual QAudioStateInfo info() const = 0;
    virtual QAudio::AudioCapabilities capabilities() const = 0;

    virtual bool isAvailable() const = 0;
    virtual bool enter(QAudio::AudioCapability capability) = 0;
    virtual bool leave() = 0;

signals:
    void availabilityChanged(bool available);
    void doNotUseHint();
    void useHint();
};

#endif
