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

#include "startup.h"
#include "project.h"
#include "functionprovider.h"
#include <QCoreApplication>
#include "qoutput.h"
#include <stdlib.h>
#include <QFile>
#include <QDir>
#include "qfastdir.h"

/*!

\file qbuild.startup.args
\brief Describes the qbuild.startup.args file and its contents.

*/

/*!

\file qbuild.startup.js
\brief Describes the qbuild.startup.js file and its contents.

*/

/*!
  \class StartupFile
  \brief The StartupFile class holds the environment specified by the startup scripts.
*/

/*!
  \internal
*/
StartupFile::StartupFile(const SolutionFile &file)
: m_file(file), m_errorLine(-1)
{
    if (!file.isValid())
        return;

    Block *block = QMakeParser::parseFile(file.fsPath());
    if (!block) return;
    runBlock(block);
    delete block;
}

/*!
  \internal
*/
SolutionFile StartupFile::file() const
{
    return m_file;
}

/*!
  \internal
*/
int StartupFile::errorLine() const
{
    return m_errorLine;
}

/*!
  \internal
*/
void StartupFile::error(Block *block)
{
    if (m_errorLine == -1)
        m_errorLine = block->lineNumber();
}

/*!
  \internal
*/
void StartupFile::runBlock(Block *block)
{
    switch(block->type()) {
        case Block::MultiBlock:
            runBlock(static_cast<MultiBlock *>(block));
            break;
        case Block::Assignment:
            runBlock(static_cast<Assignment *>(block));
            break;
        case Block::Script:
        case Block::Scope:
        case Block::Function:
            // Not supported
            error(block);
            return;
    }
}

/*!
  \internal
*/
void StartupFile::runBlock(MultiBlock *block)
{
    Blocks blocks = block->blocks();
    for (int ii = 0; ii < blocks.count(); ++ii)
        runBlock(blocks.at(ii));
}

/*!
  \internal
*/
void StartupFile::runBlock(Assignment *block)
{
    if (block->op() != Assignment::Assign) {
        error(block);
        return;
    }

    QStringList val;
    if (!expressionValue(block->expression(), val)) {
        error(block);
        return;
    }

    property(QString(block->name()))->setValue(val);
}

/*!
  \internal
*/
bool StartupFile::expressionValue(const Expression &expr, QStringList &out)
{
    QList<EvaluatableValues> vals = expr.evaluatableValues();

    for (int ii = 0; ii < vals.count(); ++ii) {
        QStringList outval;
        for (int jj = 0; jj < vals.at(ii).count(); ++jj) {
            EvaluatableValue val = vals.at(ii).at(jj);

            switch(val.type()) {
                case EvaluatableValue::Characters:
                    outval << val.name();
                    break;
                case EvaluatableValue::Environment:
                    outval << ::getenv(val.name().constData());
                    break;
                case EvaluatableValue::Variable:
                    {
                        QMakeObject *obj = property(QString(val.name()));
                        if (obj)
                            outval << obj->value();
                    }
                    break;
                case EvaluatableValue::QtVariable:
                case EvaluatableValue::Function:
                case EvaluatableValue::TFunction:
                    return false;
            }
        }
        if (outval.count() > 1 && vals.at(ii).count() == 1) {
            out << outval;
        } else if (outval.count()) {
            out << outval.join(" ");
        }
    }

    return true;
}

Q_DECLARE_METATYPE(StartupScript::Option)
Q_DECLARE_METATYPE(QMakeObject *)
Q_DECLARE_METATYPE(Solution *)

struct StartupEngine : public QScriptEngine
{
    StartupEngine(StartupScript *parent)
        : QScriptEngine(parent), script(parent) {}

    StartupScript *script;
};

/*!
  \class StartupScript
  \brief The StartupScript class manages running the QBuild startup scripts.

  Examines the <QBuild Executable Directory>/qbuild.startup.args file for
  argument descriptors and runs the
  <QBuild Executable Directory>/qbuild.startup.js startup script.

  The format of the qbuild.startup.args file is zero or more repetitions of the
  form:
  \code
    <Option Name> [<Option argument count>]
    <Tab><Help line 1>
    <Tab><Help line 2>
    ...
  \endcode

  The following object types are added:

  \code
    CommandLineOption {
        name
        arguments[]
    }
  \endcode

  The following properties are exported:
  \list type=dl
  \o qbuild.arguments The arguments passed to qbuild (read only)
  \o qbuild.mkspec The default mkspec (read/write)
  \endlist

  The following functions are bound:

  \code
    qbuild.env(<variable>): Returns the environment variable <variable>
    qbuild.value(<name>, <value>): Returns an exported value
    qbuild.setValue(<name>, <value>): Sets an exported value
    qbuild.showHelp(<message>):
    qbuild.error(<message>):
    qbuild.addExtensions(<path>)
    qbuild.addMkspecs(<path>)

    startupfile(<filename>): Absolute or solution filename
        .value(<prop name>)
        .properties(<prop name>)
        .property(<prop name>)

    solution().isSolution(<name>)
    solution().addSolution(<name>, <path>)

    solution(<name>)
        .files(<path>)
        .paths(<path>)
        .exists(<name>)
        .realToSolution(<path>)
        .addPath(<solution path>, <fs path>)
  \endcode

  \sa qbuild.startup.args, qbuild.startup.js
*/

/*!
  \internal
  \class StartupScript::Script
*/

void showHelpline(int, const char *);

static QScriptValue solution_addSolution(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() != 2)
        return eng->undefinedValue();

    QString name = ctx->argument(0).toString();
    QString path = ctx->argument(1).toString();
    if (QFastDir::isRelativePath(path))
        path = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + path);
    (void)new Solution(name, path);

    return eng->nullValue();
}

static QScriptValue solution_isSolution(QScriptContext *ctx, QScriptEngine *eng)
{
    if (1 != ctx->argumentCount())
        return eng->undefinedValue();

    QString name = ctx->argument(0).toString();

    return eng->toScriptValue(Solution::solution(name) != 0);
}

static QScriptValue solution_files(QScriptContext *ctx, QScriptEngine *eng)
{
    qWarning() << "XXX";
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());

    if (ctx->argumentCount() != 1)
        return eng->nullValue();

    return eng->toScriptValue(solution->files(ctx->argument(0).toString()));
}

static QScriptValue solution_paths(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());

    if (ctx->argumentCount() != 1)
        return eng->nullValue();

    return eng->toScriptValue(solution->paths(ctx->argument(0).toString()));
}

static QScriptValue solution_exists(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());

    if (ctx->argumentCount() != 1)
        return eng->nullValue();

    return eng->toScriptValue(solution->findFile(ctx->argument(0).toString(),
                                                 Solution::Existing).isValid());
}

static QScriptValue solution_realToSolution(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());

    if (ctx->argumentCount() != 1)
        return eng->nullValue();

    return eng->toScriptValue(solution->realToSolution(ctx->argument(0).toString()).solutionPath());
}

static QScriptValue solution_addPath(QScriptContext *ctx, QScriptEngine *eng)
{
    Solution *solution = qscriptvalue_cast<Solution *>(ctx->thisObject());

    if (ctx->argumentCount() != 2)
        return eng->undefinedValue();

    solution->addPath(ctx->argument(0).toString(), ctx->argument(1).toString());

    return eng->nullValue();
}

static QScriptValue solution_default(QScriptContext * /*ctx*/, QScriptEngine *eng)
{
    QScriptValue solution = eng->newObject();
    solution.setProperty("isSolution",
                         eng->newFunction(solution_isSolution));
    solution.setProperty("addSolution",
                         eng->newFunction(solution_addSolution));
    return solution;
}

static QScriptValue qbuild_mkspec(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 1)
        StartupScript::setValue("default_mkspec", ctx->argument(0).toString());

    return eng->toScriptValue(StartupScript::value("default_mkspec"));
}

static QScriptValue qbuild_addMkspecs(QScriptContext *ctx, QScriptEngine *eng)
{
    if (1 != ctx->argumentCount())
        return eng->undefinedValue();

    QString path = ctx->argument(0).toString();

    Project::addMkspecPath(path);

    return eng->nullValue();
}

static QScriptValue startupfile_default(QScriptContext *ctx, QScriptEngine *eng)
{
    if (1 != ctx->argumentCount())
        return eng->undefinedValue();

    QString req = ctx->argument(0).toString();
    QString solution;
    QString fileName;

    int idx = req.indexOf(':');
    if (idx != -1) {
        solution = req.left(idx);
        fileName = req.mid(idx + 1);
    } else {
        fileName = req;
    }


    SolutionFile file;

    if (solution.isEmpty()) {
        Solution *sol = Solution::defaultSolution();
        if (QFastDir::isRelativePath(fileName))
            fileName.prepend(QCoreApplication::applicationDirPath() + "/");
        file = sol->findFile("!" + fileName, Solution::Existing);
    } else {
        Solution *sol = Solution::solution(solution);
        if (!sol)
            return eng->nullValue();
        file = sol->findFile(fileName, Solution::Existing);
    }

    if (!file.isValid())
        return eng->nullValue();

    StartupFile *sfile = new StartupFile(file);
    if (sfile->errorLine() != -1) {
        qWarning() << "QBuild encountered an error while reading startup file " << req << "at line" << sfile->errorLine();
        qWarning() << "Startup files may only use a very restricted set of QBuild syntax.";
        delete sfile;
        return eng->nullValue();
    }

    return eng->newVariant(qVariantFromValue((QMakeObject *)sfile));
}

static QScriptValue qbuild_addExtensions(QScriptContext *ctx, QScriptEngine *eng)
{
    if (1 != ctx->argumentCount())
        return eng->undefinedValue();

    QString path = ctx->argument(0).toString();

    FunctionProvider::addExtensionsPath(path);

    return eng->nullValue();
}

static QScriptValue qbuild_showHelp(QScriptContext *ctx, QScriptEngine *eng)
{
    QString error;

    if (ctx->argumentCount())
        error = ctx->argument(0).toString();

    if (!error.isEmpty()) {
        showHelpline(0, error.toAscii().constData());
        qWarning();
    }

    static_cast<StartupEngine *>(eng)->script->showHelp();
    exit(-1);
}

static QScriptValue qbuild_value(QScriptContext *ctx, QScriptEngine *eng)
{
    StartupScript *startup = static_cast<StartupEngine *>(eng)->script;

    if (ctx->argumentCount() != 1)
        return eng->undefinedValue();

    return eng->toScriptValue(startup->value(ctx->argument(0).toString()));
}

static QScriptValue qbuild_setValue(QScriptContext *ctx, QScriptEngine *eng)
{
    StartupScript *startup = static_cast<StartupEngine *>(eng)->script;

    if (ctx->argumentCount() != 2)
        return eng->undefinedValue();

    startup->setValue(ctx->argument(0).toString(), ctx->argument(1).toString());

    return eng->nullValue();
}

static QScriptValue qbuild_error(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);
    Q_UNUSED(eng);
    QString error;

    if (ctx->argumentCount())
        error = ctx->argument(0).toString();

    if (error.isEmpty()) {
        showHelpline(0, "QBuild is unable to startup, due to an error it encountered while running its startup scripts.  Please ensure that QBuild is installed correctly.");
    } else {
        QStringList lines = error.split('\n');
        foreach(QString line, lines)
            showHelpline(0, line.toAscii().constData());
    }
    exit(-1);
}

static QScriptValue qbuild_env(QScriptContext *ctx, QScriptEngine *eng)
{
    if (!ctx->argumentCount())
        return eng->undefinedValue();

    QString arg = ctx->argument(0).toString();
    QString rv = ::getenv(arg.toAscii().constData());

    return eng->toScriptValue(rv);
}

static QScriptValue option_name(QScriptContext *ctx, QScriptEngine *eng)
{
    StartupScript::Option option =
        qscriptvalue_cast<StartupScript::Option>(ctx->thisObject());
    return eng->toScriptValue(option.name);
}

static QScriptValue option_default(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx);

    StartupScript *startup = static_cast<StartupEngine *>(eng)->script;

    QList<StartupScript::Option> startupOptions = startup->startupOptions();
    QScriptValue rv = eng->newArray(startupOptions.count());

    for (int ii = 0; ii < startupOptions.count(); ++ii)
        rv.setProperty(ii, eng->newVariant(qVariantFromValue(startupOptions.at(ii))));

    return rv;
}

static QScriptValue option_parameters(QScriptContext *ctx, QScriptEngine *eng)
{
    StartupScript::Option option =
        qscriptvalue_cast<StartupScript::Option>(ctx->thisObject());
    return eng->toScriptValue(option.parameters);
}

static QScriptValue qmakeobj_property(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qscriptvalue_cast<QMakeObject *>(ctx->thisObject());
    if (!obj)
        return eng->undefinedValue();
    QMakeObject *prop = obj->property(ctx->argument(0).toString());
    if (!prop)
        return eng->nullValue();
    return eng->newVariant(qVariantFromValue(prop));
}

static QScriptValue qmakeobj_value(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qscriptvalue_cast<QMakeObject *>(ctx->thisObject());
    if (!obj)
        return eng->nullValue();
    return eng->toScriptValue(obj->value());
}

static QScriptValue qmakeobj_properties(QScriptContext *ctx, QScriptEngine *eng)
{
    QMakeObject *obj = qscriptvalue_cast<QMakeObject *>(ctx->thisObject());
    if (!obj)
        return eng->nullValue();
    return eng->toScriptValue(obj->properties());
}

/*!
  \internal
*/
StartupScript::StartupScript(const char *appname, QObject *parent)
: QObject(parent), engine(0), m_appName(appname)
{
    engine = new StartupEngine(this);

    QScriptValue option = engine->newObject();
    option.setProperty("name", engine->newFunction(option_name),
                       QScriptValue::PropertyGetter);
    option.setProperty("parameters", engine->newFunction(option_parameters),
                       QScriptValue::PropertyGetter);
    engine->setDefaultPrototype(qMetaTypeId<StartupScript::Option>(), option);

    QScriptValue qbuild = engine->newObject();
    qbuild.setProperty("env", engine->newFunction(qbuild_env));
    qbuild.setProperty("value", engine->newFunction(qbuild_value));
    qbuild.setProperty("setValue", engine->newFunction(qbuild_setValue));
    qbuild.setProperty("error", engine->newFunction(qbuild_error));
    qbuild.setProperty("showHelp", engine->newFunction(qbuild_showHelp));
    qbuild.setProperty("mkspec", engine->newFunction(qbuild_mkspec),
                       QScriptValue::PropertyGetter |
                       QScriptValue::PropertySetter);
    qbuild.setProperty("addMkspecs", engine->newFunction(qbuild_addMkspecs));
    qbuild.setProperty("addExtensions", engine->newFunction(qbuild_addExtensions));
    qbuild.setProperty("options", engine->newFunction(option_default),
                       QScriptValue::PropertyGetter);
    engine->globalObject().setProperty("qbuild", qbuild);

    engine->globalObject().setProperty("solution",
                                       engine->newFunction(solution_default),
                                       QScriptValue::PropertyGetter);

    QScriptValue solution = engine->newObject();
    solution.setProperty("files",
                         engine->newFunction(solution_files));
    solution.setProperty("paths",
                         engine->newFunction(solution_paths));
    solution.setProperty("exists",
                         engine->newFunction(solution_exists));
    solution.setProperty("realToSolution",
                         engine->newFunction(solution_realToSolution));
    solution.setProperty("addPath",
                         engine->newFunction(solution_addPath));
    engine->setDefaultPrototype(qMetaTypeId<Solution *>(), solution);

    engine->globalObject().setProperty("startupfile", engine->newFunction(startupfile_default));
    QScriptValue object = engine->newObject();
    object.setProperty("property",
            engine->newFunction(qmakeobj_property));
    object.setProperty("value",
            engine->newFunction(qmakeobj_value));
    object.setProperty("properties",
            engine->newFunction(qmakeobj_properties));
    engine->setDefaultPrototype(qMetaTypeId<QMakeObject *>(), object);

    readOptions();

    // Set default stuff
#ifndef QBUILD_DEFAULT_MKSPEC
#define QBUILD_DEFAULT_MKSPEC "default"
#endif
    setValue("default_mkspec", QBUILD_DEFAULT_MKSPEC);
}

/*!
  \internal
*/
QList<StartupScript::Option> StartupScript::startupOptions() const
{
    return m_startupOptions;
}

/*!
  \internal
*/
bool StartupScript::startup(const QList<Option> &options)
{
    QString startup_script =
        QCoreApplication::applicationDirPath() + "/qbuild.startup.js";

    if (!QFastDir::exists(startup_script)) {
        qWarning() << "QBuild is missing the following startup files.  Please ensure that QBuild is";
        qWarning() << "installed correctly.";
        qWarning() << "    " << startup_script;
        return false;
    }

    QFile file(startup_script);
    if (!file.open(QFile::ReadOnly)) {
        showHelpline(0,"QBuild is unable to access the following startup files.  Please ensure that QBuild is installed correctly.");
        showHelpline(1, startup_script.toAscii().constData());
        return false;
    }

    QByteArray script = file.readAll();

    m_startupOptions = options;
    ScriptError error = runScript(Script(script, startup_script));
    if (error.isError) {
        qWarning() << "QBuild is unable to execute its startup file.  Please ensure that QBuild ";
        qWarning() << "is installed correctly.";

        qOutput() << error.error;
        foreach(QString line, error.backtrace)
            qOutput() << "    " << line;
        return false;
    }

    return true;
}

/*!
  \internal
*/
StartupScript::ScriptError
StartupScript::runScript(const Script &script)
{
    ScriptError rv;

    engine->evaluate(script.code, script.filename, script.fileline);

    if (engine->hasUncaughtException()) {
        rv.isError = true;
        rv.error = engine->uncaughtException().toString();
        rv.backtrace = engine->uncaughtExceptionBacktrace();
    } else {
        rv.isError = false;
    }

    return rv;
}

/*!
  \internal
*/
QList<StartupScript::OptionDesc> StartupScript::options() const
{
    return m_options;
}

/*!
  \internal
*/
void StartupScript::readOptions()
{
    QString options =
        QCoreApplication::applicationDirPath() + "/qbuild.startup.args";

    QFile optionsFile(options);
    if (!optionsFile.open(QFile::ReadOnly))
        return;

    enum State { Help, Description } state = Description;
    OptionDesc option;
    while (!optionsFile.atEnd()) {
        QString line = optionsFile.readLine();
        line = line.left(line.length() - 1); // Remove "\n"

        if (state == Help) {
            if (line.startsWith("\t")) {
                line = line.mid(1);
                option.help << line;
            } else {
                if (!option.name.isEmpty()) {
                    m_options << option;
                    option.name.clear();
                    option.help.clear();
                }

                state = Description;
            }
        }

        if (state == Description) {
            line = line.trimmed();
            if (line.isEmpty() || line.startsWith("#"))
                continue;

            QStringList parts = line.split(" ", QString::SkipEmptyParts);
            if (parts.count() != 1 && parts.count() != 2) // Invalid line
                return;

            option.name = parts.at(0);
            if (parts.count() == 2)
                option.count = parts.at(1).toUInt();
            else
                option.count = 0;

            state = Help;
        }
    }
    if (!option.name.isEmpty())
        m_options << option;
}

QHash<QString, QString> StartupScript::m_values;
/*!
  \internal
*/
QString StartupScript::value(const QString &value)
{
    QHash<QString,QString>::ConstIterator iter = m_values.find(value);
    if (iter == m_values.end())
        return QString();
    else
        return *iter;
}

/*!
  \internal
*/
void StartupScript::setValue(const QString &value, const QString &data)
{
    m_values[value] = data;
}

