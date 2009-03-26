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

#include <QList>
#include <QStringList>
#include <QSet>
#include <QSettings>
#include <QValueSpaceObject>
#include <QAudioStateInfo>
#include <QAudioStateManager>
#include <QAudioStateConfiguration>
#include <QDebug>

#include <qtopialog.h>

#include "domainmanager.h"


namespace mediaserver
{

// {{{ MediaDomainMonitor
class MediaDomainMonitor : public QObject
{
    Q_OBJECT

public:
    MediaDomainMonitor(QStringList const& available, QObject* parent);

public slots:
    void domainStatusChange(QStringList const& activeDomains, QStringList const& inactiveDomains);

private:
    QValueSpaceObject*  status;
};

MediaDomainMonitor::MediaDomainMonitor(QStringList const& available, QObject* parent):
    QObject(parent)
{
    status = new QValueSpaceObject("/Media/Domains", this);
    status->setAttribute("Available", available);
}

void MediaDomainMonitor::domainStatusChange
(
 QStringList const& activeDomains,
 QStringList const& inactiveDomains
)
{
    status->setAttribute("ActiveDomains", activeDomains);
    status->setAttribute("InactiveDomains", inactiveDomains);
}
// }}}

// {{{ DomainManagerPrivate
class DomainManagerPrivate : public QObject
{
    Q_OBJECT

public:
    bool                            haveAudioDomains;
    QMap<QString, int>              availableDomains;
    QMap<QString, int>              activationCount;
    QString                         activeDomain;
    QSet<QString>                   inactiveDomains;
    QAudioStateManager*             audioStateManager;
    QAudioStateConfiguration*       audioStateConfiguration;
    DomainManager*                  domainManager;

    bool activateDomain(QString const& name, bool force);
    void deactivateDomain(QString const& name);

private slots:
    void configurationInitialized();
    void stateChanged(QAudioStateInfo const& info, QAudio::AudioCapability caps);
};

bool DomainManagerPrivate::activateDomain(QString const& name, bool force)
{
    if (activeDomain != name)
    {
        if (!force) {
            // No mediaserver prioritization
            if (availableDomains.size() == 0)
                return audioStateManager->setDomain(name.toLocal8Bit(), QAudio::OutputOnly);

            // Sanity
            if (availableDomains.find(name) == availableDomains.end())
                return false;

            // Check if current has higher priority
            if (!activeDomain.isNull() && availableDomains[activeDomain] < availableDomains[name])
                return false;

            // try change audio state
            if (haveAudioDomains)
                if (!audioStateManager->setDomain(name.toLocal8Bit(), QAudio::OutputOnly))
                    return false;       // XXX: this && deactivation as transaction
        }

        // Deactivate active
        if (!activeDomain.isNull())
            inactiveDomains.insert(activeDomain);

        activeDomain = name;

        emit domainManager->domainStatusChange(QStringList(activeDomain), inactiveDomains.toList());
    }

    // Keep track of activations
    activationCount[name]++;

    return true;
}

void DomainManagerPrivate::deactivateDomain(QString const& name)
{
    QMap<QString, int>::iterator it = activationCount.find(name);

    // Sanity
    if (it == activationCount.end())
        return;

    if (--it.value() == 0) {
        // remove
        inactiveDomains.remove(name);
        activationCount.erase(it);

        if (activeDomain == name) {
            activeDomain = QString();
            if (inactiveDomains.size() == 0)
                emit domainManager->domainStatusChange(QStringList(), QStringList());
            else {
                QString toActivate;

                // Find highest priority to reactivate
                int targetPriority = 21;
                foreach (QString const& name, inactiveDomains) {
                    int pri = availableDomains[name];
                    if (pri < targetPriority) {
                        targetPriority = pri;
                        toActivate = name;
                    }
                }

                if (!toActivate.isNull()) {
                    inactiveDomains.remove(toActivate);
                    activateDomain(toActivate, false);
                }
            }
        }
        else
            emit domainManager->domainStatusChange(QStringList(activeDomain), inactiveDomains.toList());
    }
}

void DomainManagerPrivate::configurationInitialized()
{
    haveAudioDomains = !audioStateConfiguration->domains().isEmpty();
}

void DomainManagerPrivate::stateChanged(QAudioStateInfo const& info, QAudio::AudioCapability caps)
{
    Q_UNUSED(info);
    Q_UNUSED(caps);
}
// }}}

/*!
    \class mediaserver::DomainManager
    \internal
*/

// {{{ DomainManager
DomainManager::~DomainManager()
{
    delete d->audioStateManager;
    delete d->audioStateConfiguration;
    delete d;
}

bool DomainManager::activateDomain(QString const& name)
{
    return d->activateDomain(name, false);
}

void DomainManager::deactivateDomain(QString const& name)
{
    d->deactivateDomain(name);
}

bool DomainManager::isActiveDomain(QString const& name)
{
    return d->activeDomain == name;
}

QStringList DomainManager::activeDomains()
{
    return d->activeDomain.isNull() ? QStringList() : QStringList(d->activeDomain);
}

QStringList DomainManager::inactiveDomains()
{
    return d->inactiveDomains.toList();
}

DomainManager* DomainManager::instance()
{
    static DomainManager   self;

    return &self;
}

// private
DomainManager::DomainManager():
    d(new DomainManagerPrivate)
{
    d->haveAudioDomains = false;
    d->domainManager = this;

    // Read the domains from configuration
    QSettings    settings("Trolltech", "AudioDomains");

    settings.beginGroup("AvailableDomains");

    foreach (QString const& key, settings.allKeys())
        d->availableDomains.insert(key, settings.value(key).toInt());

    // monitor domain changes in valuespace
    MediaDomainMonitor* monitor = new MediaDomainMonitor(settings.allKeys(), this);
    connect(this, SIGNAL(domainStatusChange(QStringList,QStringList)),
            monitor, SLOT(domainStatusChange(QStringList,QStringList)));

    // audio state handling
    d->audioStateManager = new QAudioStateManager;
    d->audioStateConfiguration = new QAudioStateConfiguration;

    connect(d->audioStateConfiguration, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
            d, SLOT(stateChanged(QAudioStateInfo,QAudio::AudioCapability)));
    connect(d->audioStateConfiguration, SIGNAL(configurationInitialized()),
            d, SLOT(configurationInitialized()));
}
// }}}


}   // ns mediaserver

#include "domainmanager.moc"

