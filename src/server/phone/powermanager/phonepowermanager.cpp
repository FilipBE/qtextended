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

#include "phonepowermanager.h"

#include <qtopialog.h>
#include <QList>
#include <QSettings>

#include <QPowerStatus>

#include <qtopiaipcenvelope.h>
#include <QtopiaServiceRequest>
#include <QValueSpaceObject>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include <qtopiaipcenvelope.h>
#include "qtopiaserverapplication.h"
#include "systemsuspend.h"

/*!
  \class PhonePowerManager
    \inpublicgroup QtEssentialsModule
  \ingroup QtopiaServer::Task
  \brief The PhonePowerManager class implements phone specific power management
  functionality in Qtopia.

  This manager uses three levels for power management. The first level dims
  the background light, the second level turns the light off and the last level
  suspends the device.

  The PhonePowerManager class provides the \c {PhonePowerManager} task.
  It is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  \sa QtopiaPowerManager
*/

/*!
  Constructs a new PhonePowerManager instance.
*/
PhonePowerManager::PhonePowerManager() : 
    QtopiaPowerManager(), m_suspendEnabled(true)
{
    setDefaultIntervals();
}

/*!
  Destroys the PhonePowerManager instance
*/
PhonePowerManager::~PhonePowerManager() 
{
}

/*!
    This function activates the appropriate actions for the given
    power management \a level.
*/
bool PhonePowerManager::save(int level)
{
    QSettings config("Trolltech", "qpe");
    config.beginGroup("HomeScreen");
    QString showHomeScreen = config.value("ShowHomeScreen", "Never").toString();    //Note: defaults need to be in sync with
    bool autoKeyLock = config.value("AutoKeyLock", false).toBool();                //defaults in homescreen settings app
    int action = m_levelToAction.value(level);
    qLog(PowerManagement) << "PhonePowerManager::save()" << "level" << level << "action" << action << "homescreen settings:" << showHomeScreen << autoKeyLock;

    switch (action) {
        case PhonePowerManager::DimLight:
            if ( m_powerConstraint > QtopiaApplication::Disable && m_dimLightEnabled ) {
                m_vso->setAttribute("ScreenSaver/CurrentLevel", 1 );
                if (backlight() > 1)
                    setBacklight(1); // lowest non-off
                qLog(PowerManagement) << "Dimming light";
                return true;
            }
            break;
        case PhonePowerManager::LightOff:
            if ( m_powerConstraint > QtopiaApplication::DisableLightOff
                    && m_lightOffEnabled ) {
                m_vso->setAttribute("ScreenSaver/CurrentLevel", 2 );
                setBacklight(0); // off

                if (showHomeScreen == "DisplayOff") {
                    qLog(PowerManagement) << "Showing HomeScreen";
                    QtopiaIpcEnvelope showHome("QPE/System", autoKeyLock ?
                                               "showHomeScreenAndKeylock()" : "showHomeScreen()");
                }

                qLog(PowerManagement) << "turning light off";
                return true;
            }
            break;
        case PhonePowerManager::Suspend:
            qLog(PowerManagement) << "case PhonePowerManager::Suspend:" << "m_powerConstraint" << m_powerConstraint << "m_suspendEnabled" << m_suspendEnabled;
            if (m_powerConstraint > QtopiaApplication::DisableSuspend
                    && m_suspendEnabled) {
                m_vso->setAttribute("ScreenSaver/CurrentLevel", 3 );

                if (showHomeScreen == "Suspend") {
                    qLog(PowerManagement) << "Showing HomeScreen";
                    QtopiaIpcEnvelope showHome("QPE/System", autoKeyLock ?
                                               "showHomeScreenAndKeylock()" : "showHomeScreen()");
                }
                qLog(PowerManagement) << "Suspending device";
                SystemSuspend *suspend = qtopiaTask<SystemSuspend>();
                suspend->suspendSystem();
                return true;
            }
            break;
        default:
            ;
    }
    return false;
}


/*! 
    This function sets the power management internal timeouts
    to the values passed in \a ivals. \a size determines the number of entries in 
    \a ivals.


    The phone power manager maps the timeouts to the following actions:
        \list
        \o 0 -> dim light
        \o 1 -> turn off light
        \o 2 -> suspend device
        \endlist
*/
void PhonePowerManager::setIntervals(int* ivals, int size )
{
    //update value space
    QtopiaPowerManager::setIntervals( ivals, size );

    QSettings config("Trolltech","qpe");

    QString powerGroup = (powerstatus.wallStatus() == QPowerStatus::Available) ? "ExternalPower" : "BatteryPower";
    config.beginGroup( powerGroup );

    int *v = new int[size+1];
    for(int j=size; j>=0; j--)
        v[j]=0;

    m_levelToAction.clear();
    QMap<int,int> timeToAction;

    switch (size) {
        default:
        case 3:
            ivals[2] = interval(ivals[2], config, "Suspend","Interval", 60); // No tr
            v[2] = qMax( 1000*ivals[2] + 100, 100);
            m_suspendEnabled = ( (ivals[2] != 0) ? config.value("Suspend", true).toBool() : false );
            if (m_suspendEnabled)
                timeToAction.insert(v[2], PhonePowerManager::Suspend);
        case 2:
            ivals[1] = interval(ivals[1], config, "LightOff","Interval_LightOff", 30);
            if (ivals[1] == 0 && m_suspendEnabled)
                ivals[1] = ivals[2];
            v[1] = qMax( 1000*ivals[1], 100);
            if (timeToAction.contains(v[1]))
                v[1] = v[1]+100; //add few ms for next timeout
            m_lightOffEnabled = ( (ivals[1] != 0 ) ? config.value("LightOff", true).toBool() : false );
            m_lightOffEnabled |= m_suspendEnabled;
            if (m_lightOffEnabled)
                timeToAction.insert(v[1], PhonePowerManager::LightOff);
        case 1:
            ivals[0] = interval(ivals[0], config, "Dim","Interval_Dim", 20); // No tr
            v[0] = qMax( 1000*ivals[0], 100);
            while ( timeToAction.contains( v[0] ) )
                v[0] = v[0]+100; //add few ms for next timeout
            m_dimLightEnabled = ( (ivals[0] != 0) ? config.value("Dim", true).toBool() : false );
            if (m_dimLightEnabled)
                timeToAction.insert(v[0], PhonePowerManager::DimLight);
        case 0:
            break;
    }

    qLog(PowerManagement) << "PhonePowerManager::setIntervals:"
                          << ivals[0] << ivals[1] << ivals[2] <<" size: " << size;

    if ( !ivals[0] && !ivals[1] && !ivals[2] ){
#ifdef Q_WS_QWS
        QWSServer::setScreenSaverInterval(0);
#endif
        delete [] v;
        return;
    }

    QList<int> keys = timeToAction.keys();
    qStableSort(keys.begin(), keys.end());

    //first element
    v[0] = keys.at(0);
    m_levelToAction.insert(0, timeToAction.value(v[0]));
    int sum = v[0];

    for (int j=1; j<keys.count(); j++)
    {
        v[j]=keys.at(j)-sum;
        sum+= v[j];
        m_levelToAction.insert(j, timeToAction.value(sum));
    }
    v[keys.count()] = 0;

    qLog(PowerManagement) << "PhonePowerManager::setIntervals:"
                          << v[0] << v[1] << v[2];
#ifdef Q_WS_QWS
    QWSServer::setScreenSaverIntervals(v);
#endif
    delete [] v;

    // when the backlight goes off, ignore the key that wakes it up
    int blocklevel = PhonePowerManager::LightOff;
    if ( !m_lightOffEnabled ) {
        // we're not turning the backlight off, ignore the key that wakes us up from suspend
        blocklevel = PhonePowerManager::Suspend;
        if ( !m_suspendEnabled ) {
            // we're not suspending, never ignore keys
            blocklevel = -1;
        }
    }
    // Now we need to map blocklevel from "action" to "level"
    if ( blocklevel != -1 ) {
        for ( QMap<int,int>::const_iterator it = m_levelToAction.begin(); it != m_levelToAction.end(); ++it ) {
            if ( blocklevel == it.value() ) {
                blocklevel = it.key();
                break;
            }
        }
    }
#ifdef Q_WS_QWS
    qLog(PowerManagement) << "Using Block level " << blocklevel;
    QWSServer::setScreenSaverBlockLevel(blocklevel);
#endif
}

/*!
  \reimp
*/
void PhonePowerManager::forceSuspend()
{
    qLog(PowerManagement) << "PhonePowerManager::forceSuspend()";
    QSettings config("Trolltech", "qpe");
    config.beginGroup("HomeScreen");
    QString showHomeScreen = config.value("ShowHomeScreen", "Never").toString();    //Note: defaults need to be in sync with
    bool autoKeyLock = config.value("AutoKeyLock", false).toBool();                //defaults in homescreen settings app
    qLog(PowerManagement) << "HomeScreen settings:" << showHomeScreen << autoKeyLock;

    if ( m_suspendEnabled ) {
        if (showHomeScreen == "Suspend") {
            qLog(PowerManagement) << "Showing HomeScreen";
            QtopiaIpcEnvelope showHome("QPE/System", autoKeyLock ?
                    "showHomeScreenAndKeylock()" : "showHomeScreen()");
        }
        qLog(PowerManagement) << "Suspending device";
        SystemSuspend *suspend = qtopiaTask<SystemSuspend>();
        suspend->suspendSystem();
    }
}

QTOPIA_TASK(PhonePowerManager, PhonePowerManager);
