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

#include "qaudiostatemanagerservice_p.h"

#include <QAudioStateManager>
#include <qaudionamespace.h>

struct QAudioStateManagerServicePrivate
{
    QAudioStateManager *manager;
};

/*!
    \class QAudioStateManagerService
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
    \internal
*/

/*

    Constructs a new Audio State Manager Service.  The \a parent
    parameter specifies the Audio State Manager for this service.

    The service is created on QPE/AudioStateManager.
*/
QAudioStateManagerService::QAudioStateManagerService(QAudioStateManager *parent)
    : QtopiaIpcAdaptor("QPE/AudioStateManager", parent)
{
    m_data = new QAudioStateManagerServicePrivate;
    m_data->manager = parent;

    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

/*
    Destroys the Audio State Manager Service.
*/
QAudioStateManagerService::~QAudioStateManagerService()
{

}

/*
    Tries to set the Audio State to a state with \a profile.  If
    the state succeeds, the current state will be reported by
    QAudioDeviceConfiguration.  If the setting of the profile
    failed, then the profileFailed() signal will be emitted
*/
void QAudioStateManagerService::setProfile(const QByteArray &profile)
{
    if (!m_data->manager->setProfile(profile))
        emit profileFailed(profile);
}

void QAudioStateManagerService::setDomain(const QByteArray &domain, int capability)
{
    if (!m_data->manager->setDomain(domain, static_cast<QAudio::AudioCapability>(capability)))
        emit domainFailed(domain, capability);
}
