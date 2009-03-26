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

#include "lexer.h"
#include "qfastdir.h"
#include "startup.h"
#include <QApplication>
#include "ruleengine.h"
#include "qtscriptfunctions.h"
#include "parser.h"
#include "preprocessor.h"
#include <QFile>
#include "solution.h"
#include "qbuild.h"
#include "project.h"
#include "object.h"
#include <QSettings>
#include <QString>
#include "qoutput.h"
#include <QDir>
#include "options.h"
#include "ruleengine.h"
#include "gui.h"
#include <unistd.h>
#include <stdlib.h>

Options options;

static void createProject()
{
    QByteArray pwd = qgetenv("QBUILD_PWD");
    if ( !pwd.isEmpty() )
        ::chdir(pwd.constData());
    QDir dir = QDir::current();
    QStringList headers = dir.entryList(QStringList() << "*.h");
    QStringList sources = dir.entryList(QStringList() << "*.cpp");
    QStringList forms = dir.entryList(QStringList() << "*.ui");
    QStringList resources = dir.entryList(QStringList() << "*.qrc");

    QString target = dir.dirName();

    // try to determine if this is a Qtopia or just a Qt project
    int qtopia = 0;
    QStringList args;
    args << "-i" << "qtopia";
    if ( headers.count() ) args << headers;
    if ( sources.count() ) args << sources;
    if ( forms.count() ) args << forms;
    if ( resources.count() ) args << resources;
    if ( args.count() > 2 ) {
        QProcess proc;
        proc.start("grep", args);
        proc.waitForStarted(-1);
        proc.waitForFinished(-1);
        if ( proc.exitCode() == 0 )
            qtopia = 1;
    }

    // Check for a .desktop file
    QString desktop;
    QStringList _desktop = dir.entryList(QStringList() << "*.desktop");
    if (_desktop.count() == 1)
        desktop = _desktop.at(0);

    // Check for pics and help
    bool pics = dir.exists("pics");
    bool help = dir.exists("help");

    // Output qbuild.pro
    QFile file("qbuild.pro");
    if (!file.open(QIODevice::WriteOnly))
        qFatal("Can't write qbuild.pro");
    QTextStream stream(&file);
    stream << "TEMPLATE=app\n";
    stream << "TARGET=" << target << "\n";
    stream << "\n";
    if ( qtopia ) {
        stream << "CONFIG+=qtopia\n";
    } else {
        stream << "CONFIG+=qt embedded\n";
    }
    stream << "\n";
    stream << "# I18n info\n";
    stream << "STRING_LANGUAGE=en_US\n";
    stream << "LANGUAGES=en_US\n";
    stream << "\n";
    stream << "# Package info\n";
    stream << "pkg [\n";
    stream << "    name=" << target << "\n";
    stream << "    desc=\"No Description\"\n";
    stream << "    license=Unknown\n";
    stream << "    version=1.0\n";
    stream << "    maintainer=\"Unknown <unknown@example.com>\"\n";
    stream << "]\n";
    stream << "\n";
    stream << "# Input files\n";
#define DO_LIST(list, var)\
    if ( list.count() ) {\
        stream << var << "=";\
        foreach (const QString &s, list)\
            stream << "\\\n    " << s;\
        stream << "\n\n";\
    }
    DO_LIST(resources, "RESOURCES");
    DO_LIST(forms, "FORMS");
    DO_LIST(headers, "HEADERS");
    DO_LIST(sources, "SOURCES");
#undef DO_LIST
    stream << "# Install rules\n";
    stream << "target [\n";
    stream << "    hint=sxe\n";
    stream << "    domain=untrusted\n";
    stream << "]\n";
    stream << "\n";
    if ( !desktop.isEmpty() ) {
        stream << "desktop [\n";
        stream << "    hint=desktop\n";
        stream << "    files=" << desktop << "\n";
        stream << "    path=/apps/Applications\n";
        stream << "]\n";
        stream << "\n";
    }
    if ( pics ) {
        stream << "pics [\n";
        stream << "    hint=pics\n";
        stream << "    files=pics/*\n";
        stream << "    path=/pics/" << target << "\n";
        stream << "]\n";
        stream << "\n";
    }
    if ( help ) {
        stream << "help [\n";
        stream << "    hint=help\n";
        stream << "    source=help\n";
        stream << "    files=*.html\n";
        stream << "]\n";
        stream << "\n";
    }
    qWarning() << "Created qbuild.pro";
}

void dumpHierarchy(const SolutionProject &sol)
{
    SolutionFile file = sol.projectFile();

    if (file.isValid()) {
        qWarning() << file.fsPath();

        if (options.showTokens || options.showParseTree) {
            QFile infile(file.fsPath());
            infile.open(QFile::ReadOnly);
            QByteArray data = infile.readAll();
            const char *dataStr = data.constData();
            QList<LexerToken> tokens = tokenize(dataStr, file.fsPath().toAscii().constData());
            QList<PreprocessorToken> ptokens = preprocess(dataStr, tokens);
            if (options.showTokens)
                dumpTokens(ptokens);

            if (options.showParseTree) {
                QMakeParser parser;
                parser.parse(ptokens);
                parser.dump();
            }
        }

        Project *project = static_cast<Project *>(sol.project());
        if (options.showParse)
            project->dump();
        project->finalize();
        if (options.showFinalize)
            project->dump();
        if (options.showRules)
            project->dumpRules(options.rule);
    }

    /*
    if (options.recur) {
        QStringList subProjects = sol.subProjects();
        for (int ii = 0; ii < subProjects.count(); ++ii) {
            SolutionProject sub = sol.subProject(subProjects.at(ii));
            dumpHierarchy(sub);
        }
    }
    */
}

#define CONSOLE_WIDTH 80
void showHelpline(int ind, const char * _line)
{
    int indentWidth = ind*4;
    if (indentWidth)
        indentWidth--;
    QByteArray indent(indentWidth, ' ');

    QByteArray line(_line);

    if (line.isEmpty())
        qWarning();

    while (!line.isEmpty()) {

        QByteArray partLine;
        if (line.length() > (CONSOLE_WIDTH - 1 - indent.length())) {
            partLine = line.left(CONSOLE_WIDTH - 1 - indent.length());
            int idx = partLine.lastIndexOf(' ');
            if (idx != -1) {
                partLine = partLine.left(idx);
                line = line.mid(partLine.length() + 1);
            } else {
                line = line.mid(partLine.length());
            }

        } else {
            partLine = line;
            line.clear();
        }

        if (indentWidth)
            qOutput() << indent << partLine;
        else
            qOutput() << partLine;
    }
}

/*!
  \internal
*/
void StartupScript::showHelp(bool includeDebugging)
{
    qOutput() << "Usage: " << m_appName << "[<Options>] [<Action>]";
    showHelpline(0, "Options:");
    showHelpline(1, "-help -advanced-help");
    showHelpline(2, "Show this help. If the -advanced-help option is used, an expanded help that includes options for debugging QBuild is included.");
    showHelpline(1, "-project");
    showHelpline(2, "Create a qbuild.pro from the files in the current directory.");
    showHelpline(1, "-actions");
    showHelpline(2, "Display a list of available actions and a description of their purpose.");
    showHelpline(1, "-node <nodename>");
    showHelpline(2, "Specify a node to build. This can be an absolute path or a relative path. As a special case, qbuild -node . is equivalent to qbuild -node <dir> no matter what value was passed to -C (eg. qbuild -C / -node . builds the current node with full visibility).");
    showHelpline(1, "-C <dir>");
    showHelpline(2, "Change into <dir> before running QBuild. <dir> can be an absolute solution path or an absolute or relative real directory. Note that relative -node values are searched from <dir>.");
    showHelpline(1, "-gui");
    showHelpline(2, "Display a GUI window containing QBuilds progress.");
    showHelpline(1, "-progress");
    showHelpline(2, "Same as -gui, but displays the progress in the console.");
    showHelpline(1, "-f <file>");
    showHelpline(2, "Use <file> as the project file. This option is similar to the -f option available in regular make.");
    showHelpline(1, "-j <maximum jobs>");
    showHelpline(2, "Specify the maximum number of jobs (commands) to run simultaneously. This option is similar to the -j option available in regular make. By default the maximum number of jobs is equivalent to the number of available physical and virtual processors. For example, in a dual core machine, the default is 2. The maximum permitted value is 50 and the minimum is 1.");
    showHelpline(1, "-freshen <file>");
    showHelpline(2, "Freshen the specified <file>. This feature is currently immature and should be used with care. The caveat is that the filename provided must be identical to the filename used internally by the rule engine. This may be fully qualified, or it may not be.");
    showHelpline(1, "-n");
    showHelpline(2, "Print out what would happen but do not actually do anything.");
    showHelpline(1, "-force");
    showHelpline(2, "Force the specified rule to be run, even if its dependencies would not otherwise cause this.");
    showHelpline(1, "-throttle <category>:<limit>");
    showHelpline(2, "QBuild will not run more than <limit> rules for <category> at the same time. The special limit value of T equals the number of cores your machine has.");

    foreach(StartupScript::OptionDesc option, options())
        foreach(QString line, option.help)
            qOutput() << "   " << line;

    if (includeDebugging) {
        qWarning();
        showHelpline(0, "Debugging options:");
        showHelpline(1, "-solutions");
        showHelpline(2, "Display the complete list of available solutions and their configured file mappings");
        showHelpline(1, "-list-solution <solution name>");
        showHelpline(2, "Display the file system contents of the nominated solution");
        showHelpline(1, "-extensions");
        showHelpline(2, "Display the list of paths searched for QBuild extensions");
        showHelpline(1, "-mkspecs");
        showHelpline(2, "Display the list of paths searched for QBuild mkspecs");
        showHelpline(1, "-tokenize");
        showHelpline(2, "Show the tokenized output of the nominated project");
        showHelpline(1, "-parse-tree");
        showHelpline(2, "Show the parse tree output of the nominated project");
        showHelpline(1, "-parse");
        showHelpline(2, "Show the pre-finalization parse output of the nominated project");
        showHelpline(1, "-finalize");
        showHelpline(2, "Show the finalized parse output of the nominated project");
        showHelpline(1, "-rules");
        showHelpline(2, "Show the complete rule list of the nomitated project");
        showHelpline(1, "-no-run");
        showHelpline(2, "Finalize the project, but do not run the default (or user specified) rule.");
        showHelpline(1, "-perf-statistics");
        showHelpline(2, "Display performance statistics at the end of the QBuild run.");
        showHelpline(1, "-d");
        showHelpline(2, "Enable rule engine tracing. Rule engine tracing causes the rule engine to print debugging information in addition to normal processing. This option is similar to the -d option available in regular make.");
        showHelpline(1, "-df <file>");
        showHelpline(2, "Enable rule engine tracing, outputting to a file.");
        showHelpline(1, "-run-trace");
        showHelpline(2, "Print what QBuild is doing, as it does it.");
        showHelpline(1, "-trace");
        showHelpline(2, "Enable project tracing. Project tracing allows you to rapidly track down where variables are being changed or rules are created by tracking the originating file name and line number of each action. Tracing applies to the tokenizer, parse tree generator, parser, finalizer and rule-generator. Thus the -trace option is usually accompanied by a switch to enable the output from one of these modules.");
        showHelpline(1, "-v");
        showHelpline(2, "Enable verbose output. This overrides the #(e) flag in the rule engine, causing all rules to be printed before being executed.");
        showHelpline(1, "-w");
        showHelpline(2, "Print a message when a project is opened and again when rules are run from the project.");
        showHelpline(1, "-s");
        showHelpline(2, "Do not print a message when a project is opened or when rules are run.");
        showHelpline(1, "-show-stack-on-fatal");
        showHelpline(2, "Print out the internal QBuild call stack on fatal messages. This is generally only useful for debugging QBuild itself.");
        showHelpline(1, "-script");
        showHelpline(2, "Ignore options that are not useful in a scripting environment (eg. -progress)");
    }
}

void dump(PathIterator iter, int indent = 0)
{
    QByteArray ba(indent * 4, ' ');
    foreach(const QString &path, iter.paths()) {
        qOutput().nospace() << ba << path << "/";
        dump(iter.advance(path), indent + 1);
    }
    foreach(const SolutionFile &file, iter.files()) {
        QString type;
        switch((int)file.type()) {
            case SolutionFile::Absolute:
                type = "abs";
                break;
            case SolutionFile::Build:
                type = "build";
                break;
            case SolutionFile::Project:
                type = "proj";
                break;
            case SolutionFile::Project | SolutionFile::Build:
                type = "proj|build";
                break;
        };
        qOutput().nospace() << ba << file.name() << " (" << type << ")";
    }
}

static void aBetterMessageHandler(QtMsgType type, const char *msg)
{
#ifdef Q_OS_WIN32
    // Output for DebugView
    QString fstr(msg);
    OutputDebugString((fstr + "\n").utf16());
#endif
    // Output for console
    if ( ConsoleGui::instance() ) {
        LOCK(Gui);
        ConsoleGui::instance()->lock();
    }
    fprintf(stdout, "%s\n", msg);
    fflush(stdout);

    if ( type == QtFatalMsg )
        abort();

    if ( ConsoleGui::instance() )
        ConsoleGui::instance()->release();
}

static QList<QByteArray> splitStringRoughlyLikeShellArgs(const QByteArray &str)
{
    bool escaped = false;
    enum { NoQuote, SingleQuote, DoubleQuote } quoteState = NoQuote;

    QList<QByteArray> rv;
    QByteArray current;
    for (int ii = 0; ii < str.count(); ++ii) {
        switch(str.at(ii)) {
            case '\\':
                if (escaped) {
                    current.append('\\');
                } else {
                    escaped = true;
                }
                break;
            case '\'':
                if (escaped || DoubleQuote == quoteState) {
                    current.append('\'');
                    escaped = false;
                } else if (NoQuote == quoteState) {
                    quoteState = SingleQuote;
                } else if (SingleQuote == quoteState) {
                    quoteState = NoQuote;
                }
                break;
            case '"':
                if (escaped || SingleQuote == quoteState) {
                    current.append('"');
                    escaped = false;
                } else if (NoQuote == quoteState) {
                    quoteState = DoubleQuote;
                } else if (DoubleQuote == quoteState) {
                    quoteState = NoQuote;
                }
                break;
            case ' ':
                if (escaped) {
                    if (NoQuote != quoteState) {
                        current.append("\\ ");
                        escaped = false;
                    } else {
                        current.append("\\ ");
                        escaped = false;
                    }
                } else if (NoQuote != quoteState) {
                    current.append(" ");
                } else {
                    if (!current.isEmpty())
                        rv.append(current);
                    current = QByteArray();
                }
                break;
            default:
                if (escaped) {
                    current.append("\\");
                    escaped = false;
                }
                current.append(str.at(ii));
                break;
        }
    }

    if (!current.isEmpty())
        rv.append(current);

    return rv;
}

static QList<QByteArray> collectAllArguments(int argc, char ** argv)
{
    QList<QByteArray> rv;
    rv << argv[0];
    QFile file(QDir::homePath() + "/.qbuildrc");
    if (file.open(QFile::ReadOnly)) {
        while (!file.atEnd()) {
            QByteArray ba = file.readLine().trimmed();
            if (ba.startsWith("#"))
                continue;
            rv << splitStringRoughlyLikeShellArgs(ba);
        }
    }

    const char *env = ::getenv("QBUILDRC");
    if (env) {
        QByteArray ba(env);
        rv << splitStringRoughlyLikeShellArgs(ba);
    }
    for (int ii = 1; ii < argc; ++ii)
        rv << argv[ii];
    return rv;
}

QStringList openAllProjects(const SolutionProject &p)
{
    QStringList rv;
    rv << p.node();
    foreach(QString proj, p.subProjects()) {
        rv << openAllProjects(p.subProject(proj));
    }
    return rv;
}

static QList<QByteArray> setEnvironmentVariables(const QList<QByteArray> &args)
{
    QList<QByteArray> ret;

    QRegExp re("([A-Z0-9_]+)=");
    foreach (const QByteArray &arg, args) {
        if (re.indexIn(QString::fromLocal8Bit(arg)) == 0) {
            QString variable = re.cap(1);
            QByteArray value = arg.mid(re.matchedLength());
            ::setenv(variable.toLocal8Bit().constData(), value.constData(), 1);
        } else {
            ret << arg;
        }
    }

    return ret;
}

int main(int _argc, char **_argv)
{
    // Collect all arguments together
    QList<QByteArray> args = collectAllArguments(_argc, _argv);
    // Anything that looks like FOO=bar is interpreted as an environment
    // variable (for compatibility with make)
    args = setEnvironmentVariables(args);

    // Create replacement argc and argv
    int argc = args.count();
    char **argv = new char*[argc];
    for (int ii = 0; ii < args.count(); ++ii)
        argv[ii] = (char *)args.at(ii).constData();

    qInstallMsgHandler(aBetterMessageHandler);

    bool gui = false;
    for (int ii = 0; !gui && ii < argc; ++ii)
        if (0 == ::strcmp("-gui", argv[ii]))
            gui = true;

    QCoreApplication *app = 0;
    if (gui) {
        QApplication *a = new QApplication(argc, argv);
        a->setQuitOnLastWindowClosed( false );
        app = a;
    } else {
        app = new QCoreApplication(argc, argv);
    }

    FunctionProvider::addExtensionsPath("/extensions");
    FunctionProvider::addExtensionsPath("/extensions/3rdparty");

    StartupScript startup(argv[0]);
    QList<StartupScript::OptionDesc> cl_optionDescs = startup.options();

    bool help = false;
    bool debugginghelp = false;
    bool script = false;
    bool project = false;

    QList<StartupScript::Option> cl_options;

    for (int ii = 1; ii < argc; ++ii) {
        if (0 == ::strcmp(argv[ii], "-parse")) {
            options.executeRules = false;
            options.showParse = true;
        } else if (0 == ::strcmp(argv[ii], "-finalize")) {
            options.executeRules = false;
            options.showFinalize = true;
        } else if (0 == ::strcmp(argv[ii], "-mkspecs")) {
            options.executeRules = false;
            options.showMkspecs = true;
        } else if (0 == ::strcmp(argv[ii], "-extensions")) {
            options.executeRules = false;
            options.showExtensions = true;
        } else if (0 == ::strcmp(argv[ii], "-solutions")) {
            options.executeRules = false;
            options.showSolutions = true;
        } else if (0 == ::strcmp(argv[ii], "-list-solution")) {
            options.executeRules = false;
            options.dumpSolution = true;
            if (ii + 1 < argc)
                options.dumpSolutionName = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-tokenize")) {
            options.executeRules = false;
            options.showTokens = true;
        } else if (0 == ::strcmp(argv[ii], "-gui")) {
            options.showGui = true;
        } else if (0 == ::strcmp(argv[ii], "-progress")) {
            options.showProgress = true;
        } else if (0 == ::strcmp(argv[ii], "-parse-tree")) {
            options.executeRules = false;
            options.showParseTree = true;
        } else if (0 == ::strcmp(argv[ii], "-actions")) {
            options.executeRules = false;
            options.showActions = true;
        } else if (0 == ::strcmp(argv[ii], "-perf-statistics")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::PerfTiming);
        } else if (0 == ::strcmp(argv[ii], "-no-run")) {
            options.executeRules = false;
            options.noRun = true;
        } else if (0 == ::strcmp(argv[ii], "-rules")) {
            options.executeRules = false;
            options.showRules = true;
        } else if (0 == ::strcmp(argv[ii], "-d")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::RuleExec);
        } else if (0 == ::strcmp(argv[ii], "-df")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::RuleExec);
            if (ii + 1 < argc)
                options.traceFile = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-trace")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::Trace);
        } else if (0 == ::strcmp(argv[ii], "-show-stack-on-fatal")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::StackOnFatal);
        } else if (0 == ::strcmp(argv[ii], "-run-trace")) {
            options.debug =
                (Options::DebugModules)(options.debug | Options::RunTrace);
        } else if (0 == ::strcmp(argv[ii], "-help")) {
            help = true;
        } else if (0 == ::strcmp(argv[ii], "-advanced-help")) {
            help = true;
            debugginghelp = true;
        } else if (0 == ::strcmp(argv[ii], "-node")) {
            if (ii + 1 < argc)
                options.nodename = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-C")) {
            if (ii + 1 < argc)
                options.rootdir = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-f")) {
            if (ii + 1 < argc)
                options.filename = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-freshen")) {
            if (ii + 1 < argc)
                options.freshenFile = argv[++ii];
            else
                help = true;
        } else if (0 == ::strcmp(argv[ii], "-n")) {
            options.printOnly = true;
        } else if (0 == ::strcmp(argv[ii], "-force")) {
            options.forceRule = true;
        } else if (0 == ::strcmp(argv[ii], "-j")) {
            if (ii + 1 < argc) {
                options.threads = ::atoi(argv[++ii]);
                if (options.threads > 50 || options.threads < 0)
                    help = true;
            } else {
                help = true;
            }
        } else if (0 == ::strcmp(argv[ii], "-v")) {
            options.verbose = true;
        } else if (0 == ::strcmp(argv[ii], "-s")) {
            options.silent = true;
        } else if (0 == ::strcmp(argv[ii], "-w")) {
            options.silent = false;
        } else if (0 == ::strcmp(argv[ii], "-script")) {
            script = true;
        } else if (0 == ::strcmp(argv[ii], "-throttle")) {
            if (ii + 1 < argc) {
                do {
                    help = true;
                    QString cat = argv[++ii];
                    int index = cat.indexOf(":");
                    if ( index <= 0 || cat.length() < index+2 )
                        break;
                    QString num = cat.mid(index+1);
                    int lim;
                    if ( num == "T" )
                        lim = QThread::idealThreadCount();
                    else {
                        lim = num.toInt();
                        if ( lim == 0 )
                            break;
                    }
                    cat = cat.left(index);
                    options.throttle[cat] = lim;
                    help = false;
                } while (0);
            } else {
                help = true;
            }
        } else if (0 == ::strcmp(argv[ii], "-project")) {
            project = true;
        } else if (argv[ii][0] == '-') {
            QString arg = argv[ii];
            arg = arg.mid(1);

            bool found = false;
            // Check options
            for (int kk = 0; !found && kk < cl_optionDescs.count(); ++kk) {
                const StartupScript::OptionDesc &optionDesc =
                    cl_optionDescs.at(kk);

                if (arg == optionDesc.name) {
                    if (ii + optionDesc.count >= argc) {
                        help = true;
                    } else {
                        StartupScript::Option option;
                        option.name = arg;
                        for (int jj = 0; jj < optionDesc.count; ++jj)
                            option.parameters << QString(argv[ii + jj + 1]);
                        ii += optionDesc.count;
                        cl_options << option;
                        found = true;
                    }
                }
            }
            if (!found) {
                if (0 == ::strncmp(argv[ii], "-j", 2)) {
                    // I can't get my fingers to do -j x so make -jx work
                    // Implemented over here so that this code doesn't stop the user from adding
                    // new switches starting with -j (stupid if/else if/else if code)
                    bool ok = true;
                    for ( char *ch = argv[ii]+2; (*ch); ++ch ) {
                        if ( !isdigit(*ch) ) {
                            ok = false;
                            break;
                        }
                    }
                    if ( ok ) {
                        options.threads = ::atoi(argv[ii]+2);
                        if (options.threads > 50 || options.threads < 0)
                            help = true;
                    } else {
                        help = true;
                    }
                } else {
                    help = true;
                }
            }
        } else {
            options.rule = argv[ii];
        }
    }

    if (help) {
        startup.showHelp(debugginghelp);
        return -1;
    }

    if (script) {
        options.showGui = false;
        options.showProgress = false;
        options.printOnly = false;
        options.silent = true;
        options.verbose = false;
    }

    if (project) {
        createProject();
        return 0;
    }

    if (options.filename.isEmpty()) {
        Solution::createDefaultSolution();
    } else {
        QString filename = options.filename;
        if (!QFastDir::isFile(filename)) {
            qWarning() << "The file" << filename << "does not exist";
            return -1;
        }
        QString path;
        int idx = filename.lastIndexOf('/');
        if (idx != -1) {
            path = filename.left(idx);
            filename = filename.mid(idx + 1);
        }
        Q_ASSERT(!filename.isEmpty());

        Solution *defaultSolution = new Solution("default", QDir::currentPath());
        defaultSolution->addPath(QString(), path);
        Solution::setDefaultSolution(defaultSolution);
        options.filename = filename;
    }

    if (!startup.startup(cl_options)) {
        startup.showHelp(debugginghelp);
        return -1;
    }

    Solution *fs = Solution::defaultSolution();
    fs->addPath( "/extensions", app->applicationDirPath() + "/extensions" );

    if (options.showSolutions) {
        foreach(QString solution, Solution::solutions()) {
            Solution *sol = Solution::solution(solution);
            qOutput() << solution << ((fs == sol)?"(default)":"");
            Solution::solution(solution)->dump();
        }
    }

    if (options.showExtensions) {
        qWarning() << "Extension paths:";
        foreach(QString ext, FunctionProvider::extensionsPaths())
            qOutput() << "   " << ext;
    }

    if (options.showMkspecs) {
        qWarning() << "Mkspec paths:";
        foreach(QString path, Project::mkspecPaths())
            qOutput() << "   " << path;
    }

    if (options.dumpSolution) {
        QString str(options.dumpSolutionName);
        QString solutionname;
        QString solutionpath;

        int idx = str.indexOf(':');
        if (idx == -1) {
            solutionname = str;
        } else {
            solutionname = str.left(idx);
            solutionpath = str.mid(idx + 1);
        }

        Solution *sln = Solution::solution(solutionname);
        if (!sln) {
            qWarning() << "Cannot list non-existant solution" << solutionname;
        } else {
            PathIterator iter(sln);
            if (!solutionpath.isEmpty())
                iter = iter.advance(solutionpath);

            dump(iter);
        }
    }

    QString currentPath = QDir::currentPath();

    // Change into the root dir
    if ( options.rootdir ) {
        if ( QString(options.rootdir).at(0) == '/' ) {
            // Absolute path... prefer solution paths to real paths (so check in the solution first)
            QString detect = fs->findFile(options.rootdir, Solution::Existing).fsPath();
            int ret = -1;
            if ( !detect.isEmpty() ) {
                if ( !QDir(detect).exists() ) {
                    // Create the directory (otherwise we'll get a false error below)
                    bool ok = QDir().mkpath(detect);
                    if (!ok) {
                        qWarning() << "ERROR: Could not create" << detect << "(case 1)";
                        return 1;
                    }
                }
                ret = ::chdir(detect.toLocal8Bit().constData());
            }
            // The solution path was bogus, change to the real path instead
            if ( ret == -1 ) {
                if ( !QDir(options.rootdir).exists() ) {
                    // Create the directory (otherwise we'll get a false error below)
                    bool ok = QDir().mkpath(options.rootdir);
                    if (!ok) {
                        qWarning() << "ERROR: Could not create" << QString(options.rootdir) << "(case 2)";
                        return 1;
                    }
                }
                ret = ::chdir(options.rootdir);
                if (ret == -1) {
                    qWarning() << "ERROR: Could not call chdir" << QString(options.rootdir) << "(case 1)";
                    return 1;
                }
            }
        } else {
            // Relative path... just change to it
            if ( !QDir(options.rootdir).exists() ) {
                // Create the directory (otherwise we'll get a false error below)
                bool ok = QDir().mkpath(options.rootdir);
                if (!ok) {
                    qWarning() << "ERROR: Could not create" << QString(options.rootdir) << "(case 3)";
                    return 1;
                }
            }
            int ret = ::chdir(options.rootdir);
            if (ret == -1) {
                qWarning() << "ERROR: Could not call chdir" << QString(options.rootdir) << "(case 2)";
                return 1;
            }
        }
    }

    // Attempt to detect the nodename based on the current directory
    QByteArray detectedNodename;
    SolutionProject hierarchy;
    if (options.filename.isEmpty()) {
        if (!options.nodename) {
            QString detect = fs->fuzzyRealToSolution(QDir::currentPath()).solutionPath();
            detectedNodename = detect.toAscii();
            options.nodename = detectedNodename.constData();
        } else {
            QString detect = options.nodename;
            if ( detect == "." ) {
                // Handle the special case -node .
                detect = fs->fuzzyRealToSolution(currentPath).solutionPath();
                detectedNodename = detect.toAscii();
                options.nodename = detectedNodename.constData();
            } else if ( detect[0] != '/' ) {
                // Handle relative values for -node
                detect = QDir::currentPath() + "/" + detect;
                if ( !QDir(detect).exists() ) {
                    // Create the directory (otherwise we'll get a false error below)
                    bool ok = QDir().mkpath(detect);
                    if (!ok) {
                        qWarning() << "ERROR: Could not create" << detect << "(case 4)";
                        return 1;
                    }
                }
                detect = fs->fuzzyRealToSolution(QDir(detect).canonicalPath()).solutionPath();
                detectedNodename = detect.toAscii();
                options.nodename = detectedNodename.constData();
            }
        }

        if (!options.nodename) {
            startup.showHelp();
            return -1;
        }
        hierarchy = SolutionProject::fromNode(options.nodename, fs);
    } else {
        hierarchy = SolutionProject::fromSolutionFile(options.filename, fs);
    }

    // The root is $PWD, not the value you pass to -node (use -C to set both node and root)
    QBuild::setRootProject(SolutionProject::fromNode(fs->fuzzyRealToSolution(QDir::currentPath()).solutionPath()));

    if (options.showActions) {
        bool foundRules = false;

        SolutionFile file = hierarchy.projectFile();
        if (file.isValid()) {
            Project *project = static_cast<Project *>(hierarchy.project());
            project->finalize();
            QList<Rule *> rules = project->projectRules()->rules();
            for (int ii = 0; ii < rules.count(); ++ii) {
                Rule *rule = rules.at(ii);
                if (!rule->help.isEmpty()) {
                    foundRules = true;
                    showHelpline(0, rule->name.toAscii().constData());
                    if (!rule->help.isEmpty())
                        showHelpline(1, rule->help.toAscii().constData());
                }
            }
        }
        if (!foundRules)
            showHelpline(0, "No actions are available.");

        return -1;
    }

    if (options.showTokens || options.showParse || options.showFinalize ||
       options.showRules || options.showParseTree)
        dumpHierarchy(hierarchy);

    if (options.executeRules) {

        RuleEngine engine(options.threads);
        if (GuiBase::instance())
            GuiBase::instance()->setRuleEngine(&engine);

        RuleEngine::Result result;
        if (options.freshenFile) {
            result = engine.createFile(static_cast<Project *>(hierarchy.project()),
                                    QString(options.freshenFile));
        } else {
            result = engine.execute(static_cast<Project *>(hierarchy.project()),
                                    QString(options.rule));
        }

        // clear the progress bar
        if ( ConsoleGui::instance() ) {
            LOCK(Gui);
            ConsoleGui::instance()->lock();
            ConsoleGui::instance()->dumpLeftovers();
            ConsoleGui::instance()->disable();
        }

        if (GuiBase::instance())
            GuiBase::instance()->setRuleEngine(0);

        if (options.debug & Options::PerfTiming)
            QBuild::displayPerfTiming();

        if (result == RuleEngine::Failed)
            return 1;
        else
            return 0;

    } else if (options.noRun) {
        // Force project creation so we see messages etc.
        hierarchy.project();
    }

    delete app;
    delete [] argv;
    return 0;
}

