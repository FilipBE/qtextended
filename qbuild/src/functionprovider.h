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

#ifndef FUNCTIONPROVIDER_H
#define FUNCTIONPROVIDER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QMutex>
#include "project.h"

typedef QList<QStringList> QStringLists;
class FunctionProvider : public QObject
{
Q_OBJECT
public:
    FunctionProvider(const QString &type, int priority = 0);
    virtual ~FunctionProvider();

    QString type() const;

    static QList<FunctionProvider *> functionProviders();
    static bool evalFunction(Project *project,
                             QStringList &rv,
                             const QString &function,
                             const QStringLists &arguments);
    static void reset(Project *);
    static bool evalLoad(Project *project,
                         const QString &name);
    static QStringList extensionsPaths();
    static void addExtensionsPath(const QString &path);

    virtual QString fileExtension() const = 0;
    virtual bool loadFile(const QString &, Project *) = 0;
    virtual void resetProject(Project *) = 0;

    virtual bool callFunction(Project *project,
                              QStringList &rv,
                              const QString &function,
                              const QStringLists &arguments) = 0;

private:
    QString m_type;
    int m_priority;
    static QStringList m_extensionsPaths;
    static QMutex m_lock;
    typedef QHash<QPair<QString, Solution *>, QPair<QString, FunctionProvider *> > ExtensionCache;
    static ExtensionCache::Iterator find(const QPair<QString, Solution *> &);
    static ExtensionCache m_extensionCache;
};

#endif
