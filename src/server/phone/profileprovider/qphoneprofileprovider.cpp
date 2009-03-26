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

#include "qphoneprofileprovider.h"
#include <QAudioStateConfiguration>
#include <QAudioStateInfo>
#include <QPhoneProfile>
#include <QPhoneProfileManager>
#include <QValueSpaceObject>
#include <QValueSpaceItem>
#include "qtopiaserverapplication.h"

// declare QPhoneProfileProviderPrivate
class QPhoneProfileProviderPrivate
{
public:
    QPhoneProfileProviderPrivate()
    : manager(0), profile(0), preAudioProfileSelected(-1) {}

    QPhoneProfileManager *manager;
    QValueSpaceObject *profile;
    QAudioStateConfiguration *audioState;

    int preAudioProfileSelected;
};

/*!
  \class QPhoneProfileProvider
    \inpublicgroup QtTelephonyModule
  \ingroup QtopiaServer::Task
  \brief The QPhoneProfileProvider class provides the backend functionality for
         phone profiles.

  The QPhoneProfileProvider class provides the auto activation portion of the
  profile system.  Profiles may be auto activated at a specific time, controlled
  through the QPhoneProfile::schedule() value, or whenever the
  QPhoneProfile::audioProfile() profile is active.

  Currently the QPhoneProfileProvider class checks the
  QAudioStateConfiguration to obtain updates about audio states.

  Additionally, the QPhoneProfileProvider class sets the following value space
  keys to values dictated by the currently active profile:

  \table
  \header \o Key \o Description
  \row \o \c {/UI/Profile/Name} \o The name of the profile, as returned by QPhoneProfile::name().
  \row \o \c {/UI/Profile/Default} \o True if this is the default profile (profile with id 1) or false if not.
  \row \o \c {/UI/Profile/PlaneMode} \o True if plane mode is on, false if not, as returned by QPhoneProfileManager::planeMode().
  \row \o \c {/UI/Profile/RingVolume} \o Set to the ring volume of the current profile, as returned by QPhoneProfile::volume().
  \endtable

  The QPhoneProfileProvider provides the \c {PhoneProfiles} task.
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \sa QPhoneProfile, QPhoneProfileManager
 */

// define QPhoneProfileProvider
/*! \internal */
QPhoneProfileProvider::QPhoneProfileProvider(QObject *parent)
: QObject(parent), d(new QPhoneProfileProviderPrivate)
{
    d->manager = new QPhoneProfileManager(this);
    d->audioState = new QAudioStateConfiguration(this);
    d->profile = new QValueSpaceObject("/UI/Profile", this);

    QObject::connect(d->manager, SIGNAL(profileUpdated(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(profileAdded(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(profileRemoved(QPhoneProfile)),
                     this, SLOT(scheduleActivation()));
    QObject::connect(d->manager, SIGNAL(activeProfileChanged(QPhoneProfile)),
                     this, SLOT(activeChanged()));
    QObject::connect(d->manager, SIGNAL(planeModeChanged(bool)),
                     this, SLOT(activeChanged()));
    QObject::connect(d->audioState, SIGNAL(currentStateChanged(QAudioStateInfo,QAudio::AudioCapability)),
                     this, SLOT(audioProfileChanged()));
    QObject::connect(d->manager, SIGNAL(profileUpdated(QPhoneProfile)),
                     this, SLOT(audioProfileChanged()));
    QObject::connect(d->manager, SIGNAL(profileAdded(QPhoneProfile)),
                     this, SLOT(audioProfileChanged()));
    QObject::connect(QtopiaApplication::instance(),
                     SIGNAL(appMessage(QString,QByteArray)),
                     this,
                     SLOT(appMessage(QString,QByteArray)));

    activeChanged();
}

/*! \internal */
QPhoneProfileProvider::~QPhoneProfileProvider()
{
    delete d;
}

void QPhoneProfileProvider::appMessage(const QString &msg,
                                       const QByteArray &data)
{
    if(msg == "activateProfile(QDateTime,int)") {
        QDataStream ds(data);
        QDateTime when;
        int id;
        ds >> when >> id;

        QPhoneProfile prof = d->manager->profile(id);
        if(prof.id() != -1)
            d->manager->activateProfile(id);

        scheduleActivation();
    }
}

void QPhoneProfileProvider::activeChanged()
{
    QPhoneProfile active = d->manager->activeProfile();
    d->profile->setAttribute("Name", active.name());
    d->profile->setAttribute("Default", active.id() == 1);
    d->profile->setAttribute("PlaneMode", d->manager->planeMode());
    d->profile->setAttribute("RingVolume", active.volume());
}

void QPhoneProfileProvider::scheduleActivation()
{
    QDateTime now = QDateTime::currentDateTime();
    int today = now.date().dayOfWeek();
    QDateTime when;
    int id = -1;

    QList<QPhoneProfile> profiles = d->manager->profiles();
    for(int ii = 0; ii < profiles.count(); ++ii) {
        const QPhoneProfile &prof = profiles.at(ii);
        QPhoneProfile::Schedule s = prof.schedule();

        if(s.isActive()) {
            QList<Qt::DayOfWeek> days = s.scheduledOnDays();
            for(int jj = 0; jj < days.count(); ++jj) {
                QDateTime current(now.date(), s.time());
                Qt::DayOfWeek day = days.at(jj);

                if(day < today)
                    current = current.addDays(7 - today + day);
                else if(day > today)
                    current = current.addDays(day - today);
                else if(s.time() < now.time())
                    current = current.addDays(7);

                if(!when.isValid() || current < when) {
                    when = current;
                    id = prof.id();
                }
            }

        }
    }

    if ( id != -1 )
        Qtopia::addAlarm(when, "QPE/Application/" + QtopiaApplication::applicationName(), "activateProfile(QDateTime,int)", id );
    else
        Qtopia::deleteAlarm(QDateTime(), "QPE/Application/" + QtopiaApplication::applicationName(), "activateProfile(QDateTime,int)", id);
}

void QPhoneProfileProvider::audioProfileChanged()
{
    // Check whether active phone profile matches audio profile.  In the case of
    // audio profile clash (two profiles depending on the same audioprofile) we should
    // avoid switching between the two.
    QPhoneProfile activeProfile = d->manager->activeProfile();
    
    if(!activeProfile.audioProfile().isEmpty() &&
       d->audioState->currentState().profile() == activeProfile.audioProfile() )
        return;

    // See if any of the available profiles match
    QList<QPhoneProfile> profiles = d->manager->profiles();
    for(int ii = 0; ii < profiles.count(); ++ii) {
        if(!profiles.at(ii).audioProfile().isEmpty() &&
           d->audioState->currentState().profile() == profiles.at(ii).audioProfile() ) { 

            if(-1 == d->preAudioProfileSelected)
                d->preAudioProfileSelected = d->manager->activeProfile().id();

            qLog(Support) << "Auto activation of profile " << profiles.at(ii).name() 
                          << profiles.at(ii).audioProfile();

            d->manager->activateProfile(profiles.at(ii).id());
            return;

        }
    }

    // Nothing - revert to the preaudio profile
    if(-1 != d->preAudioProfileSelected) {
        QPhoneProfile preAudioProfile = d->manager->profile(d->preAudioProfileSelected);
        d->preAudioProfileSelected = -1;
        if(preAudioProfile.id() != -1) {
            d->manager->activateProfile(preAudioProfile.id());
        } else {
            // Try default
            QPhoneProfile general = d->manager->profile(1);
            if(general.id() != -1)
                d->manager->activateProfile(general.id());
        }
    }
}


QTOPIA_TASK(PhoneProfiles, QPhoneProfileProvider);
