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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qtuitestconnectionmanager_p.h"

#include <QCoreApplication>
#include <QObject>
#include <QThread>

// from qobject_p.h
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject*,int,void**);
    typedef void (*EndCallback)(QObject*,int);
    BeginCallback signal_begin_callback,
                  slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
extern void Q_CORE_EXPORT qt_register_signal_spy_callbacks(QSignalSpyCallbackSet const&);
void qtuitest_signal_begin(QObject*,int,void**);

Q_GLOBAL_STATIC(QtUiTestConnectionManager,_q_qtUiTestConnectionManager);

QtUiTestConnectionManager::QtUiTestConnectionManager()
{
    QSignalSpyCallbackSet callbacks = { qtuitest_signal_begin, 0, 0, 0 };
    qt_register_signal_spy_callbacks(callbacks);
}

QtUiTestConnectionManager::~QtUiTestConnectionManager()
{
    QSignalSpyCallbackSet callbacks = { 0, 0, 0, 0 };
    qt_register_signal_spy_callbacks(callbacks);
}

void qtuitestconnectionmanager_cleanup()
{ delete _q_qtUiTestConnectionManager(); }

QtUiTestConnectionManager* QtUiTestConnectionManager::instance()
{
    return _q_qtUiTestConnectionManager();
}

void qtuitest_signal_begin(QObject* sender, int signal, void** argv)
{
    QtUiTestConnectionManager* cm = QtUiTestConnectionManager::instance();

    // Should only ever be null when the application is shutting down, but in that case,
    // this callback should already have been unregistered.
    Q_ASSERT(cm);

    // During the destructor it isn't safe to check which thread we are.
    // We'll just ignore all signals during this time.
    if (QCoreApplication::closingDown()) return;

    // Connections are only supported in the main thread.
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) return;

    cm->activateConnections(sender,signal,argv);
}

void QtUiTestConnectionManager::connect(QObject const* sender, int senderMethod,
        QObject const* receiver, int receiverMethod)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    Connection c = {
        const_cast<QObject*>(sender),   senderMethod,
        const_cast<QObject*>(receiver), receiverMethod
    };
    m_connections << c;
}

bool QtUiTestConnectionManager::disconnect(QObject const* sender, int senderMethod,
        QObject const* receiver, int receiverMethod)
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    bool ret = false;

    QList<Connection>::iterator iter = m_connections.begin();
    while (iter != m_connections.end())
    {
        bool remove = true;

        if (sender && (sender != iter->sender))                                 remove = false;
        if ((senderMethod != -1) && (senderMethod != iter->senderMethod))       remove = false;
        if (receiver && (receiver != iter->receiver))                           remove = false;
        if ((receiverMethod != -1) && (receiverMethod != iter->receiverMethod)) remove = false;

        if (remove) {
            ret = true;
            iter = m_connections.erase(iter);
        }
        else {
            ++iter;
        }
    }

    return ret;
}

void QtUiTestConnectionManager::activateConnections(QObject* sender, int senderMethod, void** argv)
{
    static const int destroyedMethod = QObject::staticMetaObject.indexOfSignal("destroyed(QObject*)");
    const bool destroyed = (senderMethod == destroyedMethod);

    // Find all of the connections we need to activate.
    QList<Connection> toActivate;
    QList<Connection>::iterator iter = m_connections.begin();
    while (iter != m_connections.end())
    {
        if (sender == iter->sender && senderMethod == iter->senderMethod)
            toActivate << *iter;

        // Remove this connection if either the sender or receiver is being destroyed
        if (destroyed && (sender == iter->sender || sender == iter->receiver))
            iter = m_connections.erase(iter);
        else
            ++iter;
    }

    foreach (Connection const& c, toActivate)
    { c.receiver->qt_metacall(QMetaObject::InvokeMetaMethod, c.receiverMethod, argv); }
}

