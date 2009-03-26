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

#ifndef UTILS_H
#define UTILS_H

#include <qstring.h>
#include "installcontrol.h"

namespace SizeUtils
{
    bool isSufficientSpace(const InstallControl::PackageInfo &pkgInfo,
                            ErrorReporter *reporter);
    qulonglong reqInstallSpace(const InstallControl::PackageInfo &pkgInfo);
    qulonglong availableSpace(QString path);
    QString getSizeString(qulonglong size);
    qulonglong parseInstalledSize(QString installedSize);
    qulonglong usedSpace(QString path);
}

namespace LidsUtils
{
    bool maxRulesExceeded();
    bool isLidsEnabled();
};

namespace ScriptRunner
{
    void runScript(const QString &cmd);
}

#endif
