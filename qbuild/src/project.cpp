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

#include "project.h"
#include "startup.h"
#include <QCoreApplication>
#include "functionprovider.h"
#include <QStringList>
#include <QRegExp>
#include "qoutput.h"
#include <QFile>
#include "lexer.h"
#include "parser.h"
#include "preprocessor.h"
#include <QLibraryInfo>
#include <stdlib.h>
#include "options.h"
#include "qfastdir.h"
#include "qbuild.h"
#include <QStack>
#include "qtscriptfunctions.h"

class QMakeInternalVariables : public IInternalVariable
{
public:
    virtual bool value(const QString &name, QStringList &rv)
    {
        if (name == "QT_PREFIX") {
            /*!
              \qt_variable QT_PREFIX
              \brief Returns the location where Qt is installed to.
            */
            rv << QLibraryInfo::location(QLibraryInfo::PrefixPath);
            return true;
        } else if (name == "QT_DOCUMENTATION") {
            /*!
              \qt_variable QT_DOCUMENTATION
              \brief Returns the location where Qt's docs are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
            return true;
        } else if (name == "QT_HEADERS") {
            /*!
              \qt_variable QT_HEADERS
              \brief Returns the location where Qt's headers are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::HeadersPath);
            return true;
        } else if (name == "QT_LIBRARIES") {
            /*!
              \qt_variable QT_LIBRARIES
              \brief Returns the location where Qt's libraries are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
            return true;
        } else if (name == "QT_BINARIES" ||
                  name == "QT_INSTALL_BINS") {
            /*!
              \qt_variable QT_BINARIES
              \brief Returns the location where Qt's binaries are located.
            */
            /*!
              \qt_variable QT_INSTALL_BINS
              \brief Returns the location where Qt's binaries are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::BinariesPath);
            return true;
        } else if (name == "QT_PLUGINS") {
            /*!
              \qt_variable QT_PLUGINS
              \brief Returns the location where Qt's plugins are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::PluginsPath);
            return true;
        } else if (name == "QT_DATA") {
            /*!
              \qt_variable QT_DATA
              \brief Returns the location where Qt's data is located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::DataPath);
            return true;
        } else if (name == "QT_TRANSLATIONS") {
            /*!
              \qt_variable QT_DATA
              \brief Returns the location where Qt's translations are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
            return true;
        } else if (name == "QT_SETTINGS") {
            /*!
              \qt_variable QT_DATA
              \brief Returns the location where Qt's settings are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::SettingsPath);
            return true;
        } else if (name == "QT_EXAMPLES") {
            /*!
              \qt_variable QT_DATA
              \brief Returns the location where Qt's examples are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
            return true;
        } else if (name == "QT_DEMOS") {
            /*!
              \qt_variable QT_DATA
              \brief Returns the location where Qt's demos are located.
            */
            rv << QLibraryInfo::location(QLibraryInfo::DemosPath);
            return true;
        } else {
            return false;
        }
    }
};
static QMakeInternalVariables qmakeInternalVariables;

/*!

\file qbuild.pro
\brief Describes the qbuild.pro file and its contents.

\sa {QBuild Script}
*/

/*!
  \class Project
  \brief The Project class represents a single project.

  Following finalization, all methods are thread safe (the object has become
  read-only)

  \sa qbuild.pro
*/

/*!
  \enum Project::LoadMode
  \value ProjectMode The project was opened in project mode.
  \value FileMode The project was opened in file mode.
*/

/*!
  \internal
*/
Project::Project(const SolutionProject &project, LoadMode mode, QObject *parent)
: QObject(parent), m_file(project.projectFile()), m_name(project.node()),
  m_rules(0), m_solution(project.solution()), m_subscription(0),
  m_finalized(false), m_hasWarnings(false), m_isVirtual(false), m_sp(project),
  m_reseting(false), m_disabledState(NotDisabled),
  m_loadmode(mode)
{
    Q_ASSERT(m_solution);

    m_buildDir = solution()->findFile(nodePath(), Solution::Generated).fsPath();

    if (ProjectMode == m_loadmode) {
        RUN_TRACE
            qWarning() << "Opening project" << project.node()
                       << "(filename:" << project.fileName()
                       << ")";
    } else {
        RUN_TRACE
            qWarning() << "Opening project" << project.node()
                       << "(filename:" << project.fileName()
                       << ", FileMode)";
    }
    // Note: This cannot loop indefinately, as there are a finite number of
    // reset causes (and no reset cause can occur more than once)
    do {
        m_reseting = false;
        init();
        if (!reseting())
            runProject();
    } while (reseting());
}

/*!
  \internal
*/
void Project::init()
{
    QMakeObject::clear();
    m_includedFiles.clear();
    m_projectExtensions.clear();
    m_qmakeValues.clear();
    m_finalized = false;
    m_isVirtual = false;
    if (m_rules) {
        delete m_rules;
        m_rules = 0;
    }

    m_qmakeValues << &qmakeInternalVariables;

    if (!m_name.startsWith('/'))
        m_name.prepend('/');

    if (m_solution != Solution::defaultSolution()) {
        QString sname = m_solution->name() + ":" + m_name;
        setName(sname);
    } else {
        setName(m_name);
    }

    if (!m_subscription) {
        m_subscription = new QMakeObjectSubscription(this);
        QObject::connect(m_subscription,
                         SIGNAL(valueChanged(QString,QStringList,QStringList)),
                         this,
                         SLOT(valueChanged(QString,QStringList,QStringList)));
    }
    m_subscription->subscribe(QMakeObject::property("CONFIG"), "CONFIG");

    // common.pri is loaded for all projects opened in project mode (but not file mode)
    if (m_loadmode == ProjectMode) {
        QList<SolutionFile> commonFiles = solution()->commonIncludes();
        for (int ii = 0; !reseting() && ii < commonFiles.count(); ++ii)
            includeFile(commonFiles.at(ii));
    }
}

/*
   \internal
 */
void Project::runProject()
{
    if (m_disabledState == Disabled) {
        // disabled.pri is loaded for disabled projects
        QList<SolutionFile> disabledFiles = solution()->disabledIncludes();
        for (int ii = 0; !reseting() && ii < disabledFiles.count(); ++ii) {
            includeFile(disabledFiles.at(ii));
        }
        if (!reseting())
            setVirtualProject(true);
    } else if (m_file.isValid()) {
        if (m_loadmode == ProjectMode) {
            // default.pri is loaded for regular projects
            QList<SolutionFile> defaultFiles = solution()->defaultIncludes();
            for (int ii = 0; !reseting() && ii < defaultFiles.count(); ++ii) {
                includeFile(defaultFiles.at(ii));
            }
        }
        if (!reseting())
            includeFile(m_file);
    } else {
        // blank.pri is loaded for projects with no qbuild.pro
        QList<SolutionFile> blankFiles = solution()->blankIncludes();
        for (int ii = 0; !reseting() && ii < blankFiles.count(); ++ii) {
            includeFile(blankFiles.at(ii));
        }
        if (!reseting())
            setVirtualProject(true);
    }
}

/*!
  \internal
*/
Project::~Project()
{
    if (m_rules)
        delete m_rules;
}

/*!
  Returns the path of the project.
*/
QString Project::nodePath() const
{
    return m_sp.nodePath();
}

/*!
  Returns the name of the project.
*/
QString Project::name() const
{
    return m_name;
}

/*!
  Returns a solution file that represents the project.
*/
SolutionFile Project::file() const
{
    return m_file;
}

/*!
  Returns the path of this project as a generated filesystem path.

  This is equivalent to solution()->findFile(nodePath(), Solution::Generated).fsPath().
  */
QString Project::buildDir() const
{
    return m_buildDir;
}

/*!
  \internal
*/
void Project::valueChanged(const QString &name, const QStringList &added,
                           const QStringList &removed)
{
    if (name == "CONFIG")
        configChanged(added, removed);
}

/*!
  Include the file represented by \a fileName. Returns true if the file could be
  parsed, else false.
*/
bool Project::includeFile(const SolutionFile &fileName)
{
    SolutionFile _fileName = fileName.canonicalPath();
    RUN_TRACE
        qWarning() << "Including file" << _fileName;

    if (!_fileName.isValid())
        return false;

    m_includedFiles << _fileName.fsPath();

    PreprocessorToken error;
    QString errorMessage;
    Block *block = QMakeParser::parseFile(_fileName.fsPath(), &error, &errorMessage);
    if (!block) {
        // Silently ignore empty files
        if (error.token == NOTOKEN)
            return true;

        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "ERROR: Error parsing file " << _fileName.fsPath() << " at line "
            << error.line << (errorMessage.isEmpty()?errorMessage:QString(" - %1").arg(errorMessage));
        QBuild::shutdown();
    }

    SolutionFile oldFile = m_file;
    m_file = _fileName;
    Q_ASSERT(block);
    runBlock(block);
    m_file = oldFile;

    return true;
}

/*!
  \internal
*/
bool Project::runExpression(QStringList &out, const QByteArray &data,
                            IInternalVariable *variables)
{
    const char *dataStr = data.constData();
    QList<LexerToken> tokens =
        tokenize(dataStr, "EXPRESSION");
    if (tokens.isEmpty())
        return false;

    QList<PreprocessorToken> ptokens = preprocess(dataStr, tokens);
    if (ptokens.isEmpty())
        return false;

    QMakeParser parser;
    if (!parser.parseExpression(ptokens))
        return false;

    Q_ASSERT(parser.parsedBlock());
    Q_ASSERT(parser.parsedBlock()->type() == Block::Assignment);
    Assignment *assign = static_cast<Assignment *>(parser.parsedBlock());
    QList<IInternalVariable *> vars = m_qmakeValues;
    if (variables)
        vars << variables;
    out = expressionValue(assign->expression(), &vars);
    return true;
}

/*!
  Parse and run \a data using \a filename to report parsing errors.
  Returns true if the data could be parsed, false otherwise.
*/
bool Project::run(const QByteArray &data, const QString &filename)
{
    const char *dataStr = data.constData();
    QList<LexerToken> tokens =
        tokenize(dataStr, filename.toLatin1().constData());
    if (tokens.isEmpty())
        return false;

    QList<PreprocessorToken> ptokens = preprocess(dataStr, tokens);
    if (ptokens.isEmpty())
        return false;

    QMakeParser parser;
    if (!parser.parse(ptokens))
        return false;

    Q_ASSERT(parser.parsedBlock());
    runBlock(parser.parsedBlock());

    return true;
}

/*!
  Parse and run \a data as a scope using \a filename to report parsing errors.
  The return value of the scope is set in \a rv.
  Returns true if everything was parsed correctly, false otherwise.
*/
bool Project::do_if (const QByteArray &data, const QString &filename, bool &rv)
{
    // We need to add a statement to the scope or it will not parse correctly
    QString str = QString::fromLatin1(data);
    str += ":if=true";
    QByteArray statement = str.toLatin1();
    const char *dataStr = statement.constData();

    QList<LexerToken> tokens =
        tokenize(dataStr, filename.toLatin1().constData());
    if (tokens.isEmpty())
        return false;

    QList<PreprocessorToken> ptokens = preprocess(dataStr, tokens);
    if (ptokens.isEmpty())
        return false;

    QMakeParser parser;
    if (!parser.parse(ptokens))
        return false;

    // We end up with a MultiBlock containing a Scope and an Assignment.
    // We don't care about the assignment, only the Scope.
    Block *block = parser.parsedBlock();
    Q_ASSERT(block);
    Q_ASSERT(block->type() == Block::MultiBlock);
    MultiBlock *blocks = static_cast<MultiBlock*>(block);
    block = blocks->blocks().at(0);
    Q_ASSERT(block->type() == Block::Scope);
    rv = testScope(static_cast<Scope*>(block));
    return true;
}

/*!
  \internal
  Returns the list of files included when parsing the project?!?
*/
QStringList Project::includedFiles() const
{
    return m_includedFiles;
}

/*!
  Returns the value of the object specified by \a property.
*/
QStringList Project::value(const QString &property) const
{
    return QMakeObject::property(property)->value();
}

/*!
  Returns true if \a option is contained in the \l CONFIG variable.
*/
bool Project::isActiveConfig(const QString &option) const
{
    return value("CONFIG").contains(option);
}

struct DepInfo {
    QString name;

    QStringList after_me;
    bool done;
};

typedef QMap<QString, DepInfo> DepInfoMap;

/*!
  \internal
*/
bool doFinalize(DepInfoMap &infoMap,
                const QString &name,
                QStringList &order,
                QStack<QString> *finalizeStack = 0)
{
    DepInfoMap::Iterator iter = infoMap.find(name);
    if (iter == infoMap.end())
        return true;

    if (iter->done)
        return true;

    // To prevent circular dependencies we use a processing stack.
    // If the stack is null we're at the bottom of a dependency chain.
    bool die = false;
    QStack<QString> *_fstack = 0;
    if ( !finalizeStack ) {
        _fstack = new QStack<QString>();
        finalizeStack = _fstack;
        finalizeStack->push(name);
    }

    // Find items that go before me and finalize them first
    for (DepInfoMap::Iterator iter2 = infoMap.begin(); iter2 != infoMap.end(); iter2++) {
        // Skip items that have already been done (or are currently being done)
        if (!iter2->done && iter2->after_me.contains(name)) {
            if ( finalizeStack->contains(iter2->name) ) {
                qWarning() << "Circular dependency detected between"
                           << name << "and" << iter2->name;
                die = true;
                break;
            }
            finalizeStack->push(iter2->name);
            bool ok = doFinalize(infoMap, iter2->name, order, finalizeStack);
            finalizeStack->pop();
            if ( !ok ) {
                die = true;
                break;
            }
        }
    }

    if ( _fstack )
        delete _fstack;
    if ( die )
        return false;

    iter->done = true;
    order << name;

    return true;
}

/*!
  \internal
  Returns the solution that the project is in.
*/
Solution *Project::solution() const
{
    return m_solution;
}

/*!
  \internal
  Finalization also marks the object read only.
*/
void Project::finalize()
{
    RUN_TRACE
        qWarning() << "Project finalizing";

    if (m_finalized)
        return;

    m_finalized = true;

    QMap<QString, DepInfo> infoMap;

    QMakeObject *obj = isProperty("QMAKE.FINALIZE")?QMakeObject::property("QMAKE.FINALIZE"):0;
    QStringList modules = obj?obj->properties():QStringList();

    for (int ii = 0; ii < modules.count(); ++ii) {
        QMakeObject *prop = obj->property(modules.at(ii));
        QStringList after_me = prop->property("RUN_AFTER_ME")->value();
        // A hack to ensure runlast.js actually runs last
        if ( prop->name() != "runlast" )
            after_me << "runlast";

        DepInfo info = { modules.at(ii), after_me, false };
        infoMap.insert(modules.at(ii), info);
    }

    for (int ii = 0; ii < modules.count(); ++ii) {
        QMakeObject *prop = obj->property(modules.at(ii));
        QStringList before_me = prop->property("RUN_BEFORE_ME")->value();

        for (int jj = 0; jj < before_me.count(); ++jj) {
            QMap<QString, DepInfo>::Iterator iter = infoMap.find(before_me.at(jj));
            if (iter != infoMap.end())
                iter->after_me.append(modules.at(ii));
        }
    }

    QStringList depOrder;
    for (DepInfoMap::Iterator iter = infoMap.begin();
        iter != infoMap.end(); ++iter) {
        if (!iter->done) {
            if (!doFinalize(infoMap, iter->name, depOrder)) {
                qOutput().nospace()
                    << "Project " << messageDisplayName()
                    << "ERROR: Unable to determine finalization order for module " << iter->name;
                QBuild::shutdown();
            }
        }
    }

    if (options.debug & Options::Finalize || options.debug & Options::RunTrace) {
        qWarning() << "Finalize order:";
        for (int ii = depOrder.count() - 1; ii >= 0; --ii)
            qWarning() << "    " << depOrder.at(ii);
    }

    foreach ( const QString &module, depOrder ) {
        if (options.debug & Options::Finalize || options.debug & Options::RunTrace)
            qWarning() << "Finalizing" << module;

        QStringList functions = obj->property(module)->property("CALL")->value();
        for (int ii = 0; ii < functions.count(); ++ii) {
            QStringList rv;

            if (options.debug & Options::Finalize)
                qWarning() << "    " << functions.at(ii);

            if (!FunctionProvider::evalFunction(this, rv, functions.at(ii), QStringLists())) {
                qOutput().nospace()
                    << "Project " << messageDisplayName()
                    << "ERROR: Unable to run finalizer " << functions.at(ii);
                QBuild::shutdown();
            }
            if (reseting())
                goto out;
        }
    }

out:;

    if (reseting()) {
        // Note: This cannot loop indefinately, as there are a finite number of
        // reset causes (and no reset cause can occur more than once)
        do {
            m_reseting = false;
            init();
            if (!reseting())
                runProject();
        } while (reseting());
        finalize();
    } else {
        setReadOnly();
    }

    if (m_hasWarnings) {
        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "ERROR: Terminated due to warnings";
        QBuild::shutdown();
    }
}

/*!
  \internal
*/
SolutionProject Project::solutionProject() const
{
    return m_sp;
}

/*!
  \internal
*/
bool Project::reseting() const
{
    return m_reseting;
}

/*!
  Returns a string with the reason that the project was disabled.
*/
QString Project::disabledReason() const
{
    return m_resetReason.isProperty("disabled")?m_resetReason.property("disabled")->value().join(" "):QString();
}

QStringList Project::mkspecSearch;

/*!
  Returns the list of paths in which QBuild looks for mkspecs.
*/
QStringList Project::mkspecPaths()
{
    primeMkspecPath();
    return mkspecSearch;
}

/*!
  \internal
*/
void Project::primeMkspecPath()
{
    if (mkspecSearch.isEmpty()) {
        mkspecSearch << "!" + QLibraryInfo::location(QLibraryInfo::DataPath) + "/mkspecs";
        mkspecSearch << "!" + QCoreApplication::applicationDirPath() + "/mkspecs";
    }
}

/*!
  Add \a path to the list of paths in which QBuild looks for mkspecs.
*/
void Project::addMkspecPath(const QString &path)
{
    primeMkspecPath();
    QString addPath = path;
    if (!addPath.endsWith("/"))
        addPath.append("/");
    mkspecSearch << addPath;
}

/*!
  Disable the project and report the \a reason.
*/
void Project::setDisabled(const QString &reason)
{
    if (ProjectMode != m_loadmode) {
        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "ERROR: Projects opened in file mode cannot be disabled";
        QBuild::shutdown();
    }

    if (m_disabledState == Disabled) {
        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "WARNING: Cannot disable project from within disable scripts";
    } else {
        m_disabledState = Disabled;
        reset("disabled", reason);
    }
}

/*!
  Reset the project because of \a reason. Note that a project can only be reset once per reason.
  The \a value is stored and can be retrieved from the property \a reason in the
  object returned by Project::resetReason().
*/
void Project::reset(const QString &reason, const QString &value)
{
    if ( m_resetReason.value().contains(reason) ) {
        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "ERROR: Projects cannot be reset due to the same reason more than once."
            << " The reason passed is " << reason;
        QBuild::shutdown();
    }
    m_reseting = true;
    m_resetReason.addValue(QStringList() << reason);
    QMakeObject *prop = m_resetReason.property(reason);
    Q_ASSERT(prop);
    prop->setValue(QStringList() << value);
}

/*!
  Returns the reasons that have caused the project to be reset.
  \sa reset()
*/
QMakeObject *Project::resetReason()
{
    return &m_resetReason;
}

/*!
  \internal
*/
void Project::dumpRules(const QString &ruleName)
{
    qWarning() << "Project (" << name() << ")";

    if (!m_rules)
        return;

    QList<Rule *> rules;
    if (m_rules)
        rules = m_rules->rules();

    for (int ii = 0; ii < rules.count(); ++ii) {
        Rule *rule = rules.at(ii);
        if (!ruleName.isEmpty() && rule->name != ruleName)
            continue;

        qWarning() << "    " << rule->name;
        if (!rule->help.isEmpty())
            qWarning() << "      + Help:" << rule->help;

        if (!rule->outputFiles().isEmpty())
            qWarning() << "      + Output:" << rule->outputFiles();

        if (!rule->inputFiles.isEmpty())
            qWarning() << "      + Input:" << rule->inputFiles;

        if (!rule->tests.isEmpty())
            qWarning() << "      + Tests:" << rule->tests;

        if (!rule->prerequisiteActions.isEmpty())
            qWarning() << "      + Prereq:" << rule->prerequisiteActions;

        if (!rule->commands.isEmpty())
            qWarning() << "      + Commands:" << rule->commands;

        if (!rule->other.isEmpty())
            qWarning() << "      + Other:" << rule->other;

        if (rule->flags) {
            QString flag;
            if (rule->flags & Rule::SerialRule)
                flag += "Serial ";
            qOutput() << "      + Flags:" << flag;
        }

        if (!rule->trace.isEmpty()) {
            qWarning() << "      + Trace:";
            foreach(TraceContext trace, rule->trace) {
                QString traceLine = trace.fileName;
                traceLine += ":";
                traceLine += QString::number(trace.lineNumber);
                qOutput() << "           " << traceLine;
            }
        }
    }
}

/*!
  \internal
*/
void Project::dump()
{
    qWarning() << "Project (" << name() << ") =" << QMakeObject::value();
    QMakeObject::dump(1);
}

/*!
  \internal
*/
void Project::runBlock(Block *block)
{
    //block->dump();
    switch(block->type()) {
        case Block::MultiBlock:
            runBlock(static_cast<MultiBlock *>(block));
            break;
        case Block::Scope:
            runBlock(static_cast<Scope *>(block));
            break;
        case Block::Assignment:
            runBlock(static_cast<Assignment *>(block));
            break;
        case Block::Function:
            runBlock(static_cast<Function *>(block));
            break;
        case Block::Script:
            runBlock(static_cast<Script *>(block));
            break;
    }
}

/*!
  \internal
*/
void Project::runBlock(MultiBlock *block)
{
    Blocks blocks = block->blocks();
    QByteArray oldUsing = m_usingBlock;
    if (!m_usingBlock.isEmpty()) {
        m_usingBlock.append(".");
        m_usingBlock.append(block->usingName());
    } else {
        m_usingBlock = block->usingName();
    }
    for (int ii = 0; !reseting() && ii < blocks.count(); ++ii)
        runBlock(blocks.at(ii));
    m_usingBlock = oldUsing;
}

/*!
  \internal
*/
void Project::runBlock(Scope *block)
{
    for_loop_t for_loop;
    bool result = testScope(block, &for_loop);

    if ( for_loop.loop ) {
        // Since for loops require low-level support they're implemented here
        QList<QStringList> arguments;
        QList<Expression> expressions = for_loop.func.arguments();
        for (int ii = 0; ii < expressions.count(); ++ii) {
            arguments << expressionValue(expressions.at(ii), 0);
            if (reseting())
                return;
        }
        Q_ASSERT(arguments.count() == 2);
        QString it = arguments.first().first();
        QMakeObject *obj = QMakeObject::property(it);
        QStringList backup = obj->value();
        QString var = arguments.at(1).first();
        QMakeObject *vobj = QMakeObject::property(var);
        QStringList vals = vobj->value();
        foreach (const QString &val, vals) {
            obj->setValue(QStringList() << val);
            runBlock(static_cast<MultiBlock *>(block));
        }
        obj->setValue(backup);
    } else {
        if (reseting()) {
            return;
        } else if (result) {
            runBlock(static_cast<MultiBlock *>(block));
        } else if (block->elseBlock()) {
            runBlock(block->elseBlock());
        }
    }
}

/*!
  \internal
*/
bool Project::testScope(Scope *block, for_loop_t *for_loop )
{
    QList<QPair<EvaluatableValues, bool> > conditions = block->conditions();

    bool result = (block->conditionType() == Scope::And)?true:false;
    bool is_for_loop = false;

    for (int ii = 0; !reseting() && ii < conditions.count(); ++ii) {

        if ( is_for_loop ) {
            qOutput().nospace()
                << "Project " << messageDisplayName()
                << "ERROR: for() must be followed by {}";
            QBuild::shutdown();
        }

        EvaluatableValues values = conditions.at(ii).first;
        bool condition = false;

        if (values.count() == 1 &&
           values.first().type() == EvaluatableValue::TFunction) {

            if ( for_loop && values.first().name() == "for" ) {
                is_for_loop = true;
                for_loop->func = values.first();
            }
            condition = functionResultToBoolean(runFunction(values.first()));

        } else {

            // Need to check config
            QString configVal = expressionValue(values).join(" ");
            condition = QMakeObject::property("CONFIG")->value().contains(configVal);

        }

        if (conditions.at(ii).second)
            condition = !condition;

        if (block->conditionType() == Scope::And) {
            if (!condition) {
                result = false;
                break;
            }
        } else {
            if (condition) {
                result = true;
                break;
            }
        }
    }

    if ( for_loop )
        for_loop->loop = is_for_loop;
    return result;
}

/*!
  \internal
*/
void Project::runBlock(Assignment *block)
{
    QStringList rvalue = expressionValue(block->expression());

    if (reseting())
        return;

    QString lvalue = block->name();
    if (!m_usingBlock.isEmpty())
        lvalue.prepend(m_usingBlock + ".");

    TraceContext ctxt = traceContext(block);
    switch(block->op()) {
        case Assignment::Assign:
            QMakeObject::property(lvalue)->setValue(rvalue, &ctxt);
            break;
        case Assignment::Add:
            QMakeObject::property(lvalue)->addValue(rvalue, &ctxt);
            break;
        case Assignment::Remove:
            QMakeObject::property(lvalue)->subtractValue(rvalue, &ctxt);
            break;
        case Assignment::SetAdd:
            QMakeObject::property(lvalue)->uniteValue(rvalue, &ctxt);
            break;
        case Assignment::Regexp:
            regexpValue(lvalue, rvalue.join(" "), &ctxt);
            break;
    }
}

/*!
  \internal
 */
void Project::runBlock(Script *block)
{
    QtScriptFunctions::instance()->runScript(this, block->script(),
                                             traceContext(block));
}

/*!
  \internal
*/
void Project::regexpValue(const QString &property, const QString &regexp,
                          TraceContext *ctxt)
{
    if (!regexp.startsWith('s') || regexp.length() < 2) {
        qWarning() << "Invalid regexp" << regexp;
        return;
    }

    QChar sep = regexp[1];
    QStringList parts = regexp.split(sep);
    if (parts.count() != 3 && parts.count() != 4) {
        qWarning() << "Invalid regexp" << regexp;
        return;
    }

    bool global = false;
    bool case_sense = true;
    bool quote = false;
    if (parts.count() == 4) {
        global = parts[3].indexOf('g') != -1;
        case_sense = parts[3].indexOf('i') == -1;
        quote = parts[3].indexOf('q') != -1;
    }

    const QString &to = parts[2];
    QString from = parts[1];
    if (quote)
        from = QRegExp::escape(from);

    QRegExp reg(from, case_sense?Qt::CaseSensitive:Qt::CaseInsensitive);

    QMakeObject *object = this->QMakeObject::property(property);
    QStringList value = object->value();
    bool matched = false;
    for (QStringList::Iterator iter = value.begin(); iter != value.end();) {
        if (iter->contains(reg)) {
            matched = true;
            iter->replace(reg, to);
            if (iter->isEmpty())
                iter = value.erase(iter);
            else
                ++iter;
            if (!global)
                break;
        } else {
            ++iter;
        }
    }
    if (matched)
        object->setValue(value, ctxt);
}

/*!
  \internal
*/
TraceContext Project::traceContext(const ParserItem *item) const
{
    TraceContext rv = { m_file.fsPath(), item->lineNumber() };
    return rv;
}

/*!
  \internal
*/
void Project::configChanged(const QStringList &added,
                            const QStringList &)
{
    if (added.isEmpty())
        return;
    RUN_TRACE
        qWarning() << "Config added" << added;

    for (int ii = 0; !reseting() && ii < added.count(); ++ii) {
        if (FunctionProvider::evalLoad(this, added.at(ii))) {
            RUN_TRACE
                qWarning() << "Load of" << added.at(ii) << "SUCCEEDED";
        } else {
            RUN_TRACE
                qWarning() << "Load of" << added.at(ii) << "FAILED";
        }
    }
}

/*!
  \internal
*/
void Project::runBlock(Function *block)
{
    runFunction(block->function());
}

/*!
  \internal
*/
bool Project::functionResultToBoolean(const QStringList &rv) const
{
    // This logic is described in src/build/qdocscript.qdoc!
    bool ret = !(rv.isEmpty() ||
                 (rv.count() == 1 && rv.first() == "false") ||
                 (rv.count() == 1 && rv.first().isEmpty()));
    return ret;
}

/*!
  \internal
*/
QStringList Project::runFunction(const EvaluatableValue &function,
                                 QList<IInternalVariable *> *qvars)
{
    QList<QStringList> arguments;
    QList<Expression> expressions = function.arguments();
    for (int ii = 0; ii < expressions.count(); ++ii) {
        arguments << expressionValue(expressions.at(ii), qvars);
        if (reseting())
            return QStringList();
    }

    QStringList rv;
    if (!FunctionProvider::evalFunction(this, rv, function.name(), arguments)) {
        TraceContext ctxt = traceContext(&function);
        qOutput().nospace()
            << "Project " << messageDisplayName()
            << "ERROR: Unable to resolve function " << function.name()
            << "(" << ctxt.fileName << "@" << ctxt.lineNumber << ")";
        QBuild::shutdown();
    }

    return rv;
}

/*!
  \internal
*/
QStringList Project::expressionValue(const Expression &expr,
                                     QList<IInternalVariable *> *qvals)
{
    QStringList rv;

    QList<EvaluatableValues> values = expr.evaluatableValues();

    for (int ii = 0; !reseting() && ii < values.count(); ++ii)  {
        QStringList val = expressionValue(values.at(ii), qvals);
        for (int ii = 0; ii < val.count(); ++ii)
            if (!val.at(ii).isEmpty())
                rv << val.at(ii);
    }

    return rv;
}

/*!
  \internal
*/
QStringList Project::expressionValue(const EvaluatableValues &values,
                                     QList<IInternalVariable *> *qvals)
{
    QString rv;

    for (int ii = 0; !reseting() && ii < values.count(); ++ii) {
        QStringList val = expressionValue(values.at(ii), qvals);
        if (val.count() > 1 && values.count() == 1)
            return val;
        else if (val.count())
            rv.append(val.join(" "));
    }

    return QStringList() << rv;
}

/*!
  \internal
*/
QStringList Project::expressionValue(const EvaluatableValue &value,
                                     QList<IInternalVariable *> *qvals)
{
    QStringList rv;

    switch(value.type()) {
        case EvaluatableValue::Characters:
            rv << value.name();
            break;
        case EvaluatableValue::Environment:
            rv << ::getenv(value.name().constData());
            break;

        case EvaluatableValue::Variable:
            {
                QMakeObject *obj = QMakeObject::property(QString(value.name()));
                if (obj)
                    rv << obj->value();
            }
            break;

        case EvaluatableValue::QtVariable:
            rv << qmakeValue(value.name(), qvals);
            break;

        case EvaluatableValue::Function:
            rv << runFunction(value, qvals);
            break;

        case EvaluatableValue::TFunction:
            runFunction(value, qvals);
            break;
    }

    return rv;
}

/*!
  \internal
*/
QStringList Project::qmakeValue(const QString &value)
{
    return qmakeValue(value, 0);
}

/*!
  \internal
*/
QStringList Project::qmakeValue(const QString &value,
                                QList<IInternalVariable *> *values)
{
    if (!values)
        values = &m_qmakeValues;
    QStringList rv;
    for (int ii = 0; ii < values->count(); ++ii) {
        IInternalVariable *iv = values->at(ii);
        if (iv->value(value, rv))
            return rv;
    }
    return rv;
}

/*!
  \internal
*/
bool Project::virtualProject() const
{
    return m_isVirtual;
}

/*!
  \internal
*/
void Project::setVirtualProject(bool virt)
{
    m_isVirtual = virt;
}

/*!
  \internal
*/
Rules *Project::projectRules()
{
    if (!m_rules)
        m_rules = new Rules(name(), this);
    return m_rules;
}

/*!
  Returns an object specified by the absolute path \a path.

  path is in the form \c{[<project path>/]<object path>} where
  \c{<project path>} is \c{/} deliminated and \c{<object path>} is \c{.}
  deliminated.  Absence of the project path means the current project.

  Returns 0 if no such object.
  The \a ctxt object is for debugging.
*/
QMakeObject *QMakeObject::object(const QString &path, Project *ctxt)
{
    QString project = ctxt->name();
    QString object = path;

    if (path.contains('/')) {
        QStringList parts = path.split("/", QString::KeepEmptyParts);

        if (!parts.isEmpty()) {

            if (parts.first().endsWith(":")) {
                if (parts.count() == 1)
                    return 0;

                object = parts.last();
                parts.removeLast();
                project = parts.join("/");
                project.append("/");

            } else {
                QStringList currentParts =
                    path.startsWith('/')?QStringList():ctxt->name().split("/");

                for (int ii = 0; ii < parts.count() - 1; ++ii) {
                    if (parts.at(ii).isEmpty() || parts.at(ii) == ".") {
                        continue;
                    } else if (parts.at(ii) == "..") {
                        if (!currentParts.isEmpty())
                            currentParts.removeLast();
                    } else {
                        currentParts.append(parts.at(ii));
                    }
                }

                QStringList objectParts;

                objectParts = parts.last().split(".");

                project = QString();
                for (int ii = 0; ii < currentParts.count(); ++ii) {
                    project.append('/');
                    project.append(currentParts.at(ii));
                }
                project.append('/');

                object = objectParts.join(".");
            }
        }

    }

    if (project != ctxt->name()) {
        ctxt = SolutionProject::fromNode(project, ctxt->solution()).project();
        if (!ctxt)
            return 0;
    }

    if (object.isEmpty()) {
        return ctxt;
    } else if (ctxt->isProperty(object)) {
        return ctxt->QMakeObject::property(object);
    } else {
        return 0;
    }
}

void QMakeObject::newType(const QString &type)
{
    TypeFunctions::ConstIterator iter = m_newTypeFunctions.find(type);
    if (iter == m_newTypeFunctions.end())
        return;

    QStringList args;
    args << absoluteName();
    Project *proj = static_cast<Project *>(root());
    QStringList rv;

    foreach(const QString &func, *iter)
        FunctionProvider::evalFunction(proj, rv, func, QStringLists() << args);
}

void QMakeObject::delType(const QString &type)
{
    TypeFunctions::ConstIterator iter = m_delTypeFunctions.find(type);
    if (iter == m_delTypeFunctions.end())
        return;

    QStringList args;
    args << absoluteName();
    Project *proj = static_cast<Project *>(root());
    QStringList rv;

    foreach(const QString &func, *iter)
        FunctionProvider::evalFunction(proj, rv, func, QStringLists() << args);
}

/*!
  \internal
*/
QString Project::messageDisplayName() const
{
    /*
    if (QBuild::rootProject().node() == this->solutionProject().node()) {
        return QString();
    } else {
    */
        QString str = name();
        if (str.endsWith("/"))
            str = str.left(str.length() - 1);
        return "(" + str + ") ";
    //}
}

/*!
  \internal
*/
QDebug Project::warning()
{
    m_hasWarnings = true;
    return outputMessage("WARNING");
}

/*!
  \internal
*/
QDebug Project::info()
{
    return outputMessage("INFORMATION");
}

/*!
  \internal
*/
QDebug Project::message()
{
    return outputMessage("MESSAGE");
}

/*!
  \internal
*/
QDebug Project::error()
{
    m_hasWarnings = true;
    return outputMessage("ERROR");
}

/*!
  \internal
*/
QDebug Project::outputMessage(const QString &level)
{
    QDebug rv(QtWarningMsg);

    QString displayName = messageDisplayName();
    rv << "Project";
    if (!displayName.isEmpty())
        rv << displayName.trimmed().toAscii().constData();
    rv << QString("%1:").arg(level).toLatin1().constData();
    return rv;
}

