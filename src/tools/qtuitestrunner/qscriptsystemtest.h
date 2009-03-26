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

#ifndef QSCRIPTSYSTEMTEST_H
#define QSCRIPTSYSTEMTEST_H

#ifdef QTOPIA_TARGET
# include <qtopiasystemtest.h>
#else
# include <qsystemtest.h>
#endif

#include <QScriptEngine>
#include <QVariantMap>

class QTestMessage;

class QTUITEST_EXPORT QScriptSystemTest
#ifdef QTOPIA_TARGET
    : public QtopiaSystemTest
#else
    : public QSystemTest
#endif
{
Q_OBJECT
public:
    QScriptSystemTest();
    virtual ~QScriptSystemTest();

    static QString loadInternalScript(QString const &name, QScriptEngine &engine, bool withParentObject = false);
    static void loadBuiltins(QScriptEngine &engine);
    static void importIntoGlobalNamespace(QScriptEngine&, QString const&);

    virtual QString testCaseName() const;

public slots:
    virtual bool fail(QString const &message);
    virtual void expectFail( const QString &reason );
    virtual void skip(QString const &message, QSystemTest::SkipMode mode = QSystemTest::SkipSingle);
    virtual void verify(bool statement, QString const &message = QString());
    virtual void compare(const QString &actual, const QString &expected);

    QVariantMap sendRaw(const QString&, const QScriptValue& = QScriptValue());
    void installMessageHandler(const QScriptValue&);

    void dumpEngine();

protected:
    virtual QString currentFile();
    virtual int currentLine();

    virtual bool setQueryError( const QTestMessage &message );
    virtual bool setQueryError( const QString &errString );

    virtual int runTest(int argc, char *argv[]);

    virtual bool isLastData();
    virtual bool isFailExpected();

    virtual void processCommandLine(int&, char*[]);
    virtual void printUsage(int,char*[]) const;

    virtual void processMessage(const QTestMessage& message);


private:
    QString filename;
    QScriptEngine m_engine;
    QList<QScriptValue> m_messageHandlers;
    QObject* testObject;
    QString expect_fail_function;
    QString expect_fail_datatag;
    QString expect_fail_reason;
    bool m_checkOnly;
};

#endif
