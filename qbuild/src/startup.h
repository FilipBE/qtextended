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

#ifndef STARTUP_H
#define STARTUP_H

#include <QScriptEngine>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include "parser.h"
#include "solution.h"

class StartupScript : public QObject
{
Q_OBJECT
public:
    StartupScript(const char *appname, QObject *parent = 0);

    struct Option {
        QString name;
        QStringList parameters;
    };

    struct OptionDesc {
        QString name;
        int count;
        QStringList help;
    };
    QList<OptionDesc> options() const;

    bool startup(const QList<Option> &options);
    QList<Option> startupOptions() const;

    struct ScriptError {
        bool isError;
        QString error;
        QStringList backtrace;
    };
    struct Script {
        Script(const QByteArray &_code,
               const QString &_filename,
               int _fileline = 1)
            : filename(_filename), fileline(_fileline),
              code(_code) {}

        QString filename;
        int fileline;
        QByteArray code;
    };
    ScriptError runScript(const Script &script);

    void showHelp(bool includeDebugging = false);

    static QString value(const QString &);
    static void setValue(const QString &, const QString &);

private:
    void readOptions();
    QList<OptionDesc> m_options;
    QList<Option> m_startupOptions;
    QScriptEngine *engine;
    const char *m_appName;
    static QHash<QString, QString> m_values;
};

class StartupFile : public QMakeObject
{
public:
    StartupFile(const SolutionFile &file);

    SolutionFile file() const;
    int errorLine() const;

private:
    void runBlock(Block *);
    void runBlock(MultiBlock *);
    void runBlock(Assignment *);

    bool expressionValue(const Expression &, QStringList &);
    void error(Block *);

    SolutionFile m_file;
    int m_errorLine;
};

#endif
