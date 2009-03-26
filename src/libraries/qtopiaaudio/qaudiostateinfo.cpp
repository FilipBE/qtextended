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

#include "qaudiostateinfo.h"

#include <QByteArray>
#include <QString>
#include <QHash>
#include <QDebug>

struct QAudioStateInfoPrivate
{
    QByteArray domain;
    QByteArray profile;
    QString display;
    int priority;

    QAudioStateInfoPrivate(const QByteArray &domain, const QByteArray &profile,
                       const QString &display, 
                       int priority);
    QAudioStateInfoPrivate();
};

QAudioStateInfoPrivate::QAudioStateInfoPrivate(const QByteArray &d, const QByteArray &p,
                                               const QString &disp,
                                               int pr)
    : domain(d), profile(p), display(disp), priority(pr)
{

}

QAudioStateInfoPrivate::QAudioStateInfoPrivate()
{
}

/*!
    \class QAudioStateInfo
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
    \brief The QAudioStateInfo class provides a point of access to information about an audio state.

    Hardware audio devices support a discrete set of audio device
    configurations.  For instance, using the Bluetooth Headset
    and routing all phone call information to and from the headset
    can be considered an audio state.

    The QAudioStateInfo is used to enumerate the attributes and
    of of a particular audio state.  The Audio Domain
    that this state belongs to can be found by calling domain().
    The profile name can be discovered by calling profile().
    Finally a translated name suitable for display can be found
    by using the displayName() function.  The priority for this
    state can be found using priority().

    \sa QAudioStateConfiguration
*/

/*!
    Constructs a new invalid Audio State Info object.

    \sa isValid()
*/
QAudioStateInfo::QAudioStateInfo() :
    m_data(0)
{

}

/*!
    Constructs a new valid Audio State Info.  The Audio Domain is set to
    \a domain.  The profile name is given by \a profile.  The display
    name is set to \a displayName.  The priority is given by
    \a priority.

    \sa isValid()
*/
QAudioStateInfo::QAudioStateInfo(const QByteArray &domain, const QByteArray &profile,
                                 const QString &displayName,
                                 int priority)
{
    m_data = new QAudioStateInfoPrivate(domain, profile, displayName, priority);
}

/*!
    Constructs a new Audio State Info from the contents of \a other.

    \sa isValid()
*/
QAudioStateInfo::QAudioStateInfo(const QAudioStateInfo &other)
    : m_data(0)
{
    if (!other.m_data) {
        delete m_data;
        m_data = 0;
        return;
    }

    m_data = new QAudioStateInfoPrivate(other.m_data->domain, other.m_data->profile,
                                        other.m_data->display,
                                        other.m_data->priority);
}

/*!
    Destroys an Audio State Info.
*/
QAudioStateInfo::~QAudioStateInfo()
{
    delete m_data;
    m_data = 0;
}

/*!
    Assign the contents of \a other to the current audio state.

    \sa isValid()
*/
QAudioStateInfo &QAudioStateInfo::operator=(const QAudioStateInfo &other)
{
    if (this == &other)
        return *this;

    if (!other.m_data) {
        delete m_data;
        m_data = 0;
        return *this;
    }

    if (m_data) {
        m_data->domain = other.m_data->domain;
        m_data->profile = other.m_data->profile;
        m_data->display = other.m_data->display;
        m_data->priority = other.m_data->priority;
    }
    else {
        m_data = new QAudioStateInfoPrivate(other.m_data->domain, other.m_data->profile,
                                            other.m_data->display,
                                            other.m_data->priority);
    }

    return *this;
}

/*!
    Returns true if the contents of \a other are equal to the contents
    of the current object.

    \sa operator!=()
*/
bool QAudioStateInfo::operator==(const QAudioStateInfo &other) const
{
    if (this == &other)
        return true;

    if (!m_data && !other.m_data)
        return true;

    if (!m_data || !other.m_data)
        return false;

    return (m_data->domain == other.m_data->domain) &&
           (m_data->profile == other.m_data->profile) &&
           (m_data->display == other.m_data->display) && 
           (m_data->priority == other.m_data->priority);
}

/*!
    \fn bool QAudioStateInfo::operator!=(const QAudioStateInfo &other) const;

    Returns true if the contents of \a other are not equal to the
    contents of the current object.

    \sa operator==()
*/

/*!
    Returns the Audio Domain Info for this state.

    \sa setDomain()
*/
QByteArray QAudioStateInfo::domain() const
{
    if (!m_data)
        return QByteArray();

    return m_data->domain;
}

/*!
    Sets the Audio Domain for this state to \a domain.

    If you call this function for an invalid audio state, this function
    turns it into a valid one.

    \sa domain(), isValid()
*/
void QAudioStateInfo::setDomain(const QByteArray &domain)
{
    if (!m_data)
        m_data = new QAudioStateInfoPrivate;

    m_data->domain = domain;
}

/*!
    Returns the profile name for this state.

    \sa setProfile()
*/
QByteArray QAudioStateInfo::profile() const
{
    if (!m_data)
        return QByteArray();

    return m_data->profile;
}

/*!
    Sets the profile name for this state to \a profile.

    If you call this function for an invalid audio state, this function
    turns it into a valid one.

    \sa profile(), isValid()
*/
void QAudioStateInfo::setProfile(const QByteArray &profile)
{
    if (!m_data)
        m_data = new QAudioStateInfoPrivate;

    m_data->profile = profile;
}

/*!
    Returns a translated string that describes this audio state suitable
    for display.  E.g. for instance "Bluetooth Headset" or "Headphones".

    \sa setDisplayName()
*/
QString QAudioStateInfo::displayName() const
{
    if (!m_data)
        return QString();

    return m_data->display;
}

/*!
    Sets the translated display name for this state to \a displayName.

    If you call this function for an invalid audio state, this function
    turns it into a valid one.

    \sa displayName(), isValid()
*/
void QAudioStateInfo::setDisplayName(const QString &displayName)
{
    if (!m_data)
        m_data = new QAudioStateInfoPrivate;

    m_data->display = displayName;
}

/*!
    Returns true whether this Audio State is valid.  If the object
    is not valid, information returned should not be relied upon.
*/
bool QAudioStateInfo::isValid() const
{
    return m_data != 0;
}

/*!
    Returns the priority for this Audio State.  The value of 0 has
    the highest priority, while the value of MAX_INT has the lowest
    priority.

    If the object is not valid, returns -1.

    \sa setPriority(), isValid()
*/
int QAudioStateInfo::priority() const
{
    if (!m_data)
        return -1;

    return m_data->priority;
}

/*!
    Sets the priority of the audio state to \a priority.

    If you call this function for an invalid audio state, this function
    turns it into a valid one.

    \sa priority()
*/
void QAudioStateInfo::setPriority(int priority)
{
    if (!m_data)
        m_data = new QAudioStateInfoPrivate;

    m_data->priority = priority;
}

/*!
    \internal
*/
uint qHash(const QAudioStateInfo &key)
{
    QByteArray hash(key.domain());
    hash.append(key.profile());

    return qHash(hash) ^ static_cast<uint>(key.priority());
}

/*!
    \internal
*/
QDebug operator<<(QDebug debug, const QAudioStateInfo &state)
{
    debug << "QAudioStateInfo(" << state.isValid() << "," <<
            state.domain() << "," <<
            state.profile() << "," <<
            state.displayName() << "," <<
            state.priority() << ")";

    return debug;
}
