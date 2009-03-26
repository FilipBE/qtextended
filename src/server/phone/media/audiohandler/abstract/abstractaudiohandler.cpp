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

#include "abstractaudiohandler.h"

#include <QAudioStateConfiguration>
#include <QtopiaIpcAdaptor>

#include <QTimer>

class AbstractAudioHandlerPrivate
{
public:
    void _q_audioConfInitialized()
    {
        q->initialize();
        isInitialized = true;
        emit q->initialized();
    }

    AbstractAudioHandler* q;
    QByteArray audioProfile;
    bool isInitialized;
};


AbstractAudioHandler::AbstractAudioHandler(QObject* parent)
    : QObject( parent )
{
    d = new AbstractAudioHandlerPrivate();
    d->q = this;
    d->isInitialized = false;

    ipcAdaptor = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);
    audioConf = new QAudioStateConfiguration(this);

    if (audioConf->isInitialized())
        QTimer::singleShot(0, this, SLOT(_q_audioConfInitialized()));
    else 
        QObject::connect(audioConf, SIGNAL(configurationInitialized()),
                         this, SLOT(_q_audioConfInitialized()));
}

AbstractAudioHandler::~AbstractAudioHandler()
{
    delete d;
}


QByteArray AbstractAudioHandler::audioProfile() const
{
    return d->audioProfile;
}

void AbstractAudioHandler::setAudioProfile( const QByteArray& newAudioProfile )
{
    d->audioProfile = newAudioProfile;
}

QByteArray AbstractAudioHandler::profileForKey( int key )
{
    QByteArray profile;
    switch (key) {
        case Qtopia::Key_Speaker:
            profile = "speaker"; break;
        case Qtopia::Key_Hook:
            profile = "handset"; break;
        case Qtopia::Key_Headset:
            profile = "headset"; break;
        default:
            qWarning() << "Unknown dialer key pressed. Call audio routing may not work.";
            break;
    }
    return profile;
}

bool AbstractAudioHandler::isInitialized() const
{
    return audioConf->isInitialized() && d->isInitialized;
}

AbstractAudioHandler* AbstractAudioHandler::audioHandler(const QByteArray& audioType)
{
    static QList<AbstractAudioHandler*> list = qtopiaTasks<AbstractAudioHandler>();
    foreach(AbstractAudioHandler* handler, list) {
        if (handler->audioType() == audioType)
            return handler;
    }

    return 0;
}

#include "moc_abstractaudiohandler.cpp"
