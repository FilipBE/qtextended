/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include <qstring.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE


enum Compiler {
    CC_UNKNOWN = 0,
    CC_BORLAND = 0x01,
    CC_MINGW   = 0x02,
    CC_INTEL   = 0x03,
    CC_MSVC4   = 0x40,
    CC_MSVC5   = 0x50,
    CC_MSVC6   = 0x60,
    CC_NET2002 = 0x70,
    CC_NET2003 = 0x71,
    CC_NET2005 = 0x80,
    CC_NET2008 = 0x90
};

struct CompilerInfo;
class Environment
{
public:
    static Compiler detectCompiler();
    static QString detectQMakeSpec();
    static bool detectExecutable(const QString &executable);

    static int execute(QStringList arguments, const QStringList &additionalEnv, const QStringList &removeEnv);
    static bool cpdir(const QString &srcDir, const QString &destDir);
    static bool rmdir(const QString &name);

private:
    static Compiler detectedCompiler;

    static CompilerInfo *compilerInfo(Compiler compiler);
    static QString keyPath(const QString &rKey);
    static QString keyName(const QString &rKey);
    static QString readRegistryKey(HKEY parentHandle, const QString &rSubkey);
};


QT_END_NAMESPACE
