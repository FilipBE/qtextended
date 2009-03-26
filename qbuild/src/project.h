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

#ifndef PROJECT_H
#define PROJECT_H

#include "object.h"
#include "solution.h"
#include "parser.h"
#include "ruleengine.h"
#include <QSet>

struct for_loop_t {
    bool loop;
    EvaluatableValue func;
};

class IInternalVariable;
class Project : public QObject, public QMakeObject
{
    Q_OBJECT
public:
    enum LoadMode { ProjectMode, FileMode };
    Project(const SolutionProject &project,
            LoadMode = ProjectMode,
            QObject *parent = 0);
    virtual ~Project();

    QString nodePath() const;
    QString name() const;
    SolutionFile file() const;
    QString buildDir() const;

    bool includeFile(const SolutionFile &fileName);
    bool run(const QByteArray &code, const QString &filename);
    bool do_if (const QByteArray &code, const QString &filename, bool &rv);
    bool runExpression(QStringList &out, const QByteArray &code,
                       IInternalVariable * = 0);
    QStringList includedFiles() const;

    QStringList value(const QString &) const;
    bool isActiveConfig(const QString &) const;

    Solution *solution() const;

    void finalize();

    virtual void dump();
    void dumpRules(const QString & = QString());

    QStringList qmakeValue(const QString &);

    Rules *projectRules();

    bool virtualProject() const;
    void setVirtualProject(bool);

    SolutionProject solutionProject() const;

    QString disabledReason() const;
    void setDisabled(const QString &);

    QMakeObject *resetReason();
    void reset(const QString &reason, const QString &value = QString());

    static QStringList mkspecPaths();
    static void addMkspecPath(const QString &);

    QString messageDisplayName() const;

    QDebug warning();
    QDebug info();
    QDebug message();
    QDebug error();
    QDebug outputMessage(const QString &level);

private slots:
    void valueChanged(const QString &name, const QStringList &added,
                      const QStringList &removed);

private:
    static void primeMkspecPath();
    void init();
    void runProject();

    void runBlock(Block *);
    void runBlock(MultiBlock *);
    void runBlock(Scope *);
    bool testScope(Scope *, for_loop_t *for_loop = 0);
    void runBlock(Assignment *);
    QByteArray m_usingBlock;
    void runBlock(Function *);
    void runBlock(Script *);

    QStringList runFunction(const EvaluatableValue &,
                            QList<IInternalVariable *> * = 0);
    bool functionResultToBoolean(const QStringList &) const;

    QStringList qmakeValue(const QString &,
                           QList<IInternalVariable *> *);
    QStringList expressionValue(const Expression &,
                                QList<IInternalVariable *> * = 0);
    QStringList expressionValue(const EvaluatableValues &,
                                QList<IInternalVariable *> * = 0);
    QStringList expressionValue(const EvaluatableValue &,
                                QList<IInternalVariable *> * = 0);

    void regexpValue(const QString &property, const QString &regexp,
                     TraceContext *ctxt);

    void configChanged(const QStringList &added, const QStringList &removed);

    TraceContext traceContext(const ParserItem *) const;

    SolutionFile m_file;
    QString m_name;
    QStringList m_includedFiles;
    Rules *m_rules;

    Solution *m_solution;
    QList<IInternalVariable *> m_qmakeValues;
    QMakeObjectSubscription *m_subscription;

    bool m_finalized;
    bool m_hasWarnings;
    bool m_isVirtual;

    SolutionProject m_sp;

    bool reseting() const;

    bool m_reseting;
    QMakeObject m_resetReason;

    enum { NotDisabled, Disabled } m_disabledState;

    static QStringList mkspecSearch;

    LoadMode m_loadmode;
    QString m_buildDir;
public:
    // Used in qtscriptfunctions to call foo_init() or foo_reinit() when loading an extension.
    // Stored here because it's per-project data and needs to be cleared during init().
    QSet<QString> m_projectExtensions;
};

class IInternalVariable
{
public:
    virtual ~IInternalVariable() {}

    virtual bool value(const QString &, QStringList &rv) = 0;
};

#endif
