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

#include "qaudiostatemanager.h"

// Qt
#include <qglobal.h>
#include <QList>
#include <QString>
#include <QSet>
#include <QHash>
#include <QStringList>
#include <QDebug>
#include <QByteArray>

// Qtopia
#include <QPluginManager>
#include <QValueSpaceObject>
#include <QAudioState>
#include <QAudioStatePlugin>
#include <qaudionamespace.h>
#include "qaudiostatemanagerservice_p.h"
#include <QAudioStateInfo>
#include <qtopialog.h>

#include <climits>

class AudioDeviceConfiguration
{
public:
    AudioDeviceConfiguration();
    ~AudioDeviceConfiguration();

    QSet<QString> domains() const;

    QSet<QString> profiles(const QByteArray &domain) const;

    QSet<QAudioStateInfo> states(const QByteArray &domain) const;
    QSet<QAudioStateInfo> states() const;
    QAudioState *state(const QAudioStateInfo &info) const;

    bool addState(QAudioState *state);
    bool removeState(QAudioState *state);

    void clear();

    QAudioState *findHighestPriority(const QByteArray &domain,
                                     QAudio::AudioCapability capability) const;
    QAudioState *findHighestPriorityButNot(QAudioState *state,
                                           const QByteArray &domain,
                                           QAudio::AudioCapability capability) const;
    QAudioState *findState(const QByteArray &domain, const QByteArray &profile) const;
    QAudioStateInfo findStateInfo(const QByteArray &domain, const QByteArray &profile) const;

private:
    QHash<QAudioStateInfo, QAudioState *> m_states;
};

AudioDeviceConfiguration::AudioDeviceConfiguration()
{
}

AudioDeviceConfiguration::~AudioDeviceConfiguration()
{

}

QSet<QString> AudioDeviceConfiguration::domains() const
{
    QSet<QString> ret;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        ret.insert(i.key().domain());
        ++i;
    }

    return ret;
}

QSet<QAudioStateInfo> AudioDeviceConfiguration::states() const
{
    QSet<QAudioStateInfo> ret;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        ret.insert(i.key());
        ++i;
    }

    return ret;
}

QSet<QAudioStateInfo> AudioDeviceConfiguration::states(const QByteArray &domain) const
{
    QSet<QAudioStateInfo> ret;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.key().domain() == domain)
            ret.insert(i.key());
        ++i;
    }

    return ret;
}

QAudioState *AudioDeviceConfiguration::state(const QAudioStateInfo &info) const
{
    QHash<QAudioStateInfo, QAudioState *>::const_iterator it = m_states.constFind(info);
    if (it == m_states.end()) {
        return 0;
    }

    return it.value();
}

QAudioState *AudioDeviceConfiguration::findHighestPriority(const QByteArray &domain,
                                                              QAudio::AudioCapability capability) const
{
    QAudioState *ret = 0;
    int curPri = INT_MAX;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.key().domain() == domain &&
            i.value()->isAvailable() &&
            (i.value()->capabilities() & capability))
        {
            // Highest priority is lowest number
            if (!ret || (curPri > i.key().priority())) {
                ret = i.value();
                curPri = i.key().priority();
            }
        }
        ++i;
    }

    return ret;
}

QAudioState *AudioDeviceConfiguration::findHighestPriorityButNot(QAudioState *state,
        const QByteArray &domain,
        QAudio::AudioCapability capability) const
{
    QAudioState *ret = 0;
    int curPri = INT_MAX;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.value() != state) {
            if (i.key().domain() == domain &&
                i.value()->isAvailable() &&
                (i.value()->capabilities() & capability))
            {
                // Highest priority is lowest number
                if (!ret || (curPri > i.key().priority())) {
                    ret = i.value();
                    curPri = i.key().priority();
                }
            }
        }

        ++i;
    }

    return ret;
}

QAudioStateInfo AudioDeviceConfiguration::findStateInfo(const QByteArray &domain,
                                                        const QByteArray &profile) const
{
    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.key().domain() == domain && i.key().profile() == profile)
            return i.key();
        ++i;
    }

    return QAudioStateInfo();
}

QAudioState *AudioDeviceConfiguration::findState(const QByteArray &domain,
                                                 const QByteArray &profile) const
{
    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.key().domain() == domain && i.key().profile() == profile)
            return i.value();
        ++i;
    }

    return 0;
}

QSet<QString> AudioDeviceConfiguration::profiles(const QByteArray &domain) const
{
    QSet<QString> ret;

    QHash<QAudioStateInfo, QAudioState *>::const_iterator i = m_states.constBegin();
    while (i != m_states.constEnd()) {
        if (i.key().domain() == domain)
            ret.insert(i.key().profile());
        ++i;
    }

    return ret;
}

bool AudioDeviceConfiguration::addState(QAudioState *state)
{
    if (m_states.contains(state->info()))
        return false;

    m_states.insert(state->info(), state);
    return true;
}

bool AudioDeviceConfiguration::removeState(QAudioState *state)
{
    return m_states.remove(state->info());
}

void AudioDeviceConfiguration::clear()
{
    m_states.clear();
}

class QAudioStateManagerPrivate : public QObject
{
    Q_OBJECT
public:
    QAudioStateManagerPrivate();
    ~QAudioStateManagerPrivate();

    void publishConfiguration();

    bool setDefaultState(const QByteArray &domain, QAudio::AudioCapability capability);
    bool setHighestPriorityButNot(QAudioState *state, const QByteArray &domain,
                                  QAudio::AudioCapability capability);
    bool setProfile(const QByteArray &profile);
    bool setState(const QAudioStateInfo &state, QAudio::AudioCapability capability);
    bool setState(QAudioState *state, QAudio::AudioCapability capability);

    bool isValid() const;

private slots:
    void stateChanged(QAudioState *newState, QAudio::AudioCapability capability);
    void stateAvailable(bool avail);
    void doNotUseHint();
    void useHint();

private:
    QPluginManager *m_loader;
    AudioDeviceConfiguration *m_conf;
    QList<QAudioStatePlugin *> m_interfaces;
    QValueSpaceObject *m_valueSpace;
    QAudioState *m_current;
    QAudioState *m_prev;
    QAudio::AudioCapability m_currentCap;
};


QAudioStateManagerPrivate::QAudioStateManagerPrivate()
    : m_loader(0), m_conf(0), m_valueSpace(0), m_current(0), m_prev(0)
{
    m_loader = new QPluginManager("audiohardware");

    if (!m_loader) {
        return;
    }

    m_conf = new AudioDeviceConfiguration;

    QByteArray p("/Hardware/Audio/Configuration");
    m_valueSpace = new QValueSpaceObject(p);
    m_valueSpace->setAttribute("Initialized", false);

    foreach (QString name, m_loader->list()) {
        QObject *plugin = m_loader->instance(name);
        if ( !plugin ){
            continue;
        }

        QAudioStatePlugin *iface = 0;
        iface = qobject_cast<QAudioStatePlugin*>(plugin);
        if (iface) {
            m_interfaces.push_back(iface);
            QList<QAudioState *> states = iface->statesProvided();

            foreach (QAudioState *state, states) {
                QObject::connect(state, SIGNAL(availabilityChanged(bool)),
                                 this, SLOT(stateAvailable(bool)));
                QObject::connect(state, SIGNAL(useHint()),
                                 this, SLOT(useHint()));
                QObject::connect(state, SIGNAL(doNotUseHint()),
                                 this, SLOT(doNotUseHint()));
                m_conf->addState(state);
            }
        }
        else {
            delete iface;
        }
    }

    if (m_interfaces.size() == 0) {
        qWarning() << "AudioStateManager - No interfaces found, no configuration created";
        delete m_conf;
        m_conf = 0;

        delete m_loader;
        m_loader = 0;

        m_valueSpace->setAttribute("Initialized", true);
    }

}

QAudioStateManagerPrivate::~QAudioStateManagerPrivate()
{
    delete m_valueSpace;

    foreach (QAudioStatePlugin *iface, m_interfaces) {
        delete iface;
    }

    delete m_loader;
    delete m_conf;
}

bool QAudioStateManagerPrivate::isValid() const
{
    return m_conf != NULL;
}

bool QAudioStateManagerPrivate::setState(const QAudioStateInfo &info, QAudio::AudioCapability capability)
{
    QAudioState *state = m_conf->state(info);
    if (!state)
        return false;

    return setState(state, capability);
}

bool QAudioStateManagerPrivate::setState(QAudioState *state, QAudio::AudioCapability capability)
{
    if (!(state->capabilities() & capability)) {
        qLog(AudioState) << "Warning: State " << state->info() << "does not have capability:" << capability;
        return false;
    }

    if (m_current) {
        if (!m_current->leave()) {
            qLog(AudioState) << "Warning: Could not leave current state!";
            return false;
        }
    }

    bool ret = state->enter(capability);

    if (!ret)
        qLog(AudioState) << "Warning: Unable to enter state:" << state->info();

    if (ret)
        stateChanged(state, capability);

    return ret;
}

bool QAudioStateManagerPrivate::setProfile(const QByteArray &profile)
{
    qLog(AudioState) << "Trying to set profile to:" << profile;

    // No domain active
    if (!m_current)
        return false;

    QAudioState *state = m_conf->findState(m_current->info().domain(), profile);
    if (state && state == m_current)
        return true;

    if (state && state->isAvailable())
        return setState(state, m_currentCap);

    return false;
}

bool QAudioStateManagerPrivate::setDefaultState(const QByteArray &domain,
        QAudio::AudioCapability capability)
{
    qLog(AudioState) << "Finding highest priority state for domain:" << domain << "and capability:"
            << capability;
    QAudioState *defState = m_conf->findHighestPriority(domain, capability);

    if (!defState)
        return false;

    qLog(AudioState) << "AudioState found was:" << defState->info();

    return setState(defState, capability);
}

bool QAudioStateManagerPrivate::setHighestPriorityButNot(QAudioState *state, const QByteArray &domain, QAudio::AudioCapability capability)
{
    qLog(AudioState) << "Finding highest priority state for domain:" << domain << "and capability:" << capability << "that is not:" << state->info();
    QAudioState *defState = m_conf->findHighestPriorityButNot(state, domain, capability);

    if (!defState)
        return false;

    qLog(AudioState) << "AudioState found was:" << defState->info();

    return setState(defState, capability);
}

void QAudioStateManagerPrivate::stateChanged(QAudioState *newState, QAudio::AudioCapability capability)
{
    qLog(AudioState) << "QAudioStateManager::stateChanged";
    if (m_current)
        qLog(AudioState) << "From State:" << m_current->info() << m_current->capabilities();
    qLog(AudioState) << "To State:" << newState->info() << newState->capabilities();

    m_prev = m_current;
    m_current = newState;
    m_currentCap = capability;

    QAudioStateInfo info = m_current->info();

    m_valueSpace->setAttribute("Current/Domain", info.domain());
    m_valueSpace->setAttribute("Current/Profile", info.profile());
    m_valueSpace->setAttribute("Current/Capability", static_cast<int>(m_currentCap));
    m_valueSpace->setAttribute("Current/DisplayName", info.displayName());
    m_valueSpace->setAttribute("Current/Priority", info.priority());
}

void QAudioStateManagerPrivate::stateAvailable(bool avail)
{
    qLog(AudioState) << "AudioStateManager::notifyStateAvailable:";
    QAudioState *state = qobject_cast<QAudioState *>(sender());

    if (!state) {
        qWarning("Unknown object sent availability changed signal!");
        return;
    }

    qLog(AudioState) << "State availability for:" << state->info() << "changed, now is:" << avail;

    QAudioStateInfo info = state->info();

    QByteArray relPath("Domains/");
    relPath.append(info.domain());
    relPath.append('/');
    relPath.append(info.profile());
    relPath.append('/');
    relPath.append("Available");

    m_valueSpace->setAttribute(relPath, avail);

    if (!m_current)
        return;

    // Case where the current state became unavailable
    // Go back to previous state, or the highest priority
    // state
    if (state == m_current && !avail) {
        if (m_prev && (m_prev->info().domain() == m_current->info().domain()) &&
             (m_prev->capabilities() & m_currentCap) && (m_prev->isAvailable())) {
            setState(m_prev, m_currentCap);
        } else {
            setDefaultState(m_current->info().domain(), m_currentCap);
        }
    }

    // The priority of the new state is higher than
    // priority of the current state, and the higher
    // priority state has just became available
    if ((info.domain() == m_current->info().domain()) &&
         (info.priority() < m_current->info().priority()) && avail &&
         (state->capabilities() & m_currentCap)) {
        setState(state, m_currentCap);
    }
}

void QAudioStateManagerPrivate::doNotUseHint()
{
    qLog(AudioState) << "AudioStateManager::doNotUseHint:";
    QAudioState *state = qobject_cast<QAudioState *>(sender());

    if (!state) {
        qWarning("Unknown object sent doNotUseHint signal!");
        return;
    }

    qLog(AudioState) << "State" << state->info() << "sent the do not use hint";

    if (state != m_current) {
        qLog(AudioState) << "State sent the do not use hint, but it is not current, ignoring";
        return;
    }

    if (m_prev && (m_prev->info().domain() == m_current->info().domain()) &&
            (m_prev->capabilities() & m_currentCap) && (m_prev->isAvailable())) {
        qLog(AudioState) << "Setting the Audio State to the previous state";
        setState(m_prev, m_currentCap);
    } else {
        qLog(AudioState) << "Finding highest priority state";
        setHighestPriorityButNot(m_current, m_current->info().domain(), m_currentCap);
    }
}

void QAudioStateManagerPrivate::useHint()
{
    qLog(AudioState) << "AudioStateManager::useHint:";

    if (!m_current)
        return;

    QAudioState *state = qobject_cast<QAudioState *>(sender());

    if (!state) {
        qWarning("Unknown object sent useHint signal!");
        return;
    }

    qLog(AudioState) << "State" << state->info() << "sent the use hint";

    if (state == m_current) {
        qWarning() << "State is already current!";
        return;
    }

    if (!state->isAvailable()) {
        qWarning() << "State is not available!";
        return;
    }

    if (m_current->info().domain() != state->info().domain()) {
        qLog(AudioState) << "Current domain does not match the the state that sent the useHint, ignoring";
        return;
    }

    setState(state, m_currentCap);
}

void QAudioStateManagerPrivate::publishConfiguration()
{
    foreach (QAudioStateInfo info, m_conf->states()) {
        QByteArray relPath("Domains/");
        relPath.append(info.domain());
        relPath.append('/');
        relPath.append(info.profile());
        relPath.append('/');

        QAudioState *st = m_conf->state(info);

        m_valueSpace->setAttribute(relPath + "Display", info.displayName());
        m_valueSpace->setAttribute(relPath + "Available", st->isAvailable());
        m_valueSpace->setAttribute(relPath + "Capabilities",
                                   static_cast<unsigned int>(st->capabilities()));
        m_valueSpace->setAttribute(relPath + "Priority", info.priority());
    }

    m_valueSpace->setAttribute("Initialized", true);
}

/*!
    \class QAudioStateManager
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
    \brief The QAudioStateManager class manages QAudioStatePlugin objects.

    This class performs basic management of the audio device
    and various audio states.  It is responsible for loading
    all QAudioStatePlugin objects and obtaining all
    available audio states described by the QAudioState object(s).

    You generally do not need to use this class unless you are writing
    your own media server responsible for managing the audio device.

    \sa QAudioState, QAudioStatePlugin
  */

/*!
    Construct a new QAudioStateManager object with \a parent
*/
QAudioStateManager::QAudioStateManager(QObject *parent)
    : QObject(parent)
{
    m_data = new QAudioStateManagerPrivate;

    if (!m_data->isValid()) {
// Lets not scare anyone
//        qWarning("No Audio States found!  Media functionality will probably NOT work correctly!");
        delete m_data;
        m_data = 0;
        return;
    }

    // Now Publish the Configuration to ValueSpace
    m_data->publishConfiguration();

    //TODO: Set the current state to Media by default
    //Might need to revisit this to see whether this is necessary
    QValueSpaceObject::sync();

    m_data->setDefaultState("Media", QAudio::OutputOnly);

    new QAudioStateManagerService(this);
}

/*!
    Destroys a QAudioStateManager object.
*/
QAudioStateManager::~QAudioStateManager()
{
    delete m_data;
}

/*!
    Ask the manager to set a particular state.  The state to
    set is given by \a state.  The capability to set is
    given by \a capability.

    Returns true if the state could be set and false otherwise.

    \sa setProfile(), setDomain()
*/
bool QAudioStateManager::setState(const QAudioStateInfo &state, QAudio::AudioCapability capability)
{
    if (!m_data)
        return false;

    return m_data->setState(state, capability);
}

/*!
    Ask the manager to set a particular profile.  The profile is
    given by \a profile.  It is an error to call this function
    if no domain is currently active.  Effectively this sets
    the audio state with the same domain and capabilities
    but different inputs/outputs.

    Returns true if the profile could be set and false otherwise.

    \sa setDomain(), setState()
*/
bool QAudioStateManager::setProfile(const QByteArray &profile)
{
    if (!m_data)
        return false;

    return m_data->setProfile(profile);
}

/*!
    Ask the manager to enter into a particular \a domain.  The
    audio state with the highest priority (lowest number)
    for this domain will be used.  The audio state will be
    set with the \a capability.

    Returns true if the domain could be entered and false otherwise.

    \sa setProfile(), setState()
*/
bool QAudioStateManager::setDomain(const QByteArray &domain, QAudio::AudioCapability capability)
{
    if (!m_data)
        return false;

    return m_data->setDefaultState(domain, capability);
}

#include "qaudiostatemanager.moc"
