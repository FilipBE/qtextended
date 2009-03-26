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

#include "qaudiostateconfiguration.h"

#include <QValueSpace>
#include <QString>
#include <QtAlgorithms>
#include <QStringList>
#include <QAudioStateInfo>
#include <QHash>
#include <QSet>
#include <QDebug>

class QAudioStateConfigurationPrivate : public QObject
{
    Q_OBJECT

public:
    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> > m_states;
    QValueSpaceItem *m_valueSpace;
    QValueSpaceItem *m_initSpace;
    QValueSpaceItem *m_currentSpace;
    bool m_initialized;
    QAudioStateConfiguration *m_parent;
    QAudioStateInfo m_current;
    QAudio::AudioCapability m_currentCap;

    void readConfiguration();

public slots:
    void contentsChanged();
    void initializedChanged();
    void currentChanged();
};

void QAudioStateConfigurationPrivate::readConfiguration()
{
    m_states.clear();

    QStringList paths = m_valueSpace->subPaths();

    QAudioStateInfo state;

    // Domain / Profile / Available
    //                  / Capability
    //                  / Display
    foreach (const QString &path, paths) {
        state.setDomain(path.toLatin1());
        QValueSpaceItem profileList(*m_valueSpace, path);

        foreach (const QString &profilePath, profileList.subPaths()) {
            state.setProfile(profilePath.toLatin1());
            QValueSpaceItem profile(profileList, profilePath);

            QPair<bool, QAudio::AudioCapabilities> pair;
            pair.first =  profile.value("Available").toBool();
            pair.second = static_cast<QAudio::AudioCapabilities>
                    (profile.value("Capabilities").toInt());
            state.setDisplayName(profile.value("Display").toString());
            state.setPriority(profile.value("Priority").toInt());

            m_states.insert(state, pair);
        }
    }
}

void QAudioStateConfigurationPrivate::contentsChanged()
{
    readConfiguration();

    // First time through, don't emit availabiltyChanged after initialized
    if (m_initialized)
        emit m_parent->availabilityChanged();
}

void QAudioStateConfigurationPrivate::currentChanged()
{
    m_current.setDomain(m_currentSpace->value("Domain").toByteArray());
    m_current.setProfile(m_currentSpace->value("Profile").toByteArray());
    m_current.setDisplayName(m_currentSpace->value("DisplayName").toString());
    m_current.setPriority(m_currentSpace->value("Priority").toInt());

    m_currentCap = static_cast<QAudio::AudioCapability>(m_currentSpace->value("Capability").toInt());

    emit m_parent->currentStateChanged(m_current, m_currentCap);
}

void QAudioStateConfigurationPrivate::initializedChanged()
{
    if (m_initialized == false) {
        bool initialized = m_initSpace->value().toBool();

        if (initialized) {
            contentsChanged();
            QObject::connect(m_valueSpace, SIGNAL(contentsChanged()),
                    this, SLOT(contentsChanged()));
            QObject::connect(m_currentSpace, SIGNAL(contentsChanged()),
                    this, SLOT(currentChanged()));
            m_initialized = initialized;
            emit m_parent->configurationInitialized();
        }
    }
}

/*!
    \class QAudioStateConfiguration
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
    \brief The QAudioStateConfiguration class defines a read-only view to the current
    audio device configuration.

    The QAudioStateConfiguration class provides a read-only view
    of the audio state configuration of the device.  The user of this
    class can query all possible audio hardware paths that the current
    device supports, (e.g. audio routed to the Headset, Speaker,
    Bluetooth Headset) as well as the capabilities of each audio state.

    \sa QAudioStateInfo
*/

/*!
    Constructs a new Audio Device Configuration object with \a parent.
    The configuration object is only a read-only view to the
    different set of audio states that the system supports.  It is
    possible that the configuration has not been initialized yet,
    in this case the isInitialized() method will return false.

    Once the configuration has been initialized, the
    configurationInitialized() signal will be emitted.

    \sa configurationInitialized(), isInitialized()
*/
QAudioStateConfiguration::QAudioStateConfiguration(QObject *parent) :
    QObject(parent)
{
    m_data = new QAudioStateConfigurationPrivate;

    QByteArray p("/Hardware/Audio/Configuration/");

    QByteArray pi(p);
    pi.append("Initialized");

    QByteArray pd(p);
    pd.append("Domains");

    QByteArray pc(p);
    pc.append("Current");

    m_data->m_valueSpace = new QValueSpaceItem(pd);
    m_data->m_initSpace = new QValueSpaceItem(pi);
    m_data->m_currentSpace = new QValueSpaceItem(pc);

    m_data->m_parent = this;
    m_data->m_initialized = false;
    QObject::connect(m_data->m_initSpace, SIGNAL(contentsChanged()),
                     m_data, SLOT(initializedChanged()));

    m_data->initializedChanged();
}

/*!
    Destroys an Audio Device Configuration object.
*/
QAudioStateConfiguration::~QAudioStateConfiguration()
{
    delete m_data->m_valueSpace;
    delete m_data;
}

/*!
    Returns whether the audio device configuration has been
    initialized.  The configurationInitialized() signal
    will be emitted once the configuration has been initialized.

    \sa configurationInitialized()
*/
bool QAudioStateConfiguration::isInitialized() const
{
    return m_data->m_initialized;
}

/*!
    Returns a set of domains supported by the audio device.  All
    devices should provide \c{RingTone}, \c{Media} and \c{Phone} domains.

    \sa states(), availabilityChanged()
*/
QSet<QByteArray> QAudioStateConfiguration::domains() const
{
    QSet<QByteArray> ret;

    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> >::const_iterator i =
            m_data->m_states.constBegin();
    while (i != m_data->m_states.constEnd()) {
        ret.insert(i.key().domain());
        ++i;
    }

    return ret;
}

/*!
    Returns a set of states supported by the audio device.

    \sa domains(), availabilityChanged()
*/
QSet<QAudioStateInfo> QAudioStateConfiguration::states() const
{
    QSet<QAudioStateInfo> ret;

    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> >::const_iterator i =
            m_data->m_states.constBegin();
    while (i != m_data->m_states.constEnd()) {
        ret.insert(i.key());
        ++i;
    }

    return ret;
}

/*!
    Returns a set of states supported by the audio device for a particular
    \a domain.

    \sa domains(), availabilityChanged()
*/
QSet<QAudioStateInfo> QAudioStateConfiguration::states(const QByteArray &domain) const
{
    QSet<QAudioStateInfo> ret;

    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> >::const_iterator i =
            m_data->m_states.constBegin();
    while (i != m_data->m_states.constEnd()) {
        if (i.key().domain() == domain)
            ret.insert(i.key());
        ++i;
    }

    return ret;
}

/*!
    Returns true whether the \a state is available.  For instance,
    if the Headphones are not plugged in, then the state that encapsulates
    routing audio data to the headphones would not be available.

    \sa availabilityChanged()
*/
bool QAudioStateConfiguration::isStateAvailable(const QAudioStateInfo &state) const
{
    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> >::const_iterator it =
            m_data->m_states.find(state);
    if (it == m_data->m_states.end()) {
        qWarning("Trying to find availability for an undefined state.");
        return false;
    }

    return it.value().first;
}

/*!
    Returns all available capabilities for a particular \a state.  Some
    devices support multiple multiple states (e.g. half-duplex, full-duplex).
    For various reasons (e.g. hardware capability, power consumption) it
    might be better to enable the state with only the capabilities
    that are required.

    Returns the audio capabilities supported by the state.  Returns an
    empty set if the state is not found.

    \sa isStateAvailable()
*/
QAudio::AudioCapabilities QAudioStateConfiguration::availableCapabilities(const QAudioStateInfo &state) const
{
    QHash<QAudioStateInfo, QPair<bool, QAudio::AudioCapabilities> >::const_iterator it =
            m_data->m_states.find(state);
    if (it == m_data->m_states.end()) {
        qWarning("Trying to find availability for an undefined state.");
        return false;
    }

    return it.value().second;
}

/*!
    Returns the current state of the audio device.

    \sa currentStateChanged(), currentCapability()
*/
QAudioStateInfo QAudioStateConfiguration::currentState() const
{
    return m_data->m_current;
}

/*!
    Returns the current capability enabled for the audio device.

    \sa currentState(), currentStateChanged()
*/
QAudio::AudioCapability QAudioStateConfiguration::currentCapability() const
{
    return m_data->m_currentCap;
}

/*!
    \fn void QAudioStateConfiguration::availabilityChanged()

    This signal is emitted whenever changes have been made to the device
    configuration.  This occurs ifthe available status of a state(s) has
    changed.

    \sa configurationInitialized()
*/

/*!
    \fn void QAudioStateConfiguration::configurationInitialized()

    This signal is emitted when the audio device configuration enters
    the initialized state.  Once this signal is emitted the user
    can assume that the current object will return useful information
    about the current audio device configuration.

    \sa availabilityChanged()
*/

/*!
    \fn void QAudioStateConfiguration::currentStateChanged(const QAudioStateInfo &state, QAudio::AudioCapability capability)

    This signal is emitted whenever the current audio state has
    changed.  The \a state contains the current audio state and
    \a capability contains the current capability.

    \sa currentState(), currentCapability()
*/

#include "qaudiostateconfiguration.moc"
