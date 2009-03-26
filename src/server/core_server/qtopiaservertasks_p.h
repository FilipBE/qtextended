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

#ifndef QTOPIASERVERTASKS_P_H
#define QTOPIASERVERTASKS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtopiaserverapplication.h>

#include <QConstCString>
#include <QPluginManager>
#include <QPointer>
#include <QValueSpaceObject>

class QPluginManager;
class ServerTaskPlugin;
class TaskStartupInfo;

class QtopiaServerTasksPrivate : public QObject
{
Q_OBJECT
public:
    inline
    QtopiaServerTasksPrivate() : argc(0), argv(0),
                                 taskList(0), application(0),
                                 m_shutdown(QtopiaServerApplication::NoShutdown),
                                 m_constructingTask(false), m_taskPluginLoader(0) {}
    virtual inline ~QtopiaServerTasksPrivate() {
        qDeleteAll(m_availableTasks);
        delete taskList;
        if (m_taskPluginLoader)
            delete m_taskPluginLoader;
    }

    static QString taskConfigFile();
    struct Task;

    void enableTaskReporting();
    bool determineStartupOrder(const QList<QByteArray> &, QList<Task *> &);
    void determineStartupOrder(const QByteArray &, bool, bool,
                               TaskStartupInfo &);

    void setupInterfaceList(const QList<Task *> &);
    QObject* startTask(Task *, bool onlyActive);
    QObject* taskForInterface(Task *task, const QConstCString &iface, bool onlyActive);

    int *argc;
    char **argv;
    QValueSpaceObject *taskList;
    QtopiaApplication *application;

    struct Task {
        inline Task() {}
        inline
        Task(const QConstCString &_name) : name(_name),
                                           object(0),
                                           staticOrder(0),
                                           launchOrder(0),
                                           interfaceOrder(0),
                                           create(0),
                                           createArg(0),
                                           excluded(false),
                                           demand(false),
                                           plugin(0) {}
        inline
        Task(const Task &other) : name(other.name),
                                  object(other.object),
                                  staticOrder(other.staticOrder),
                                  launchOrder(other.launchOrder),
                                  interfaceOrder(other.interfaceOrder),
                                  create(other.create),
                                  createArg(other.createArg),
                                  interfaces(other.interfaces),
                                  excluded(other.excluded),
                                  demand(other.demand) {}

        QConstCString name;
        QObject *object;
        QObjectList aggregates;
        unsigned int staticOrder;
        unsigned int launchOrder;
        unsigned int interfaceOrder;
        QtopiaServerApplication::CreateTaskFunc create;
        void *createArg;
        QList<QConstCString> interfaces;
        bool excluded;
        bool demand;
        ServerTaskPlugin* plugin;
        QByteArray taskPluginName;     //required as store for plugin names (QConstCString) only keeps references
    };

    QMap<QObject *, Task *> m_activeTasks;
    QMap<QConstCString, Task *> m_availableTasks;
    QMap<QConstCString, QList<Task *> > m_availableInterfaces;
    QMap<QConstCString, QList<QPointer<QObject> > > m_additionalInterfaces;
    QList<QPointer<QObject> > m_startedTaskObjects;
    QSet<QObject *> m_excludedManagement;

    QList<SystemShutdownHandler *> m_shutdownObjects;
    QtopiaServerApplication::ShutdownType m_shutdown;
    inline bool shutdownDone() const { return m_shutdownObjects.isEmpty(); }
    void setShutdownObjects(const QList<SystemShutdownHandler *> &objs);
    void ackShutdownObject(SystemShutdownHandler *obj);
    void cleanupTasks();

    bool m_constructingTask;
    QPluginManager* m_taskPluginLoader;
    QMap<QObject *, QList<QObject *> > m_constructingAggregates;

public slots:
    void shutdownProceed();
};

#endif

