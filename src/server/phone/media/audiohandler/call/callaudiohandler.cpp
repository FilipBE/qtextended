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

#include "callaudiohandler.h"

#include <QtopiaIpcEnvelope>
#include <QAudioStateConfiguration>


CallAudioHandler::CallAudioHandler(QObject *parent)
    : AbstractAudioHandler(parent), audioActive(false)
{
    connect(audioConf, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            this, SLOT(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)));
    connect(audioConf, SIGNAL(availabilityChanged()),
            this, SLOT(availabilityChanged()));

    actions = new QActionGroup(this);

    connect(actions, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

CallAudioHandler::~CallAudioHandler()
{
}

void CallAudioHandler::initialize()
{
    //audio mode initialization is delayed until the modes are actually discovered by the media framework
    QSet<QAudioStateInfo> states = audioConf->states("Phone");

    foreach (QAudioStateInfo state, states) {
        QAction *action = new QAction(state.displayName(), this);
        action->setVisible(false);
        action->setCheckable(true);
        actions->addAction(action);
        audioModes.insert(action, state);
    }
}

void CallAudioHandler::addOptionsToMenu( QMenu* menu )
{
    foreach (QAction *action, audioModes.keys()) {
	menu->addAction(action);
    }
}

void CallAudioHandler::actionTriggered(QAction *action)
{
    if (!audioActive)
        return;

    if (!audioModes.contains(action)) {
        qWarning("CallAudioHandler::actionTriggered - Invalid action!");
        return;
    }

    ipcAdaptor->send("setProfile(QByteArray)", audioModes[action].profile());
}

void CallAudioHandler::activateAudio(bool enableAudio)
{
    if (enableAudio == audioActive)
        return;

    audioActive = enableAudio;

    if (audioActive) {
        QByteArray domain("Phone");
        int capability = static_cast<int>(QAudio::OutputOnly);
        ipcAdaptor->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
        e << QString("Phone");

        availabilityChanged();

        // might start with audio profile other than handset
        if ( !audioProfile().isEmpty() )
            transferAudio(audioProfile());
    }
    else {
        foreach (QAction *action, actions->actions()) {
            action->setChecked(false);
            action->setVisible(false);
        }

        //TODO: This needs to be fixed  up later to send the release
        // domain message instead
        QByteArray domain("Media");
        int capability = static_cast<int>(QAudio::OutputOnly);
        ipcAdaptor->send("setDomain(QByteArray,int)", domain, capability);

        QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "resetActiveDomain(QString)");
        e << QString("Phone");

        setAudioProfile( QByteArray() );
    }
}

void CallAudioHandler::transferAudio(const QByteArray& profile)
{
    if (profile.isEmpty())
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = audioModes.constBegin();
    while (it != audioModes.constEnd()) {
        if ( QString(audioModes[it.key()].profile()).contains( profile, Qt::CaseInsensitive ) ) {
            setAudioProfile( profile );
            ipcAdaptor->send("setProfile(QByteArray)", audioModes[it.key()].profile());
            break;
        }
        ++it;
    }
}

void CallAudioHandler::availabilityChanged()
{
    if (!audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = audioModes.constBegin();
    while (it != audioModes.constEnd()) {
        bool vis = audioConf->isStateAvailable(audioModes[it.key()]);
        it.key()->setVisible(vis);
        ++it;
    }
}

/*!
    \reimp
*/
QByteArray CallAudioHandler::audioType()
{
    return QByteArray("CallAudio");
}

void CallAudioHandler::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability)
{
    if (!audioActive)
        return;

    QHash<QAction *, QAudioStateInfo>::const_iterator it = audioModes.constBegin();
    while (it != audioModes.constEnd()) {
        if (it.value() == state) {
            it.key()->setChecked(true);
            return;
        }
        ++it;
    }
}

QTOPIA_TASK_PROVIDES(CallAudioHandler, AbstractAudioHandler);
QTOPIA_TASK_PROVIDES(CallAudioHandler, CallAudioHandler);
QTOPIA_TASK(CallAudioHandler, CallAudioHandler);
