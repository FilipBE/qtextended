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

#include <QtTest/QtTest>

#include "qscriptengine.h"
#include "qscriptvalue.h"
#include "qscriptvalueiterator.h"
#include "qscriptcontext.h"

#include "qtscript_bindings.h"
#include "qtscript_qtcore.h"

QStringList builtinFiles;

void QtScript::addInternalFile(QString const &filename)
{
    builtinFiles << filename;
}

static QString readFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return QString();
    QTextStream stream(&file);
    return stream.readAll();
}

static void appendCString(QVector<char> *v, const char *s)
{
    char c;
    do {
        c = *(s++);
        *v << c;
    } while (c != '\0');
}

void QtScript::getLocation(QScriptContext *ctx, QString *fileName, int *lineNumber)
{
    Q_ASSERT(ctx);
    if (fileName) *fileName = QString();
    if (lineNumber) *lineNumber = 0;
    QStringList bt = ctx->backtrace();
    foreach (QString str, bt) {
        int pos = str.lastIndexOf(QLatin1Char('@'));
        if (pos != -1) {
            int pos2 = str.lastIndexOf(QLatin1Char(':'));
            if (pos2 > pos) {
                QString fn = str.mid(pos + 1, pos2 - pos - 1);
                if (builtinFiles.contains(fn)) continue;
                if (fileName)   *fileName = fn;
                if (lineNumber) *lineNumber = str.mid(pos2 + 1).toInt();
                if (fileName && !fileName->isEmpty())
                    return;
            }
        }
    }
}

QMetaObject QtScriptTest::staticMetaObject;

Q_GLOBAL_STATIC(QVector<uint>, qt_meta_data_QtScriptTest)
Q_GLOBAL_STATIC(QVector<char>, qt_meta_stringdata_QtScriptTest)

const QMetaObject *QtScriptTest::metaObject() const
{
    return &staticMetaObject;
}

void *QtScriptTest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtScriptTest()->constData()))
        return static_cast<void*>(const_cast<QtScriptTest*>(this));
    return QObject::qt_metacast(_clname);
}

int QtScriptTest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        const QMetaObject *mo = metaObject();
        QMetaMethod method = mo->method(mo->methodOffset() + _id);
        QByteArray sig = method.signature();
        QString name = sig.left(sig.lastIndexOf('('));
        QScriptValue testcase = m_engine->globalObject().property("testcase");
        QScriptValue val = testcase.property(name);
        if (name.endsWith("_data")) {
            QTest::addColumn<int>("dummy");
            QScriptValueIterator it(val);
            while (it.hasNext()) {
                it.next();
                QString tag = it.name();
                QTest::newRow(tag.toLatin1());
            }
        } else {
            QScriptValue args;
            QScriptValue data = testcase.property(name + "_data");
            if (data.isObject()) {
                QString tag = QTest::currentDataTag();
                QScriptValue v = data.property(tag);
                if (v.isValid()) {
                    if (v.isArray()) {
                        args = v;
                    } else {
                        args = m_engine->newArray();
                        args.setProperty(0, v);
                    }
                }
            }
            if (!args.isArray())
                args = m_engine->newArray();
            QScriptValue ret;
            if (name == "init") {
                ret = m_engine->evaluate("init_global();");
            }
            else if (name == "initTestCase") {
                ret = m_engine->evaluate("initTestCase_global();");
            }
            if (!ret.isError() || ret.property("name").toString() == "QTestFailure") {
                ret = val.call(m_engine->globalObject(), args);
            }
            if (name == "cleanup") {
                m_engine->evaluate("cleanup_global();");
            }
            else if (name == "cleanupTestCase") {
                m_engine->evaluate("cleanupTestCase_global();");
            }
            if (ret.isError() && (ret.property("name").toString() != "QTestFailure")) {
                QString backtrace = m_engine->uncaughtExceptionBacktrace().join("\n");
                QString message = ret.toString()
                // xxx makes the failure message look cluttered; correct fix
                // xxx is to implement proper generic saving of backtrace info
                // xxx in test results
                //    + "\n" + backtrace
                    ;
                QString fileName = "unknown";
                int lineNumber = -1;
                QRegExp locationRe("@([^:]+):([0-9]+)");
                if (-1 != locationRe.indexIn(backtrace)) {
                    fileName = locationRe.cap(1);
                    lineNumber = locationRe.cap(2).toInt();
                }
                QTest::qFail(qPrintable(message), qPrintable(fileName), lineNumber);
            }
        }

        _id = -1;
    }
    return _id;
}

QtScriptTest::QtScriptTest(QString const &testFilePath, QString const &scriptData, QScriptEngine *engine)
    : QObject(), m_testFilePath(testFilePath), m_engine(engine)
{
    if (m_testFilePath.isEmpty())
        m_testFilePath = qgetenv("Q_TEST_FILE");
    if (m_testFilePath.isEmpty())
        m_testFilePath="testcase.js";

    if (!m_engine) m_engine = new QScriptEngine(this);
    qscript_initialize_Qt_classes(m_engine);

    QScriptValue qtestObject = m_engine->newObject();
    qtestObject.setProperty("SkipSingle", QScriptValue(m_engine, QTest::SkipSingle));
    qtestObject.setProperty("SkipAll", QScriptValue(m_engine, QTest::SkipAll));
    qtestObject.setProperty("Abort", QScriptValue(m_engine, QTest::Abort));
    qtestObject.setProperty("Continue", QScriptValue(m_engine, QTest::Continue));
    m_engine->globalObject().setProperty("QTest", qtestObject);

    m_engine->evaluate("function QTestFailure() { Error.apply(this, arguments); }"
                      "QTestFailure.prototype = new Error();"
                      "QTestFailure.prototype.name = 'QTestFailure';");

//    m_engine->globalObject().setProperty("qVerify", m_engine->newFunction(qVerify));
//    m_engine->globalObject().setProperty("qCompare", m_engine->newFunction(qCompare));
//    m_engine->globalObject().setProperty("qExpectFail", m_engine->newFunction(qExpectFail));
//    m_engine->globalObject().setProperty("qFail", m_engine->newFunction(qFail));
/*
    m_engine->evaluate("function QVERIFY() {"
                      "  if (!qVerify.apply(this, arguments))"
                      "    throw new QTestFailure('QVERIFY');"
                      "}");
    m_engine->evaluate("function QCOMPARE() {"
                      "  if (!qCompare.apply(this, arguments))"
                      "    throw new QTestFailure('QCOMPARE');"
                      "}");
*/
/*
    m_engine->evaluate("function QEXPECT_FAIL() {"
                      "  if (!qExpectFail.apply(this, arguments))"
                      "    throw new QTestFailure('QEXPECT_FAIL');"
                      "}");
*/
/*
    m_engine->evaluate("function QSKIP() {"
                      "  qSkip.apply(this, arguments);"
                      "  throw new QTestFailure('QSKIP');"
                      "}");
*/
/*
    m_engine->evaluate("function QFAIL() {"
                      "  qFail.apply(this, arguments);"
                      "  throw new QTestFailure('QFAIL');"
                      "}");
*/

    QStringList slotNames;
    QString script = scriptData;
    if (script.isEmpty()) script = readFile(m_testFilePath);
    if (!script.isEmpty()) {
        QScriptValue ret = m_engine->evaluate(script, m_testFilePath);
        QString error;
        if (m_engine->hasUncaughtException()) {
            error = m_engine->uncaughtException().toString();
        } else if (ret.isError()) {
            error = ret.toString();
        }
        if (!error.isEmpty()) {
            QString backtrace = m_engine->uncaughtExceptionBacktrace().join("\n");
            qFatal("%s\n%s", qPrintable(error),
                     qPrintable(backtrace));
        }
        QScriptValue testcase = m_engine->globalObject().property("testcase");
        QScriptValueIterator it(testcase);
        while (it.hasNext()) {
            it.next();
            QScriptValue val = it.value();
            if (val.isFunction() || (val.isObject() && it.name().endsWith("_data")))
                slotNames << it.name();
        }

        QStringList requiredSlots;
        requiredSlots
            << "init"
            << "cleanup"
            << "initTestCase"
            << "cleanupTestCase";

        foreach (QString required, requiredSlots) {
            if (!slotNames.contains(required)) {
                m_engine->evaluate("testcase." + required + " = function() {}");
                slotNames << required;
            }
        }
    } else {
        qWarning("*** Failed to read testcase!");
    }

    QVector<uint> *data = qt_meta_data_QtScriptTest();
    // content:
    *data << 1 // revision
          << 0 // classname
          << 0 << 0 // classinfo
          << slotNames.count() << 10 // methods
          << 0 << 0 // properties
          << 0 << 0 // enums/sets
        ;

    QString testcaseName = QFileInfo(m_testFilePath).baseName();

    QVector<char> *stringdata = qt_meta_stringdata_QtScriptTest();
    appendCString(stringdata, testcaseName.toLocal8Bit() );
    int namelen = stringdata->size();
    appendCString(stringdata, "");

    // slots: signature, parameters, type, tag, flags
    foreach (QString slotName, slotNames) {
        QString slot = slotName + QLatin1String("()");
        *data << stringdata->size() << namelen << namelen << namelen << 0x08;
        appendCString(stringdata, slot.toLatin1());
    }
    *data << 0; // eod

    // initialize staticMetaObject
    staticMetaObject.d.superdata = &QObject::staticMetaObject;
    staticMetaObject.d.stringdata = stringdata->constData();
    staticMetaObject.d.data = data->constData();
    staticMetaObject.d.extradata = 0;
}

QtScriptTest::~QtScriptTest()
{
}
