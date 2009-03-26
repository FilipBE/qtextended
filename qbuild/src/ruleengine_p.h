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

#ifndef RULEENGINE_P_H
#define RULEENGINE_P_H

#include "ruleengine.h"
#include "project.h"
#include "qfastdir.h"

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class RuleEngineThread;
class RuleEngineThread;
class RuleVariables;
class SchedulerRule;
class RuleEngineState;
class RuleEngineScheduler;

class RuleVariables : public IInternalVariable
{
public:
    RuleVariables(Rule *rule, RuleEngineThread *thread);

    virtual bool value(const QString &, QStringList &rv);

private:
    RuleEngineThread *m_thread;
    Rule *m_rule;
};

// =====================================================================

class SchedulerRule
{
public:
    SchedulerRule() : _state(0) {}

    SchedulerRule(const SchedulerRule &o) : _state(o._state) {}
    SchedulerRule &operator=(const SchedulerRule &o) { _state = o._state; return *this; }
    bool operator==(const SchedulerRule &o) { return (_state == o._state); }

    inline Project *project() const;
    inline Rule *rule(bool allow_null = false) const;
    inline RuleEngineState *state() const;

private:
    friend class RuleEngineScheduler;
    RuleEngineState *_state;
};

// =====================================================================

class RuleEngineState
{
public:
    RuleEngineState();

    int waitCount;

    enum ExecuteState {
        NotStarted,
        Queued,
        Waiting,
        Throttled,
        Executing,
        DoneNothingToDo,
        DoneFailed,
        DoneSucceeded,
        DoneNotFound,
        DoneStart = DoneNothingToDo
    };
    ExecuteState executeState;

    QList<SchedulerRule> depRules;
    int serialProgression;
    bool succeeded;
    QList<uint> depOptions;
    QStringList inputFiles;

    enum Error {
        NoError,
        PrereqFailed,
        CommandEvalFail,
        CommandExecFail,
        PrereqEvalFail,
        PrereqFuncFail,
        PrereqMissing,
        TestEvalFail,
        InputEvalFail,
        InputFuncFail,
        InputMissing,
        NotFound
    };
    Error error;
    QString errorDesc;
    Rule *errorRule;

    Rule *rule;
    QString ruleName;
};

#define RULE_ERROR_TYPE(rule, type, causerule) \
    { \
        rule->state->error = RuleEngineState::type; \
        rule->state->errorRule = causerule; \
    }
#define RULE_ERROR_DESC(rule, type, desc) \
    { \
        rule->state->error = RuleEngineState::type; \
        rule->state->errorDesc = (desc); \
    }
#define RULESTATE_ERROR_DESC(state, type, desc) \
    { \
        state->error = RuleEngineState::type; \
        state->errorDesc = (desc); \
    }

// =====================================================================

class RuleEngineScheduler : public QObject
{
public:
    RuleEngineScheduler();

    void scheduleRule(const SchedulerRule &);
    void waitUntilDone();

    void waitOnRule(const SchedulerRule &me,
                    const QList<SchedulerRule> &requiredRules);
    void completeRule(const SchedulerRule &, RuleEngineState::ExecuteState);
    void throttledRule(const SchedulerRule &);

    bool beginCategory(const QStringList &);
    void endCategory(const QStringList &);

    SchedulerRule threadIdle(RuleEngineThread *, Project *);

    SchedulerRule sruleFromName(const QString &);
    SchedulerRule sruleFromRule(Rule *);

    typedef QList<RuleEngineThread *> Threads;
    Threads threads;

    void dumpCirc(Rule *);
    void dumpCirc(RuleEngineState *state, QList<SchedulerRule> &rules);

    QHash<QString, int> guiCategoryRules() { QMutexLocker locker(&lock); return m_guiCategoryRules; }

private:
    QMutex lock; // Protects readyQueue and waitingQueue
    QWaitCondition wait;

    void addReadyRule(const SchedulerRule &);

    typedef QMultiHash<Project *, SchedulerRule> ReadyQueue;
    ReadyQueue readyQueue;

    typedef QMultiHash<RuleEngineState *, SchedulerRule> WaitingQueue;
    WaitingQueue waitingQueue;

    typedef QList<RuleEngineThread *> IdleThreads;
    IdleThreads idleThreads;

    int ruleCount;
    int ruleCompleteCount;

    typedef QHash<QString, RuleEngineState *> EngineStates;
    EngineStates engineStates;

    virtual bool event(QEvent *);

    bool waitingUntilDone;

    bool _beginCategory(const QStringList &);
    void _endCategory(const QStringList &);
    QHash<QString, int> runningCategories; // for throttling
    QHash<QString, int> m_guiCategoryRules; // for the GUI
};

// =====================================================================

class RuleEngineThread : public QThread
{
public:
    RuleEngineThread(int id, RuleEngineScheduler *, bool projectThread);
    virtual ~RuleEngineThread();

    void runRule(const SchedulerRule &);

    bool m_projectThread;

private:
    void run();

    void enterProject(Project *);

    SchedulerRule prereqSrule(const QString &);
    SchedulerRule sruleForFile(const QString &);

    bool prereqs();
    bool inputFiles();
    RuleEngine::Result commands();

    void runRule();
    void getRule();

    friend class RuleVariables;
    QString absfile(const QString &);

    QWaitCondition m_wait;
    QMutex m_lock;
    bool m_isDone;

    SchedulerRule m_srule;

    QString m_cwd;
    Project *m_lastProject;

    RuleEngineScheduler *m_scheduler;

    int m_id;

    QFastDirCache fastDir;
};

#endif

