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

#ifndef CALLAUDIOHANDLER_H
#define CALLAUDIOHANDLER_H

#include "abstractaudiohandler.h"
#include <QMenu>
#include <QAudioStateInfo>
#include <QAudio>

class QAction;
class QActionGroup;
class CallAudioHandler : public AbstractAudioHandler
{
    Q_OBJECT
public:
    CallAudioHandler(QObject *parent = 0);
    ~CallAudioHandler();

    QByteArray audioType();

    void addOptionsToMenu(QMenu* menu);

    void activateAudio(bool active);
    void transferAudio(const QByteArray& profile);

protected:
    void initialize();

private slots:
    void actionTriggered(QAction *action);
    void availabilityChanged();
    void currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability);

private:
    QHash<QAction *, QAudioStateInfo> audioModes;
    QActionGroup *actions;
    bool audioActive;
};

QTOPIA_TASK_INTERFACE(CallAudioHandler);

#endif
