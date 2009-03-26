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

#include "ruleengine.h"
#include "ruleengine_p.h"
#include "qbuild.h"
#include "project.h"
#include <QWaitCondition>
#include <QMutex>
#include "qfastdir.h"
#include "process.h"
#include <QDir>
#include <QThread>
#include "qoutput.h"
#include <QSet>
#include <QCoreApplication>
#include "options.h"
#include "functionprovider.h"
#include "gui.h"
#include <QFile>

class TraceFile : public QFile
{
public:
    TraceFile()
        : QFile(::options.traceFile)
    {
        open(QIODevice::WriteOnly);
    }
};
Q_GLOBAL_STATIC(TraceFile,traceFile)
inline QOutput qTraceOutput() { return (::options.traceFile)?QOutput(traceFile()).eol():QOutput(QtWarningMsg); }

// Show categories (in the -gui) for rules that are Queued, Waiting and Throttled
#define EXTRA_CATEGORIES

// Only process "qbuild" stuff on the first QThread::idealThreadCount() threads
//#define PROJECT_THREADS

/*!
  \class Rule
  \sa {Rule JavaScript Class Reference}
*/

/*!
  \enum Rule::Flags
  \value None
  \value SerialRule
*/

/*!
  Construct an anonymous rule in group \a rules.
*/
Rule::Rule(Rules *rules)
: name("Anonymous" + QString::number(rules->ruleNum())),
        flags(None), state(0), m_rules(rules), forceRule(false)
{
}

/*!
  Construct a rule with name \a nme.
*/
Rule::Rule(const QString &nme, Rules *rules)
: name(nme), flags(None), state(0), m_rules(rules), forceRule(false)
{
    Q_ASSERT(!name.isEmpty());
}

/*!
  \internal
*/
Rule::Rule(const Rule &o)
: name(o.name), help(o.help),
  inputFiles(o.inputFiles),
  prerequisiteActions(o.prerequisiteActions),
  other(o.other), commands(o.commands),
  tests(o.tests),
  category(o.category),
  flags(o.flags),
  state(o.state),
  m_rules(o.m_rules),
  m_outputFiles(o.m_outputFiles),
  forceRule(false)
{
}

/*!
  Returns the name of the rule.
*/
QString Rule::ruleName() const
{
    return m_rules->name() + name;
}

/*!
  Returns the rule group?
*/
Rules *Rule::rules() const
{
    return m_rules;
}

/*!
  \internal
*/
Rule &Rule::operator=(const Rule &o)
{
    name = o.name;
    help = o.help;
    m_outputFiles = o.m_outputFiles;
    inputFiles = o.inputFiles;
    prerequisiteActions = o.prerequisiteActions;
    other = o.other;
    commands = o.commands;
    tests = o.tests;
    category = o.category;
    flags = o.flags;
    state = o.state;
    m_rules = o.m_rules;

    return *this;
}

/*!
  Returns the output files for the rule.
*/
const QStringList &Rule::outputFiles() const
{
    return m_outputFiles;
}

/*!
  Set the output files for the rule to \a val.
*/
void Rule::setOutputFiles(const QStringList &val)
{
    foreach(QString file, m_outputFiles) {
        m_rules->m_outputFiles.remove(file);
    }
    m_outputFiles = val;
    foreach(QString file, m_outputFiles) {
        m_rules->m_outputFiles.insert(file, this);
    }
}

/*!
  Add \a val to the list of output files for the rule.
*/
void Rule::appendOutputFile(const QString &val)
{
    Q_ASSERT(!m_outputFiles.contains(val));
    m_outputFiles << val;
    m_rules->m_outputFiles.insert(val, this);
}

/*!
  \variable Rule::help
*/

/*!
  \variable Rule::inputFiles
*/

/*!
  \variable Rule::prerequisiteActions
*/

/*!
  \variable Rule::other
*/

/*!
  \variable Rule::commands
*/

/*!
  \variable Rule::tests
*/

/*!
  \variable Rule::category
*/

/*!
  \variable Rule::flags
*/

/*!
  \variable Rule::state
*/

// =====================================================================

/*!
  \class Rules
*/

/*!
  Construct a Rules with name \a name and project \a p.
*/
Rules::Rules(const QString &name, Project *p)
: m_ruleNum(0), m_name(name), m_project(p)
{
}

/*!
  Returns the name of the rules group.
*/
QString Rules::name() const
{
    return m_name;
}

/*!
  Returns the project.
*/
Project *Rules::project() const
{
    return m_project;
}

/*!
  Returns a unique rule number.
*/
int Rules::ruleNum()
{
    return m_ruleNum++;
}

/*!
  Returns the rules.
*/
QList<Rule *> Rules::rules() const
{
    return m_rules.values();
}

/*!
  Returns a rule by \a name.
  Returns null if the rule cannot be found.
*/
Rule *Rules::ruleByName(const QString &name) const
{
    Q_ASSERT(!name.isEmpty());

    RuleMap::ConstIterator iter = m_rules.find(name);
    if (iter == m_rules.end())
        return 0;
    else
        return *iter;
}

/*!
  Returns the rule that freshens \a name.
  Returns null if a rule cannot be found.
*/
Rule *Rules::ruleForFile(const QString &name) const
{
    Q_ASSERT(!name.isEmpty());

    RuleMap::ConstIterator iter = m_outputFiles.find(name);
    if (iter == m_outputFiles.end())
        return 0;
    else
        return *iter;
}

/*!
  Add a \a rule to the group.
*/
Rule * Rules::addRule(const Rule &rule)
{
    Rule *rv = new Rule(rule);
    m_rules.insert(rv->name, rv);
    return rv;
}

/*!
  Remove a \a rule from the group.
*/
void Rules::removeRule(Rule *rule)
{
    RuleMap::Iterator iter = m_rules.find(rule->name);
    Q_ASSERT(iter != m_rules.end());
    m_rules.erase(iter);
}

/*!
  Remove a rule by \a name.
*/
void Rules::removeRule(const QString &name)
{
    RuleMap::Iterator iter = m_rules.find(name);
    Q_ASSERT(iter != m_rules.end());
    m_rules.erase(iter);
}


// =====================================================================

/*!
  \class SchedulerRule
*/

/*!
  \internal
  \fn SchedulerRule::SchedulerRule()
*/
/*!
  \internal
  \fn SchedulerRule::SchedulerRule(const SchedulerRule&)
*/
/*!
  \internal
  \fn SchedulerRule::operator=(const SchedulerRule&)
*/
/*!
  \internal
  \fn SchedulerRule::operator==(const SchedulerRule&)
*/

/*!
  Returns the project associated with this SchedulerRule.
*/
Project *SchedulerRule::project() const
{
    if (_state && _state->rule)
        return _state->rule->rules()->project();
    else
        return 0;
}

/*!
  Returns the rule associated with this SchedulerRule.
  Asserts if rule is null unless \a allow_null is set to true;
*/
Rule *SchedulerRule::rule(bool allow_null) const
{
    Rule *rule = 0;
    if ( _state ) {
        rule = _state->rule;
        if ( !allow_null && !rule ) {
            qWarning() << "SchedulerRule::rule got 0 for rule name" << _state->ruleName;
            abort();
        }
    } else {
        if ( !allow_null ) {
            qWarning() << "SchedulerRule::rule got 0 for rule with no state!";
            abort();
        }
    }
    return rule;
}

/*!
  Returns the state associated with this SchedulerRule.
*/
RuleEngineState *SchedulerRule::state() const
{
    return _state;
}

// =====================================================================

/*!
  \class RuleEngineState
*/

/*!
  \internal
*/
RuleEngineState::RuleEngineState()
    : waitCount(0),
    executeState(NotStarted),
    serialProgression(0),
    error(NoError),
    errorRule(0),
    rule(0)
{
}

/*!
  \variable RuleEngineState::waitCount
*/

/*!
  \enum RuleEngineState::ExecuteState
  \value NotStarted Nothing done
  \value Queued Been queued for run
  \value Waiting Waiting for prerequisites
  \value Throttled Waiting for non-prerequisites
  \value Executing Running commands
  \value DoneNothingToDo Completed with no action
  \value DoneStart
  \value DoneFailed Completed with failed action
  \value DoneSucceeded Completed successfully
  \value DoneNotFound Proxy rule not found
*/

/*!
  \variable RuleEngineState::executeState
*/

/*!
  \variable RuleEngineState::depRules
*/

/*!
  \variable RuleEngineState::serialProgression
*/

/*!
  \variable RuleEngineState::depOptions
*/

/*!
  \variable RuleEngineState::inputFiles
*/

/*!
  \enum RuleEngineState::Error
  \value NoError
  \value PrereqFailed
  \value CommandEvalFail
  \value CommandExecFail
  \value PrereqEvalFail
  \value PrereqFuncFail
  \value PrereqMissing
  \value TestEvalFail
  \value InputEvalFail
  \value InputFuncFail
  \value InputMissing
  \value NotFound
*/

/*!
  \variable RuleEngineState::error
*/

/*!
  \variable RuleEngineState::errorDesc
*/

/*!
  \variable RuleEngineState::errorRule
*/

/*!
  \variable RuleEngineState::rule
*/

/*!
  \variable RuleEngineState::ruleName
*/


/*!
  \macro RULE_ERROR_TYPE(rule, type, causerule)
  \relates RuleEngineState
  Set \a rule RuleEngineState::error to \a type and
  RuleEngineState::errorRule to \a causerule.
*/

/*!
  \macro RULE_ERROR_DESC(rule, type, desc)
  \relates RuleEngineState
  Set \a rule RuleEngineState::error to \a type and
  RuleEngineState::errorDesc to \a desc.
*/

/*!
  \macro RULESTATE_ERROR_DESC(state, type, desc)
  \relates RuleEngineState
  Set \a state RuleEngineState::error to \a type and
  RuleEngineState::errorDesc to \a desc.
*/

// =====================================================================

/*!
  \class RuleEngineThread
*/

/*!
  \internal
*/
RuleEngineThread::RuleEngineThread(int id, RuleEngineScheduler *scheduler, bool projectThread)
: m_projectThread(projectThread), m_isDone(false), m_lastProject(0), m_scheduler(scheduler), m_id(id)
{
    start();
}

/*!
  \internal
*/
RuleEngineThread::~RuleEngineThread()
{
}

/*!
  Run scheduler rule \a srule on this thread.

  Only call with this unlocked.
*/
void RuleEngineThread::runRule(const SchedulerRule &srule)
{
    LOCK(RuleEngineThread);
    m_lock.lock();
    Q_ASSERT(0 == m_srule.state());
    m_srule = srule;
    m_wait.wakeAll();
    m_lock.unlock();
}

/*!
  Main loop for the thread.
*/
void RuleEngineThread::run()
{
    LOCK(RuleEngineThread);
    m_lock.lock();
    while (!m_isDone) {
        getRule();

        while (m_srule.state()) {
            runRule();
            getRule();
        }

        m_wait.wait(&m_lock);
    }
    m_lock.unlock();
}

/*!
  Fetch a scheduler rule to run.

  Only call with this locked.
*/
void RuleEngineThread::getRule()
{
    if (m_srule.state()) return;

    m_srule = m_scheduler->threadIdle(this, m_lastProject);
}

// This would be a static variable in the RuleEngineThread::enterProject function
// but that is not thread-safe
struct EnteredProjects {
    QMutex lock;
    QSet<Project *> projects;
};
Q_GLOBAL_STATIC(EnteredProjects, enteredProjects)

/*!
  Open \a project. If it has beeen opened before, the cached copy will be returned.
*/
void RuleEngineThread::enterProject(Project *project)
{
    Q_ASSERT(project);
    m_lastProject = project;
    QString dir = project->solution()->findFile(project->nodePath(),
                                                Solution::Generated).fsPath();

    EnteredProjects *e = enteredProjects();
    LOCK(RuleEngineThread::EnteredProjects);
    e->lock.lock();
    if (!e->projects.contains(project)) {
        QString name = project->name();
        if (name.length() > 1)
            name.chop(1);
        if ( !options.silent ) qOutput() << "QBuild: Running rules for project" << name;
        e->projects.insert(project);
    }
    e->lock.unlock();

    if (!fastDir.exists(dir)) {
        QDir d;
        d.mkpath(dir);
    }

    m_cwd = dir + "/";
}

/*!
  Run the current scheduler rule.

  Only call with this locked.
*/
void RuleEngineThread::runRule()
{
    // First ensure that this scheduler rule points to a rule
    if (!m_srule.rule(true)) {
//        qWarning() << "Resolving proxy rule" << m_srule.ruleName() << m_srule.state();
        QString projName = m_srule.state()->ruleName;
        int idx = projName.lastIndexOf('/');
        Q_ASSERT(idx != -1);
        QString ruleName = projName.mid(idx + 1);
        projName = projName.left(idx);

        SolutionProject hierarchy = SolutionProject::fromNode(projName);
        Project *project = hierarchy.project();
        Rule *rule = project->projectRules()->ruleByName(ruleName);
        if (!rule) {
            RULESTATE_ERROR_DESC(m_srule.state(), NotFound, "");
            m_scheduler->completeRule(m_srule, RuleEngineState::DoneNotFound);
            m_srule = SchedulerRule();
            return;
        } else {
            m_srule = m_scheduler->sruleFromRule(rule);
        }
    }

    if (m_srule.project() && m_srule.project() != m_lastProject)
        enterProject(m_srule.project());

    Rule *rule = m_srule.rule();

    Project *project = m_srule.project();

    if (options.debug & Options::RuleExec)
        qTraceOutput() << "QBuild(" << m_id << "): Running rule"
                   << rule->ruleName();

    Q_ASSERT(rule->state->executeState == RuleEngineState::Queued ||
             rule->state->executeState == RuleEngineState::Waiting ||
             rule->state->executeState == RuleEngineState::Throttled);

    if (rule->state->executeState == RuleEngineState::Queued) {
        if (options.debug & Options::RuleExec)
            qTraceOutput() << "QBuild(" << m_id << "):     Priming...";
        // Calculate prerequisites
        if (!prereqs() || !inputFiles()) {
            // Something went wrong
            m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
            m_srule = SchedulerRule();
            return;
        }

        if (options.debug & Options::RuleExec) {
            QStringList prereqList;
            for (int ii = 0; ii < rule->state->depRules.count(); ++ii) {
                prereqList << rule->state->depRules.at(ii).state()->ruleName;
            }

            qTraceOutput() << "QBuild(" << m_id << "): Primed rule:" << rule->ruleName();
            qTraceOutput() << "QBuild(" << m_id << "):         Prerequisites:" << prereqList;
            qTraceOutput() << "QBuild(" << m_id << "):         Input files:" << rule->state->inputFiles;

#if 0
            // Code to write out a (huge) .dot file showing all of the rules and how they relate to each other
            // I suspect that to be useful, this would need to be pruned (eg. don't show the dependencies of .o files)
            QFile file("rules.dot");
            file.open(QIODevice::Append);
            {
                QTextStream stream( &file );
                stream << "digraph rules {" << endl;
                foreach ( QString prereq, prereqList )
                    stream << "\"" << rule->ruleName() << "\" -> \"" << prereq << "\";" << endl;
                foreach ( QString prereq, rule->state->inputFiles )
                    stream << "\"" << rule->ruleName() << "\" -> \"" << prereq << "\";" << endl;
                stream << "}" << endl;
            }
#endif
        }

        rule->state->executeState = RuleEngineState::Waiting;
#ifdef EXTRA_CATEGORIES
        m_scheduler->endCategory(QStringList() << "RuleEngineState::Queued");
        m_scheduler->beginCategory(QStringList() << "RuleEngineState::Waiting");
#endif

        if (!rule->state->depRules.isEmpty()) {
            if (rule->flags & Rule::SerialRule) {
                if (options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):     Waiting on" << rule->state->depRules.first().state()->ruleName;

                rule->state->serialProgression = 1;
                m_scheduler->waitOnRule(m_srule, QList<SchedulerRule>() << rule->state->depRules.first());
            } else {
                if (options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):     Waiting on prerequisites.";
                m_scheduler->waitOnRule(m_srule, rule->state->depRules);
            }
            m_srule = SchedulerRule();
            return;
        }
    }

    if (rule->state->executeState == RuleEngineState::Waiting) {
        bool run = false;
        rule->state->succeeded = false;

        if (rule->flags & Rule::SerialRule &&
            rule->state->serialProgression < rule->state->depRules.count())
        {
            SchedulerRule srule = rule->state->depRules.at(rule->state->serialProgression - 1);
            RuleEngineState *state = srule.state();
            if ( state->executeState == RuleEngineState::DoneNotFound ) {
                if (rule->state->depOptions.at(rule->state->serialProgression - 1) & RuleEngine::Optional) {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Ignoring non-existant optional prereq" << state->ruleName;
                } else {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Failing on non-existant prereq" << state->ruleName;
                    RULE_ERROR_DESC(rule, PrereqMissing, state->ruleName);
                    m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                    m_srule = SchedulerRule();
                    return;
                }
            }

            Rule *depRule = srule.rule(true);
            if (depRule && depRule->state->executeState == RuleEngineState::DoneFailed) {
                if (!(rule->state->depOptions.at(rule->state->serialProgression - 1) & RuleEngine::NoFail)) {
                    if (options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Failing serial rule due to failed prerequisite" << depRule->ruleName();

                    RULE_ERROR_TYPE(rule, PrereqFailed, depRule);
                    m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                    m_srule = SchedulerRule();
                    return;
                }
            }

            if (options.debug & Options::RuleExec)
                qTraceOutput() << "QBuild(" << m_id << "):     Waiting on" << rule->state->depRules.at(rule->state->serialProgression).state()->ruleName;

            rule->state->serialProgression++;
            m_scheduler->waitOnRule(m_srule, QList<SchedulerRule>() << rule->state->depRules.at(rule->state->serialProgression - 1));
            m_srule = SchedulerRule();
            return;
        }

        if (options.debug & Options::RuleExec)
            qTraceOutput() << "QBuild(" << m_id << "):     Testing...";

        // Test dep rule states
        for (int ii = 0; ii < rule->state->depRules.count(); ++ii) {
            SchedulerRule srule = rule->state->depRules.at(ii);
            RuleEngineState *state = srule.state();
            if ( state->executeState == RuleEngineState::DoneNotFound ) {
                if (rule->state->depOptions.at(ii) & RuleEngine::Optional) {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Ignoring non-existant optional prereq" << state->ruleName;
                    continue;
                } else {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Failing on non-existant prereq" << state->ruleName;
                    RULE_ERROR_DESC(rule, PrereqMissing, state->ruleName);
                    m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                    m_srule = SchedulerRule();
                    return;
                }
            }

            Rule *depRule = srule.rule();
            switch(state->executeState) {
                case RuleEngineState::NotStarted:
                case RuleEngineState::Queued:
                case RuleEngineState::Waiting:
                case RuleEngineState::Throttled:
                case RuleEngineState::Executing:
                case RuleEngineState::DoneNotFound:
                    Q_ASSERT(!"Invalid dep-rule state");
                    break;
                case RuleEngineState::DoneNothingToDo:
                    break;
                case RuleEngineState::DoneFailed:
                    if (!(rule->state->depOptions.at(ii) & RuleEngine::NoFail)) {
                        if (options.debug & Options::RuleExec)
                            qTraceOutput() << "QBuild(" << m_id << "):         Failing rule due to failed prerequisite" << depRule->ruleName();

                        RULE_ERROR_TYPE(rule, PrereqFailed, depRule);
                        m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                        m_srule = SchedulerRule();
                        return;
                    } else {
                        if (options.debug & Options::RuleExec)
                            qTraceOutput() << "QBuild(" << m_id << "):         Non-fail rule" << depRule->ruleName() << "failed";
                    }
                    break;
                case RuleEngineState::DoneSucceeded:
                    if (!(rule->state->depOptions.at(ii) & RuleEngine::Hidden)) {
                        rule->state->succeeded = true;
                        run = true;
                        if (options.debug & Options::RuleExec)
                            qTraceOutput() << "QBuild(" << m_id << "):         Executed rule" << depRule->ruleName() << "will cause rule execution";
                    } else {
                        if (options.debug & Options::RuleExec)
                            qTraceOutput() << "QBuild(" << m_id << "):         Hidden executed rule" << depRule->ruleName() << "will NOT cause rule execution";
                    }
                    break;
            }

        }

        if (rule->outputFiles().isEmpty() && !rule->commands.isEmpty()) {
            if (options.debug & Options::RuleExec && rule->tests.isEmpty())
                qTraceOutput() << "QBuild(" << m_id << "):         Empty output files will cause rule execution";
            run = true;
        }

        if (rule->forceRule)
            run = true;

        if (!run) {
            // Test input files vs output files
            time_t earliestOutput = -1;
            QString earliestOutputFile;

            for (int ii = 0; !run && ii < rule->outputFiles().count(); ++ii) {

                QString file = absfile(rule->outputFiles().at(ii));

                if (!fastDir.exists(file)) {
                    if (options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Missing output file" << file << "will cause rule execution";
                    run = true;
                } else {
                    time_t time = fastDir.lastModified(file);

                    if (earliestOutput == -1 || time < earliestOutput) {
                        if (options.debug & Options::RuleExec)
                            earliestOutputFile = file;
                        earliestOutput = time;
                    }
                }

            }

            for (int ii = 0; !run && ii < rule->state->inputFiles.count(); ++ii) {

                QString file = absfile(rule->state->inputFiles.at(ii));
                time_t inputFileTime = fastDir.lastModified(file);
                if (inputFileTime > earliestOutput) {
                    if (options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Input file" << file << "(" << inputFileTime << ") newer than output file" << earliestOutputFile << "(" << earliestOutput << ") will cause rule execution";
                    run = true;
                }

            }

        }

        // Try the tests
        for (int ii = 0; !run && ii < rule->tests.count(); ++ii) {
            const QString &test = rule->tests.at(ii);

            RuleVariables variables(rule, this);
            QStringList out;
            if (!project->runExpression(out, test.toAscii(), &variables)) {
                if (options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):         Failing due to test expression" << test;

                RULE_ERROR_DESC(rule, TestEvalFail, test);
                m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                m_srule = SchedulerRule();
                return;
            }

            run = !(out.isEmpty() ||
                   (out.count() == 1 && out.first() == "false") ||
                   (out.count() == 1 && out.first().isEmpty()));

            if (options.debug & Options::RuleExec)
                if (!run)
                    qTraceOutput() << "QBuild(" << m_id << "):         Test function" << test << "output" << out << "will NOT cause rule execution";
                else
                    qTraceOutput() << "QBuild(" << m_id << "):         Test function" << test << "output" << out << "will cause rule execution";

        }

        if (!run) {

            if (options.debug & Options::RuleExec)
                qTraceOutput() << "QBuild(" << m_id << "):     Completed: Nothing to do";
            m_scheduler->completeRule(m_srule, RuleEngineState::DoneNothingToDo);
            m_srule = SchedulerRule();
            return;

        } else {

            // The rule always goes into this state though if it doesn't get throttled
            // it will immediately transition to the Executing state
            rule->state->executeState = RuleEngineState::Throttled;
#ifdef EXTRA_CATEGORIES
            m_scheduler->endCategory(QStringList() << "RuleEngineState::Waiting");
            m_scheduler->beginCategory(QStringList() << "RuleEngineState::Throttled");
#endif

            if (!m_scheduler->beginCategory(m_srule.rule()->category)) {
                m_scheduler->throttledRule(m_srule);
                m_srule = SchedulerRule();
                return;
            }

        }
    }

    if (rule->state->executeState == RuleEngineState::Throttled) {

        rule->state->executeState = RuleEngineState::Executing;
#ifdef EXTRA_CATEGORIES
        m_scheduler->endCategory(QStringList() << "RuleEngineState::Throttled");
#endif
        m_lock.unlock();
        RuleEngine::Result result = commands();
        LOCK(RuleEngineThread);
        m_lock.lock();
        m_scheduler->endCategory(m_srule.rule()->category);

        switch(result) {
            case RuleEngine::Failed:
                if (options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):     Completed: Failed";
                m_scheduler->completeRule(m_srule, RuleEngineState::DoneFailed);
                break;
            case RuleEngine::NothingToDo:
                if (!rule->state->succeeded) {
                    if (options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):     Completed: Nothing to do";
                    m_scheduler->completeRule(m_srule, RuleEngineState::DoneNothingToDo);
                    break;
                }
            case RuleEngine::Succeeded:
                if (options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):     Completed: Succeeded";
                m_scheduler->completeRule(m_srule, RuleEngineState::DoneSucceeded);
                break;
        }

        m_srule = SchedulerRule();
    }
}

/*!
  Return an absolute path for \a file.
*/
QString RuleEngineThread::absfile(const QString &file)
{
    if (file.startsWith('/'))
        return file;
    return m_cwd + file;
}

/*!
  Run a rule's commands
  Returns the result code.
*/
RuleEngine::Result RuleEngineThread::commands()
{
    Rule *rule = m_srule.rule();
    Project *project = m_srule.project();

    int commands = 0;

    if (options.debug & Options::RuleExec)
        qTraceOutput() << "QBuild(" << m_id << "):     Running...";

    foreach(QString command, rule->commands) {

        RuleEngine::StringFlags options = RuleEngine::flags(command, command);

        QString origCommand = command;
        if (!(options & RuleEngine::Verbatim)) {
            RuleVariables variables(rule, this);
            QStringList out;
            if (!project->runExpression(out, command.toAscii(), &variables)) {
                RULE_ERROR_DESC(m_srule.rule(), CommandEvalFail, command);
                return RuleEngine::Failed;
            }
            command = out.join(" ");
        }

        if ( (!(options & RuleEngine::NoEcho) &&
              !(options & RuleEngine::EchoIfNeeded)) ||
             ::options.verbose )
            qOutput() << command;

        QString commandOutput;
        bool success = false;

        if (command.isEmpty() || ::options.printOnly) {
            success = true;
        } else if (options & RuleEngine::CallFunction) {
            QStringList rv;
            QStringLists args;
            success = FunctionProvider::evalFunction(project, rv, command, args);
        } else {
            ShellProcess process(command, m_cwd,
                    (options&RuleEngine::TTy)?ShellProcess::TTy:ShellProcess::Normal,
                    options & RuleEngine::EchoIfNeeded);
            success = (0 == process.exitCode());
            if (!success)
                commandOutput = QString::fromLocal8Bit(process.output());
        }

        if (!success && (options & RuleEngine::Test))
            break;

        if (!success && !(options & RuleEngine::NoFail)) {
            if (!(options & RuleEngine::Verbatim)) {
                QString output = origCommand + "\n" + command;
                if (!commandOutput.isEmpty())
                    output += "\n" + commandOutput;

                RULE_ERROR_DESC(m_srule.rule(), CommandExecFail, output);
                return RuleEngine::Failed;
            } else {
                QString output = command;
                if (!commandOutput.isEmpty())
                    output += "\n" + commandOutput;

                RULE_ERROR_DESC(m_srule.rule(), CommandExecFail, output);
                return RuleEngine::Failed;
            }
        }

        if (!(options & RuleEngine::Hidden))
            commands++;
    }

    return commands?RuleEngine::Succeeded:RuleEngine::NothingToDo;
}

/*!
  Fetch a prerequisite for \a ruleName.
*/
SchedulerRule RuleEngineThread::prereqSrule(const QString &ruleName)
{
    if (ruleName.contains('/')) {
        int idx = ruleName.lastIndexOf('/');
        Q_ASSERT(idx != -1);

        QString projectName;
        QString rule;

        projectName = ruleName.left(idx);
        rule = ruleName.mid(idx + 1);
        if (rule.isEmpty())
            rule = "default";

        if (!projectName.startsWith('/'))
            projectName = m_srule.project()->name() + projectName;

        return m_scheduler->sruleFromName(projectName + "/" + rule);
    } else {
        Project *project = m_srule.project();

        return m_scheduler->sruleFromRule(project->projectRules()->ruleByName(ruleName));
    }
}

/*!
  Return the srule that freshens \a fileName.
*/
SchedulerRule RuleEngineThread::sruleForFile(const QString &fileName)
{
    return m_scheduler->sruleFromRule(m_srule.project()->projectRules()->ruleForFile(fileName));
}

/*!
  Calculate prerequisites?
*/
bool RuleEngineThread::prereqs()
{
    QBuild::beginPerfTiming("RuleEngine::Prereqs");
    Rule *rule = m_srule.rule();
    Project *project = m_srule.project();

    QSet<QString> prereqs;
    QList<QPair<QString, RuleEngine::StringFlags> > actions;

    foreach(QString action, rule->prerequisiteActions) {
        RuleEngine::StringFlags options = RuleEngine::flags(action, action);
        actions.append(qMakePair(action, options));
    }

    QList<QPair<QString, RuleEngine::StringFlags> > finalActions;

    for (int ii = 0; ii < actions.count(); ++ii) {

        QString action = actions.at(ii).first;
        if (action.isEmpty())
            continue;

        RuleEngine::StringFlags options = actions.at(ii).second;

        if (options & RuleEngine::Evaluatable) {
            RuleVariables variables(rule, this);
            QStringList out;
            if (!project->runExpression(out, action.toAscii(), &variables)) {
                if (::options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):         Prerequisite expression " << action << "failed";
                RULE_ERROR_DESC(m_srule.rule(), PrereqEvalFail, action);
                QBuild::endPerfTiming();
                return false;
            }

            for (int jj = 0; jj < out.count(); ++jj) {
                RuleEngine::StringFlags options =
                    RuleEngine::flags(out.at(jj), out[jj]);
                actions.append(qMakePair(out.at(jj), options));
            }

            continue;
        }

        if (options & RuleEngine::CallFunction) {
            QStringList out;
            QStringLists args;
            args << (QStringList() << rule->name);

            if (!FunctionProvider::evalFunction(project, out, action, args)) {
                if (::options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):         Prerequisite function " << action << "failed";
                RULE_ERROR_DESC(m_srule.rule(), PrereqFuncFail, action);
                QBuild::endPerfTiming();
                return false;
            }

            for (int jj = 0; jj < out.count(); ++jj) {
                RuleEngine::StringFlags options =
                    RuleEngine::flags(out.at(jj), out[jj]);
                actions.append(qMakePair(out.at(jj), options));
            }

            continue;
        }

        if (prereqs.contains(action))
            continue;
        prereqs.insert(action);

        finalActions.append(qMakePair(action, options));
    }

#if 0
    if (finalActions.count() >= 2)
        qWarning() << finalActions;
#endif

    for (int ii = 0; ii < finalActions.count(); ++ii) {
        QString action = finalActions.at(ii).first;
        RuleEngine::StringFlags options = finalActions.at(ii).second;
        SchedulerRule prereq_srule = this->prereqSrule(action);

        if (!prereq_srule.state()) {
            // XXX

        } else {
            m_srule.rule()->state->depRules << prereq_srule;
            m_srule.rule()->state->depOptions <<
                (options & (RuleEngine::NoFail | RuleEngine::Hidden | RuleEngine::Optional));
        }
    }

    QBuild::endPerfTiming();
    return true;
}

/*!
  Calculate the input files?
*/
bool RuleEngineThread::inputFiles()
{
    QBuild::beginPerfTiming("RuleEngine::InputFiles");
    Rule *rule = m_srule.rule();
    Project *project = m_srule.project();

    QList<QPair<QString, RuleEngine::StringFlags> > files;
    foreach(QString file, rule->inputFiles) {
        RuleEngine::StringFlags options = RuleEngine::flags(file, file);
        files.append(qMakePair(file, options));
    }

    for (int ii = 0; ii < files.count(); ++ii) {

        QString file = files.at(ii).first;
        if (file.isEmpty())
            continue;
        RuleEngine::StringFlags options = files.at(ii).second;

        if (options & RuleEngine::Evaluatable) {
            RuleVariables variables(rule, this);
            QStringList out;
            if (!project->runExpression(out, file.toAscii(), &variables)) {
                qWarning() << "QBuild(" << m_id << "):         Input file expression " << file << "failed";
                RULE_ERROR_DESC(m_srule.rule(), InputEvalFail, file);
                QBuild::endPerfTiming();
                return false;
            }

            for (int jj = 0; jj < out.count(); ++jj) {
                RuleEngine::StringFlags options =
                    RuleEngine::flags(out.at(jj), out[jj]);
                files.append(qMakePair(out.at(jj), options));
            }

            continue;
        }

        if (options & RuleEngine::CallFunction) {
            QStringList out;
            QStringLists args;
            args << (QStringList() << rule->name);

            if (!FunctionProvider::evalFunction(project, out, file, args)) {
                if (::options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):         Input file function " << file << "failed";
                RULE_ERROR_DESC(m_srule.rule(), InputFuncFail, file);
                QBuild::endPerfTiming();
                return false;
            }

            for (int jj = 0; jj < out.count(); ++jj) {
                RuleEngine::StringFlags options =
                    RuleEngine::flags(out.at(jj), out[jj]);
                files.append(qMakePair(out.at(jj), options));
            }

            continue;
        }

        QString afile = absfile(file);
        bool exists = fastDir.exists(afile);
        SchedulerRule fileRule = sruleForFile(file);

        if (!exists && !fileRule.rule(true)) {
            // Doesn't exist and can't build it
            if (options & RuleEngine::Optional) {
                if (options & RuleEngine::Test) {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Triggering due to non-existant optional file" << file;
                    m_srule.rule()->forceRule = true;
                } else {
                    if (::options.debug & Options::RuleExec)
                        qTraceOutput() << "QBuild(" << m_id << "):         Ignoring non-existant optional file" << file;
                    continue;
                }
            } else {
                if (::options.debug & Options::RuleExec)
                    qTraceOutput() << "QBuild(" << m_id << "):         Failing on non-existant file" << file;
                RULE_ERROR_DESC(m_srule.rule(), InputMissing, file);
                QBuild::endPerfTiming();
                return false;
            }
        } else if (fileRule.rule(true)) {
            if (fileRule.rule()->name != m_srule.rule()->name) {
                m_srule.rule()->state->depRules << fileRule;
                m_srule.rule()->state->depOptions << (options & RuleEngine::NoFail);
                m_srule.rule()->state->inputFiles << file;
            }
        } else {
            m_srule.rule()->state->inputFiles << file;
        }
    }

    QBuild::endPerfTiming();
    return true;
}

// =====================================================================

/*!
  \class RuleEngineScheduler

  There is one RuleEngineScheduler per RuleEngine instance.
*/

/*!
  \typedef RuleEngineScheduler::Threads
  \relates RuleEngineState
  Equivalent to QList<RuleEngineThread *>
*/

/*!
  \variable RuleEngineScheduler::threads
  All the available threads.  Used for tracking and shutdown.
*/

/*!
  \fn RuleEngineScheduler::guiCategoryRules()
  Returns a list with the number of job running for each category.
*/

/*!
  \internal
*/
RuleEngineScheduler::RuleEngineScheduler()
: ruleCount(0), ruleCompleteCount(0), waitingUntilDone(false)
{
}

/*!
  \internal

  Returns a Rule to the RuleEngineThread \a thread, or null if no rules are
  available for immediate processing.

  If no rules are available, the \a thread will sleep until the scheduler wakes
  it by calling RuleEngineThread::runRule().

  If \a project is not null, only find rules for that project.
*/
SchedulerRule RuleEngineScheduler::threadIdle(RuleEngineThread *thread, Project *project)
{
    SchedulerRule rv;
    LOCK(RuleEngineScheduler1);
    lock.lock();
    bool gotRule = false;

    if (!readyQueue.isEmpty()) {

        ReadyQueue::Iterator iter = readyQueue.end();
        if (project)
            iter = readyQueue.find(project);
        if (iter == readyQueue.end())
            iter = readyQueue.begin();

        for (; iter != readyQueue.end(); ++iter) {
            SchedulerRule srule = *iter;

#ifdef PROJECT_THREADS
            // Don't take a rule in these states unless we're a project thread
            if (srule.state()->executeState >= RuleEngineState::Throttled) {
                if (thread->m_projectThread)
                    continue;
            }
#endif

            // Don't take a throttled rule unless it's allowed to run
            if (srule.state()->executeState == RuleEngineState::Throttled) {
                if (!_beginCategory(srule.rule()->category))
                    continue;
            }

            gotRule = true;
            rv = srule;
            readyQueue.erase(iter);
            break;
        }

    }

    if (!gotRule) {

        idleThreads << thread;
        RUN_TRACE
            qWarning() << "Idle threads: " << idleThreads.count() << "/" << threads.count();
        if (waitingUntilDone && idleThreads.count() == threads.count()) {
            RUN_TRACE
                qWarning() << "Rule engine shutting down";
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
        }

    }

    lock.unlock();

    return rv;
}

/*!
  Returns the scheduler rule that matches \a n.

  Call unlocked.
*/
SchedulerRule RuleEngineScheduler::sruleFromName(const QString &n)
{
    QString name = n;
    while (name.contains("//"))
        name.replace("//", "/");

    SchedulerRule rv;
    LOCK(RuleEngineScheduler2);
    lock.lock();

    EngineStates::Iterator iter = engineStates.find(name);
    if (iter == engineStates.end()) {
        iter = engineStates.insert(name, new RuleEngineState);
        (*iter)->ruleName = name;
    }
    rv._state = *iter;

    lock.unlock();

    return rv;
}

/*!
  Returns the scheduler rule for \a r.

  Call unlocked.
*/
SchedulerRule RuleEngineScheduler::sruleFromRule(Rule *r)
{
    SchedulerRule rv;
    if (!r) {
    } else if (r->state) {
        rv._state = r->state;
    } else {
        LOCK(RuleEngineScheduler3);
        lock.lock();

        QString name = r->ruleName();
        EngineStates::Iterator iter = engineStates.find(name);
        if (iter == engineStates.end()) {
            iter = engineStates.insert(name, new RuleEngineState);
        }
        (*iter)->rule = r;
        (*iter)->ruleName = name;
        r->state = *iter;
        rv._state = *iter;

        lock.unlock();
    }
    return rv;
}

/*!
  Schedule \a srule.

  Call unlocked.
*/
void RuleEngineScheduler::scheduleRule(const SchedulerRule &srule)
{
    LOCK(RuleEngineScheduler4);
    lock.lock();
    Q_ASSERT(srule.rule()->state->executeState == RuleEngineState::NotStarted);
    srule.rule()->state->executeState = RuleEngineState::Queued;
#ifdef EXTRA_CATEGORIES
    _beginCategory(QStringList() << "RuleEngineState::Queued");
#endif
    ++ruleCount;
    if ( GuiBase::instance() )
        GuiBase::instance()->setRuleProgress(ruleCompleteCount, ruleCount, false);
    addReadyRule(srule);
    lock.unlock();
}

/*!
  Wait until there are no more rules to process (or deadlock is reached).

  Call unlocked.
*/
void RuleEngineScheduler::waitUntilDone()
{
    LOCK(RuleEngineScheduler5);
    lock.lock();
    if (idleThreads.count() == threads.count() && readyQueue.isEmpty()) {
        // Already done;
        lock.unlock();
        return;
    }
    waitingUntilDone = true;
    lock.unlock();
    QCoreApplication::instance()->exec();
    LOCK(RuleEngineScheduler6);
    lock.lock();
    waitingUntilDone = false;
    lock.unlock();

#if 0
    lock.lock();
    while (idleThreads.count() != threads.count())
        wait.wait(&lock);
    lock.unlock();
#endif
}

/*!
  \internal
*/
bool RuleEngineScheduler::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        QCoreApplication::instance()->quit();
        return true;
    }
    return QObject::event(e);
}

/*!
  \internal

  Call locked.
*/
void RuleEngineScheduler::addReadyRule(const SchedulerRule &srule)
{
    if (idleThreads.isEmpty()) {
        readyQueue.insert(srule.project(), srule);
    } else {
#ifdef PROJECT_THREADS
        // These rules can only be executed on project threads
        if (srule.rule() &&
            srule.rule()->state->executeState < RuleEngineState::Throttled)
        {
            foreach (RuleEngineThread *thread, idleThreads) {
                if (thread->m_projectThread) {
#if QT_VERSION >= 0x040400
                    idleThreads.removeOne(thread);
#else
                    idleThreads.removeAll(thread);
#endif
                    thread->runRule(srule);
                    return;
                }
            }
            readyQueue.insert(srule.project(), srule);
            return;
        }
#endif

        // These rules can only be executed if beginCategory returns true
        if (srule.rule(true) &&
            srule.rule()->state->executeState == RuleEngineState::Throttled &&
            !_beginCategory(srule.rule()->category))
        {
            readyQueue.insert(srule.project(), srule);
            return;
        }

        RuleEngineThread *thread = idleThreads.takeFirst();
        thread->runRule(srule);
    }
}

/*!
  Dump circular dependencies into a .dot file and inform the user where this file has been written.
  This happens after the engine has run all of the rules it can run.

  Note that \a rule is not used.

  Scheduler stalled - no locking needed.
*/
void RuleEngineScheduler::dumpCirc(Rule * /*rule*/)
{
    /*
    qWarning() << "Dumping circular dependencies."
               << "readyQueue.count" << readyQueue.count()
               << "waitingQueue.count" << waitingQueue.count();
    */

    // We get lots of duplicates so store it in a set.
    QSet<QString> lines;
    for (WaitingQueue::Iterator iter = waitingQueue.begin(); iter != waitingQueue.end(); ++iter) {
        QList<SchedulerRule> srules;
        dumpCirc(iter.key(), srules);

        QString out;
        for (int ii = 0; ii < srules.count(); ++ii) {
            if ( !out.isEmpty() )
                lines << QString("\"%1\" -> \"%2\";\n").arg(out).arg(srules.at(ii).project()->name() + srules.at(ii).rule()->name);
            out = srules.at(ii).project()->name() + srules.at(ii).rule()->name;
        }
    }

    // Dump a .dot file. This can be viewed to more easily spot the circular dependencies.
    qOutput() << "Dumping" << QString("%1/circ.dot").arg(QDir::currentPath());
    QFile file("circ.dot");
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << "digraph circ {" << endl;
    foreach ( QString line, lines )
        stream << line;
    stream << "}" << endl;
}

/*!
  \internal
*/
void RuleEngineScheduler::dumpCirc(RuleEngineState *state, QList<SchedulerRule> &srules)
{
    WaitingQueue::Iterator iter = waitingQueue.find(state);
    if ( iter == waitingQueue.end() ) return;

    for (int ii = 0; ii < srules.count(); ++ii) {
        if (srules.at(ii).state() == iter.key()) {
            srules.append(iter.value());
            return;
        }
    }

    srules.append(iter.value());
    dumpCirc(iter.value().state(), srules);
}

/*!
  Scheduler rule \a me must wait for \a requiredRules to complete before running.
  If the rules have already run the rule may be executed immediately.

  Call unlocked.
*/
void RuleEngineScheduler::waitOnRule(const SchedulerRule &me,
                                     const QList<SchedulerRule> &requiredRules)
{
    LOCK(RuleEngineScheduler8);
    lock.lock();

    int waitCount = 0;
    for (int ii = 0; ii < requiredRules.count(); ii++) {
        SchedulerRule srule = requiredRules.at(ii);
        RuleEngineState *state = srule.state();

        switch(state->executeState) {
            case RuleEngineState::NotStarted:
                ++ruleCount;
                if ( GuiBase::instance() )
                    GuiBase::instance()->setRuleProgress(ruleCompleteCount, ruleCount, false);
                state->executeState = RuleEngineState::Queued;
#ifdef EXTRA_CATEGORIES
                _beginCategory(QStringList() << "RuleEngineState::Queued");
#endif
                addReadyRule(srule);
            case RuleEngineState::Queued:
            case RuleEngineState::Waiting:
            case RuleEngineState::Throttled:
            case RuleEngineState::Executing:
                waitingQueue.insert(state, me);
                waitCount++;
                break;
            case RuleEngineState::DoneNotFound:
            case RuleEngineState::DoneNothingToDo:
            case RuleEngineState::DoneFailed:
            case RuleEngineState::DoneSucceeded:
                break;
        }
    }

    if (0 == waitCount)
        addReadyRule(me);
    else
        me.rule()->state->waitCount = waitCount;

    lock.unlock();
}

/*!
  Do the post-run cleanup for \a me. Set the state to \a state.

  Call unlocked.
*/
void RuleEngineScheduler::completeRule(const SchedulerRule &me,
                                       RuleEngineState::ExecuteState state)
{
    LOCK(RuleEngineScheduler9);
    lock.lock();

#ifdef EXTRA_CATEGORIES
    switch (me.state()->executeState) {
        case RuleEngineState::Queued:
            _endCategory(QStringList() << "RuleEngineState::Queued");
            break;
        case RuleEngineState::Waiting:
            _endCategory(QStringList() << "RuleEngineState::Waiting");
            break;
        case RuleEngineState::Throttled:
            _endCategory(QStringList() << "RuleEngineState::Throttled");
            break;
        default:
            break;
    }
    switch (state) {
        case RuleEngineState::Queued:
            _beginCategory(QStringList() << "RuleEngineState::Queued");
            break;
        case RuleEngineState::Waiting:
            _beginCategory(QStringList() << "RuleEngineState::Waiting");
            break;
        case RuleEngineState::Throttled:
            _beginCategory(QStringList() << "RuleEngineState::Throttled");
            break;
        default:
            break;
    }
#endif
    me.state()->executeState = state;

    WaitingQueue::Iterator iter = waitingQueue.find(me.state());
    while (iter != waitingQueue.end() && iter.key() == me.state()) {
        // This rule depends on me
        SchedulerRule &srule = *iter;

        Q_ASSERT(srule.state()->waitCount);
        srule.state()->waitCount--;
        if (!srule.state()->waitCount) {
            // This rule has no more prerequisites so it can run
            addReadyRule(srule);
        }
        iter = waitingQueue.erase(iter);
    }

    ++ruleCompleteCount;
    if ( GuiBase::instance() )
        GuiBase::instance()->setRuleProgress(ruleCompleteCount, ruleCount, false);
    lock.unlock();
}

/*!
  Add a throttled \a srule to the ready queue.
  It will not be taken off the queue unless \l beginCategory() returns true.

  Call unlocked.
*/
void RuleEngineScheduler::throttledRule(const SchedulerRule &srule)
{
    LOCK(RuleEngineScheduler9.5);
    QMutexLocker locker(&lock);
    addReadyRule(srule);
}

/*!
  Attempt to begin running commands for \a cats.
  Returns true if the rule can run, false if it is throttled.

  Call unlocked.
*/
bool RuleEngineScheduler::beginCategory(const QStringList &cats)
{
    LOCK(RuleEngineScheduler10);
    QMutexLocker locker(&lock);
    return _beginCategory(cats);
}

/*!
  Attempt to begin running commands for \a cats.
  Returns true if the rule can run, false if it is throttled.

  Call locked.
*/
bool RuleEngineScheduler::_beginCategory(const QStringList &cats)
{
    if (!cats.isEmpty()) {
        // The default state is to run the rule.
        // If one of the categories is full, the rule is denied.
        foreach (const QString &cat, cats) {
            if (options.throttle[cat] && options.throttle[cat] <= runningCategories[cat])
                return false;
        }
    }
    if (cats.isEmpty())
        m_guiCategoryRules["Default"]++;
    else
        m_guiCategoryRules[cats.at(0)]++;
    foreach (const QString &cat, cats)
        runningCategories[cat]++;
    return true;
}

/*!
  Finish running commands for \a cats.

  Call unlocked.
*/
void RuleEngineScheduler::endCategory(const QStringList &cats)
{
    LOCK(RuleEngineScheduler11);
    QMutexLocker locker(&lock);
    _endCategory(cats);
}

/*!
  Finish running commands for \a cats.

  Call locked.
*/
void RuleEngineScheduler::_endCategory(const QStringList &cats)
{
    if (cats.isEmpty())
        m_guiCategoryRules["Default"]--;
    else
        m_guiCategoryRules[cats.at(0)]--;
    foreach (const QString &cat, cats)
        runningCategories[cat]--;
}

// =====================================================================

/*!
  \class RuleEngine
*/

/*!
  \enum RuleEngine::Result
  \value Succeeded
  \value Failed
  \value NothingToDo
*/

/*!
  \enum RuleEngine::StringFlags
  \value None
  \value Evaluatable
  \value NoEcho
  \value Hidden
  \value NoFail
  \value Verbatim
  \value Optional
  \value CallFunction
  \value TTy
  \value Ordered
  \value Test
  \value EchoIfNeeded
*/

/*!
  \internal
*/
RuleEngine::RuleEngine(int threads)
: m_scheduler(0)
{
    int idealThreadCount = QThread::idealThreadCount();
    if (threads == 0)
        threads = idealThreadCount;

    if ( options.showGui ) {
        Gui *gui = new Gui;
        gui->show();
    } else if ( options.showProgress ) {
        ConsoleGui *gui = new ConsoleGui;
        gui->draw();
    }

    m_scheduler = new RuleEngineScheduler;
    for (int ii = 0; ii < threads; ++ii)
        m_scheduler->threads << new RuleEngineThread(ii, m_scheduler, (ii>=idealThreadCount));
}

/*!
  Execute rule \a r in project \a project.

  For each rule, the following occurs:

  \list
  \o Run each prereq *once*, updating the list of prereqs each time
     (rules can create new prereqs)
  \o Run the commands if:
     \list
     \o Any inputFiles are newer than any of the outputFiles
     \o There are no outputFiles
     \o An inputFile is missing, but can be created by another rule
        (rescan *new* inputFiles and prereqs)
     \o Any (non-hidden) prerequisite was run (not just tested)
     \o -forceRule was passed
     \endlist
  \o Fail the rule if:
     \list
     \o An input File is missing, and cannot be created by another rule
     \o A (non-optional) prerequisite is missing
     \endlist
  \endlist
*/
RuleEngine::Result RuleEngine::execute(Project *project, const QString &r)
{
    QString ruleName = r;
    if (ruleName.isEmpty())
        ruleName = "default";

    Rule *rule = project->projectRules()->ruleByName(ruleName);
    if (!rule) {
        qWarning() << "No such rule" << ruleName;
        return Failed;
    }
    // The rule might be forced to run (though it's prerequisites aren't forced to run)
    rule->forceRule = options.forceRule;

    if (options.debug & Options::RuleExec)
        qTraceOutput() << "QBuild: Commencing execution of" << rule->ruleName();

    SchedulerRule srule = m_scheduler->sruleFromRule(rule);
    m_scheduler->scheduleRule(srule);
    m_scheduler->waitUntilDone();

    switch(rule->state->executeState) {
        case RuleEngineState::DoneNothingToDo:
            qWarning() << "Nothing to be done for" << ruleName;
            return NothingToDo;
            break;
        case RuleEngineState::DoneSucceeded:
            return Succeeded;
            break;
        case RuleEngineState::DoneFailed:
            qWarning() << "*** Error";
            dumpRuleErrors(rule);
            return Failed;
            break;
        default:
            // Can only occur when there's a circular dependency
            qWarning() << "*** Error";
            qWarning() << "Circular dependency detected!";
            qWarning() << "Executing rule" << ruleName << "in project" << project->nodePath();
            m_scheduler->dumpCirc(rule);
            break;
    }
    return Failed;
}

/*!
  Create \a filename for \a project.
*/
RuleEngine::Result RuleEngine::createFile(Project *project,
                                          const QString &filename)
{
    Rule *rule = project->projectRules()->ruleForFile(filename);
    if (!rule) {
        qWarning() << "Cannot freshen file" << filename;
        return Failed;
    }

    SchedulerRule srule = m_scheduler->sruleFromRule(rule);
    m_scheduler->scheduleRule(srule);
    m_scheduler->waitUntilDone();

    switch(rule->state->executeState) {
        case RuleEngineState::DoneNothingToDo:
            qWarning() << "File" << filename << "fresh";
            return NothingToDo;
            break;
        case RuleEngineState::DoneSucceeded:
            return Succeeded;
            break;
        case RuleEngineState::DoneFailed:
            qWarning() << "*** Error";
            dumpRuleErrors(rule);
            return Failed;
            break;
        default:
            // Can only occur when there's a circular dependency
            qWarning() << "*** Error";
            qWarning() << "Circular dependency detected!";
            qWarning() << "Executing rule for file" << filename << "in project" << project->nodePath();
            m_scheduler->dumpCirc(rule);
            break;
    }
    return Failed;
}

/*!
  \internal
*/
void RuleEngine::dumpRuleErrors(Rule *rule)
{
    if (!rule->state)
        return;

    switch(rule->state->error) {
        case RuleEngineState::NoError:
            break;
        case RuleEngineState::PrereqFailed:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Prerequisite failed";
            break;
        case RuleEngineState::CommandEvalFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Command expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::CommandExecFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Command execution failed";
            qOutput().nospace()
                << rule->state->errorDesc;
            break;
        case RuleEngineState::PrereqEvalFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Prerequisite action expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::PrereqFuncFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Prerequisite action function expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::PrereqMissing:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Prerequisite missing";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::TestEvalFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Test expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::InputEvalFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Input file expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::InputFuncFail:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Input file function expansion failed";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
        case RuleEngineState::NotFound:
            qOutput() << "ERROR";
            QBuild::shutdown();
            break;
        case RuleEngineState::InputMissing:
            qOutput().nospace()
                << rule->rules()->name()
                << rule->name
                << ": Input file is missing and cannot be created";
            qOutput().nospace()
                << "\t"
                << rule->state->errorDesc;
            break;
    }

    if (rule->state->errorRule)
        dumpRuleErrors(rule->state->errorRule);
}

/*!
  Returns the flags for \a input. Return the corrected string in \a output.
*/
RuleEngine::StringFlags RuleEngine::flags(const QString &input, QString &output)
{
    output = input;

    if (!output.startsWith("#("))
        return None;

    int chomp = 2;
    StringFlags rv = None;
    bool done = false;
    for (; !done && chomp < output.length(); ++chomp) {
        switch(output.at(chomp).cell()) {
            case 'f':
                // Call function
                rv = (StringFlags)(rv | Evaluatable);
                break;
            case 'e':
                // No Echo
                rv = (StringFlags)(rv | NoEcho);
                break;
            case 'E':
                // Echo if needed
                rv = (StringFlags)(rv | EchoIfNeeded);
                break;
            case 'h':
                // Hidden command
                rv = (StringFlags)(rv | Hidden);
                break;
            case 'n':
                // No fail
                rv = (StringFlags)(rv | NoFail);
                break;
            case 'v':
                // Verbatim
                rv = (StringFlags)(rv | Verbatim);
                break;
            case 'o':
                // Optional
                rv = (StringFlags)(rv | Optional);
                break;
            case 'c':
                // Call function (rule passed)
                rv = (StringFlags)(rv | CallFunction);
                break;
            case 't':
                // TTy
                rv = (StringFlags)(rv | TTy);
                break;
            case 's':
                // Test
                rv = (StringFlags)(rv | Test);
                break;
            case ')':
                done = true;
                break;
            default:
                break;
        }
    }

    output = output.mid(chomp);
    return rv;
}

/*!
  \internal
*/
QHash<QString, int> RuleEngine::categories() const
{
    return m_scheduler->guiCategoryRules();
}

// =====================================================================

/*!
  \class RuleVariables
  \brief Expands rule command variables.

  This is used when evaluating rule commands.
  \code
  rule.commands="echo $$[INPUT]"
  \endcode

  The supported variables are:

  \list type=dl
  \o INPUT equivalent to rule.inputFiles
  \o OUTPUT equivalent to rule.outputFiles
  \o OTHER equivalent to rule.other
  \endlist

  You can use the following modifiers:

  \list type=dl
  \o .[n] Returns the \i{n}th item in the list.
  \o .ABS Returns absolute paths.
  \endlist

  Note that .ABS must appear at the end.
*/

/*!
  Construct a RuleVariables instance for \a rule. Set \a thread to the
  current thread.
*/
RuleVariables::RuleVariables(Rule *rule, RuleEngineThread *thread)
: m_thread(thread), m_rule(rule)
{
}

/*!
  Fetch the variable \a name from the rule. The value is returned in \a rv.
  Returns true if successful, false otherwise.
*/
bool RuleVariables::value(const QString &name, QStringList &rv)
{
    QString _name = name;
    bool abs = false;
    if (_name.endsWith(".ABS")) {
        abs = true;
        _name = _name.left(_name.length() - 4);
    }

    if (_name.startsWith("INPUT")) {

        if (_name.length() == 5) {
            // "INPUT" exactly
            if (abs) {
                for (int ii = 0; ii < m_rule->inputFiles.count(); ++ii)
                    rv << m_thread->absfile(m_rule->inputFiles.at(ii));
            } else {
                rv = m_rule->inputFiles;
            }
            return true;
        } else if (_name.at(5) == QChar('.')) {
            int idx = _name.mid(6).toInt();
            if (idx >= 0 && idx < m_rule->inputFiles.count())
                if (abs)
                    rv << m_thread->absfile(m_rule->inputFiles.at(idx));
                else
                    rv << m_rule->inputFiles.at(idx);
            return true;
        }

    } else if (_name.startsWith("OTHER")) {

        if (_name.length() == 5) {
            // "OTHER" exactly
            if (abs) {
                for (int ii = 0; ii < m_rule->other.count(); ++ii)
                    rv << m_thread->absfile(m_rule->other.at(ii));
            } else {
                rv = m_rule->other;
            }
            return true;
        } else if (_name.at(5) == QChar('.')) {
            int idx = _name.mid(6).toInt();
            if (idx >= 0 && idx < m_rule->other.count())
                if (abs)
                    rv << m_thread->absfile(m_rule->other.at(idx));
                else
                    rv << m_rule->other.at(idx);
            return true;
        }
    } else if (_name.startsWith("OUTPUT")) {

        if (_name.length() == 6) {
            // "OUTPUT" exactly
            if (abs) {
                for (int ii = 0; ii < m_rule->outputFiles().count(); ++ii)
                    rv << m_thread->absfile(m_rule->outputFiles().at(ii));
            } else {
                rv = m_rule->outputFiles();
            }
            return true;
        } else if (_name.at(6) == QChar('.')) {
            int idx = _name.mid(7).toInt();
            if (idx >= 0 && idx < m_rule->outputFiles().count())
                if (abs)
                    rv << m_thread->absfile(m_rule->outputFiles().at(idx));
                else
                    rv << m_rule->outputFiles().at(idx);
            return true;
        }
    }

    return false;
}

