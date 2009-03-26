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

#include "applicationmonitor.h"
#include "windowmanagement.h"
#include <QStringList>
#include <QtopiaChannel>
#include <QTime>
#include <QValueSpaceItem>
#include <QtAlgorithms>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <QCopChannel>
#endif
#include <QTimer>

inline UIApplicationMonitor::ApplicationState operator|(const UIApplicationMonitor::ApplicationState &lhs, const UIApplicationMonitor::ApplicationState &rhs)
{
    return (UIApplicationMonitor::ApplicationState)((int)lhs | (int)rhs);
}

inline UIApplicationMonitor::ApplicationState operator&(const UIApplicationMonitor::ApplicationState &lhs, const UIApplicationMonitor::ApplicationState &rhs)
{
    return (UIApplicationMonitor::ApplicationState)((int)lhs & (int)rhs);
}

inline UIApplicationMonitor::ApplicationState operator~(const UIApplicationMonitor::ApplicationState &lhs)
{
    return (UIApplicationMonitor::ApplicationState)~(unsigned int)lhs;
}

// declare UIApplicationMonitorPrivate
class UIApplicationMonitorPrivate : public QObject
{
Q_OBJECT
public:
    UIApplicationMonitorPrivate();

    QStringList runningApplications() const;
    QStringList notRespondingApplications() const;
    QStringList busyApplications() const;
    UIApplicationMonitor::ApplicationState applicationState(const QString &) const;

    static int notRespondingTimeout();
    static void setNotRespondingTimeout(int);

signals:
    void applicationStateChanged(const QString &,
                                 UIApplicationMonitor::ApplicationState);
    void busy();
    void notBusy();

private slots:
    void updateRunningList();
    void applicationMessage(const QString &, const QByteArray &);
    void sysMessage(const QString &, const QByteArray &);
    void applicationTerminated(const QString &);
    void heartBeatTimer();
    void topLevelWindowChanged();

private:

    QValueSpaceItem * m_apps;

    typedef QHash<QString, UIApplicationMonitor::ApplicationState> Applications;
    Applications m_appsList;

    typedef QList<QPair<QString, int> > OutstandingHeartBeats;
    OutstandingHeartBeats m_beats;
    QTimer m_beatTimer;
    QTime m_beatElapsed;
    void startHeartBeat(const QString &);
    void stopHeartBeat(const QString &);

    int m_busyApplications;

    static int m_notRespondingTimeout;
    void doChange(Applications::Iterator, UIApplicationMonitor::ApplicationState);
};
Q_GLOBAL_STATIC(UIApplicationMonitorPrivate, uiApplicationMonitorPrivate);

// define UIApplicationMonitorPrivate
int UIApplicationMonitorPrivate::m_notRespondingTimeout = 5000;

UIApplicationMonitorPrivate::UIApplicationMonitorPrivate()
: m_apps(0), m_busyApplications(0)
{
    m_apps = new QValueSpaceItem("/System/Applications", this);

    updateRunningList();

    QObject::connect(m_apps, SIGNAL(contentsChanged()),
                     this, SLOT(updateRunningList()));

    // NotResponding test
    QObject::connect(&m_beatTimer, SIGNAL(timeout()),
                     this, SLOT(heartBeatTimer()));
    m_beatElapsed.start();
    m_beatTimer.setSingleShot(true);

    // For tracking raise()
    QCopChannel *channel = new QCopChannel( "QPE/Application/*", this );
    connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(applicationMessage(QString,QByteArray)) );


    // For tracking busy(QString), notBusy(QString) and appRaised(QString)
    QtopiaChannel *sysChannel = new QtopiaChannel("QPE/QtopiaApplication", this);
    QObject::connect(sysChannel,
                     SIGNAL(received(QString,QByteArray)),
                     this,
                     SLOT(sysMessage(QString,QByteArray)));

    // For tracking application terminations
    ApplicationLauncher *launcher = qtopiaTask<ApplicationLauncher>();
    QObject::connect(launcher, SIGNAL(applicationTerminated(QString,ApplicationTypeLauncher::TerminationReason,bool)), this, SLOT(applicationTerminated(QString)));
    QObject::connect(launcher, SIGNAL(applicationNotFound(QString)), this, SLOT(applicationTerminated(QString)));

    // For tracking of active window
    WindowManagement *man = new WindowManagement(this);
    QObject::connect(man, SIGNAL(windowActive(QString,QRect,WId)),
                this, SLOT(topLevelWindowChanged()));
}

void UIApplicationMonitorPrivate::topLevelWindowChanged()
{
    QString app = WindowManagement::activeAppName();
    //find currently active one
    for(Applications::Iterator iter = m_appsList.begin();
            iter != m_appsList.end(); ++iter) {
        if ( (*iter & UIApplicationMonitor::Active) && iter.key() != app) {
            doChange(iter, *iter & ~UIApplicationMonitor::Active);
            break;
        }
    }

    //set new active app
    Applications::Iterator iter = m_appsList.find(app);
    if (iter != m_appsList.end())
        doChange(iter, *iter | UIApplicationMonitor::Active);
}

int UIApplicationMonitorPrivate::notRespondingTimeout()
{
    return m_notRespondingTimeout;
}

void UIApplicationMonitorPrivate::setNotRespondingTimeout(int newTimeout)
{
    Q_ASSERT(newTimeout > 0);
    m_notRespondingTimeout = newTimeout;
}

void UIApplicationMonitorPrivate::heartBeatTimer()
{
    Q_ASSERT(!m_beats.isEmpty());

    while(!m_beats.isEmpty() && 0 == m_beats.at(0).second) {
        // Not responding!
        Applications::Iterator iter = m_appsList.find(m_beats.at(0).first);
        doChange(iter, *iter | UIApplicationMonitor::NotResponding);
        m_beats.removeAt(0);
    }

    if(!m_beats.isEmpty()) {
        int diff = m_beats.at(0).second;
        for(int ii = 0; ii < m_beats.count(); ++ii)
            m_beats[ii].second -= diff;
        m_beatTimer.start(m_notRespondingTimeout - diff);
        m_beatElapsed.restart();
    }
}

void UIApplicationMonitorPrivate::startHeartBeat(const QString &appName)
{
    for(int ii = 0; ii < m_beats.count(); ++ii) {
        if(m_beats.at(ii).first == appName)
            return; // Already waiting
    }

    if(m_beats.isEmpty()) {
        // Need to start the beat timer
        m_beatTimer.start(m_notRespondingTimeout);
        m_beatElapsed.restart();
        m_beats.append(qMakePair(appName, 0));
    } else {
        m_beats.append(qMakePair(appName, m_beatElapsed.elapsed()));
    }
}

void UIApplicationMonitorPrivate::stopHeartBeat(const QString &appName)
{
    for(int ii = 0; ii < m_beats.count(); ++ii) {
        if(m_beats.at(ii).first == appName) {
            m_beats.removeAt(ii);
            if(m_beats.isEmpty()) {
                m_beatTimer.stop();
            }
            return;
        }
    }
}

void UIApplicationMonitorPrivate::updateRunningList()
{
    QStringList apps = m_apps->subPaths();
    QSet<QString> runningApps;

    for(QStringList::Iterator iter = apps.begin(); iter != apps.end(); ++iter)
    {
        if(m_apps->value(*iter + "/Tasks/UI").toBool()) {
            runningApps.insert(*iter);
        }
    }

    for(Applications::Iterator iter = m_appsList.begin();
        iter != m_appsList.end();
        ++iter) {

        const QString & app = iter.key();
        QSet<QString>::Iterator riter = runningApps.find(app);

        if(riter == runningApps.end()) {
            // Not running
            doChange(iter, *iter & ~UIApplicationMonitor::Running);
        } else {
            // Running
            doChange(iter, (*iter & ~UIApplicationMonitor::StateMask) | UIApplicationMonitor::Running);
        }

        runningApps.erase(riter);
    }
    for(QSet<QString>::ConstIterator iter = runningApps.begin();
        iter != runningApps.end();
        ++iter) {

        Applications::Iterator newIter = m_appsList.insert(*iter, UIApplicationMonitor::NotRunning);
        doChange(newIter, UIApplicationMonitor::Running);
    }

}

void UIApplicationMonitorPrivate::applicationMessage(const QString &message,
                                                     const QByteArray &data)
{
    if ( message == QLatin1String("forwardedMessage(QString,QString,QByteArray)") ) {

        QDataStream stream( data );
        QString channel, message;
        stream >> channel;
        stream >> message;
        if("raise()" == message) {
            QString app = channel.mid(16 /* ::strlen("QPE/Application/") */);
            Applications::Iterator iter = m_appsList.find(app);
            if(iter == m_appsList.end()) {
                iter = m_appsList.insert(app, UIApplicationMonitor::NotRunning);
                doChange(iter, UIApplicationMonitor::Starting);
            }

            startHeartBeat(app);
        }
    }
}

void UIApplicationMonitorPrivate::sysMessage(const QString &message,
                                             const QByteArray &data)
{
    QDataStream ds(data);

    if("notBusy(QString)" == message) {
        QString app;
        ds >> app;

        Applications::Iterator iter = m_appsList.find(app);
        if(iter != m_appsList.end()) {
            doChange(iter, *iter & ~UIApplicationMonitor::Busy);
        }

    } else if("busy(QString)" == message) {
        QString app;
        ds >> app;

        Applications::Iterator iter = m_appsList.find(app);
        if(iter != m_appsList.end()) {
            doChange(iter, *iter | UIApplicationMonitor::Busy);
        } else {
            iter = m_appsList.insert(app, UIApplicationMonitor::NotRunning);
            doChange(iter, UIApplicationMonitor::Busy);
        }

    } else if("appRaised(QString)" == message) {
        QString app;
        ds >> app;

        Applications::Iterator iter = m_appsList.find(app);
        if(iter != m_appsList.end()) {
            doChange(iter, (*iter & ~UIApplicationMonitor::NotResponding));
            stopHeartBeat(app);
        }
    }
}

void UIApplicationMonitorPrivate::applicationTerminated(const QString &app)
{
    Applications::Iterator iter = m_appsList.find(app);
    if(iter != m_appsList.end()) {
        doChange(iter, UIApplicationMonitor::NotRunning);
        stopHeartBeat(app);
        m_appsList.erase(iter);
    } else {
        // Just fake it for consistency (not found etc.);
        emit applicationStateChanged(app, UIApplicationMonitor::NotRunning);
    }

}

void UIApplicationMonitorPrivate::doChange(Applications::Iterator iter,
        UIApplicationMonitor::ApplicationState newState)
{
    if(*iter == newState)
        return;

    bool wasSysBusy = (*iter & UIApplicationMonitor::StateMask) &&
                      (*iter & UIApplicationMonitor::Busy) ||
                      (*iter & UIApplicationMonitor::Starting);
    bool isSysBusy = (newState & UIApplicationMonitor::StateMask) &&
                     (newState & UIApplicationMonitor::Busy) ||
                     (newState & UIApplicationMonitor::Starting);

    bool wasVisible = (*iter & UIApplicationMonitor::StateMask);
    bool isVisible = (newState & UIApplicationMonitor::StateMask);

    UIApplicationMonitor::ApplicationState visibleState =
        !isVisible?UIApplicationMonitor::NotRunning:newState;
    if(isSysBusy)
        visibleState = visibleState | UIApplicationMonitor::Busy;

    *iter = newState;

    if(wasSysBusy && !isSysBusy) {
        Q_ASSERT(m_busyApplications);
        --m_busyApplications;
        if(!m_busyApplications)
            emit notBusy();
    } else if(!wasSysBusy && isSysBusy) {
        bool emitBusy = (0 == m_busyApplications);
        ++m_busyApplications;
        if(emitBusy)
            emit busy();
    }

    QString app = iter.key();
    if(wasVisible || isVisible) {
        emit applicationStateChanged(app, visibleState);
    }
}

QStringList UIApplicationMonitorPrivate::runningApplications() const
{
    QStringList rv;
    for(Applications::ConstIterator iter = m_appsList.begin();
        iter != m_appsList.end();
        ++iter) {
        if(*iter & UIApplicationMonitor::Running)
            rv.append(iter.key());
    }
    return rv;
}

QStringList UIApplicationMonitorPrivate::notRespondingApplications() const
{
    QStringList rv;
    for(Applications::ConstIterator iter = m_appsList.begin();
        iter != m_appsList.end();
        ++iter) {
        if(*iter & UIApplicationMonitor::StateMask && *iter & UIApplicationMonitor::NotResponding)
            rv.append(iter.key());
    }
    return rv;
}

QStringList UIApplicationMonitorPrivate::busyApplications() const
{
    QStringList rv;
    for(Applications::ConstIterator iter = m_appsList.begin();
        iter != m_appsList.end();
        ++iter) {
        if(*iter & UIApplicationMonitor::StateMask && *iter & UIApplicationMonitor::Busy)
            rv.append(iter.key());
    }
    return rv;
}

UIApplicationMonitor::ApplicationState
UIApplicationMonitorPrivate::applicationState(const QString &app) const
{
    QHash<QString, UIApplicationMonitor::ApplicationState>::ConstIterator iter =
        m_appsList.find(app);

    if(iter != m_appsList.end() && *iter & UIApplicationMonitor::StateMask)
        return *iter & (*iter & UIApplicationMonitor::Starting)?UIApplicationMonitor::Busy:UIApplicationMonitor::NotRunning;
    else
        return UIApplicationMonitor::NotRunning;
}


// define UIApplicationMonitor
/*!
  \class UIApplicationMonitor
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::AppLaunch
  \brief The UIApplicationMonitor class monitors the running state of UI
         applications.

  The UI application monitor provides a friendly interface to monitoring the
  state of UI applications.  The UIApplicationMonitor class uses the following
  messages to monitor application state.

  \table
  \header \o To Application \o From Application \o Description
  \row \o \c {raise()} \o \c {QPE/QtopiaApplication:appRaised(QString)} \o Used to raise, and confirm the raise of, the application UI.
  \row \o \o \c {QPE/QtopiaApplication:busy(QString)} \o The application has notified the system that it is busy.
  \row \o \o \c {QPE/QtopiaApplication:notBusy(QString)} \o The application has notified the system that it is not busy.
  \endtable

  Additionally the UIApplicationMonitor class uses the following value space
  keys to monitor application state.

  \table
  \header \o Value Space Key \o Description
  \row \o \c {/System/Applications/<app name>/Tasks/UI} \o True when the application is showing UI, false if not.
  \endtable

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  \enum UIApplicationMonitor::ApplicationState

  The ApplicationState enum represents the current state of a UI application.

  Every application will have at least on of the following states:

  \value NotRunning The application is not currently running, does not appear to be a UI application, or is not currently showing a UI.  An application is considered not running whenever the value space key \c {/System/Applications/<application>/Tasks/UI} is false.
  \value Starting The application is currently starting, in response to a \c {raise()} message.  The busy flag is automatically set for applications in the Starting state.  It is unset when they enter the Running state or terminate.
  \value Running The application is running and displaying a UI.  An application is considered running whenever the value space key \c {/System/Applications/<application>/Tasks/UI} is true.

  Additionally, zero or more of the following flags may be applied to any
  application in the Starting or Running state.

  \value NotResponding The application is not responding.  An application is marked as not responding if it does not send an \c {appRaised(QString)} message in response to a \c {raise()} message within the notRespondingTimeout() time limit.
  \value Busy The application is busy.
  \value Active The application is active.  The active application is the application that created the currently active window.

  The following masks are also supported.

  \value StateMask Masks the enumeration bits used by application states.
*/
/*!
  \fn void UIApplicationMonitor::applicationStateChanged(const QString &application, UIApplicationMonitor::ApplicationState newState)

  Emitted whenever \a application's state changes.  \a newState will be set
  to the application's new state.
 */

/*!
  \fn void UIApplicationMonitor::busy()

  Emitted when any application is busy.
 */

/*!
  \fn void UIApplicationMonitor::notBusy()

  Emitted when no applications are busy.
 */

/*!
  Construct a new UIApplicationMonitor instance with the provided \a parent.
 */
UIApplicationMonitor::UIApplicationMonitor(QObject *parent)
: QObject(parent), d(uiApplicationMonitorPrivate())
{
    QObject::connect(d, SIGNAL(applicationStateChanged(QString,UIApplicationMonitor::ApplicationState)), this, SIGNAL(applicationStateChanged(QString,UIApplicationMonitor::ApplicationState)));
    QObject::connect(d, SIGNAL(busy()), this, SIGNAL(busy()));
    QObject::connect(d, SIGNAL(notBusy()), this, SIGNAL(notBusy()));
}

/*!
  Destroy the UIApplicationMonitor instance.
 */
UIApplicationMonitor::~UIApplicationMonitor()
{
}

/*!
  Return a list of all running UI applications.  An application is "running" if
  it is showing a UI.
 */
QStringList UIApplicationMonitor::runningApplications() const
{
    return d->runningApplications();
}

/*!
  Return a list of all not responding applications.
 */
QStringList UIApplicationMonitor::notRespondingApplications() const
{
    return d->notRespondingApplications();
}

/*!
  Return a list of all busy applications.
 */
QStringList UIApplicationMonitor::busyApplications() const
{
    return d->busyApplications();
}

/*!
  Returns the current state of \a application.
 */
UIApplicationMonitor::ApplicationState
UIApplicationMonitor::applicationState(const QString &application) const
{
    return d->applicationState(application);
}

/*!
  Returns the current not responding timeout in milliseconds.
 */
int UIApplicationMonitor::notRespondingTimeout()
{
    return UIApplicationMonitorPrivate::notRespondingTimeout();
}

/*!
  Set the not responding timeout to \a timeout milliseconds.
 */
void UIApplicationMonitor::setNotRespondingTimeout(int timeout)
{
    UIApplicationMonitorPrivate::setNotRespondingTimeout(timeout);
}

#include "applicationmonitor.moc"

