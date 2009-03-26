/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the qmake application of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef WINMAKEFILE_H
#define WINMAKEFILE_H

#include "makefile.h"

QT_BEGIN_NAMESPACE

// In the Qt evaluation and educational version, we have a postfix in the
// library name (e.g. qtmteval301.dll). QTDLL_POSTFIX is used for this.
// A script modifies these lines when building eval/edu version, so be careful
// when changing them.
#ifndef QTDLL_POSTFIX
#define QTDLL_POSTFIX ""
#endif

class Win32MakefileGenerator : public MakefileGenerator
{
public:
    Win32MakefileGenerator();
    ~Win32MakefileGenerator();
protected:
    virtual QString defaultInstall(const QString &);
    virtual void writeCleanParts(QTextStream &t);
    virtual void writeStandardParts(QTextStream &t);
    virtual void writeIncPart(QTextStream &t);
    virtual void writeLibDirPart(QTextStream &t);
    virtual void writeLibsPart(QTextStream &t);
    virtual void writeObjectsPart(QTextStream &t);
    virtual void writeImplicitRulesPart(QTextStream &t);
    virtual void writeBuildRulesPart(QTextStream &);
    virtual QString escapeFilePath(const QString &path) const;

    virtual void writeRcFilePart(QTextStream &t);

    int findHighestVersion(const QString &dir, const QString &stem, const QString &ext = QLatin1String("lib"));
    bool findLibraries(const QString &);
    virtual bool findLibraries();

    virtual void processPrlFiles();
    virtual void processVars();
    virtual void fixTargetExt();
    virtual void processRcFileVar();
    virtual void processFileTagsVar();
    virtual QString getLibTarget();
};

inline Win32MakefileGenerator::~Win32MakefileGenerator()
{ }

inline bool Win32MakefileGenerator::findLibraries()
{ return findLibraries("QMAKE_LIBS"); }

QT_END_NAMESPACE

#endif // WINMAKEFILE_H
