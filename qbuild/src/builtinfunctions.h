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

#ifndef BUILTINFUNCTIONS_H
#define BUILTINFUNCTIONS_H

#include "functionprovider.h"

class BuiltinFunctions : public FunctionProvider
{
    Q_OBJECT
public:
    BuiltinFunctions();

    virtual QString fileExtension() const;
    virtual bool loadFile(const QString &, Project *);
    virtual void resetProject(Project *);

    virtual bool callFunction(Project *project,
                              QStringList &rv,
                              const QString &function,
                              const QStringLists &arguments);
};

#endif
