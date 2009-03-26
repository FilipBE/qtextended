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

#ifndef RULEENGINE_H
#define RULEENGINE_H

#include <QHash>
#include <QString>
#include <QStringList>
#include "object.h"

class RuleEngineState;
class Project;
class Rules;
class Rule
{
public:
    Rule(Rules *);
    Rule(const QString &, Rules *);
    Rule(const Rule &);
    Rule &operator=(const Rule &);

    QString ruleName() const;

    QString name; // Cannot be changed after construction!

    QString help;

    const QStringList &outputFiles() const;
    void setOutputFiles(const QStringList &);
    void appendOutputFile(const QString &);

    QStringList inputFiles;
    QStringList prerequisiteActions;
    QStringList other;
    QStringList commands;
    QStringList tests;
    QStringList category;

    enum Flags { None = 0x0000,
                 SerialRule = 0x0001 };
    Flags flags;

    RuleEngineState *state;

    QList<TraceContext> trace;

    Rules *rules() const;
private:
    Rules *m_rules;
    QStringList m_outputFiles;
public:
    bool forceRule;
};

class Rules
{
public:
    Rules(const QString &name, Project *p);

    QList<Rule *> rules() const;
    Rule *ruleByName(const QString &name) const;
    Rule *ruleForFile(const QString &name) const;

    Rule *addRule(const Rule &);

    void removeRule(Rule *);
    void removeRule(const QString &name);

    int ruleNum();

    QString name() const;

    Project *project() const;
private:
    friend class Rule;
    typedef QHash<QString, Rule *> RuleMap;
    RuleMap m_outputFiles;
    RuleMap m_rules;
    int m_ruleNum;
    QString m_name;
    Project *m_project;
};

class RuleEngineScheduler;

class RuleEngine {
public:
    RuleEngine(int threads = 0);
    enum Result { Succeeded, Failed, NothingToDo };

    Result execute(Project *, const QString &rule = QString());
    Result createFile(Project *, const QString &filename);

    QHash<QString, int> categories() const;

    enum StringFlags {
        None = 0x0000,
        Evaluatable = 0x0001,
        NoEcho = 0x0002,
        Hidden = 0x0004,
        NoFail = 0x0008,
        Verbatim = 0x0010,
        Optional = 0x0020,
        CallFunction = 0x0040,
        TTy = 0x0080,
        Ordered = 0x0100,
        Test = 0x0200,
        EchoIfNeeded = 0x0400
    };
    static StringFlags flags(const QString &input, QString &output);
private:
    void dumpRuleErrors(Rule *);
    void dumpRuleCurcDep(Rule *);

    RuleEngineScheduler *m_scheduler;
};

#endif
