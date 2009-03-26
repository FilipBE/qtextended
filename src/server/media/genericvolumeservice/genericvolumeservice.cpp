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

#include <qtopiaserverapplication.h>
#include <qtopiaipcenvelope.h>
#include <QTimer>

#include <QAudioMixer>

#include "genericvolumeservice.h"

class GenericVolumeServicePrivate
{
public:

    void sendCurrentVolume()
    {
        QString volume;
        if(m_masterElement)
            volume.setNum((int)currVolume);
        else
            volume.setNum(100);
        QtopiaIpcEnvelope e("QPE/AudioVolumeManager","currentVolume(QString)");
        e << volume;
        m_timer->start(2000);
    };

    int            currVolume;
    QAudioMixer*   m_mixer;
    QAudioElement* m_masterElement;
    QTimer*        m_timer;
    int            m_keyStack;
};

GenericVolumeService::GenericVolumeService():
    QtopiaIpcAdaptor("QPE/AudioVolumeManager/GenericVolumeService")
{
    publishAll(Slots);

    m_d = new GenericVolumeServicePrivate;
    m_d->currVolume = 0;
    m_d->m_keyStack = 0;
    m_d->m_mixer = new QAudioMixer();
    connect(m_d->m_mixer, SIGNAL(audioChanged()), this, SLOT(updateVolume()));

    //Can overide auto detect of master by setting name of master element
    //In config file
    QString master;
    QSettings cfg("Trolltech", "Sound");
    cfg.beginGroup("System");
    master = cfg.value("MasterName","").toString();

    m_d->m_masterElement = 0;

    if(master.isEmpty()) {
        for(int i = 0; i < m_d->m_mixer->elements().size(); ++i) {
            if(m_d->m_mixer->elements().at(i)->isPlayback() &&
                    (m_d->m_masterElement == 0))
                m_d->m_masterElement = m_d->m_mixer->elements().at(i);
        }
        if(m_d->m_masterElement)
            m_d->currVolume = (int)(m_d->m_masterElement->getValue()*100/m_d->m_masterElement->getMaximum());
        else
            m_d->currVolume = 100;
    } else {
        for(int i = 0; i < m_d->m_mixer->elements().size(); ++i) {
            if(m_d->m_mixer->elements().at(i)->getName().startsWith(master)) {
                if(m_d->m_mixer->elements().at(i)->isPlayback() &&
                        (!m_d->m_masterElement) &&
                        (m_d->m_mixer->elements().at(i)->getMaximum()!=0)) {
                    m_d->m_masterElement = m_d->m_mixer->elements().at(i);
                    break;
                }
            }
        }
        if(!m_d->m_masterElement) {
            for(int i = 0; i < m_d->m_mixer->elements().size(); ++i) {
                if(m_d->m_mixer->elements().at(i)->isPlayback() &&
                        (m_d->m_mixer->elements().at(i)->getMaximum()!=0)) {
                    m_d->m_masterElement = m_d->m_mixer->elements().at(i);
                    break;
                }
            }
        }
        if(!m_d->m_masterElement)
            m_d->currVolume = 100;
        else
            m_d->currVolume = (int)(m_d->m_masterElement->getValue()*100/m_d->m_masterElement->getMaximum());
    }
    if(m_d->m_masterElement) {
        if(cfg.value("MasterControl",false).toBool() == true) {
            m_d->currVolume = cfg.value("MasterVolume",100).toInt();
            m_d->m_masterElement->setValue((int)(m_d->currVolume*m_d->m_masterElement->getMaximum()/100));
        }
    }

    QTimer::singleShot(0, this, SLOT(registerService()));

    m_d->m_timer = new QTimer(this);
    connect(m_d->m_timer,SIGNAL(timeout()),this,SLOT(timeout()));
}

GenericVolumeService::~GenericVolumeService()
{
    delete m_d;
}

//public slots:
void GenericVolumeService::setVolume(int volume)
{
    adjustVolume(volume, volume, Absolute);
}

void GenericVolumeService::setVolume(int leftChannel, int rightChannel)
{
    adjustVolume(leftChannel, rightChannel, Absolute);
}

void GenericVolumeService::increaseVolume(int increment)
{
    adjustVolume(increment, increment, Relative);
    m_d->sendCurrentVolume();
}

void GenericVolumeService::decreaseVolume(int decrement)
{
    decrement *= -1;

    adjustVolume(decrement, decrement, Relative);
    m_d->sendCurrentVolume();
}

void GenericVolumeService::setMute(bool)
{
}

void GenericVolumeService::registerService()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "registerHandler(QString,QString)");

    e << QString("Generic") << QString("QPE/AudioVolumeManager/GenericVolumeService");

    QTimer::singleShot(0, this, SLOT(setCallDomain()));
}

void GenericVolumeService::setCallDomain()
{
    QtopiaIpcEnvelope   e("QPE/AudioVolumeManager", "setActiveDomain(QString)");

    e << QString("Generic");
}

void GenericVolumeService::adjustVolume(int leftChannel, int rightChannel, AdjustType adjust)
{
    m_d->m_keyStack++;
    if(m_d->m_keyStack > 3)m_d->m_keyStack=3;

    if(!m_d->m_masterElement) return;

    if(adjust == Relative)
        m_d->currVolume = m_d->currVolume+(leftChannel+rightChannel)/2*m_d->m_keyStack;
    else
        m_d->currVolume = (leftChannel+rightChannel)/2*m_d->m_keyStack;

    if(m_d->currVolume < 0) {
        m_d->currVolume=0;
        m_d->m_keyStack=0;
        m_d->m_masterElement->setMute(true);
    } else if(m_d->m_masterElement->isMuted())
        m_d->m_masterElement->setMute(false);

    if(m_d->currVolume > 100) {
        m_d->currVolume=100;
        m_d->m_keyStack=0;
    }
    if((m_d->m_masterElement->getValue()*100/m_d->m_masterElement->getMaximum()) != m_d->currVolume)
        m_d->m_masterElement->setValue((int)(m_d->currVolume*m_d->m_masterElement->getMaximum()/100));
}

void GenericVolumeService::updateVolume()
{
    if(!m_d->m_masterElement) return;

    int value = (int)(m_d->m_masterElement->getValue()*100/m_d->m_masterElement->getMaximum());
    if(value != m_d->currVolume) {
        m_d->currVolume = value;
        m_d->sendCurrentVolume();
    }
}

void GenericVolumeService::timeout()
{
    m_d->m_keyStack = 0;

    m_d->m_timer->stop();
    if(!m_d->m_masterElement) return;

    QSettings cfg("Trolltech", "Sound");
    cfg.beginGroup("System");

    if(cfg.value("MasterControl",false).toBool() == true) {
        cfg.setValue("MasterVolume",m_d->currVolume);
        cfg.setValue("MasterName",m_d->m_masterElement->getName());
    } else {
        cfg.setValue("MasterControl",false);
    }
}

QTOPIA_TASK(GenericVolumeService, GenericVolumeService);

