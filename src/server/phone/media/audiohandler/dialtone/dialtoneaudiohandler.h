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

#ifndef DIALTONEAUDIOHANDLER_H
#define DIALTONEAUDIOHANDLER_H

#include "abstractaudiohandler.h"
#include "qtopiaserverapplication.h"

#include <QHash>

#include <QAudioStateInfo>
#include <QAudio>

class DtmfAudio;
class DialtoneAudioHandler : public AbstractAudioHandler
{
    Q_OBJECT
public:
    DialtoneAudioHandler( QObject *parent = 0 );
    ~DialtoneAudioHandler();

    QByteArray audioType();

    void activateAudio(bool active);
    void transferAudio(const QByteArray& profile);

protected:
    void initialize();

private slots:
    void currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability);

private:
    QHash<QByteArray, QAudioStateInfo> keyAudioMap;
    DtmfAudio *dtmf;
    bool audioActive;
};


#endif
