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

#include "qscriptsystemtest.h"
#include "scriptpreprocessor.h"

#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>

#ifdef QTOPIA_TARGET
# include "qscriptsystemtestlog_p.h"
# include <qtopiasystemtestmodem.h>
#endif

#include <qtestprotocol_p.h>
#include <QtTest/QtTest>

#include "qtscript_bindings.h"

#ifdef QTOPIA_TARGET
// Add any other QObject subclasses to be used from within scripts here, and
// at the end of this file.
#ifdef QTUITEST_USE_PHONESIM
Q_DECLARE_METATYPE(QtopiaSystemTestModem*)
#endif
#endif

#ifdef QTOPIA_TARGET
#define Super QtopiaSystemTest
#else
#define Super QSystemTest
#endif

Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(QList<qint64>)

template <typename Tp>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, Tp const &qobject)
{
    return engine->newQObject(qobject, QScriptEngine::AutoOwnership);
}

template <typename Tp>
void qScriptValueToQObject(const QScriptValue &value, Tp &qobject)
{    qobject = qobject_cast<Tp>(value.toQObject());
}

template <typename Tp>
int qScriptRegisterQObjectMetaType(
    QScriptEngine *engine,
    const QScriptValue &prototype = QScriptValue(),
    Tp * /* dummy */ = 0
    )
{
    return qScriptRegisterMetaType<Tp>(engine, qScriptValueFromQObject,
                                        qScriptValueToQObject, prototype);
}

struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
        { return &static_cast<StaticQtMetaObject*> (0)->staticQtMetaObject; }
};

void setupEnums(QScriptEngine *engine) {
    QScriptValue qtObject = engine->newQMetaObject( StaticQtMetaObject::get() );
    engine->globalObject().setProperty("Qt", qtObject );

    QScriptValue qsvObject = engine->newObject();
#define S(val) qsvObject.setProperty(#val, engine->toScriptValue((int)QScriptValue::val));
    S(PropertySetter);S(ReadOnly);S(Undeletable);S(SkipInEnumeration);
    S(PropertyGetter);S(QObjectMember);S(KeepExistingFlags);S(UserRange);
#undef S
    engine->globalObject().setProperty("QScriptValue", qsvObject );
}

static QScriptValue dateToString(QScriptContext *ctx, QScriptEngine *eng)
{
    QDateTime dt = ctx->thisObject().toDateTime();
    QString fmt = ctx->argument(0).toString();

    return eng->toScriptValue(dt.toString(fmt));
}


// Include a single script into an engine.
static QScriptValue includeScriptFunction
        (QScriptEngine *engine, const QString& name)
{
    // Find the script, relative to the entry script.
    QString script;
    QtScript::getLocation(engine->currentContext(), &script, 0);
    QString directory = QFileInfo(QFileInfo(script).canonicalFilePath()).dir().canonicalPath();
    QDir dir(directory);
    QFileInfo file(dir, name);
    QString filename = name;
    if (file.exists()) filename = file.canonicalFilePath();

    // Check if the script has already been loaded.
    static const char includesProperty[] = "_qtuitest_includes";
    QStringList included = qscriptvalue_cast<QStringList>(engine->globalObject().property(includesProperty));
    if (included.contains(filename)) {
        return QScriptValue();
    }

    // Try to load the script into memory.
    QFile scriptFile(filename);
    if (!scriptFile.exists() || !scriptFile.open(QIODevice::ReadOnly)) {
        int pos = directory.indexOf("/src/");
        if (pos > 0) {
            directory = directory.left(pos) + "/tests/shared";
            QDir dir(directory);
            QFileInfo file(dir, name);
            filename = file.filePath();
            scriptFile.setFileName(filename);
        }
    }

    if (!scriptFile.exists() || (!scriptFile.isOpen() && !scriptFile.open(QIODevice::ReadOnly))) {
        return engine->currentContext()->throwError("Could not find " + name + " in either the shared or testcase directory");
    }

    QString contents = QTextStream(&scriptFile).readAll();
    scriptFile.close();

    contents.prepend("with(ParentTestMetaObject) {");
    contents.append("\n}");

    ScriptPreprocessor().preprocess(contents);

    // Evaluate the contents of the script.
    engine->pushContext()->setActivationObject(engine->globalObject());
    // Note that we have included this script.
    if (!engine->globalObject().property(includesProperty).isValid()) {
        engine->globalObject().setProperty(includesProperty, engine->newArray());
    }
    engine->globalObject().property(includesProperty).setProperty(
            engine->globalObject().property(includesProperty).property("length").toUInt32(),
            qScriptValueFromValue(engine, filename));
    QScriptValue ret = engine->evaluate(contents, filename);
    engine->popContext();

    return ret;
}

// Implementation of the "include()" function in scripts, which imports scripts.
static QScriptValue includeFunction
        (QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue value;
    if (context->argumentCount() == 0) {
        return context->throwError
            ("script name must be supplied to include()");
    }
    value = context->argument(0);
    if (!value.isString()) {
        return context->throwError
            ("invalid script name supplied to include()");
    }

    QString name = value.toString();
    if (name.contains(QChar('*'))) {
        //FIXME: This path doesn't look in the tests/shared/ directory.
        // Expand the wildcard and include all matching scripts.
        QString script;
        QtScript::getLocation(context, &script, 0);
        QDir dir(QFileInfo(script).dir());
        QStringList files = dir.entryList(QStringList(name));
        foreach (QString filename, files) {
            value = includeScriptFunction(engine, dir.filePath(filename));
            if (engine->hasUncaughtException())
                return value;
        }
        return engine->undefinedValue();
    } else {
        // Include a single script file.
        return includeScriptFunction(engine, name);
    }
}

static QScriptValue setFlags
        (QScriptContext *context, QScriptEngine* /*engine*/)
{
    Q_ASSERT(context);
    if (context->argumentCount() != 3) {
        return context->throwError
            ("setFlags() needs three arguments");
    }
    QScriptValue o = context->argument(0);
    if (!o.isObject()) return QScriptValue();
    QString name = context->argument(1).toString();
    int flags = context->argument(2).toInt32();

    o.setProperty(name, o.property(name), QFlag(flags));
    return QScriptValue();
}


QScriptSystemTest::QScriptSystemTest()
    :   testObject(0),
        m_checkOnly(false)
{
    // Ensure we process events periodically.
    // Without this, a script containing an infinite loop would make us
    // unable to respond to messages from Qtopia or the IDE.
    m_engine.setProcessEventsInterval(100);
}

QScriptSystemTest::~QScriptSystemTest()
{
}

QString QScriptSystemTest::testCaseName() const
{
    if (testObject)
        return testObject->metaObject()->className();
    return QAbstractTest::testCaseName();
}

void QScriptSystemTest::loadBuiltins(QScriptEngine &engine)
{
    QScriptEngine configEngine;
    QScriptSystemTest::loadInternalScript("config.js", configEngine);
    for (int i = 0; i < configEngine.globalObject().property("builtin_files").property("length").toInt32(); ++i) {
        QString file = configEngine.globalObject().property("builtin_files").property(i).toString();
        QtScript::addInternalFile( QScriptSystemTest::loadInternalScript(file, engine, true) );
    }
}

void QScriptSystemTest::importIntoGlobalNamespace(QScriptEngine& engine, QString const& object)
{
    QScriptValue obj = engine.globalObject().property(object);

    QScriptValueIterator it(obj);
    while (it.hasNext()) {
        it.next();
        QString name = it.name();

        // Transform name of enter(QString,QString) to enter
        if (it.value().isFunction() && name.contains('(')) {
            name = name.left(name.indexOf('('));
        }

        // Import this property into the global object iff one doesn't already
        // exist with this name
        if (engine.globalObject().property(name).isValid()) continue;

        // For functions, to keep the QObject slot resolution working right, we
        // must wrap the property instead of simply copying it.
        if (it.value().isFunction()) {
            engine.evaluate(QString("%1 = function(){ return %2.%1.apply(this, arguments); };")
                        .arg(name)
                        .arg(object));
        } else {
            engine.globalObject().setProperty(name, it.value());
        }
    }
}

QString QScriptSystemTest::loadInternalScript(QString const &name, QScriptEngine &engine, bool withParentObject)
{
    QString filename = QFileInfo(QString::fromAscii(__FILE__)).absolutePath() + "/" + name;
    if (!QFileInfo(filename).exists()) filename = ":/" + name;
    QFile f(filename);
    QString data;
    if (!f.open(QIODevice::ReadOnly) || (data = QTextStream(&f).readAll()).isEmpty()) {
        qWarning("QtUiTest: Couldn't load config file '%s' (using '%s')", qPrintable(name), qPrintable(filename));
        return QString();
    }

    if (withParentObject) {
        data.prepend("with(ParentTestMetaObject) {");
        data.append("\n}");
    }

    QScriptValue e = engine.evaluate(data, filename);
    if (e.isError()) {
        QString backtrace = engine.uncaughtExceptionBacktrace().join("\n");
        qWarning("In QtUiTest config file %s:\n%s\n%s", qPrintable(filename), qPrintable(e.toString()),
            qPrintable(backtrace));
    }
    return filename;
}

QScriptValue variantToScriptValue(QScriptEngine *engine, const QVariant &v)
{
    QScriptValue ret;
    if (v.isNull()) {
        ret = QScriptValue( engine, QScriptValue::NullValue );
    } else if (v.canConvert<QStringList>()) {
        ret = engine->toScriptValue(v.value<QStringList>());
    } else if (v.canConvert<qreal>()) {
        ret = engine->toScriptValue(v.value<qreal>());
    } else if (v.canConvert<int>()) {
        ret = engine->toScriptValue(v.value<int>());
    } else if (v.canConvert<QString>()) {
        ret = engine->toScriptValue(v.value<QString>());
    } else {
        ret = engine->newVariant(v);
    }
    return ret;
}

void variantFromScriptValue(const QScriptValue &obj, QVariant &v)
{
    v = obj.toVariant();
}

QString QScriptSystemTest::currentFile()
{
    QString fileName = QString();
    int lineNumber = 0;

    QtScript::getLocation(m_engine.currentContext(), &fileName, &lineNumber);

    return fileName;
}

int QScriptSystemTest::currentLine()
{
    QString fileName = QString();
    int lineNumber = 0;

    QtScript::getLocation(m_engine.currentContext(), &fileName, &lineNumber);

    return lineNumber;
}

bool QScriptSystemTest::isLastData()
{
/*
    QString s = currentDataTag();

    QTestTable *table = QTestTable::currentTestTable();
    if (table) {
        if (table->dataCount() == 0) return true;

        int count = 0;
        for (int i=0; i<table->dataCount(); i++) {
            QTestData *data = table->testData(i);
            if (data && s == data->dataTag()) {
                count++;
                if (count > 1)
                    qDebug( "Multiple datasets have been given the same datatag." );
                if (i+1 == table->dataCount()) return true;
            }
        }
    }
*/
    return false;
}

void QScriptSystemTest::skip(QString const &message, QSystemTest::SkipMode mode)
{
    QTest::qSkip( qPrintable(message), static_cast<QTest::SkipMode>(mode), qPrintable(currentFile()), currentLine() );
/*
    QTestResult::addSkip( qPrintable(message), static_cast<QTest::SkipMode>(mode), qPrintable(currentFile()), currentLine() );

    if (mode == QSystemTest::SkipAll || isLastData())
        QTestResult::setSkipCurrentTest(true);
*/
    m_engine.evaluate("throw new QTestFailure('QSKIP');");
}

bool QScriptSystemTest::fail(QString const &message)
{
    bool ret = Super::fail( message );
    if (!ret) {
        m_engine.evaluate("throw new QTestFailure('QFAIL');");
    }
    return ret;
}

void QScriptSystemTest::verify(bool statement, QString const &message)
{
    if (!QTest::qVerify(statement, "<statement>", qPrintable(message), qPrintable(currentFile()), currentLine() ))
        m_engine.evaluate("throw new QTestFailure('QFAIL');");
}

void QScriptSystemTest::compare(const QString &actual, const QString &expected)
{
    if (!QTest::qCompare( actual, expected, qPrintable(actual), qPrintable(expected), qPrintable(currentFile()), currentLine() ))
            m_engine.evaluate("throw new QTestFailure('QFAIL');");
}

bool QScriptSystemTest::isFailExpected()
{
    return (expect_fail_function == currentTestFunction() && expect_fail_datatag == currentDataTag());
}

void QScriptSystemTest::expectFail( const QString &reason )
{
    expect_fail_function = currentTestFunction();
    expect_fail_datatag = currentDataTag();
    expect_fail_reason = reason;
    bool ok = QTest::qExpectFail(currentDataTag().toLatin1(), reason.toLatin1(),
                                    QTest::TestFailMode(1),//mode),
                                    qPrintable(currentFile()), currentLine());
    if (!ok)
        m_engine.evaluate("throw new QTestFailure('QFAIL');");
}

bool QScriptSystemTest::setQueryError( const QTestMessage &message )
{
    QString errorString = message["status"].toString();
    QVariant response   = message["_q_inResponseTo"];
    if (response.isValid()) {
        errorString += QString("\nIn response to message: %2").arg(response.toString());
    }
    return setQueryError( errorString );
}

bool QScriptSystemTest::setQueryError( const QString &errString )
{
    if (queryFailed()) return false;
    Super::setQueryError(errString);
    bool ret = fail(errString);
    if (!ret) {
        m_engine.evaluate("throw new QTestFailure('QFAIL');");
    }
    return ret;
}

int QScriptSystemTest::runTest(int argc, char *argv[])
{
    if (argc > 1) filename = argv[1];

    if (filename.isEmpty()) qFatal("No test script specified.");

    /* Consume the first argument */
    argv[1] = argv[0];
    char **c_argv = &argv[1];
    int    c_argc = argc-1;

    {
        QFileInfo fi(filename);
        if (!fi.exists()) qFatal("%s doesn't exist", qPrintable(filename));
        filename = fi.absoluteFilePath();
    }

    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) qFatal("Can't open %s", qPrintable(filename));
    QTextStream stream(&file);
    QString script = stream.readAll();

    ScriptPreprocessor().preprocess(script);

    setupEnums(&m_engine);

    // include() imports scripts directly into the parent script.
    m_engine.globalObject().setProperty
        ("include", m_engine.newFunction(includeFunction, 1));
    m_engine.globalObject().setProperty
        ("setFlags", m_engine.newFunction(setFlags, 3));


    m_engine.globalObject().setProperty("_dateToString", m_engine.newFunction(dateToString));
    m_engine.evaluate("_old_date_toString = Date.prototype.toString;"
                        "Date.prototype.toString = function() {"
                        "  if (arguments[0] == undefined)"
                        "    return _old_date_toString.apply(this, arguments);"
                        "  return _dateToString.apply(this, arguments);"
                        "}");

    m_engine.globalObject().setProperty("ParentTestObject", m_engine.newQObject(this));
    m_engine.globalObject().setProperty("ParentTestMetaObject", m_engine.newQMetaObject(metaObject()));

#ifdef QTOPIA_TARGET
    /* FIXME: remove when underlying cause of bug 229118 is fixed */
    m_engine.globalObject().setProperty("LogImpl", m_engine.newQObject(new QScriptSystemTestLog));
#endif

    loadBuiltins(m_engine);
    importIntoGlobalNamespace(m_engine, "ParentTestObject");

    // Allow shebangs without giving syntax errors.
    if (script.startsWith("#!")) script.prepend("//");
    script.prepend("with(ParentTestMetaObject){");
    script.append("\n}");

    QtScriptTest tc(filename, script, &m_engine);
    testObject = &tc;

    qScriptRegisterMetaType(&m_engine, variantToScriptValue, variantFromScriptValue);
    qScriptRegisterSequenceMetaType<QList<qint64> >(&m_engine);

#ifdef QTOPIA_TARGET
    // Add any other QObject subclasses to be used from scripts here,
    // and at the top of this file.
#ifdef QTUITEST_USE_PHONESIM
    qScriptRegisterQObjectMetaType<QtopiaSystemTestModem*>(&m_engine);
#endif
#endif

    // Only set up the test data path if not explicitly set by user
    if (!QCoreApplication::arguments().contains("-data")) {
        setupTestDataPath(qPrintable(filename));
    }

    enableQueryWarnings(false);

    // If we get here, the syntax of the script is definitely OK
    // (a syntax error causes a qFatal in the QtScriptTest ctor).
    if (m_checkOnly)
        return 0;

    int retval = QTest::qExec(&tc, c_argc, c_argv);

    testObject = 0;

    // After a full test run, QTestLib sometimes returns 0 or sometimes returns
    // the number of test failures, depending on how it was compiled.  In both
    // cases, a negative number denotes an error.
    // We don't want test failures to affect the exit code.
    return (retval < 0) ? retval : 0;
}

/*!
    \internal
    Send a raw message from within script to the connected system.

    This can be used to extend the API for new messages on the target device
    without modifying QSystemTest or QtopiaSystemTest.

    If the message doesn't elicit a response of "OK", the current test fails.
*/
QVariantMap QScriptSystemTest::sendRaw(const QString& event, const QScriptValue& object)
{
    QVariantMap ret;
    if (object.isValid() && !object.isObject()) {
        setQueryError("Second argument to sendRaw must be an object.");
        return ret;
    }
    if (event.isEmpty()) {
        setQueryError("First argument to sendRaw cannot be an empty string.");
        return ret;
    }

    QTestMessage message(event);

    // Treat object like a variant map and load it into message.
    QScriptValueIterator it(object);
    while (it.hasNext()) {
        it.next();
        QScriptValue v = it.value();
        // Map must be flat; we don't handle objects within objects.
        if (v.isObject() && !v.isArray()) {
            setQueryError("Object passed to sendRaw must not have any child objects.");
            return ret;
        }
        // toVariant appears to flatten stringlists into strings, which we don't want.
        if (v.isArray()) {
            QVariantList list;
            for (int i = 0, max = qscriptvalue_cast<int>(v.property("length")); i < max; ++i)
                list << v.property(i).toVariant();
            message[it.name()] = list;
        } else {
            message[it.name()] = v.toVariant();
        }
    }

    QTestMessage reply;
    if (!doQuery(message, QString(), &reply)) {
        setQueryError("Raw " + event + " message failed: " + reply["status"].toString());
        return ret;
    }

    foreach (QString const& key, reply.keys())
        ret[key] = reply[key];
    return ret;
}

/*!
    \internal
    Print any special usage information which should be shown when test is launched
    with -help.
*/
void QScriptSystemTest::printUsage(int argc, char *argv[]) const
{
    Super::printUsage(argc, argv);
    qWarning(
        "  Script options:\n"
        "    -c                : Check the syntax of the test only.  Returns a non-zero exit code if the test\n"
        "                        contains any syntax errors.\n"
    );
}

/*!
    \internal
    If \a func is a function, install it as a handler for all messages received from the test
    system.

    Whenever a new message is received, \a func will be called with two arguments.
    The first is the message event as a string.
    The second is an object containing one property for each parameter of the message.

    If \a func returns true, processing of the message ends.
*/
void QScriptSystemTest::installMessageHandler(const QScriptValue& func)
{
    if (!func.isFunction()) {
        setQueryError("Argument to installMessageHandler must be a function.");
        return;
    }
    m_messageHandlers << func;
}


QString qDumpScriptValue(QString const& name, QScriptValue const& v, int indent = 0)
{
    const QString spc = QString().fill(' ', indent);

    QString ret;

    ret += spc + name + ": ";
    if (name != "global" && v.engine()->globalObject().strictlyEquals(v))
        ret += "global";
    else if (v.isBoolean())
        ret += "Boolean:" + v.toString();
    else if (v.isDate())
        ret += "Date:" + v.toString();
    else if (v.isFunction())
        ret += "Function";
    else if (v.isNull())
        ret += "null";
    else if (v.isNumber())
        ret += "Number:" + v.toString();
    else if (v.isString())
        ret += "String:" + v.toString();
    else if (v.isUndefined())
        ret += "undef";
    else {
        QString inner;
        QScriptValueIterator it(v);
        QString sep;
        while (it.hasNext()) {
            it.next();
            inner += sep + qDumpScriptValue(it.name(), it.value(), indent+2);
            sep = ",\n";
        }
        if (inner.isEmpty()) {
            ret += "{}";
        } else {
            ret += "{\n" + inner + "\n" + spc + "} /* " + name + " */";
        }
    }

    return ret;
}

/*!
    \internal
    Write out most of the state of the script engine to stderr.
*/
void QScriptSystemTest::dumpEngine()
{
    QString state;

    {
        QScriptContext* ctx = m_engine.currentContext();
        state += "context: {";
        int i = 0;
        QString sep;
        while (ctx) {
            state += QString("%1\n  %2: {\n").arg(sep).arg(i++);
            state += "    toString: " + ctx->toString() + "\n";
            state += qDumpScriptValue("activationObject", ctx->activationObject(), 4) + ",\n";
            state += qDumpScriptValue("thisObject", ctx->thisObject(), 4) + "\n";
            state += "  }";
            sep = ",";
            ctx = ctx->parentContext();
        }
        state += "\n};\n";
    }

    state += qDumpScriptValue("global", m_engine.globalObject());
    state += ";";

    fprintf(stderr, "%s\n", qPrintable(state));
}

/*!
    \internal
    Passes the test message through any installed QtScript message handlers.
    If none of them handle the message, it will be passed to the superclass.
*/
void QScriptSystemTest::processMessage(const QTestMessage& message)
{
    if (m_messageHandlers.count()) {
        QVariantMap map;
        foreach (QString const& key, message.keys())
            map[key] = message[key];

        QScriptValueList args;
        args << m_engine.toScriptValue(message.event());
        args << m_engine.toScriptValue(map);

        for (int i = 0; i < m_messageHandlers.count(); ++i) {
            QScriptValue out = m_messageHandlers[i].call(QScriptValue(), args);
            if (out.isBoolean() && out.toBoolean())
                return;
        }
    }

    Super::processMessage(message);
}

/*!
    \internal
    Processes the command line parameters.
*/
void QScriptSystemTest::processCommandLine( int &argc, char *argv[] )
{
    int offset = 0;

    for (int i=1; i<argc; ++i) {

        if ( !strcasecmp(argv[i], "-c") ) {
            argv[i] = 0;
            offset++;
            m_checkOnly = true;

        } else {
            if (offset > 0) {
                argv[i-offset] = argv[i];
                argv[i] = 0;
            }
        }
    }
    argc-=offset;

    Super::processCommandLine(argc, argv);
}

