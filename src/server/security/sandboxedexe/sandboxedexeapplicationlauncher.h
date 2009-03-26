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

#ifndef SANDBOXEDEXEAPPLICATIONLAUNCHER_H
#define SANDBOXEDEXEAPPLICATIONLAUNCHER_H

#include "applicationlauncher.h"

class SandboxedExeApplicationLauncherPrivate;
class SandboxedExeApplicationLauncher: public SimpleExeApplicationLauncher
{
Q_OBJECT
public:
    SandboxedExeApplicationLauncher();
    virtual ~SandboxedExeApplicationLauncher();

    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);
    virtual QString name() {
	return QString("SandboxedExeApplicationLauncher");
    }
private slots:
    void init();
private:
    static QStringList applicationExecutable(const QString &app);

    SandboxedExeApplicationLauncherPrivate *d;
};

#endif
