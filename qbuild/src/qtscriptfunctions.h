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

#ifndef QTSCRIPTFUNCTIONS_H
#define QTSCRIPTFUNCTIONS_H

#include "functionprovider.h"
#include <QMap>
#include <QThreadStorage>
#include <QMutex>
#include <QtScript>
#include <QSet>


void qScript_PathIterator(QScriptEngine *engine);
void qScript_Project(QScriptEngine *);
void qScript_Rule(QScriptEngine *);
void qScript_Solution(QScriptEngine *);
void qScript_SolutionProject(QScriptEngine *);
void qScript_SolutionFile(QScriptEngine *);
void qScript_File(QScriptEngine *);

class TraceEngine;
class EngineLineMapping;
class QtScriptFunctions : public FunctionProvider
{
public:
    static QtScriptFunctions *instance();

    QtScriptFunctions();

    virtual QString fileExtension() const;
    virtual bool loadFile(const QString &, Project *);
    virtual void resetProject(Project *);

    virtual bool callFunction(Project *project,
                              QStringList &rv,
                              const QString &function,
                              const QStringLists &arguments);

    Project *project(QScriptEngine *engine);
    bool runScript(Project *project,
                   const QByteArray &,
                   const TraceContext &);

    void lock();
    void unlock();

private:
    struct ProjectPtr {
        ProjectPtr() : engine(0), project(0) {}
        TraceEngine *engine;
        Project *project;
    };
    struct ThreadStorage : public QThreadStorage<ProjectPtr *> {
        ThreadStorage() {}

        TraceEngine *engine() const {
            if (!hasLocalData())
                return 0;
            else
                return QThreadStorage<ProjectPtr *>::localData()->engine;
        }

        void setEngine(TraceEngine *e) {
            if (!hasLocalData()) {
                QThreadStorage<ProjectPtr *>::setLocalData(new ProjectPtr);
            }
            QThreadStorage<ProjectPtr *>::localData()->engine = e;
        }

        Project *project() const {
            if (!hasLocalData())
                return 0;
            else
                return QThreadStorage<ProjectPtr *>::localData()->project;
        }
        void setProject(Project *p) {
            if (!hasLocalData()) {
                QThreadStorage<ProjectPtr *>::setLocalData(new ProjectPtr);
            }
            QThreadStorage<ProjectPtr *>::localData()->project = p;
        }
    };

    ThreadStorage m_currentProject;
    QSet<QString> m_loadedExtensions;
    QStringList m_engineVersions;

    void unhandledException(Project *proj, const QString & = QString());

    bool readFile(const QString &file, QByteArray &contents,
                  EngineLineMapping *mapping);
    bool callFunction(Project *project,
                      QScriptValue &rv,
                      const QString &function,
                      const QStringLists &arguments);

    bool loadFile(TraceEngine *, const QString &, Project *proj);
    static TraceEngine *createEngine();
    void syncEngine();
    void syncEngine(TraceEngine *);
    TraceEngine *engine();
    QMutex m_runLock;

    void updateEngineFunctions(TraceEngine *);
    QSet<QString> m_functions;
};

#endif
