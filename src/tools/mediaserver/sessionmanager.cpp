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

#include <QTimer>
#include <QValueSpaceObject>
#include <QMediaServerSession>
#include <QMediaSessionRequest>
#include <QDebug>

#include "mediaagent.h"
#include "domainmanager.h"
#include "sessionmanagersession.h"
#ifdef QTOPIA_TELEPHONY
#include "callmonitor.h"
#endif

#include "sessionmanager.h"


namespace mediaserver
{

// {{{ SessionStatusMonitor
class SessionStatusMonitor : public QObject
{
    Q_OBJECT

public:
    SessionStatusMonitor(QValueSpaceObject* statusStore,
                   QMediaServerSession* session,
                   QObject* parent = 0);

private slots:
    void sessionStateChanged(QtopiaMedia::State state);
    void sessionDestroyed();

private:
    QValueSpaceObject*      m_statusStore;
    QString                 m_sessionId;
    QMediaServerSession*    m_session;
};

SessionStatusMonitor::SessionStatusMonitor
(
 QValueSpaceObject* statusStore,
 QMediaServerSession* session,
 QObject* parent
):
    QObject(parent),
    m_statusStore(statusStore),
    m_session(session)
{
    connect(m_session, SIGNAL(playerStateChanged(QtopiaMedia::State)),
            this, SLOT(sessionStateChanged(QtopiaMedia::State)));

    connect(m_session, SIGNAL(destroyed(QObject*)),
            this, SLOT(sessionDestroyed()));

    m_sessionId = session->id();

    m_statusStore->setAttribute(m_sessionId + "/Domain", session->domain());
    m_statusStore->setAttribute(m_sessionId + "/Data", session->reportData());
    m_statusStore->setAttribute(m_sessionId + "/State", int(session->playerState()));
}

void SessionStatusMonitor::sessionStateChanged(QtopiaMedia::State state)
{
    m_statusStore->setAttribute(m_sessionId + "/State", int(state));
}

void SessionStatusMonitor::sessionDestroyed()
{
    m_statusStore->removeAttribute(m_sessionId);
    deleteLater();
}
// }}}

// {{{ SessionInfo
class SessionInfo
{
public:
    enum SessionState { Starting, Activated, Deactivated, Stopped };

    SessionInfo():state(Stopped) {}

    SessionState    state;
};
// }}}

// {{{ SessionManagerPrivate
class SessionManagerPrivate : public QObject
{
    Q_OBJECT

    typedef QMap<SessionManagerSession*, SessionInfo>   ManagedSessions;

public:
    SessionManagerPrivate(SessionManager* manager);
    ~SessionManagerPrivate();

public slots:
    void sessionStateChange(QtopiaMedia::State state);
    void domainStatusChange(QStringList const& newDomains, QStringList const& oldDomains);
    void callActivityChanged(bool active);

public:
    MediaAgent*         mediaAgent;
    DomainManager*      domainManager;
    SessionManager*     sessionManager;
    ManagedSessions     managedSessions;
    QValueSpaceObject*  status;
    QList<QMediaServerSession*> sessions;
    QSet<QMediaServerSession*> activeSessions;
};

SessionManagerPrivate::SessionManagerPrivate(SessionManager* manager):
    sessionManager(manager)
{
    mediaAgent = MediaAgent::instance();
    domainManager = DomainManager::instance();

    status = new QValueSpaceObject("/Media/Sessions");

    connect(domainManager, SIGNAL(domainStatusChange(QStringList,QStringList)),
            this, SLOT(domainStatusChange(QStringList,QStringList)));

#ifdef QTOPIA_TELEPHONY
    CallMonitor* monitor = new CallMonitor(this);
    connect(monitor, SIGNAL(callActivityChanged(bool)), SLOT(callActivityChanged(bool)));
#endif
}

SessionManagerPrivate::~SessionManagerPrivate()
{
    delete status;
}

void SessionManagerPrivate::domainStatusChange
(
 QStringList const& activeDomains,
 QStringList const& inactiveDomains
)
{
    ManagedSessions::iterator it;

    // Deactivate any
    for (it = managedSessions.begin(); it != managedSessions.end(); ++it)
    {
        if (!domainManager->isActiveDomain(it.key()->domain()))
        {
            switch (it.value().state)
            {
            case SessionInfo::Activated:
                it.key()->deactivate();
                it.value().state = SessionInfo::Deactivated;
                break;

            case SessionInfo::Starting:
            case SessionInfo::Deactivated:
            case SessionInfo::Stopped:
                break;
            }
        }
    }

    // Activate any
    for (it = managedSessions.begin(); it != managedSessions.end(); ++it)
    {
        if (domainManager->isActiveDomain(it.key()->domain()))
        {
            switch (it.value().state)
            {
            case SessionInfo::Starting:
            case SessionInfo::Deactivated:
                it.key()->activate();
                it.value().state = SessionInfo::Activated;
                break;

            case SessionInfo::Activated:
            case SessionInfo::Stopped:
                break;
            }
        }
    }

    Q_UNUSED(activeDomains);
    Q_UNUSED(inactiveDomains);
}

void SessionManagerPrivate::callActivityChanged(bool active)
{
    if (active)
        domainManager->activateDomain("RingTone");
    else
        domainManager->deactivateDomain("RingTone");
}

SessionManager* SessionManager::instance()
{
    static SessionManager self;

    return &self;
}

void SessionManagerPrivate::sessionStateChange(QtopiaMedia::State state)
{
    QMediaServerSession *session = qobject_cast<QMediaServerSession *>(sender());
    if (!session)
        return;

    int current = activeSessions.count();

    if (state == QtopiaMedia::Playing || state == QtopiaMedia::Buffering)
        activeSessions.insert(session);
    else
        activeSessions.remove(session);

    if (current != activeSessions.count())
        sessionManager->activeSessionCountChanged(activeSessions.count());
}
// }}}

/*!
    \class mediaserver::SessionManager
    \internal
*/

// {{{ SessionManager
SessionManager::SessionManager()
{
    d = new SessionManagerPrivate(this);
}

SessionManager::~SessionManager()
{
    delete d;
}

QMediaServerSession* SessionManager::createSession(QMediaSessionRequest const& sessionRequest)
{
    QMediaServerSession*      mediaSession;

    // Create the session
    mediaSession = d->mediaAgent->createSession(sessionRequest);

    if (mediaSession != 0)
    {
        QString const& domain = sessionRequest.domain();

        // Set domain
        mediaSession->setDomain(domain);

        // Listen to aggregated status to set suspend state of device
        connect(mediaSession, SIGNAL(playerStateChanged(QtopiaMedia::State)),
                d, SLOT(sessionStateChange(QtopiaMedia::State)));

        // Monitor status of the session
        new SessionStatusMonitor(d->status, mediaSession, this);

        // Wrap
        SessionManagerSession*  session = new SessionManagerSession(this, mediaSession);

        d->managedSessions.insert(session, SessionInfo());

        mediaSession = session;

        d->sessions.append(mediaSession);
    }

    return mediaSession;
}

void SessionManager::destroySession(QMediaServerSession* mediaSession)
{
    // Unwrap
    SessionManagerSession*  wrapper = qobject_cast<SessionManagerSession*>(mediaSession);
    QMediaServerSession*    session = wrapper->wrappedSession();

    d->managedSessions.remove(wrapper);
    delete wrapper;

    // send to agent to remove
    d->mediaAgent->destroySession(session);

    d->sessions.removeAll(mediaSession);
}

bool SessionManager::sessionCanStart(SessionManagerSession* session)
{
    bool            rc = false;
    SessionInfo&    info = d->managedSessions[session];

    switch (info.state)
    {
    case SessionInfo::Starting:
        d->domainManager->activateDomain(session->domain());
    case SessionInfo::Deactivated:
        rc = false;
        break;

    case SessionInfo::Activated:
        rc = true;
        break;

    case SessionInfo::Stopped:
        rc = d->domainManager->activateDomain(session->domain());
        info.state = rc ? SessionInfo::Activated : SessionInfo::Starting;
        break;
    }

    return rc;
}

void SessionManager::sessionStopped(SessionManagerSession* session)
{
    //to ensure the domain is deactivated only once even if multiple stopped status messages are received
    if (d->managedSessions[session].state != SessionInfo::Stopped) {
        d->domainManager->deactivateDomain(session->domain());
        d->managedSessions[session].state = SessionInfo::Stopped;
    }
}

QList<QMediaServerSession*> const& SessionManager::sessions() const
{
    return d->sessions;
}
// }}} SessionManager

}   // ns mediaserver


#include "sessionmanager.moc"




