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

#include "qtscriptfunctions.h"
#include "options.h"
#include "qfastdir.h"
#include "object.h"
#include "solution.h"
#include <QFile>
#include "qbuild.h"
#include "ruleengine.h"
#include "qoutput.h"

struct RuleStringList
{
    Rule *rule;
    enum Item { InputFiles, Prereqs, Other, Commands, Tests, OutputFiles, Category };
    Item item;

    QStringList value() const;
    void append(const QStringList &);
    void set(const QStringList &);
};

QStringList RuleStringList::value() const
{
    switch(item) {
        case InputFiles:
            return rule->inputFiles;
            break;
        case Prereqs:
            return rule->prerequisiteActions;
            break;
        case Other:
            return rule->other;
            break;
        case Commands:
            return rule->commands;
            break;
        case Tests:
            return rule->tests;
            break;
        case OutputFiles:
            return rule->outputFiles();
            break;
        case Category:
            return rule->category;
            break;
    }
    return QStringList();
}

void RuleStringList::append(const QStringList &v)
{
    switch(item) {
        case InputFiles:
            rule->inputFiles << v;
            break;
        case Prereqs:
            rule->prerequisiteActions << v;
            break;
        case Other:
            rule->other << v;
            break;
        case Commands:
            rule->commands << v;
            break;
        case Tests:
            rule->tests << v;
            break;
        case OutputFiles:
            foreach(QString file, v)
                rule->appendOutputFile(file);
            break;
        case Category:
            rule->category << v;
            break;
    }
}

void RuleStringList::set(const QStringList &v)
{
    switch(item) {
        case InputFiles:
            rule->inputFiles = v;
            break;
        case Prereqs:
            rule->prerequisiteActions = v;
            break;
        case Other:
            rule->other = v;
            break;
        case Commands:
            rule->commands = v;
            break;
        case Tests:
            rule->tests = v;
            break;
        case OutputFiles:
            rule->setOutputFiles(v);
            break;
        case Category:
            rule->category = v;
            break;
    }
}


static QtScriptFunctions qtscriptFunctions;
static QList<QPair<QByteArray, QString> > qtscriptSnippets;
static QMutex qtscriptSnippetsLock;

//
// ProjectFile binding
//
struct ProjectFile {
    QString name;
};

static QScriptValue pf_new(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file;
    file.name = ctx->argument(0).toString();
    return eng->newVariant(qVariantFromValue(file));
}

static QScriptValue pf_escapedname(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    QString str = file.name;
    str.replace(" ", "\\ ");

    return eng->toScriptValue(str);
}

static QScriptValue pf_simplename(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    QString rv;
    for (int ii = 0; ii < file.name.length(); ++ii) {
        QChar c = file.name.at(ii);
        if (c.isLetterOrNumber()) {
            rv.append(c);
        } else {
            rv.append("_");
            rv.append(QString::number(c.unicode()));
        }
    }

    return eng->toScriptValue(rv);
}

static QScriptValue pf_canonicalName(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());
    QString rv = QFileInfo(file.name).canonicalFilePath();
    return eng->toScriptValue(rv);
}

static QScriptValue pf_isRelative(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());
    return eng->toScriptValue(!file.name.startsWith('/'));
}

static QScriptValue pf_extension(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    int indexOf = file.name.lastIndexOf(".");
    if (-1 == indexOf)
        return eng->nullValue();
    else
        return eng->toScriptValue(file.name.mid(indexOf + 1));
}

static QScriptValue pf_name(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    QString str = file.name;
    int indexDot = str.lastIndexOf('.');
    int indexSlash = str.lastIndexOf('/');

    if (indexDot != -1 && indexDot > indexSlash)
        str = str.left(indexDot);

    if (indexSlash != -1)
        str = str.mid(indexSlash + 1);

    return eng->toScriptValue(str);
}

static QScriptValue pf_path(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    QString str = file.name;
    int indexSlash = str.lastIndexOf('/');

    if (indexSlash != -1)
        str = str.left(indexSlash);
    else
        str = QString();

    return eng->toScriptValue(str);
}

static QScriptValue pf_stripextension(QScriptContext *ctx, QScriptEngine *eng)
{
    ProjectFile file = qscriptvalue_cast<ProjectFile>(ctx->thisObject());

    QString str = file.name;
    int indexDot = str.lastIndexOf('.');

    if (indexDot != -1)
        str = str.left(indexDot);

    file.name = str;
    return eng->newVariant(qVariantFromValue(file));
}

void qScript_File(QScriptEngine *engine)
{
    QScriptValue fproto = engine->newObject();
    engine->globalObject().setProperty("File", engine->newFunction(pf_new));
    fproto.setProperty("extension", engine->newFunction(pf_extension));
    fproto.setProperty("isRelative", engine->newFunction(pf_isRelative));
    fproto.setProperty("name", engine->newFunction(pf_name));
    fproto.setProperty("path", engine->newFunction(pf_path));
    fproto.setProperty("stripextension", engine->newFunction(pf_stripextension));
    fproto.setProperty("escapedname", engine->newFunction(pf_escapedname));
    fproto.setProperty("simplename", engine->newFunction(pf_simplename));
    fproto.setProperty("canonicalName", engine->newFunction(pf_canonicalName));
    engine->setDefaultPrototype(qMetaTypeId<ProjectFile>(), fproto);
}

//
// SolutionProject binding
//
static QScriptValue sproject_isValid(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    return eng->toScriptValue(proj.isValid());
}

static QScriptValue sproject_isSubproject(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    if (!ctx->argumentCount())
        return eng->nullValue();

    SolutionProject subproj =
        qscriptvalue_cast<SolutionProject>(ctx->argument(0));

    return eng->toScriptValue(proj.isSubproject(subproj));
}

static QScriptValue sproject_node(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    return eng->toScriptValue(proj.node());
}

static QScriptValue sproject_nodePath(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    return eng->toScriptValue(proj.nodePath());
}

static QScriptValue sproject_filename(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    return eng->toScriptValue(proj.fileName());
}

static QScriptValue sproject_subprojects(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    return eng->toScriptValue(proj.subProjects());
}

static QScriptValue sproject_fileModeProject(QScriptContext *ctx, QScriptEngine *eng)
{
    ctx->setActivationObject(eng->globalObject());

    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());
    proj.setFileMode(true);

    Project *project = proj.project();

    return eng->newVariant(qVariantFromValue(project));
}

static QScriptValue sproject_project(QScriptContext *ctx, QScriptEngine *eng)
{
    ctx->setActivationObject(eng->globalObject());

    SolutionProject proj =
        qscriptvalue_cast<SolutionProject>(ctx->thisObject());

    Project *project = proj.project();

    return eng->newVariant(qVariantFromValue(project));
}

void qScript_SolutionProject(QScriptEngine *engine)
{
    QScriptValue sproject = engine->newObject();
    sproject.setProperty("isValid", engine->newFunction(sproject_isValid));
    sproject.setProperty("isSubproject", engine->newFunction(sproject_isSubproject));
    sproject.setProperty("node", engine->newFunction(sproject_node));
    sproject.setProperty("nodePath", engine->newFunction(sproject_nodePath));
    sproject.setProperty("filename", engine->newFunction(sproject_filename));
    sproject.setProperty("subprojects", engine->newFunction(sproject_subprojects));
    sproject.setProperty("project", engine->newFunction(sproject_project));
    sproject.setProperty("fileModeProject", engine->newFunction(sproject_fileModeProject));
    engine->setDefaultPrototype(qMetaTypeId<SolutionProject>(), sproject);
}

Q_DECLARE_METATYPE(Rule*)
Q_DECLARE_METATYPE(ProjectFile)
Q_DECLARE_METATYPE(Project*)
Q_DECLARE_METATYPE(QMakeObject*)
Q_DECLARE_METATYPE(Solution*)
Q_DECLARE_METATYPE(RuleStringList)
Q_DECLARE_METATYPE(SolutionProject)
Q_DECLARE_METATYPE(SolutionFile)
Q_DECLARE_METATYPE(QMakeObject::TraceInfo)
Q_DECLARE_METATYPE(PathIterator)

static QScriptValue qbuild_root(QScriptContext *, QScriptEngine *eng)
{
    return eng->newVariant(qVariantFromValue(QBuild::rootProject()));
}

static QScriptValue rsl_value(QScriptContext *ctx, QScriptEngine *eng)
{
    RuleStringList rsl = qscriptvalue_cast<RuleStringList>(ctx->thisObject());
    if (!rsl.rule)
        return eng->undefinedValue();

    return eng->toScriptValue(rsl.value());
}

static QScriptValue rsl_append(QScriptContext *ctx, QScriptEngine *eng)
{
    RuleStringList rsl = qscriptvalue_cast<RuleStringList>(ctx->thisObject());
    if (!rsl.rule)
        return eng->undefinedValue();

    if (ctx->argumentCount()) {
        QScriptValue arg = ctx->argument(0);
        QStringList val;
        if (arg.isArray())
            val = qscriptvalue_cast<QStringList>(arg);
        else
            val << qscriptvalue_cast<QString>(arg);

        rsl.append(val);
    }

    return ctx->thisObject();
}

//
// Rule binding
//
static QScriptValue rule_name(QScriptContext *ctx, QScriptEngine *eng)
{
    Rule *rule = qscriptvalue_cast<Rule *>(ctx->thisObject());
    if (!rule)
        return eng->undefinedValue();

    return eng->toScriptValue(rule->name);
}

static QStringList rule_QScriptValueToStringList(const QScriptValue &value)
{
    QStringList val;
    if (value.isArray())
        val = qscriptvalue_cast<QStringList>(value);
    else
        val << qscriptvalue_cast<QString>(value);
    return val;
}

static QScriptValue rule_set(QScriptContext *ctx, QScriptEngine *eng)
{
    Rule *rule = qscriptvalue_cast<Rule *>(ctx->thisObject());
    if (!rule || !ctx->argumentCount())
        return eng->undefinedValue();

    QScriptValue v = ctx->argument(0);

    QScriptValue help = v.property("help");
    if (help.isValid())
        rule->help = help.toString();
    QScriptValue input = v.property("inputFiles");
    if (input.isValid())
        rule->inputFiles = rule_QScriptValueToStringList(input);
    QScriptValue output = v.property("outputFiles");
    if (output.isValid())
        rule->setOutputFiles(rule_QScriptValueToStringList(output));
    QScriptValue prereq = v.property("prerequisiteActions");
    if (prereq.isValid())
        rule->prerequisiteActions = rule_QScriptValueToStringList(prereq);
    QScriptValue commands = v.property("commands");
    if (commands.isValid())
        rule->commands = rule_QScriptValueToStringList(commands);
    QScriptValue tests = v.property("tests");
    if (tests.isValid())
        rule->tests = rule_QScriptValueToStringList(tests);
    QScriptValue other = v.property("other");
    if (other.isValid())
        rule->other = rule_QScriptValueToStringList(other);
    QScriptValue serial = v.property("serial");
    if (serial.isValid()) {
        bool isserial = serial.toBoolean();
        if (isserial) rule->flags = (Rule::Flags)(rule->flags | Rule::SerialRule);
        else rule->flags = (Rule::Flags)(rule->flags & ~Rule::SerialRule);
    }

    return eng->nullValue();
}

static QScriptValue rule_serial(QScriptContext *ctx, QScriptEngine *eng)
{
    Rule *rule = qscriptvalue_cast<Rule *>(ctx->thisObject());
    if (!rule)
        return eng->undefinedValue();

    if (ctx->argumentCount()) {
        if (ctx->argument(0).toBoolean())
            rule->flags = (Rule::Flags)(rule->flags | Rule::SerialRule);
        else
            rule->flags = (Rule::Flags)(rule->flags & ~Rule::SerialRule);
    }

    return eng->toScriptValue(rule->flags & Rule::SerialRule);
}

static QScriptValue rule_help(QScriptContext *ctx, QScriptEngine *eng)
{
    Rule *rule = qscriptvalue_cast<Rule *>(ctx->thisObject());
    if (!rule)
        return eng->undefinedValue();

    if (ctx->argumentCount())
        rule->help = ctx->argument(0).toString();

    return eng->toScriptValue(rule->help);
}

static QScriptValue rule_item(QScriptContext *ctx, QScriptEngine *eng,
                              RuleStringList::Item item)
{
    Rule *rule = qscriptvalue_cast<Rule *>(ctx->thisObject());
    if (!rule)
        return eng->undefinedValue();

    RuleStringList rsl;
    rsl.rule = rule;
    rsl.item = item;

    if (ctx->argumentCount()) {
        QStringList val;
        QScriptValue arg = ctx->argument(0);
        RuleStringList argrsl = qscriptvalue_cast<RuleStringList>(arg);
        if (!argrsl.rule) {
            if (arg.isArray())
                val = qscriptvalue_cast<QStringList>(arg);
            else
                val << qscriptvalue_cast<QString>(arg);
        } else {
            val = argrsl.value();
        }

        rsl.set(val);
    }

    return eng->newVariant(qVariantFromValue(rsl));
}

static QScriptValue rule_prerequisiteActions(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::Prereqs);
}

static QScriptValue rule_tests(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::Tests);
}

static QScriptValue rule_commands(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::Commands);
}

static QScriptValue rule_other(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::Other);
}

static QScriptValue rule_inputFiles(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::InputFiles);
}

static QScriptValue rule_outputFiles(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::OutputFiles);
}

static QScriptValue rule_category(QScriptContext *ctx, QScriptEngine *eng)
{
    return rule_item(ctx, eng, RuleStringList::Category);
}

void qScript_Rule(QScriptEngine *engine)
{
    QScriptValue rule = engine->newObject();
    rule.setProperty("name", engine->newFunction(rule_name), QScriptValue::PropertyGetter);
    rule.setProperty("help", engine->newFunction(rule_help), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("inputFiles", engine->newFunction(rule_inputFiles), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("outputFiles", engine->newFunction(rule_outputFiles), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("prerequisiteActions", engine->newFunction(rule_prerequisiteActions), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("commands", engine->newFunction(rule_commands), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("tests", engine->newFunction(rule_tests), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("other", engine->newFunction(rule_other), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("serial", engine->newFunction(rule_serial), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("category", engine->newFunction(rule_category), QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    rule.setProperty("set", engine->newFunction(rule_set), QScriptValue::PropertySetter);

    engine->setDefaultPrototype(qMetaTypeId<Rule *>(), rule);

    QScriptValue rslproto = engine->newObject();
    rslproto.setProperty("value", engine->newFunction(rsl_value));
    rslproto.setProperty("append", engine->newFunction(rsl_append));
    engine->setDefaultPrototype(qMetaTypeId<RuleStringList>(), rslproto);
}

//
// Solution binding
//
static QScriptValue solution_default(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    if (!project)
        return eng->nullValue();
    if (0 == ctx->argumentCount()) {
        return eng->newVariant(qVariantFromValue(project->solution()));
    } else {
        QString solutionName = ctx->argument(0).toString();
        if (solutionName == "current") {
            return eng->newVariant(qVariantFromValue(project->solution()));
        } else {
            Solution *solution = Solution::solution(solutionName);
            if (!solution)
                return eng->nullValue();
            else
                return eng->newVariant(qVariantFromValue(solution));
        }
    }
}

static QScriptValue solution_sproject(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    QString projectName;
    if (ctx->argumentCount())
        projectName = ctx->argument(0).toString();
    SolutionProject solutionproject =
        SolutionProject::fromNode(projectName, solution);
    return eng->newVariant(qVariantFromValue(solutionproject));
}

static QScriptValue solution_files(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    QString filename;
    if (ctx->argumentCount())
        filename = ctx->argument(0).toString();

    return eng->toScriptValue(solution->files(filename));
}

static QScriptValue solution_paths(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    QString pathName;
    if (ctx->argumentCount())
        pathName = ctx->argument(0).toString();

    return eng->toScriptValue(solution->paths(pathName, SolutionFile()));
}

static QScriptValue solution_pathMappings(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    QString pathName;
    if (ctx->argumentCount())
        pathName = ctx->argument(0).toString();

    return eng->toScriptValue(solution->pathMappings(pathName));
}

static QScriptValue solution_project(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    QString projectName;
    if (ctx->argumentCount())
        projectName = ctx->argument(0).toString();
    SolutionProject solutionproject =
        SolutionProject::fromNode(projectName, solution);

    Project *project = solutionproject.project();

    if (project)
        return eng->newVariant(qVariantFromValue(project));
    else
        return eng->nullValue();
}

static QScriptValue solution_pi(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());
    return eng->toScriptValue(PathIterator(solution));
}

void qScript_Solution(QScriptEngine *engine)
{
    QScriptValue solution = engine->newObject();
    solution.setProperty("files", engine->newFunction(solution_files));
    solution.setProperty("paths", engine->newFunction(solution_paths));
    solution.setProperty("pathMappings", engine->newFunction(solution_pathMappings));
    solution.setProperty("project", engine->newFunction(solution_project));
    solution.setProperty("sproject", engine->newFunction(solution_sproject));
    solution.setProperty("pathIterator", engine->newFunction(solution_pi));
    engine->setDefaultPrototype(qMetaTypeId<Solution *>(), solution);
    engine->globalObject().setProperty("solution",
            engine->newFunction(solution_default),
            QScriptValue::PropertyGetter);
}

//
// Project binding
//
static QScriptValue project_error(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    QString str;
    if (ctx->argumentCount())
        str = ctx->argument(0).toString();
    project->error() << str.toLatin1().constData();
    abort();
    return eng->nullValue();
}

static QScriptValue project_expression(QScriptContext *ctx, QScriptEngine *eng)
{
    ctx->setActivationObject(eng->globalObject());
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString code;
    if (ctx->argumentCount() == 1)
        code = ctx->argument(0).toString();
    else
        return eng->nullValue();

    QStringList out;
    bool rv = project->runExpression(out, code.toLatin1(), 0);
    if (rv)
        return eng->toScriptValue(out);
    else
        return eng->nullValue();
}

static QScriptValue project_run(QScriptContext *ctx, QScriptEngine *eng)
{
    ctx->setActivationObject(eng->globalObject());
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString code;
    if (ctx->argumentCount() >= 1) {
        for (int ii = 0; ii < ctx->argumentCount(); ++ii) {
            if (ii)
                code += "\n";
            code += ctx->argument(ii).toString();
        }
    }

    bool rv = project->run(code.toLatin1(), "project.run");

    return eng->toScriptValue(rv);
}

static QScriptValue project_file(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFile sf = project->solution()->file(name, (Solution::Type)(Solution::_Project | Solution::Absolute | Solution::Wildcard), project->solutionProject().projectFile());
    if (sf.isValid())
        return eng->toScriptValue(sf);
    else
        return eng->nullValue();
}

static QScriptValue project_projectFile(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFile sf = project->solution()->file(name, (Solution::Type)(Solution::_Project | Solution::Absolute | Solution::Wildcard), project->solutionProject().projectFile());
    if (sf.isValid())
        return eng->toScriptValue(sf);
    else
        return eng->nullValue();
}

static QScriptValue project_files(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->files(name, (Solution::Type)(Solution::_Project | Solution::Absolute | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_projectFiles(QScriptContext *ctx,
                                         QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->files(name, (Solution::Type)(Solution::_Project | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_filesystemFile(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFile file = project->solution()->file(name, (Solution::Type)(Solution::Absolute), project->solutionProject().projectFile());

    if (file.isValid())
        return eng->toScriptValue(file);
    else
        return eng->nullValue();
}

static QScriptValue project_filesystemFiles(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->files(name, (Solution::Type)(Solution::Absolute | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_buildFile(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    return eng->toScriptValue(project->solution()->buildFile(name, project->solutionProject().projectFile()));
}

static QScriptValue project_buildPath(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    return eng->toScriptValue(project->solution()->filesystemBuildPath(name, project->solutionProject().projectFile()));
}

static QScriptValue project_paths(QScriptContext *ctx,
                                  QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->_paths(name, (Solution::Type)(Solution::Absolute | Solution::_Project | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_projectPaths(QScriptContext *ctx,
                                         QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->_paths(name, (Solution::Type)(Solution::_Project | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_filesystemPaths(QScriptContext *ctx,
                                            QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount() >= 1)
        name = ctx->argument(0).toString();

    SolutionFiles files = project->solution()->_paths(name, (Solution::Type)(Solution::Absolute | Solution::Wildcard), project->solutionProject().projectFile());

    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));

    return rv;
}

static QScriptValue project_disabledReason(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    return eng->toScriptValue(project->disabledReason());
}

static QScriptValue project_reset(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() < 1)
        return eng->undefinedValue();
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    QString reason = ctx->argument(0).toString();
    QString value;
    if (ctx->argumentCount() == 2)
        value = ctx->argument(1).toString();
    project->reset(reason, value);
    return eng->undefinedValue();
}

static QScriptValue project_resetReason(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    return eng->toScriptValue(project->resetReason());
}

static QScriptValue project_pathIterator(QScriptContext *ctx,
                                          QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    PathIterator pi(project->solution());
    pi = pi.advance(project->nodePath());
    return eng->toScriptValue(pi);
}

static QScriptValue project_message(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    QString str;
    if (ctx->argumentCount())
        str = ctx->argument(0).toString();
    project->message() << str.toLatin1().constData();

    return eng->nullValue();
}

static QScriptValue project_info(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    QString str;
    if (ctx->argumentCount())
        str = ctx->argument(0).toString();
    project->info() << str.toLatin1().constData();

    return eng->nullValue();
}

static QScriptValue project_warning(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    QString str;
    if (ctx->argumentCount())
        str = ctx->argument(0).toString();
    project->warning() << str.toLatin1().constData();

    return eng->nullValue();
}

static QScriptValue project_default(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);
    if (0 == ctx->argumentCount()) {
        Q_ASSERT(project);
        return eng->newVariant(qVariantFromValue(project));
    } else {
        QString projectName = ctx->argument(0).toString();
        SolutionProject solutionproject =
            SolutionProject::fromNode(projectName, project);

        Project *project = solutionproject.project();

        if (project)
            return eng->newVariant(qVariantFromValue(project));
        else
            return eng->nullValue();
    }
}

static QScriptValue project_virtual(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    if (ctx->argumentCount()) {
        QString name = ctx->argument(0).toString();
        SolutionProject proj = SolutionProject::fromNode(name, project);
        return eng->toScriptValue(proj.projectFile().isValid());
    }

    return eng->toScriptValue(project->virtualProject());
}

static QScriptValue project_config(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    bool rv = false;

    if (ctx->argumentCount())
        rv = project->isActiveConfig(ctx->argument(0).toString());

   return eng->toScriptValue(rv);
}

static QScriptValue project_sproject(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount())
        name = ctx->argument(0).toString();

    SolutionProject solutionproject;
   if (name.isEmpty())
       solutionproject = project->solutionProject();
   else
       solutionproject = SolutionProject::fromNode(name, project);

    return eng->newVariant(qVariantFromValue(solutionproject));
}

static QScriptValue project_solutionfile(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    SolutionFile sfile = project->file();
    return eng->toScriptValue(sfile);
}

//
// PathIterator binding
//
static QScriptValue pi_default(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount()) {
        PathIterator pi(ctx->argument(0).toString());
        return eng->toScriptValue(pi);
    } else {
        return eng->nullValue();
    }
}

static QScriptValue pi_files(QScriptContext *ctx, QScriptEngine *eng)
{
    PathIterator pi = qscriptvalue_cast<PathIterator>(ctx->thisObject());
    QList<SolutionFile> files;
    if (ctx->argumentCount())
        files = pi.files(ctx->argument(0).toString().toUtf8());
    else
        files = pi.files();
    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < files.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(files.at(ii)));
    return rv;
}

static QScriptValue pi_paths(QScriptContext *ctx, QScriptEngine *eng)
{
    PathIterator pi = qscriptvalue_cast<PathIterator>(ctx->thisObject());
    QStringList paths;
    if (ctx->argumentCount())
        paths = pi.paths(ctx->argument(0).toString().toUtf8());
    else
        paths = pi.paths();
    return eng->toScriptValue(paths);
}

static QScriptValue pi_filesystemPath(QScriptContext *ctx, QScriptEngine *eng)
{
    PathIterator pi = qscriptvalue_cast<PathIterator>(ctx->thisObject());
    return eng->toScriptValue(pi.fsPath());
}

static QScriptValue pi_cd(QScriptContext *ctx, QScriptEngine *eng)
{
    PathIterator pi = qscriptvalue_cast<PathIterator>(ctx->thisObject());
    if (ctx->argumentCount()) {
        QString cd = ctx->argument(0).toString();
        if (QFastDir::isRelativePath(cd)) {
            return eng->toScriptValue(pi.advance(ctx->argument(0).toString()));
        } else {
            PathIterator rv;
            if (pi.solution())
                rv = PathIterator(pi.solution());
            else
                rv = PathIterator("/");
            return eng->toScriptValue(rv.advance(cd));
        }
    } else {
        return ctx->thisObject();
    }
}

static QScriptValue pi_solution(QScriptContext *ctx, QScriptEngine *eng)
{
    PathIterator pi = qscriptvalue_cast<PathIterator>(ctx->thisObject());
    return eng->toScriptValue(pi.solution());
}

void qScript_PathIterator(QScriptEngine *engine)
{
    QScriptValue piproto = engine->newObject();
    piproto.setProperty("files", engine->newFunction(pi_files));
    piproto.setProperty("paths", engine->newFunction(pi_paths));
    piproto.setProperty("filesystemPath", engine->newFunction(pi_filesystemPath));
    piproto.setProperty("cd", engine->newFunction(pi_cd));
    piproto.setProperty("solution", engine->newFunction(pi_solution),
                        QScriptValue::PropertyGetter);
    engine->setDefaultPrototype(qMetaTypeId<PathIterator>(), piproto);

    engine->globalObject().setProperty("PathIterator",
                                       engine->newFunction(pi_default));
}

//
// SolutionFile binding
//
static QScriptValue sf_isFilesystemFile(QScriptContext *ctx,
                                        QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.type() & SolutionFile::Absolute);
}

static QScriptValue sf_exists(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    bool exists = false;
    if (!sf.isValid()) {
        exists = false;
    } else if (sf.isDir()) {
        exists = QFastDir::isDir(sf.fsPath());
    } else {
        exists = QFastDir::isFile(sf.fsPath());
    }

    return eng->toScriptValue(exists);
}

static QScriptValue sf_isProjectFile(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(bool(sf.type() & SolutionFile::Project));
}

static QScriptValue sf_isDirectory(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(bool(sf.isDir()));
}

static QScriptValue sf_isBuildFile(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(bool(sf.type() & SolutionFile::Build));
}

static QScriptValue sf_name(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.name());
}

static QScriptValue sf_path(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.solutionPath());
}

static QScriptValue sf_dir(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.solutionDir());
}

static QScriptValue sf_filesystemPath(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.fsPath());
}

static QScriptValue sf_filesystemDir(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.fsDir());
}

static QScriptValue sf_solution(QScriptContext *ctx, QScriptEngine *eng)
{
    SolutionFile sf = qscriptvalue_cast<SolutionFile>(ctx->thisObject());
    return eng->toScriptValue(sf.solution());
}

void qScript_SolutionFile(QScriptEngine *engine)
{
    QScriptValue sfproto = engine->newObject();
    sfproto.setProperty("isFilesystemFile", engine->newFunction(sf_isFilesystemFile));
    sfproto.setProperty("isProjectFile", engine->newFunction(sf_isProjectFile));
    sfproto.setProperty("isBuildFile", engine->newFunction(sf_isBuildFile));
    sfproto.setProperty("isDirectory", engine->newFunction(sf_isDirectory));
    sfproto.setProperty("exists", engine->newFunction(sf_exists));
    sfproto.setProperty("name", engine->newFunction(sf_name));
    sfproto.setProperty("path", engine->newFunction(sf_path));
    sfproto.setProperty("dir", engine->newFunction(sf_dir));
    sfproto.setProperty("filesystemPath",
            engine->newFunction(sf_filesystemPath));
    sfproto.setProperty("filesystemDir",
            engine->newFunction(sf_filesystemDir));
    sfproto.setProperty("solution", engine->newFunction(sf_solution));
    engine->setDefaultPrototype(qMetaTypeId<SolutionFile>(), sfproto);
}


class EngineLineMapping
{
public:
    EngineLineMapping()
        : numLines(0) {}
    EngineLineMapping(const EngineLineMapping &o)
        : mapping(o.mapping), numLines(o.numLines) {}
    EngineLineMapping &operator=(const EngineLineMapping &o)
    { mapping = o.mapping; numLines = o.numLines; return *this; }

    void addLines(int lines, const QString &file, int fileLineStart)
    {
        mapping.append(qMakePair(numLines, qMakePair(file, fileLineStart)));
        numLines += lines;
    }

    void dump()
    {
        qWarning() << "Line mapping dump" << numLines;
        for (int ii = 0; ii < mapping.count(); ++ii)
            qWarning() << mapping.at(ii).first << mapping.at(ii).second.first << mapping.at(ii).second.second;
    }

    TraceContext traceContextForLine(int line)
    {
        Q_ASSERT(!mapping.isEmpty());
        int found = -1;
        for (int ii = 1; found == -1 && ii < mapping.count(); ++ii) {
            if (mapping.at(ii).first > line)
                found = ii - 1;
        }
        if (-1 == found)
            found = mapping.count() - 1;

        TraceContext rv = { mapping.at(found).second.first,
                            line - mapping.at(found).first +
                                   mapping.at(found).second.second + 1 };
        return rv;
    }

    int numberOfLines() const { return numLines; }

private:
    QList<QPair<int, QPair<QString, int> > > mapping;
    int numLines;
};

struct TraceEngine  : public QScriptEngine
{
    TraceEngine() : version(0) {}

    int version;
    EngineLineMapping mapping;
};

static TraceContext qmakeobj_traceContext(QScriptContext *ctx,
                                          QScriptEngine *_eng)
{
    TraceEngine *eng = static_cast<TraceEngine *>(_eng);

    TraceContext rv = { QString(), -1 };
    if (options.debug & Options::Trace) {
        QString line = ctx->parentContext()->backtrace().first();
        int idx = line.lastIndexOf(':');
        Q_ASSERT(idx != -1);
        int lineNo = line.mid(idx + 1).toInt();
        rv = eng->mapping.traceContextForLine(lineNo);
    }
    return rv;
}

//
// Project binding
//
static QScriptValue project_dump(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());
    project->dump();
    return eng->nullValue();
}

static QScriptValue project_rule(QScriptContext *ctx, QScriptEngine *eng)
{
    Project *project = qscriptvalue_cast<Project *>(ctx->thisObject());

    QString name;
    if (ctx->argumentCount())
        name = ctx->argument(0).toString();
    Rule *rule = 0;

    if (name.isEmpty()) {
        rule = project->projectRules()->addRule(Rule(project->projectRules()));
    } else {
        rule = project->projectRules()->ruleByName(name);
        if (!rule)
            rule = project->projectRules()->addRule(Rule(name, project->projectRules()));
    }

    if (options.debug & Options::Trace)
        rule->trace.append(qmakeobj_traceContext(ctx, eng));
    return eng->newVariant(qVariantFromValue(rule));
}

//
// Object binding
//
static QScriptValue QMake_Object(QScriptContext *context, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);

    QString name = context->argument(0).toString();

    QMakeObject *obj = 0;
    if (name.contains('/')) {
        obj = QMakeObject::object(name, project);
    } else {
        obj = project->QMakeObject::property(name);
    }

    return eng->newVariant(qVariantFromValue(obj));
}

static QScriptValue QMake_QtValue(QScriptContext *context, QScriptEngine *eng)
{
    Project *project = qtscriptFunctions.project(eng);

    return eng->toScriptValue(project->qmakeValue(context->argument(0).toString()));
}

/* \internal

   This method is not part of the javascript API, but is used internall to run inline .pro syntax "snippets".
 */
static QScriptValue QMake_Run(QScriptContext *context, QScriptEngine *eng)
{
    context->setActivationObject(eng->globalObject());
    Project *project = qtscriptFunctions.project(eng);

    if (context->argumentCount() >= 1) {
        QStringList snippetReplacements;
        for (int ii = 1; ii < context->argumentCount(); ++ii) {
            snippetReplacements << context->argument(ii).toString();
        }

        int snippetNum = context->argument(0).toInt32();

        LOCK(QtScriptSnippets);
        qtscriptSnippetsLock.lock();
        QByteArray snippet = qtscriptSnippets.at(snippetNum).first;
        QString snippetFile = qtscriptSnippets.at(snippetNum).second;
        qtscriptSnippetsLock.unlock();
        if (snippetReplacements.count()) {
            for (int ii = 0; ii < snippetReplacements.count(); ++ii) {
                QString number = "<" + QString::number(ii) + ">";
                snippet.replace(number.toAscii(), snippetReplacements.at(ii).toAscii());
            }
        }

        if (!project->run(snippet, snippetFile)) {
            qOutput().nospace()
                << "Project " << project->messageDisplayName()
                << "ERROR: Unable to run snippet " << quote(snippetFile);
            QBuild::shutdown();
        }
    }

    return QScriptValue(eng, QScriptValue::NullValue);
}

static QScriptValue qbuild_invoke(QScriptContext *context, QScriptEngine *eng)
{
    if (!context->argumentCount())
        return QScriptValue(eng, QScriptValue::NullValue);

    Project *project = qtscriptFunctions.project(eng);

    if (!context->argumentCount()) {
        project->warning() << "qbuild.invoke() takes at least one argument";
        return eng->nullValue();
    }

    QString function = context->argument(0).toString();
    QStringLists arguments;

    for (int ii = 1; ii < context->argumentCount(); ++ii) {
        QStringList argument;

        QScriptValue arg = context->argument(ii);
        if (arg.isArray()) {
            for (int jj = 0; jj < arg.property("length").toInt32(); ++jj)
                argument << arg.property(jj).toString();
        } else {
            argument << arg.toString();
        }
        arguments << argument;
    }

    QStringList rv;

    context->setActivationObject(eng->globalObject());

    bool evalRv = FunctionProvider::evalFunction(project, rv, function, arguments);
    if (!evalRv) {
        project->warning() << "qbuild.invoke() cannot execute invalid function" << function;
        return QScriptValue(eng, QScriptValue::NullValue);
    }
    else if (rv.isEmpty()) {
        return QScriptValue(eng, QScriptValue::NullValue);
    } else {
        QScriptValue result = eng->newArray(rv.count());
        for (int ii = 0; ii < rv.count(); ++ii)
            result.setProperty(ii, QScriptValue(eng, rv.at(ii)));
        return result;
    }
}

static QScriptValue qbuild_varname(QScriptContext *context, QScriptEngine *eng)
{
    if (!context->argumentCount())
        return QScriptValue(eng, QScriptValue::NullValue);

    QString string = context->argument(0).toString();

    for (int ii = 0; ii < string.length(); ++ii) {
        QChar chr = string[ii];
        if (!chr.isLetterOrNumber()) {
            QString replace = "_" + QString::number(chr.unicode(), 16) + "_";
            string.replace(ii, 1, replace);
            ii += replace.length() - 1;
        }
    }

    return QScriptValue(eng, string);
}

static QScriptValue traceinfo_file(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject::TraceInfo to =
        qscriptvalue_cast<QMakeObject::TraceInfo>(ctx->thisObject());
    return QScriptValue(eng, to.fileName);
}

static QScriptValue traceinfo_line(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject::TraceInfo to =
        qscriptvalue_cast<QMakeObject::TraceInfo>(ctx->thisObject());
    return QScriptValue(eng, to.lineNo);
}

static QScriptValue traceinfo_value(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject::TraceInfo to =
        qscriptvalue_cast<QMakeObject::TraceInfo>(ctx->thisObject());
    return eng->toScriptValue(to.value);
}

static QScriptValue traceinfo_op(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject::TraceInfo to =
        qscriptvalue_cast<QMakeObject::TraceInfo>(ctx->thisObject());
    return QScriptValue(eng, QString(to.op));
}

static QMakeObject *qmakeobj_cast(QScriptValue value)
{
    QMakeObject *obj = qscriptvalue_cast<QMakeObject *>(value);
    if (!obj)
        obj = qscriptvalue_cast<Project *>(value);
    return obj;
}

static QScriptValue qmakeobj_find(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();

    if (ctx->argumentCount() < 1)
        return eng->nullValue();

    QString variable = ctx->argument(0).toString();
    QString value;
    QString result;
    if (ctx->argumentCount() >= 2)
        value = ctx->argument(1).toString();
    if (ctx->argumentCount() >= 3)
        result = ctx->argument(2).toString();

    QList<QMakeObject *> objs = obj->find(variable, value, result);
    QStringList rv;
    foreach(QMakeObject *obj, objs)
        rv << obj->absoluteName();

    return eng->toScriptValue(rv);
}

static QScriptValue qmakeobj_isproperty(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();

    QString prop = ctx->argument(0).toString();
    if (prop.startsWith("~")) {
        int idx = prop.mid(1).toUInt();
        return eng->toScriptValue(obj->isProperty(idx));
    } else {
        return eng->toScriptValue(obj->isProperty(ctx->argument(0).toString()));
    }
}

static QScriptValue qmakeobj_property(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    QMakeObject *prop = obj->property(ctx->argument(0).toString());
    if (!prop)
        return eng->nullValue();
    return eng->newVariant(qVariantFromValue(prop));
}

static QScriptValue qmakeobj_assignValue(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();

    QString rv;
    QStringList values = obj->value();
    foreach(QString value, values) {
        value.replace("\"", "\"\"");
        rv.append(value);
        rv.append(" ");
    }
    return eng->toScriptValue(rv);
}

static QScriptValue qmakeobj_strValue(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();

    QString val = obj->value().join(" ");
    return eng->toScriptValue(val);
}

static QScriptValue qmakeobj_name(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();

    return eng->toScriptValue(obj->name());
}

static QScriptValue qmakeobj_absName(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();

    return eng->toScriptValue(obj->absoluteName());
}

static QScriptValue qmakeobj_contains(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();

    QString cont = ctx->argument(0).toString();

    return QScriptValue(eng,obj->value().contains(cont));
}

static QScriptValue qmakeobj_properties(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();
    return eng->toScriptValue(obj->properties());
}

static QScriptValue qmakeobj_traceInfo(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();
    QList<QMakeObject::TraceInfo> ti = obj->traceInfo();
    QScriptValue rv = eng->newArray();
    for (int ii = 0; ii < ti.count(); ++ii)
        rv.setProperty(ii, eng->toScriptValue(ti.at(ii)));

    return rv;
}

static QScriptValue qmakeobj_isEmpty(QScriptContext *ctx, QScriptEngine *eng)
{
    bool rv = true;
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (obj) {
        QStringList val = obj->value();
        rv = val.isEmpty() || (val.count() == 1 && val[0].isEmpty());
    }
    return eng->toScriptValue(rv);
}

static QScriptValue qmakeobj_value(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->nullValue();
    return eng->toScriptValue(obj->value());
}

static QScriptValue qmakeobj_setValue(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    QScriptValue arg = ctx->argument(0);
    TraceContext fctx = qmakeobj_traceContext(ctx, eng);
    if (arg.isArray()) {
        obj->setValue(qscriptvalue_cast<QStringList>(arg), &fctx);
    } else {
        QString a = qscriptvalue_cast<QString>(arg);
        if ( a.isEmpty() )
            // Assigning an empty string clears the object, it does not create a list with a single empty item!
            obj->setValue(QStringList(), &fctx);
        else
            obj->setValue(QStringList(a), &fctx);
    }
    return eng->undefinedValue();
}

static QScriptValue qmakeobj_clear(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    TraceContext fctx = qmakeobj_traceContext(ctx, eng);
    obj->clear(&fctx);
    return eng->undefinedValue();
}

static QScriptValue qmakeobj_unite(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    QScriptValue arg = ctx->argument(0);
    TraceContext fctx = qmakeobj_traceContext(ctx, eng);
    if (arg.isArray()) {
        obj->uniteValue(qscriptvalue_cast<QStringList>(arg), &fctx);
    } else {
        obj->uniteValue(QStringList(qscriptvalue_cast<QString>(arg)), &fctx);
    }
    return eng->undefinedValue();
}

static QScriptValue qmakeobj_append(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    TraceContext fctx = qmakeobj_traceContext(ctx, eng);
    QScriptValue arg = ctx->argument(0);
    if (arg.isArray()) {
        obj->addValue(qscriptvalue_cast<QStringList>(arg), &fctx);
    } else {
        obj->addValue(QStringList(qscriptvalue_cast<QString>(arg)), &fctx);
    }
    return eng->undefinedValue();
}

static QScriptValue qmakeobj_remove(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    TraceContext fctx = qmakeobj_traceContext(ctx, eng);
    QScriptValue arg = ctx->argument(0);
    if (arg.isArray())
        obj->subtractValue(qscriptvalue_cast<QStringList>(arg), &fctx);
    else
        obj->subtractValue(QStringList(qscriptvalue_cast<QString>(arg)), &fctx);
    return eng->undefinedValue();
}

static QScriptValue qmakeobj_watch(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qmakeobj_cast(ctx->thisObject());
    if (!obj || ctx->argumentCount() != 1)
        return eng->undefinedValue();

    QString function = ctx->argument(0).toString();
    obj->watch( function );

    return eng->undefinedValue();
}

void qScript_Project(QScriptEngine *engine)
{
    // QMakeObject::TraceInfo binding
    QScriptValue to = engine->newObject();
    to.setProperty("file", engine->newFunction(traceinfo_file), QScriptValue::PropertyGetter);
    to.setProperty("line", engine->newFunction(traceinfo_line), QScriptValue::PropertyGetter);
    to.setProperty("value", engine->newFunction(traceinfo_value), QScriptValue::PropertyGetter);
    to.setProperty("op", engine->newFunction(traceinfo_op), QScriptValue::PropertyGetter);
    engine->setDefaultPrototype(qMetaTypeId<QMakeObject::TraceInfo>(), to);

    // QMakeObject binding
    QScriptValue object = engine->newObject();
    object.setProperty("find",
                       engine->newFunction(qmakeobj_find));
    object.setProperty("isProperty",
                       engine->newFunction(qmakeobj_isproperty));
    object.setProperty("property",
                       engine->newFunction(qmakeobj_property));
    object.setProperty("value",
                       engine->newFunction(qmakeobj_value));
    object.setProperty("isEmpty",
                       engine->newFunction(qmakeobj_isEmpty));
    object.setProperty("contains",
                       engine->newFunction(qmakeobj_contains));
    object.setProperty("strValue",
                       engine->newFunction(qmakeobj_strValue));
    object.setProperty("assignValue",
                       engine->newFunction(qmakeobj_assignValue));
    object.setProperty("setValue",
                       engine->newFunction(qmakeobj_setValue));
    object.setProperty("clear",
                       engine->newFunction(qmakeobj_clear));
    object.setProperty("name",
                       engine->newFunction(qmakeobj_name),
                       QScriptValue::PropertyGetter);
    object.setProperty("absName",
                       engine->newFunction(qmakeobj_absName),
                       QScriptValue::PropertyGetter);
    object.setProperty("append",
                       engine->newFunction(qmakeobj_append));
    object.setProperty("unite",
                       engine->newFunction(qmakeobj_unite));
    object.setProperty("remove",
                       engine->newFunction(qmakeobj_remove));
    object.setProperty("properties",
                       engine->newFunction(qmakeobj_properties));
    object.setProperty("traceInfo",
                       engine->newFunction(qmakeobj_traceInfo));
    object.setProperty("watch", engine->newFunction(qmakeobj_watch));
    engine->setDefaultPrototype(qMetaTypeId<QMakeObject *>(), object);

    // Project binding
    QScriptValue project = object;
    project.setProperty("dump", engine->newFunction(project_dump));
    project.setProperty("rule", engine->newFunction(project_rule));
    project.setProperty("isVirtual",
                        engine->newFunction(project_virtual));
    project.setProperty("sproject",
                        engine->newFunction(project_sproject));
    project.setProperty("solutionFile",
                        engine->newFunction(project_solutionfile));
    project.setProperty("config",
                        engine->newFunction(project_config));
    project.setProperty("message",
                        engine->newFunction(project_message));
    project.setProperty("info",
                        engine->newFunction(project_info));
    project.setProperty("warning",
                        engine->newFunction(project_warning));
    project.setProperty("error",
                        engine->newFunction(project_error));
    project.setProperty("run",
                        engine->newFunction(project_run));
    project.setProperty("expression",
                        engine->newFunction(project_expression));


    project.setProperty("file", engine->newFunction(project_file));
    project.setProperty("files", engine->newFunction(project_files));
    project.setProperty("projectFile",
                        engine->newFunction(project_projectFile));
    project.setProperty("projectFiles",
                        engine->newFunction(project_projectFiles));
    project.setProperty("filesystemFiles",
                        engine->newFunction(project_filesystemFiles));
    project.setProperty("filesystemFile",
                        engine->newFunction(project_filesystemFile));
    project.setProperty("buildFile",
                        engine->newFunction(project_buildFile));
    project.setProperty("buildPath",
                        engine->newFunction(project_buildPath));
    project.setProperty("paths",
                        engine->newFunction(project_paths));
    project.setProperty("projectPaths",
                        engine->newFunction(project_projectPaths));
    project.setProperty("filesystemPaths",
                        engine->newFunction(project_filesystemPaths));
    project.setProperty("disabledReason",
                        engine->newFunction(project_disabledReason));
    project.setProperty("pathIterator",
                        engine->newFunction(project_pathIterator));
    project.setProperty("reset",
                        engine->newFunction(project_reset));
    project.setProperty("resetReason",
                        engine->newFunction(project_resetReason));


    engine->setDefaultPrototype(qMetaTypeId<Project *>(), project);
    engine->globalObject().setProperty("project",
            engine->newFunction(project_default),
            QScriptValue::PropertyGetter);
}


QtScriptFunctions *QtScriptFunctions::instance()
{
    return &qtscriptFunctions;
}

/*!
  \class QtScriptFunctions
  \brief The QtScriptFunctions class exposes JavaScript functions to QBuild.

  The QtScriptFunctions class exports functions from the JavaScript extension files for QBuild.
  This allows these functions to be called from \l{QBuild Script} though there are many JavaScript
  types that cannot be converted to QBuild Script so not all functions are suitable for use.

  \sa {functions Extension}
*/

/*!
  Construct a QtScriptFunctions instance.
*/
QtScriptFunctions::QtScriptFunctions()
: FunctionProvider("qtscript") /*, m_engine(0) */
{
}

/*!
  \reimp
*/
QString QtScriptFunctions::fileExtension() const
{
    return "js";
}

/*!
  \internal
  Call locked
*/
bool
QtScriptFunctions::readFile(const QString &file, QByteArray &contents,
                            EngineLineMapping *mapping)
{
    QFile progfile(file);
    if (!progfile.open(QFile::ReadOnly))
        return false;

    contents.clear();
    QByteArray line;
    int fileLineNo = 0;
    int contentsLines = 0;

    while (!progfile.atEnd()) {
        ++fileLineNo;
        line = progfile.readLine();

        if (line.startsWith("###")) {

            QList<QByteArray> vars;
            foreach(QByteArray var, line.split(' ')) {
                var = var.trimmed();
                if (var.isEmpty() || var == "###")
                    continue;
                vars << var;
            }

            // Replace the line
            contents.append("\n");
            ++contentsLines;

            QByteArray qmake_contents;

            bool seenEnd = false;
            while (!seenEnd && !progfile.atEnd()) {
                line = progfile.readLine();
                QString snippetFile = file + ":" + QString::number(fileLineNo);
                ++fileLineNo;

                if (line.startsWith("###")) {
                    seenEnd = true;
                    if (!qmake_contents.isEmpty()) {
                        LOCK(LOCK QtScriptSnippets);
                        qtscriptSnippetsLock.lock();
                        qtscriptSnippets.append(qMakePair(qmake_contents, snippetFile));
                        contents.append("QMake_Run(" + QString::number(qtscriptSnippets.count() - 1));
                        qtscriptSnippetsLock.unlock();
                        foreach(QByteArray var, vars)
                            contents.append("," + var);
                        contents.append(");\n");

                    } else {
                        contents.append("\n");
                    }
                    ++contentsLines;

                } else {
                    // Replace the line
                    contents.append("\n");
                    ++contentsLines;
                    qmake_contents.append(line);
                }
            }
        } else if (line.startsWith("#include ")) {
            mapping->addLines(contentsLines, file,
                              fileLineNo - contentsLines - 1);
            contentsLines = 0;

            QByteArray incfile = line.trimmed();
            incfile = incfile.mid(9 /* ::strlen("#include ") */);

            if (QFastDir::isRelativePath(incfile)) {
                int idx = file.lastIndexOf('/');
                Q_ASSERT(idx != -1);
                QString path = file.left(idx + 1);
                incfile = path.toLatin1() + incfile;
            }

            QByteArray incContents;
            if (!readFile(incfile, incContents, mapping)) {
                qWarning() << "Project WARNING: Unable to include file" << file;
            } else {
                contents.append("\n");
                contents.append(incContents);
                contents.append("\n");
                contentsLines += 2;
            }
        } else {
            contents.append(line);
            ++contentsLines;
        }
    }

    if (contentsLines)
        mapping->addLines(contentsLines, file, fileLineNo - contentsLines);

    return true;
}

/*!
  \internal
*/
void QtScriptFunctions::lock()
{
    LOCK(QtScriptFunctions);
    m_runLock.lock();
}

/*!
  \internal
*/
void QtScriptFunctions::unlock()
{
    m_runLock.unlock();
}

/*!
  \internal
  Call locked

  Load the \a file into the \a tengine.  This does not update the engines
  version or have any other fancy side effects.  Returns false if the file
  could not be read and true if it succeeds.
*/
bool QtScriptFunctions::loadFile(TraceEngine *tengine,
                                 const QString &file, Project *proj)
{
    QByteArray contents;

    EngineLineMapping mapping = tengine->mapping;

    if (!readFile(file, contents, &mapping))
        return false;

    int startLine = tengine->mapping.numberOfLines();
    tengine->mapping = mapping;

    Project *oldProj = m_currentProject.project();
    m_currentProject.setProject(0);
    // Engine remains locked - loads cannot call out
    QBuild::beginPerfTiming("QtScriptFunctions::Load", file);
    tengine->evaluate(contents, QString(), startLine);
    QBuild::endPerfTiming();
    if (tengine->hasUncaughtException())
        unhandledException(proj);
    m_currentProject.setProject(oldProj);
    return true;
}

/*!
  \reimp
  Call unlocked
*/
bool QtScriptFunctions::loadFile(const QString &file, Project *proj)
{
    lock();

    // Bring engine up to date
    syncEngine();

    if (!m_loadedExtensions.contains(file)) {
        RUN_TRACE
            qWarning() << "Loading extension file" << file;
        m_loadedExtensions.insert(file);

        TraceEngine *tengine = engine();
        Q_ASSERT(tengine->version == m_engineVersions.count());

        if (!loadFile(tengine, file, proj)) {
            unlock();
            return false;
        }

        m_engineVersions.append(file);
        tengine->version = m_engineVersions.count();
        Project *oldProj = m_currentProject.project();
        m_currentProject.setProject(proj);
        updateEngineFunctions(tengine);
        m_currentProject.setProject(oldProj);
    } else {
        RUN_TRACE
            qWarning() << "Skipping existing extension file" << file;
    }

    bool need_init = !proj->m_projectExtensions.contains(file);
    if ( need_init ) proj->m_projectExtensions.insert(file);

    QString init_name;
    init_name = file.left(file.length() - 3 /* ::strlen(".js") */);
    int index = init_name.lastIndexOf('/');
    if (index != -1)
        init_name = init_name.mid(index + 1);

    if (need_init)
        init_name.append("_init");
    else
        init_name.append("_reinit");

    unlock();

    if (engine()->globalObject().property(init_name).isFunction()) {
        RUN_TRACE
            qWarning() << "Running initializer" << init_name;
        Project *oldProj = m_currentProject.project();
        m_currentProject.setProject(proj);
        QScriptEngine *e = engine();
        QBuild::beginPerfTiming("QtScriptFunctions::Init", init_name);
        e->evaluate(init_name + "()");
        QBuild::endPerfTiming();
        if ( e->hasUncaughtException() )
            unhandledException(proj);
        m_currentProject.setProject(oldProj);
    } else {
        RUN_TRACE
            qWarning() << "No such initializer" << init_name;
    }

    return true;
}

/*!
  \reimp
*/
void QtScriptFunctions::resetProject(Project * /*project*/)
{
    qOutput() << "Unsupported";
    QBuild::shutdown();
}

/*!
  \internal
*/
bool QtScriptFunctions::runScript(Project *project,
                                  const QByteArray &code,
                                  const TraceContext &ctxt)
{
    lock();

    // Bring engine up to date
    syncEngine();

    QList<QByteArray> codeLines = code.split('\n');

    Project *oldProj = m_currentProject.project();

    m_currentProject.setProject(project);
    TraceEngine *e = static_cast<TraceEngine *>(engine());
    int startLine = e->mapping.numberOfLines();
    e->mapping.addLines(codeLines.count(), ctxt.fileName, ctxt.lineNumber);
    unlock();

    QScriptValue val = e->evaluate("function() {" + code + "}",
                                   QString(), startLine);
    if (e->hasUncaughtException())
        unhandledException(project);
    val.call();
    if (e->hasUncaughtException())
        unhandledException(project, code);
    m_currentProject.setProject(oldProj);


    return true;
}

/*!
  \reimp
*/
bool QtScriptFunctions::callFunction(Project *project,
                                     QStringList &rv,
                                     const QString &function,
                                     const QStringLists &arguments)
{
    bool funcReturn = false;
    lock();

    // Check if this function even exists
    if (m_functions.contains(function)) {
        // Bring engine up to date
        syncEngine();

        unlock();

        QScriptValue val;
        if (callFunction(project, val, function, arguments))  {
            funcReturn = true;

            if (val.isBoolean()) {
                rv = QStringList() << (val.toBoolean()?"true":"false");
            } else if (val.isNull()) {
                rv = QStringList();
            } else if (val.isArray()) {
                rv = QStringList();
                for ( int i = 0; ; i++ ) {
                    QScriptValue v = val.property(i);
                    if ( !v.isValid() )
                        break;
                    rv << v.toString();
                }
            } else {
                QString str = val.toString();
                if (str.isEmpty())
                    rv = QStringList();
                else
                    rv = QStringList() << str;
            }
        }
    } else {
        unlock();
    }

    return funcReturn;
}

/*!
  \internal
  Call unlocked
*/
bool QtScriptFunctions::callFunction(Project *project,
                                     QScriptValue &rv,
                                     const QString &function,
                                     const QStringLists &arguments)
{
    QScriptEngine *e = engine();
    QScriptValue func = e->globalObject().property(function);
    if (!func.isFunction())
        return false;

    QScriptValueList args;
    for (int ii = 0; ii < arguments.count(); ++ii) {
        const QStringList &arg = arguments.at(ii);
        if (arg.isEmpty())
            args << QScriptValue();
        else if (arg.count() == 1)
            args << QScriptValue(e, arg.first());
        else
            args << e->toScriptValue(arg);
    }

    Project *oldProj = m_currentProject.project();
    m_currentProject.setProject(project);
    QBuild::beginPerfTiming("QtScriptFunctions::Call", function);
    rv = func.call(QScriptValue(), args);
    QBuild::endPerfTiming();
    m_currentProject.setProject(oldProj);
    bool ret = !e->hasUncaughtException();
    if (!ret)
        unhandledException(project, "Calling function " + function);

    return true;
}

/*!
  \internal
*/
Project *QtScriptFunctions::project(QScriptEngine *)
{
    Project *rv = m_currentProject.project();
    return rv;
}

/*!
  \internal
*/
void QtScriptFunctions::unhandledException(Project *proj, const QString &ctxt)
{
    TraceContext tc = static_cast<TraceEngine *>(engine())->mapping.traceContextForLine(engine()->uncaughtExceptionLineNumber());

    QString error = "Project " + (proj?(proj->messageDisplayName()):"INTERNAL") + "ERROR: An error occured while evaluating a QBuild script extension.\n";
    if (!ctxt.isEmpty())
        error += "    Context: " + ctxt + "\n";
    error += "    File:    " + tc.fileName + "@" + QString::number(tc.lineNumber) + "\n";
    error += "    Error:   " + engine()->uncaughtException().toString() + "\n";
    foreach(QString str, engine()->uncaughtExceptionBacktrace())
        error += "             " + str + "\n";
    qOutput() << error;
    QBuild::shutdown();
}

/*!
  \internal
*/
TraceEngine *QtScriptFunctions::createEngine()
{
    TraceEngine *engine = new TraceEngine();
    qScriptRegisterSequenceMetaType<QStringList>(engine);

    // Functions
    QScriptValue qmake_call = engine->newFunction(qbuild_invoke);
    QScriptValue qmake_qtvalue = engine->newFunction(QMake_QtValue);
    QScriptValue qmake_run = engine->newFunction(QMake_Run);

    engine->globalObject().setProperty("QMake_QtValue", qmake_qtvalue);
    engine->globalObject().setProperty("QMake_Run", qmake_run);

    engine->globalObject().setProperty("qbuild_varname",
                                       engine->newFunction(qbuild_varname));

    QScriptValue qbuild = engine->newObject();
    qbuild.setProperty("invoke", qmake_call);
    qbuild.setProperty("object", engine->newFunction(QMake_Object));
    qbuild.setProperty("qtValue", qmake_qtvalue);
    qbuild.setProperty("root", engine->newFunction(qbuild_root),
                       QScriptValue::PropertyGetter);
    qbuild.setProperty("varname", engine->newFunction(qbuild_varname));

    engine->globalObject().setProperty("qbuild", qbuild);

    qScript_Project(engine);
    qScript_Rule(engine);
    qScript_Solution(engine);
    qScript_SolutionProject(engine);
    qScript_SolutionFile(engine);
    qScript_File(engine);
    qScript_PathIterator(engine);
    return engine;
}

/*!
  \internal
  Call locked
*/
void QtScriptFunctions::syncEngine()
{
    syncEngine(engine());
}

/*!
  \internal
  Call locked
*/
void QtScriptFunctions::syncEngine(TraceEngine *e)
{
    while (e->version != m_engineVersions.count()) {
        if (!loadFile(e, m_engineVersions.at(e->version), 0)) {
            QString error = "QBuild: An internal error occurred while trying to synchronize QScript threads.\n";
            error += "        This is the result of removing or modifying QBuild extensions while";
            error += "        QBuild is running.";
            qOutput() << error;
            QBuild::shutdown();
        }
        e->version++;
    }
}

/*!
  \internal
*/
TraceEngine *QtScriptFunctions::engine()
{
    TraceEngine *rv = m_currentProject.engine();
    if (!rv) {
        m_currentProject.setEngine(createEngine());
        rv = m_currentProject.engine();
    }

    return rv;
}

/*!
  \internal
  Call locked
*/
void QtScriptFunctions::updateEngineFunctions(TraceEngine *e)
{
    m_functions.clear();
    QScriptValue go = e->globalObject();
    QScriptValueIterator iter(go);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value().isFunction())
            m_functions.insert(iter.name());
    }
}

/*!
  \fn QtScriptFunctions::instance()
  Returns the singleton instance of QtScriptFunctions.
*/

