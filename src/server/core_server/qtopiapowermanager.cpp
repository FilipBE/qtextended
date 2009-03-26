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
#include "qtopiapowermanager.h"

#include <QList>
#include <QMap>
#include <QSettings>
#include <QtAlgorithms>
#include <QtopiaServiceRequest>
#include <qtopianetwork.h>
#include <qtopiaipcenvelope.h>
#include <qvaluespace.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include "applicationlauncher.h"
#include "qtopiapowermanagerservice.h"
#include "qtopiaserverapplication.h"
#include "systemsuspend.h"
#include "windowmanagement.h"

static bool forced_off = false;
static int currentBacklight = -1;
QValueSpaceObject *QtopiaPowerManager::m_vso = 0;

/*  Apply light/power settings for current power source */
static void applyLightSettings(QPowerStatus *p)
{
    int intervalDim;
    int intervalLightOff;
    int intervalSuspend;
    bool dim;
    bool lightoff;
    bool suspend;

    bool canSuspend;

    QSettings config("Trolltech","qpe");

    QSettings hwConfig("Trolltech", "Hardware");
    hwConfig.beginGroup("PowerManagement");
    if (p->wallStatus() == QPowerStatus::Available) {
        config.beginGroup("ExternalPower");
        canSuspend = hwConfig.value("CanSuspendAC", false).toBool();
    } else {
        config.beginGroup("BatteryPower");
        canSuspend = hwConfig.value("CanSuspend", false).toBool();
    }

    intervalDim = config.value("Interval_Dim", 20).toInt();
    config.setValue("Interval_Dim", intervalDim);
    intervalLightOff = config.value("Interval_LightOff", 30).toInt();
    config.setValue("Interval_LightOff", intervalLightOff);
    if (canSuspend) {
        if (p->wallStatus() == QPowerStatus::Available) {
            intervalSuspend = config.value("Interval", 240).toInt();
            config.setValue("Interval", intervalSuspend);
        } else {
            intervalSuspend = config.value("Interval", 60).toInt();
            config.setValue("Interval", intervalSuspend);
        }
        suspend = config.value("Suspend", true).toBool();
        config.setValue("Suspend", suspend);
    } else {
        intervalSuspend = 0;
        config.setValue("Interval", intervalSuspend);
        suspend = false;
        config.setValue("Suspend", suspend);
    }

    dim = config.value("Dim", true).toBool();
    config.setValue("Dim", dim);
    lightoff = config.value("LightOff", false).toBool();
    config.setValue("LightOff", lightoff);

    config.sync(); //write out for syncronisation with light app

    int i_dim =      (dim ? intervalDim : 0);
    int i_lightoff = (lightoff ? intervalLightOff : 0);
    int i_suspend = (suspend ? intervalSuspend : 0);

    QtopiaServiceRequest eB("QtopiaPowerManager", "setBacklight(int)" );
    eB << -3; //forced on
    eB.send();

    QtopiaServiceRequest e("QtopiaPowerManager", "setIntervals(int,int,int)" );
    e << i_dim << i_lightoff << i_suspend;
    e.send();
}

//--------------------------------------------------
// customised power management for Phone

static QtopiaPowerManager *g_qtopiaPowerManager = 0;


/*!
  Constructs a QtopiaPowerManager instance.
  */
QtopiaPowerManager::QtopiaPowerManager() : m_powerConstraint(QtopiaApplication::Enable), m_dimLightEnabled(true), m_lightOffEnabled(true) {
    g_qtopiaPowerManager = this;
    if (!m_vso)
        m_vso = new QValueSpaceObject("/Hardware");
    setBacklight(-3); //forced on

    // Create the screen saver and the associated service.
#ifdef Q_WS_QWS
    QWSServer::setScreenSaver(this);

    //QWSServer takes ownership of task
    QtopiaServerApplication::excludeFromTaskCleanup( this, true );
#endif
    (void)new QtopiaPowerManagerService( this, this );

    QtopiaPowerConstraintManager *tsmMonitor = new QtopiaPowerConstraintManager(this);
    connect( tsmMonitor, SIGNAL(forceSuspend()), this, SLOT(_forceSuspend()) );

    QObject::connect(&powerstatus, SIGNAL(wallStatusChanged(QPowerStatus::WallStatus)), this, SLOT(powerStatusChanged()));
    QObject::connect(&powerstatus, SIGNAL(batteryStatusChanged(QPowerStatus::BatteryStatus)), this, SLOT(powerStatusChanged()));
    QObject::connect(&powerstatus, SIGNAL(batteryChargingChanged(bool)), this, SLOT(powerStatusChanged()));
}

/*!
  \internal
  */
void QtopiaPowerManager::powerStatusChanged()
{
    applyLightSettings(&powerstatus);
}
/*!
    This function sets the internal timeouts for the power manager.
    It expects an array \a v containing the timeout values and \a size
    being the number of entries in \a v. This allows any arbitrary number of
    power saving levels.

    This function needs to be reimplemented by subclasses. However any overriding
    function should call QtopiaPowerManager::setIntervals() as part of its
    implementation.
*/
void QtopiaPowerManager::setIntervals(int *v, int size)
{
    Q_UNUSED( v );

    QSettings cfg( QLatin1String("Trolltech"), QLatin1String("qpe"));
    if (powerstatus.wallStatus() == QPowerStatus::Available) {
        cfg.beginGroup("ExternalPower");
    } else {
        cfg.beginGroup("BatteryPower");
    }

    m_vso->setAttribute("ScreenSaver/State/DimEnabled", cfg.value(QLatin1String("Dim"),true ).toBool() );
    m_vso->setAttribute("ScreenSaver/State/LightOffEnabled", cfg.value(QLatin1String("LightOff"), false ).toBool() );
    m_vso->setAttribute("ScreenSaver/State/SuspendEnabled", cfg.value(QLatin1String("Suspend"),true ).toBool() );
    switch( size )
    {
        default:
        case 3:
            m_vso->setAttribute( QLatin1String("ScreenSaver/Timeout/Suspend"),
                    cfg.value(QLatin1String("Interval"), 60 ) );
        case 2:
            m_vso->setAttribute( "ScreenSaver/Timeout/LightOff",
                    cfg.value(QLatin1String("Interval_LightOff"), 30)  );
        case 1:
            m_vso->setAttribute( "ScreenSaver/Timeout/Dim",
                    cfg.value(QLatin1String("Interval_Dim"), 20 ) );
        case 0:
           break;
    }
}

/*!
    \fn void QtopiaPowerManager::setDefaultIntervals()

    Resets the intervals to the default configuration for the power manager.
    The default values are defined in configuration files and can be edited
    by the user using the Power Management application.
*/
void QtopiaPowerManager::setDefaultIntervals() {
    int v[3];
    v[0]=-1;
    v[1]=-1;
    v[2]=-1;
    setIntervals(v, sizeof(v)/sizeof(int));
}

/*! \internal

   Returns \a interval if the given power manager setting is enabled. If the given
   interval is <0 it returns the value given by \a cfg.
 */
int QtopiaPowerManager::interval(int interval, QSettings& cfg, const QString &enable,
        const QString& value, int deflt)
{
    if ( !enable.isEmpty() && cfg.value(enable,false).toBool() == false )
        return 0;

    if ( interval < 0 ) {
        // Restore screen blanking and power saving state
        interval = cfg.value( value, deflt ).toInt();
    }
    return interval;
}

/*!
  \internal

  Sets the back light to \a bright.

  \sa backlight
  */
void QtopiaPowerManager::setBacklight(int bright)
{
    qLog(PowerManagement) << "QtopiaPowerManager: setBacklight =>" << bright ;
    if ( bright == -3 ) {
        // Forced on
        forced_off = false;
        bright = -1;
    }
    if ( forced_off && bright != -2 )
        return;
    if ( bright == -2 ) {
        // Toggle between off and on
        bright = currentBacklight ? 0 : -1;
        forced_off = !bright;
    }
    if ( bright == -1 ) {
        // Read from config
        QSettings config("Trolltech","qpe");
        if (powerstatus.wallStatus() == QPowerStatus::Available)
            config.beginGroup( "ExternalPower" );
        else
            config.beginGroup( "BatteryPower" );
        bright = config.value("Brightness", 255).toInt();
        bright = bright * qpe_sysBrightnessSteps() / 255;
        if ( bright < 1 )
            bright = 1;
    }
    qpe_setBrightness(bright);
    currentBacklight = bright;

    if (m_vso)
        m_vso->setAttribute("Display/0/Backlight", bright);

    QtopiaIpcEnvelope e( "Qtopia/PowerStatus", "brightnessChanged(int)" );
    e << bright;
}

/*!
  \internal

  Returns the current level of the backlight. The return value has a range between
  0 and the vlaue set by  qpe_sysBrightnessSteps() (  qpe_sysBrightnessSteps() being the brightest backlight setting ).

  \sa setBacklight
  */
int QtopiaPowerManager::backlight()
{
    if (currentBacklight == -1) {
        QSettings config("Trolltech","qpe");
        if (powerstatus.wallStatus() == QPowerStatus::Available)
            config.beginGroup( "ExternalPower" );
        else
            config.beginGroup( "BatteryPower" );
        currentBacklight = config.value("Brightness", 255).toInt();
        currentBacklight = currentBacklight * qpe_sysBrightnessSteps() / 255;
        if ( currentBacklight < 1 )
            currentBacklight = 1;
    }
    return currentBacklight;
}

/*!
  Activates or deactivates the power manager given the flag \a on.
  When activated, power management is enabled. When deactivated, power management is completely disabled.
  */
void QtopiaPowerManager::setActive(bool on)
{
    Q_UNUSED(on);
#ifdef Q_WS_QWS
    QWSServer::screenSaverActivate(on);
#endif
}

/*!
  \internal

  Applies the power constraint \a c to this power manager.
  */
void QtopiaPowerManager::setConstraint(QtopiaApplication::PowerConstraint c)
{
    qLog(PowerManagement) << "New power constraint:" << c;
    m_powerConstraint = c;
}

/*!
    \fn void QtopiaPowerManager::restore()

    Restores the state of the device when power saving is active. This
    usually happens when the user interacts with the device.
*/
void QtopiaPowerManager::restore()
{
    m_vso->setAttribute("ScreenSaver/CurrentLevel", 0 );
    qLog(PowerManagement) << "QtopiaPowerManager: restoring screen saver";
    if ( backlight() <= 1 ) //if dimmed or off
        setBacklight(-1);
}

/*!
    \fn bool QtopiaPowerManager::save(int level)

    This function is called by Qt Extended when a timeout has occurred
    and dynamically maps \a level onto a power saving action.

    This function needs to be reimplemented by subclasses.
*/
bool QtopiaPowerManager::save(int level)
{
    Q_UNUSED(level);
    return false;
}

/*!
  \internal
*/
void QtopiaPowerManager::_forceSuspend()
{
    qLog(PowerManagement) << "QtopiaPowerManager::forceSuspend()";
    forceSuspend();
}

/*!
  This function is called when the QtopiaPowerConstraintManager::forceSuspend() signal is
  emitted. The default implementation suspends the system using the Suspend service.
  Custom implementations may want to re-implement this method if suspending can be
  prevented so as to avoid processing the SystemSuspendHandler tasks.
*/
void QtopiaPowerManager::forceSuspend()
{
    SystemSuspend *suspend = qtopiaTask<SystemSuspend>();
    suspend->suspendSystem();
}

//------------------------------------------------------------

/*!
    \class QtopiaPowerManager
    \inpublicgroup QtBaseModule
    \ingroup QtopiaServer
    \brief The QtopiaPowerManager class implements default device power management behaviour.

    Qt Extended provides an implementation for a phone device.
    These implementations support the following three timeouts and actions.

    PhonePowerManager (phone/phonepowermanager.cpp)
    \list
    \o dim backlight
    \o turn backlight off
    \o suspend device
    \endlist

    To extend the default behaviour it is necessary to subclass either PhonePowerManager
    or QtopiaPowerManager. A minimal subclass of QtopiaPowerManager has to reimplement the
    following functions:

    \list
    \o \l save()
    \o \l setIntervals()
    \endlist

    Applications can interact with the Qt Extended power management via the QtopiaPowerManagerService.
    For more details on how to utilize services see the \l {Services}{Services} documentation.

    This class provides functionality to temporarily restrict power management.
    It might be necessary to surpress the diming of the backlight e.g., when the mediaplayer is
    showing a video or to prevent the suspension of the device when the user is playing music.
    The QtopiaPowerConstraintManager ensures that application cannot request restriction
    beyond their own life time.

    \table
    \header
        \o ValueSpace path
        \o Description
    \row
        \o \c {/Hardware/ScreenSaver/CurrentLevel}
        \o Returns current level of the screen saver. A value of zero means that the system
        is up and running and no power management option has been activated yet. This state is
        active while the user interacts with the device. The mapping of the remaining levels depends on 
        this classes subclass that is in use (e.g. see \l PhonePowerManager).
    \row
        \o \c {/Hardware/ScreenSaver/Timeout/Dim}
        \o Contains the number of seconds until the displays dims.
    \row
        \o \c {/Hardware/ScreenSaver/Timeout/LightOff}
        \o Contains the number of seconds until the display turns off. This timeout 
        starts once the Dim state has been reached.
    \row
        \o \c {/Hardware/ScreenSaver/Timeout/Suspend}
        \o Contains the number of seconds until the system suspends. This timeout 
        starts once the Lightoff state has been reached.
    \row
        \o \c {/Hardware/ScreenSaver/State/DimEnabled}
        \o Returns \c true if display dimming is enabled; other \c false.
    \row
        \o \c {/Hardware/ScreenSaver/State/LightOffEnabled}
        \o Returns \c true if turning the display of is part of the power management process; other \c false.
    \row
        \o \c {/Hardware/ScreenSaver/State/SuspendEnabled}
        \o Returns \c true if the system will suspend; otherwise false.
    \endtable

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa setIntervals(), save(), PhonePowerManager, QtopiaPowerManagerService, QtopiaPowerConstraintManager
*/



/**********************************************************/

#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
#define QTOPIA_MIN_SCREEN_DISABLE_TIME ((int) 300)  // min 5 minutes before forced suspend kicks in
#endif

static QtopiaPowerConstraintManager *g_managerinstance= 0;

/*!
    \ingroup QtopiaServer
    \class QtopiaPowerConstraintManager
    \inpublicgroup QtBaseModule
    \brief The QtopiaPowerConstraintManager class keeps track of power management constraints set by Qt Extended applications.

    In some use cases Qt Extended applications may want to disable
    power saving acitivities in order to perform their tasks (e.g. videos app
    disables the dimming of the backlight when it plays a video). This monitor
    keeps track of all of all constraints set by applications and applies a common denominator
    to the QtopiaPowerManager.

    QtopiaPowerConstraintManager is a singleton, which you can access through QtopiaPowerConstraintManager::instance().

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QtopiaPowerManager
*/

/*!
  \fn void QtopiaPowerConstraintManager::forceSuspend()

  Emitted when the device was forced to suspend due to the forced-suspend timeout.
  This timeout is set by defining the QTOPIA_MAX_SCREEN_DISABLE_TIME, usually in custom.h.
  \sa {Hardware Configuration}, QtopiaPowerManager::forceSuspend()
*/

/*!
  Constructs the power constraint manager. \a parent is passed to QObject.
  */
QtopiaPowerConstraintManager::QtopiaPowerConstraintManager(QObject *parent)
    : QObject(parent)
{
    g_managerinstance = this;
    currentMode = QtopiaApplication::Enable;
    timerId = 0;


    ApplicationLauncher *launcher = qtopiaTask<ApplicationLauncher>();
    if(launcher) {
        QObject::connect(launcher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason)), this, SLOT(applicationTerminated(QString)));
    }

    WindowManagement *windowManager = new WindowManagement(this);
    QObject::connect(windowManager, SIGNAL(windowActive(QString,QRect,WId)),
                     this, SLOT(topLevelWindowChanged()));
}

/*!
    The application \a app temporarily requests \a mode.
*/
void QtopiaPowerConstraintManager::setConstraint(QtopiaApplication::PowerConstraint mode, const QString &app)
{
    removeOld(app);

    switch(mode) {
        case QtopiaApplication::Disable:
            sStatus[0].append(app);
            break;
        case QtopiaApplication::DisableLightOff:
            sStatus[1].append(app);
            break;
        case QtopiaApplication::DisableSuspend:
            sStatus[2].append(app);
            break;
        case QtopiaApplication::Enable:
            // need to reset the screen saver timer, and this is how you do that!
            g_qtopiaPowerManager->setDefaultIntervals();
            break;
        default:
            qWarning("Unrecognized temp power setting.  Ignored");
            return;
    }
    updateAll();
}

// Returns true if app had set a temp Mode earlier
bool QtopiaPowerConstraintManager::removeOld(const QString &pid)
{
    for (int i = 0; i < 3; i++) {
        int idx = sStatus[i].indexOf(pid);
        if ( idx != -1 ) {
            sStatus[i].removeAt( idx );
            return true;
        }
    }

    return false;
}

void QtopiaPowerConstraintManager::updateAll()
{
    int mode = QtopiaApplication::Enable;
    if (sStatus[0].contains(WindowManagement::activeAppName()))
        mode = QtopiaApplication::Disable;
    else if (sStatus[1].contains(WindowManagement::activeAppName()))
        mode = QtopiaApplication::DisableLightOff;
    else if (sStatus[0].count() || sStatus[1].count() || sStatus[2].count())
        mode = QtopiaApplication::DisableSuspend;

    if ( mode != currentMode ) {
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
        if ( currentMode == QtopiaApplication::Enable) {
            int tid = timerValue();
            if ( tid )
                timerId = startTimer( tid * 1000 );
        } else if ( mode == QtopiaApplication::Enable ) {
            if ( timerId ) {
                killTimer(timerId);
                timerId = 0;
            }
        }
#endif
        currentMode = mode;
        if ( g_qtopiaPowerManager ) {
            g_qtopiaPowerManager->setConstraint( (QtopiaApplication::PowerConstraint) mode );
        }
    }
}

/*!
  The application \a app has been terminated.
  Any power constraint applied by this application will
  be removed.
*/
void QtopiaPowerConstraintManager::applicationTerminated(const QString &app)
{
    if (removeOld(app))
        updateAll();
}

/*!
  The top level window has changed.  Update only when visible power constraints.
*/
void QtopiaPowerConstraintManager::topLevelWindowChanged()
{
    updateAll();
}

int QtopiaPowerConstraintManager::timerValue()
{
    int tid = 0;
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    tid = QTOPIA_MAX_SCREEN_DISABLE_TIME;

    char *env = getenv("QTOPIA_DISABLED_APM_TIMEOUT");
    if ( !env )
        return tid;

    QString strEnv = env;
    bool ok = false;
    int envTime = strEnv.toInt(&ok);

    if ( ok ) {
        if ( envTime < 0 )
            return 0;
        else if ( envTime <= QTOPIA_MIN_SCREEN_DISABLE_TIME )
            return tid;
        else
            return envTime;
    }
#endif

    return tid;
}

/*!
  \reimp
  */
void QtopiaPowerConstraintManager::timerEvent(QTimerEvent *t)
{
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    if ( timerId && (t->timerId() == timerId) ) {

        /*  Clean up    */
        killTimer(timerId);
        timerId = 0;
        currentMode = QtopiaApplication::Enable;
        if ( g_qtopiaPowerManager ) {
            g_qtopiaPowerManager->setConstraint
                ( (QtopiaApplication::PowerConstraint) currentMode );
        }

        // signal starts on a merry-go-round, which ends up in Desktop::togglePower()
        emit forceSuspend();
        // if we have apm we are asleep at this point, next line will be executed when we
        // awake from suspend.
        if ( QFile::exists( "/proc/apm" ) ) {
            QTime t;
            t = t.addSecs( timerValue() );
            QString str = tr("<qt>The running applications disabled power saving "
                             "for more than the allowed time (%1)."
                             "<br>The system was forced to suspend</qt>", "%1 = time span").arg( t.toString() );
            QMessageBox::information(0, tr("Forced suspend"), str);
        }

        // Reset all requests.
        for (int i = 0; i < 3; i++)
            sStatus[i].clear();

        updateAll();
    }
#else
    Q_UNUSED(t);
#endif
}

/*!
  Returns the singleton instance of the QtopiaPowerConstraintManager.
  */
QtopiaPowerConstraintManager *QtopiaPowerConstraintManager::instance()
{
    return g_managerinstance;
}

