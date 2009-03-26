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

#ifndef OPTIONS_H
#define OPTIONS_H

struct Options
{
    Options()
        : showParse(false),
          showFinalize(false),
          showRules(false),
          showTokens(false),
          showParseTree(false),
          showSolutions(false),
          dumpSolution(false),
          showExtensions(false),
          showMkspecs(false),
          showActions(false),
          noRun(false),
          showGui(false),
          showProgress(false),
          executeRules(true),
          debug(None),
          threads(0),
          filename(0),
          nodename(0),
          rootdir(0),
          rule(""),
          freshenFile(0),
          dumpSolutionName(0),
          verbose(false),
          silent(true),
          printOnly(false),
          forceRule(false),
          traceFile(0) {}

    bool showParse;
    bool showFinalize;
    bool showRules;
    bool showTokens;
    bool showParseTree;
    bool showSolutions;
    bool dumpSolution;
    bool showExtensions;
    bool showMkspecs;
    bool showActions;
    bool noRun;
    bool showGui;
    bool showProgress;

    bool executeRules;

    enum DebugModules {
        None = 0x00000000,
        Finalize = 0x00000001,
        Trace = 0x00000002,
        RuleExec = 0x00000004,
        RunTrace = 0x00000008,
        StackOnFatal = 0x00000010,
        PerfTiming = 0x00000020,
        All = 0xFFFFFFFF
    };
    DebugModules debug;

    int threads;

    QString filename;
    const char * nodename;
    const char * rootdir;
    const char * rule;
    const char * freshenFile;
    const char * dumpSolutionName;

    bool verbose;
    bool silent;
    bool printOnly;
    bool forceRule;

    QHash<QString,int> throttle;

    const char *traceFile;
};

extern Options options;
#define RUN_TRACE if (options.debug & Options::RunTrace)

#endif
