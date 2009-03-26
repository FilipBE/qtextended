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

#include "builtinfunctions.h"
#include "startup.h"
#include <QFile>
#include <QCoreApplication>
#include "qoutput.h"
#include "project.h"
#include "ruleengine.h"
#include "solution.h"
#include <QMetaObject>
#include <QMetaMethod>
#include <QFileInfo>
#include "qbuild.h"
#include <ctype.h>

// This is the helper code for the IMPLEMENT_FUNCTION macro.
typedef void (*FuncImpl)(Project*,const QStringLists&,QStringList&);
static QMap<QString,FuncImpl> *funcMap;
static int register_func(const char *func,FuncImpl f)
{
    if ( !funcMap )
        funcMap = new QMap<QString,FuncImpl>();
    funcMap->insert(func,f);
    return 0;
}

// =====================================================================

static BuiltinFunctions builtinFunctions;

/*!
  \class BuiltinFunctions
  \brief The BuiltinFunctions class provides functions for the qmake environment.

  The functions provided are implemented using the \l IMPLEMENT_FUNCTION() macro.

  \sa {QBuild Functions}
*/

/*!
  Construct a BuiltinFunctions instance.
  This function provider has a higher priority than \l QtScriptFunctions so its functions
  override functions that are also specified in JavaScript.
*/
BuiltinFunctions::BuiltinFunctions()
: FunctionProvider("builtin", 1)
{
}

/*!
  \reimp
*/
QString BuiltinFunctions::fileExtension() const
{
    return QString();
}

/*!
  \reimp
*/
bool BuiltinFunctions::loadFile(const QString &, Project *)
{
    return false;
}

/*!
  \reimp
*/
void BuiltinFunctions::resetProject(Project *)
{
}

/*!
  This runs \a function with \a arguments.
  The function is passed the \a project and returns data via \a rv.
  Returns true if the function exists, false otherwise.
  \sa IMPLEMENT_FUNCTION()
*/
bool BuiltinFunctions::callFunction(Project *project,
                                    QStringList &rv,
                                    const QString &function,
                                    const QStringLists &arguments)
{
    rv = QStringList();

    Q_ASSERT(funcMap);
    if ( funcMap->contains(function) ) {
        FuncImpl f = funcMap->value(function);
        f(project, arguments, rv);
        return true;
    }

    return false;
}

/*!
  \macro IMPLEMENT_FUNCTION(x,proj,args,rv)
  \brief Define a builtin function for QBuild.
  \relates BuiltinFunctions

  The function is called \a x.
  You can specify the name of the project variable by setting \a proj or you can leave it empty.
  You can specify the name of the arguments variable by setting \a args or you can leave it empty.
  You can specify the name of the return variable by setting \a rv or you can leave it empty.

  For example, to define a function foo that returns data but does not take arguments or use the
  project you would do:
  \code
  IMPLEMENT_FUNCTION(foo,,,rv);
  \endcode

  \i Note that this macro only works in \c builtinfunctions.cpp.

  \sa {QBuild Functions}
*/
#define IMPLEMENT_FUNCTION(x,proj,args,rv)\
    static void _func_##x(Project*proj,const QStringLists&args,QStringList&rv);\
    static int _si_func_##x = register_func(#x,_func_##x);\
    static void _func_##x(Project*proj,const QStringLists&args,QStringList&rv)

// =====================================================================

/*!
  \builtin_function shellQuote()
  \brief Quotes strings for passing to shell commands.
  \usage
  \code
  FILENAME = "/home/qbuild/path with spaces/file.txt"
  foo.TYPE=RULE
  foo.commands="cat "$$shellQuote($$FILENAME)
  \endcode
  \description
  This function returns the passed parameters quoted suitably for passing to
  shell commands. This is not perfect as the QBuild parser will prevent
  passing certain items to the shell (such as the backslash character). You
  should use a literal command in order to pass such characters to the shell.
  \sa {Rule Javascript Class Reference}
*/
IMPLEMENT_FUNCTION(shellQuote,,args,rv)
{
    if ( args.count() ) {
        // Collapse everything into a single string
        // This tries to cope with raw data
        // (eg. $$shellQuote(raw data))
        QStringList bits;
        foreach (QStringList argl, args) {
            QString arg = argl.join(" ");
            // Preserve any raw commas
            arg += ",";
            bits << arg;
        }
        // Don't leave a trailing comma
        bits.last().chop(1);
        QString arg = bits.join(" ");
        arg.replace("\"", "\\\"");
        arg = "\"" + arg + "\"";
        rv << arg;
    } else {
        rv << "\"\"";
    }
}

/*!
  \builtin_function exists()
  \brief Test if a file exists.
  \usage
  \code
  exists(file)
  \endcode
  \description
  This function retrns true if it can locate the file in the solution filesystem.
  \sa {Solution Filesystem}
*/
IMPLEMENT_FUNCTION(exists,proj,args,rv)
{
    foreach(QStringList arg, args) {
        QString str = arg.join(" ");
        SolutionFile file = proj->solution()->file(str, Solution::All, proj->solutionProject().projectFile());
        if (file.isValid()) {
            rv << "true";
            return;
        }
        QStringList paths = proj->solution()->pathMappings(str);
        foreach ( const QString &path, paths ) {
            if ( QFileInfo(path).exists() ) {
                rv << "true";
                return;
            }
        }
        if ( QFileInfo(str).exists() ) {
            rv << "true";
            return;
        }
    }
    rv << "false";
}

/*!
  \builtin_function join()
  \brief Join a list of items into a single item.
  \usage
  \code
  VAR_1=foo bar baz
  VAR_2=one two three
  VAR_3=$$join($$VAR_1,$$VAR_2) #VAR_3="foo bar baz" "one two three"
  \endcode
  \description
  Returns a list is with one item per input argument. Multiple items in an input argument are
  catenated with a space.
*/
IMPLEMENT_FUNCTION(join,,args,rv)
{
    foreach(QStringList arg, args)
        rv << arg.join(" ");
}

/*!
  \builtin_function value()
  \brief Return the value of a variable.
  \usage
  \code
  VAR_1=foo
  VAR_2=VAR_1
  VAR_3=$$value($$VAR_2) #VAR_3=foo
  \endcode
  \description
  The value of the argument is treated as the name of a variable and that variable's contents are returned.
*/
IMPLEMENT_FUNCTION(value,proj,args,rv)
{
    if (args.isEmpty() || args.first().isEmpty())
        return;

    QString str = args.first().join(" ");
    QMakeObject *obj = QMakeObject::object(str, proj);
    if (obj)
        rv << obj->value();
}

/*!
  \builtin_function globalValue()
  \brief Return a value from the startup scripts.

  \sa {Startup Scripts}
*/
IMPLEMENT_FUNCTION(globalValue,,args,rv)
{
    if (args.isEmpty() || args.first().isEmpty())
        return;

    QString str = args.first().join(" ");
    rv << StartupScript::value(str);
}

/*!
  \builtin_function path()
  \brief Return the path to a file.
  \usage
  \code
  foo=$$path(foo.cpp,project)
  \endcode
  \description
  The path() function takes 2 arguments. A filename and an enum that tells it where to look.

  The locations that can be searched are:
  \list type=dl
  \o generated In the build tree
  \o project In the project tree (the directory with qbuild.pro)
  \o existing
  \endlist

  Note that files specified with project must exist.
  \code
  foo=$$path(foo,project)
  isEmpty(foo):message(foo does not exist in the project directory!)
  \endcode

  You can use named solutions too.
  \code
  foo=$$path(QtopiaSdk:/bin/qbuild,generated)
  \endcode
*/
IMPLEMENT_FUNCTION(path,proj,args,rv)
{
    if (args.count() != 2)
        return;

    const QString &type = args.at(1).join(" ");
    const QString &file = args.at(0).join(" ");

    Solution::FileType ftype;
    if (type == "generated") {
        ftype = Solution::Generated;
    } else if (type == "project") {
        ftype = Solution::Project;
    } else if (type == "existing") {
        ftype = Solution::Existing;
    } else {
        proj->warning() << "Invalid type to path() function";
        return;
    }

    rv << proj->solution()->findFile(file, ftype, proj->file()).fsPath();
}

/*!
  \builtin_function project()
  \brief Returns the name of the project.
  \usage
  \code
  foo.TYPE=DEPENDS
  foo.commands="foo.prerequisiteActions+=""#(h)"$$project()"default"""
  \endcode
  \description
  The name of the project is the same as it's path in the solution filesystem.
  This function is typically used when creating dependencies.
*/
IMPLEMENT_FUNCTION(project,proj,,rv)
{
    rv << proj->name();
}

/*!
  \builtin_function equals()
  \brief Test if a variable is equal to a value.
  \usage
  \code
  FOO=bar
  equals(FOO,bar):THESAME=yes
  \endcode
  \description
  The first argument is the name of a variable. The second argument is a value.
  The function retrns true if the variable's value is the same as the passed value.
  Note that this is a value check, it does not convert the values to strings first.
  Thus the following is not equal.
  \code
  FOO="foo bar"
  equals(FOO,foo bar):THESAME=yes
  \endcode
  \sa {QBuild Variables}
*/
IMPLEMENT_FUNCTION(equals,project,args,rv)
{
    if (args.count() < 2)
        return;

    QString var = args.at(0).join(" ");
    QStringList value = args.at(1);

    if ( project->QMakeObject::property(var)->value() == value )
        rv << "true";
    else
        rv << "false";
}

/*!
  \builtin_function isEmpty()
  \brief Check if a variable is empty.
  \usage
  \code
  isEmpty(FOO):FOO=something
  \endcode
  \description
  This function takes one argument which is the name of a variable.
  Returns true if the variable is empty.
*/
IMPLEMENT_FUNCTION(isEmpty,project,args,rv)
{
    if (args.isEmpty() || args.first().isEmpty()) {
        rv << "true";
        return;
    }

    if ( project->QMakeObject::property(args.first().join(" "))->value().isEmpty() )
        rv << "true";
    else
        rv << "false";
}

/*!
  \builtin_function error()
  \brief Report a fatal error.
  \usage
  \code
  error("Cannot continue because of foo.")
  \endcode
  \description
  This function reports the error and causes QBuild to abort.
  It should be used only in cases where you cannot continue as it halts all processing.
  The preferred alternative is to use the \l warning function instead.
*/
IMPLEMENT_FUNCTION(error,proj,args,)
{
    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    proj->error() << str.toLatin1().constData();
    abort();
}

/*!
  \builtin_function info()
  \brief Report information to the user.
  \usage
  \code
  info("Something has happened")
  \endcode
  \description
  This function reports the information to the user.
  It is designed to be used by the build system.
  \sa message
*/
IMPLEMENT_FUNCTION(info,proj,args,)
{
    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    proj->info() << str.toLatin1().constData();
}

/*!
  \builtin_function warning()
  \brief Report a warning to the user.
  \usage
  \code
  warning("Something has happened")
  \endcode
  \description
  This function reports the warning to the user. It also causes QBuild to fail.
  Unlike \l error, the failure happens at the end of processing. This is preferred
  as it lets all applicable warnings be emitted.
*/
IMPLEMENT_FUNCTION(warning,proj,args,)
{
    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    proj->warning() << str.toLatin1().constData();
}

/*!
  \builtin_function message()
  \brief Report information to the user.
  \usage
  \code
  message("Something has happened")
  \endcode
  \description
  This function reports the information to the user.
  It is designed to be used by the user or for debug.
  \sa info
*/
IMPLEMENT_FUNCTION(message,proj,args,)
{
    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    proj->message() << str.toLatin1().constData();
}

/*!
  \builtin_function include()
  \brief Include the contents of another file.
  \usage
  \code
  include(foo.pri)
  \endcode
  \description
  The included file modifies the current project. The function returns
  false if the file could not be parsed successfully.
  \i Note that the file must exist or an error will occur. You can verify
  the file exists first like this.
  \code
  exists(foo.pri):include(foo.pri)
  \endcode
*/
IMPLEMENT_FUNCTION(include,ctxt,args,)
{
    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    SolutionFile file = ctxt->solution()->findFile(str,
            Solution::Existing, ctxt->file());
    if (!file.isValid()) {
        qOutput().nospace()
            << "Project " << ctxt->messageDisplayName()
            << "ERROR: Unable to include missing file: " << quote(str);
        QBuild::shutdown();
    }

    if (!ctxt->includeFile(file)) {
        qOutput().nospace()
            << "Project " << ctxt->messageDisplayName()
            << "ERROR: Unable to include invalid file: " << quote(str);
        QBuild::shutdown();
    }
}

/*!
  \builtin_function include_depends_rule()
  \brief Read a .d file.
  \usage
  \code
  foo.TYPE=RULE
  foo.inputFiles=$$include_depends_rule(file.d)
  \endcode
  \description
  This is not designed to be used by users but by the build system.
  The returned output looks like this.
  \code
  #(os)file1 #(os)file2
  \endcode
*/
IMPLEMENT_FUNCTION(include_depends_rule,ctxt,args,rv)
{
    Q_UNUSED(ctxt);

    QString str;
    if (!args.isEmpty())
        str = args.first().join(" ");
    if (str.isEmpty())
        return;

    QString fileName = args.at(0).join(" ");

    QFile depends(fileName);
    if (!depends.open(QFile::ReadOnly))
        return;

    /*
       .d files are in Makefile syntax (foo: deps).
       Skip everything until the colon.
       Process dependencies taking into account escaped
       characters (\) and split on whitespace.
       For now, assume there isn't anything else in the .d
       file (cheat and ignore \\\n).
    */
    bool readingFileName = true;
    bool foundColon = false;
    QByteArray os = "#(os)";
    QByteArray file;
    QByteArray line;
    int len;
    while (!depends.atEnd()) {
        line = depends.readLine();
        len = line.length();
        for ( int i = 0; i < len; i++ ) {
            char ch = line.at(i);
            if (isspace(ch)) {
                if (!readingFileName)
                    continue;
                readingFileName = false;
                if (foundColon) {
                    rv << QString(file);
                    file = os;
                }
            } else if (ch == ':') {
                Q_ASSERT(!foundColon);
                foundColon = true;
                readingFileName = false;
                file = os;
            } else if (ch == '\\') {
                if (!readingFileName)
                    continue;
                file.append(line.at(++i));
            } else {
                readingFileName = true;
                file.append(line.at(i));
            }
        }
    }
}

/*!
  \builtin_function contains()
  \brief Check if a variable contains a value.
  \usage
  \code
  VAR_1=foo
  contains(VAR_1,bar):VAR_1+=baz #VAR_1=foo
  VAR_1+=bar
  contains(VAR_1,bar):VAR_1+=baz #VAR_1=foo bar baz
  \endcode
  \description
  The first argument is a variable name. The second argument is a value. Returns true if the value
  is in the variable.
  \a Note that this does not to a string comparrison but a value comparrison
  so the following does not work.
  \code
  FOO="foo bar"
  contains(FOO,bar):SOMETHING=yes
  \endcode
  \sa CONFIG
*/
IMPLEMENT_FUNCTION(contains,ctxt,args,rv)
{
    if (args.count() < 2)
        return;

    QString var = args.at(0).join(" ");
    QStringList list = args.at(1);
    QStringList value = ctxt->QMakeObject::property(var)->value();

    for (int ii = 0; ii < list.count(); ++ii)
        if (!value.contains(list.at(ii))) {
            rv << "false";
            return;
        }

    rv << "true";
}

/*!
  \builtin_function not()
  \brief Reverse a truth value.
  \usage
  \code
  not(foo()):SOMETHING=1
  \endcode
  \description
  This function takes one argment that is a value. It returns true if the value matches
  the false logic, true otherwise. Note that the function does not evaluate its contents
  so you cannot do this.
  \code
  CONFIG+=foo
  not(foo):CONFIG+=bar
  \endcode
  Calling a function is acceptable because it will be evaluated before not().
*/
IMPLEMENT_FUNCTION(not,,args,rv)
{
    if (args.isEmpty()) {
        rv << "true";
        return;
    }
    if (args.first().isEmpty() ||
            (args.first().count() == 1 &&
             args.first().first() == "false")) {
        rv << "true";
        return;
    }

    rv << "false";
}

/*!
  \builtin_function testFile()
  \brief Check the contents of a file.
  \usage
  \code
  testFile(file,contents)
  \endcode
  \description
  The first argument is a filename. The second argument is the contents of the file.
  Note that the parser makes it impossible to pass arbitrary characters into this function.
  This is designed to be used by the build system, in conjunction with the \l writeFile() function.
*/
IMPLEMENT_FUNCTION(testFile,project,args,rv)
{
    if (args.count() < 2)
        return;


    QString filename = args.at(0).join(" ");
    QByteArray content = args.at(1).join(" ").toLocal8Bit();

    SolutionFile file = project->solution()->findFile(filename,
            Solution::Existing, project->file());
    if (!file.isValid())  {
        rv << "false";
        return;
    }

    QFile infile(file.fsPath());
    if (!infile.open(QFile::ReadOnly)) {
        rv << "false";
        return;
    }

    QByteArray contents = infile.readAll();
    if ( contents == content )
        rv << "true";
    else
        rv << "false";
}

/*!
  \builtin_function writeFile()
  \brief Write data to a file.
  \usage
  \code
  writeFile(file,contents)
  \endcode
  \description
  The first argument is a filename. The second argument is the contents to put in the file.
  Note that the parser makes it impossible to pass arbitrary characters into this function.
  This is designed to be used by the build system, in conjunction with the \l testFile() function.
*/
IMPLEMENT_FUNCTION(writeFile,project,args,)
{
    if (args.count() < 2)
        return;

    QString filename = args.at(0).join(" ");
    QString content = args.at(1).join(" ");

    SolutionFile file = project->solution()->findFile(filename, Solution::Generated, project->file());
    if (!file.isValid())
        return;

    QFile infile(file.fsPath());
    if (!infile.open(QFile::WriteOnly))
        return;

    infile.write(content.toLocal8Bit());
}

/*!
  \builtin_function disable_project()
  \brief Turn off a project that cannot be built.
  \usage
  \code
  disable_project("reason")
  \endcode
  \description
  The project is not built and an error occurs if another project requires this project.
  The reason is reported to the user in some circumstances.
*/
IMPLEMENT_FUNCTION(disable_project,project,args,)
{
    QString reason;
    if (!args.isEmpty())
       reason = args.first().join(" ");

    project->setDisabled(reason);
}

/*!
  \builtin_function upper()
  \brief Convert a value to uppercase.
  \usage
  \code
  FOO=foo bar
  BAR=$$upper($$FOO)
  \endcode
  \description
  This converts the passed value to uppercase.
  It does not convert its input to a string.
*/
IMPLEMENT_FUNCTION(upper,,args,rv)
{
    if (args.isEmpty())
        return;
    for (int ii = 0; ii < args.first().count(); ++ii)
        rv << args.first().at(ii).toUpper();
}

/*!
  \builtin_function lower()
  \brief Convert a value to lowercase.
  \usage
  \code
  FOO=FOO BAR
  BAR=$$lower($$FOO)
  \endcode
  \description
  This converts the passed value to lowercase.
  It does not convert its input to a string.
*/
IMPLEMENT_FUNCTION(lower,,args,rv)
{
    if (args.isEmpty())
        return;
    for (int ii = 0; ii < args.first().count(); ++ii)
        rv << args.first().at(ii).toLower();
}

/*!
  \builtin_function eval()
  \brief Evaluate commands.
  \usage
  \code
  cmd="FOO += ""BAR BAZ"""
  eval($$cmd)
  \endcode
  \description
  The statements are evaluated by the current project.
*/
IMPLEMENT_FUNCTION(eval,project,args,)
{
    if (args.isEmpty() && args.first().isEmpty())
        return;

    QString str = args.first().join(" ");
    project->run(str.toLatin1(), "eval() function call");
}

/*!
  \builtin_function load()
  \brief Load an extension.
  \usage
  \code
  load("foo.js")
  \endcode
  \description
  This loads an extension. The extension is loaded in the same way that
  it would be if it were loaded via \l CONFIG. The difference is that
  extensions can be loaded from any location with this function.
  It is designed to allow private extensions.
*/
IMPLEMENT_FUNCTION(load,proj,args,)
{
    QString name;
    if (!args.isEmpty())
        name = args.first().join(" ");
    FunctionProvider::evalLoad(proj, name);
}

/*!
  \builtin_function if()
  \brief Group conditionals into a single statement
  \usage
  \code
  if (foo|bar):baz:message(foo)
  \endcode
  \description
  This function is provided to enable 'and' and 'or' statements to be grouped on a single line.
*/
IMPLEMENT_FUNCTION(if,project,args,rv)
{
    if (args.isEmpty() && args.first().isEmpty()) {
        qOutput().nospace()
            << "Project " << project->messageDisplayName()
            << "ERROR: Unable to parse empty if() statement.";
        QBuild::shutdown();
    }

    QString str = args.first().join(" ");
    bool ret;
    if (!project->do_if (str.toLatin1(), "if ()function call", ret)) {
        qOutput().nospace()
            << "Project " << project->messageDisplayName()
            << "ERROR: Unable to parse if() statement: " << quote(str);
        QBuild::shutdown();
    }

    if (ret)
        rv << "true";
    else
        rv << "false";
}

/*!
  \builtin_function for()
  \brief Loop over the items in a variable
  \usage
  \code
  VAR=foo bar baz
  for (v,VAR) {
    message($$v)
  }
  \endcode
  \description
  This function is similar to but not as completely implemented as the for() function in qmake.
  There is no break() or continue(), there is no infinite loop.

  Note that the value of \i v will NOT be modified outside of the scope of the for loop. Its
  value is saved and restored after the loop as finished.
*/
IMPLEMENT_FUNCTION(for,project,args,rv)
{
    if (args.count() != 2) {
        qOutput().nospace()
            << "Project " << project->messageDisplayName()
            << "ERROR: Unable to parse invalid for() statement.";
        QBuild::shutdown();
    }

    // No actualy work happens here. Instead, it happens in Project::runBlock.
    rv << "true";
}

/*!
  \builtin_function rule_for_file()
  \brief Find the rule that freshens a file
  \usage
  \code
  rulename=$$rule_for_file(foo.moc)
  isEmpty(rulename):message("Nobody freshens foo.moc")
  \endcode
  \description
  Find the rule that freshens a file. Returns an empty string if no rule freshens the file.
*/
IMPLEMENT_FUNCTION(rule_for_file,project,args,rv)
{
    if (args.count() != 1) {
        qOutput().nospace()
            << "Project " << project->messageDisplayName()
            << "ERROR: rule_for_file() takes 1 argument.";
        QBuild::shutdown();
    }

    QString str = args.first().join(" ");
    Rule *rule = project->projectRules()->ruleForFile(str);
    if ( rule )
        rv << rule->name;
}

