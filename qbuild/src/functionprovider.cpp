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

#include "functionprovider.h"
#include "qfastdir.h"
#include "qoutput.h"

static QList<FunctionProvider *> *providers = 0;

/*!
  \class FunctionProvider
  \brief The FunctionProvider class exports functions into the QBuild environment.

  Function providers are typically instantiated like this.
  \code
  static MyFunctionProvider myFunctionProvider;
  \endcode
*/

/*!
  Construct a FunctionProvider with the specified \a type and \a priority.
  Higher priority values take precedence over lower priority values.
  Valid priority values are positive numbers greater than 0. If an invalid
  priority is passed, the provider is added at the end of the list.
*/
FunctionProvider::FunctionProvider(const QString &type, int priority)
: m_type(type), m_priority(priority)
{
    if (!providers)
        providers = new QList<FunctionProvider *>();

    if ( priority > 0 ) {
        for (int ii = 0; ii < providers->count(); ++ii) {
            if (priority > providers->at(ii)->m_priority) {
                providers->insert(ii, this);
                return;
            }
        }
    }
    providers->append(this);
}

/*!
  Destruct the FunctionProvider, removing it from the list of providers.
*/
FunctionProvider::~FunctionProvider()
{
    providers->removeAll(this);
}

/*!
  Returns the name of the function provider.
*/
QString FunctionProvider::type() const
{
    return m_type;
}

/*!
  Returns the list of function providers ordered by priority.
*/
QList<FunctionProvider *> FunctionProvider::functionProviders()
{
    return *providers;
}

/*!
  Attempt to execute function \a function with arguments \a arguments.
  The function providers are queried in order. They are passed the \a project.
  The return value is set via \a rv. Returns true if the function was called,
  false otherwise.
  \sa callFunction()
*/
bool FunctionProvider::evalFunction(Project *project,
                                    QStringList &rv,
                                    const QString &function,
                                    const QStringLists &arguments)
{

    for (int ii = 0; ii < providers->count(); ++ii) {
        FunctionProvider *provider = providers->at(ii);
        if (provider->callFunction(project, rv, function, arguments))
            return true;
    }
    return false;
}

/*!
  \fn FunctionProvider::callFunction(Project *project, QStringList &rv, const QString &function, const QStringLists &arguments)
  This runs \a function with \a arguments.
  The function is passed the \a project and returns data via \a rv.
  Returns true if the function exists, false otherwise.
*/

/*!
  Reset project \a project. The providers will clear out any associated data.
  \sa resetProject()
*/
void FunctionProvider::reset(Project *project)
{
    foreach(FunctionProvider *prov, *providers)
        prov->resetProject(project);
}

/*!
  \fn FunctionProvider::resetProject(Project *project)
  This function is called because \a project has been reset. Any state associated with this project
  should be cleared.
*/

QStringList FunctionProvider::m_extensionsPaths;
QMutex FunctionProvider::m_lock;
FunctionProvider::ExtensionCache FunctionProvider::m_extensionCache;

/*!
  Returns the list of paths that extensions can be found in.
*/
QStringList FunctionProvider::extensionsPaths()
{
    return m_extensionsPaths;
}

/*!
  Add \a path to the list of paths that extensions can be found in.
*/
void FunctionProvider::addExtensionsPath(const QString &path)
{
    QString addPath = path;
    if (!addPath.endsWith("/"))
        addPath.append("/");
    m_extensionsPaths.prepend(addPath);
}

/*!
  Load \a name as an extension for \a project. The \a name is either a path to
  a file or a name to search for in the extensions paths.
  Returns true if the extension was loaded, false otherwise.
*/
bool FunctionProvider::evalLoad(Project *project,
                                const QString &name)
{
    // Try in the extensions directory - assuming it isn't an absolution
    // name
    if (name.startsWith('/')) {
        SolutionFile file = project->solution()->file(name, Solution::All, project->file());
        if (file.isValid()) {
            for (int ii = 0; ii < providers->count(); ++ii) {
                FunctionProvider *provider = providers->at(ii);
                if (provider->loadFile(file.fsPath(), project)) {
                    return true;
                }
            }
        }
        return false;
    }

    m_lock.lock();
    ExtensionCache::Iterator iter = find(qMakePair(name, project->solution()));
    QPair<QString, FunctionProvider *> prov = *iter;
    m_lock.unlock();

    bool rv = false;
    if (prov.second) {
        if (prov.second->loadFile(prov.first, project)) {
            rv = true;
        } else {
            qWarning() << "Unable to load" << prov.first;
        }
    }

    return rv;
}

/*!
  \internal
  Call locked
*/
FunctionProvider::ExtensionCache::Iterator FunctionProvider::find(const QPair<QString, Solution *> &name)
{
    ExtensionCache::Iterator iter = m_extensionCache.find(name);


    if (iter == m_extensionCache.end()) {
        foreach(QString path, m_extensionsPaths) {
            QString baseName = path + name.first + ".";
            for (int ii = 0; ii < providers->count(); ++ii) {
                FunctionProvider *provider = providers->at(ii);
                QString provExt = provider->fileExtension();
                if (provExt.isEmpty())
                    continue;
                QString provName = baseName + provExt;
                SolutionFile file = name.second->findFile(provName, Solution::Existing);
                if (file.isValid()) {
                    return m_extensionCache.insert(name, qMakePair(file.fsPath(), provider));
                }
            }
        }

        if (iter == m_extensionCache.end())
            iter = m_extensionCache.insert(name, qMakePair(QString(), (FunctionProvider *)0));
    }

    return iter;
}

/*!
  \fn FunctionProvider::fileExtension() const
  Returns the extension of files that this FunctionProvider can load.
*/

/*!
  \fn FunctionProvider::loadFile(const QString &file, Project *project)
  Load extension \a file for \a project. Returns true if successful, false otherwise.
*/

