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

#ifndef SOLUTION_H
#define SOLUTION_H

#include <QStringList>
#include <QString>
#include <QPair>
#include <QHash>
#include <QMutex>
#include "object.h"

class QDir;

class Project;
class Solution;
class SolutionFile
{
public:
    SolutionFile();
    SolutionFile(const SolutionFile &);
    SolutionFile &operator=(const SolutionFile &);

    enum Type { Absolute = 0x01,
                Project = 0x02,
                Build = 0x04 };
    bool isDir() const;
    Type type() const;
    QString name() const;
    QString solutionPath() const;
    QString solutionDir() const;
    QString fsPath() const;
    QString fsDir() const;
    bool isValid() const;

    Solution *solution() const;

    SolutionFile canonicalPath() const;

private:
    friend class PathIterator;
    friend class Solution;

    Type m_type;
    const Solution *m_fs;
    QString m_request;
    QString m_path;
    bool m_isDir;
};
typedef QList<SolutionFile> SolutionFiles;
QDebug &operator<<(QDebug &, const SolutionFile &);

class SolutionDescriptor
{
public:
    SolutionDescriptor();
    SolutionDescriptor(const QString &rootPath);
    SolutionDescriptor(const SolutionDescriptor &);
    SolutionDescriptor &operator=(const SolutionDescriptor &);

    static bool findSolutionDescriptor(const QString &rootPath, SolutionDescriptor &);
    bool isDefault() const;

    QString rootPath() const;

    QStringList solutions() const;
    struct Path {
        Path(const QString &s, const QString &f)
            : solutionPath(s), filesystemPath(f) {}
        Path() {}
        Path(const Path &o)
            : solutionPath(o.solutionPath), filesystemPath(o.filesystemPath) {}
        Path &operator=(const Path &o) {
            solutionPath = o.solutionPath; filesystemPath = o.filesystemPath;
            return *this;
        }
        QString solutionPath;
        QString filesystemPath;
    };
    QList<Path> paths(const QString &solution) const;
    QString defaultSolution() const;

    void createSolutions() const;

private:
    QString path(const QString &, const QString &) const;

    void init();
    bool m_default;
    QHash<QString, QList<Path> > m_paths;
    QString m_rootPath;
    QString m_defaultSolution;
};


class Solution;
class PathIterator
{
public:
    PathIterator();
    PathIterator(const PathIterator &);
    PathIterator &operator=(const PathIterator &);
    PathIterator(const Solution *);
    PathIterator(const QString &);

    typedef QList<SolutionFile> Files;

    Files files() const;
    Files files(const QByteArray &) const;
    QStringList paths() const;
    QStringList paths(const QByteArray &) const;
    QString path() const;
    QStringList fsPath() const;
    PathIterator advance(const QString &path) const;
    PathIterator up() const;

    bool isValid() const;

    Solution* solution() const;
private:
    QString _absPath;
    const Solution *_solution;
    bool _isValid;
};

class SolutionDir;
class SolutionSub;
class PathGlob;
class Solution
{
public:
    static bool createDefaultSolution();
    static void setDefaultSolution(Solution *);
    static QStringList solutions();
    static Solution *solution(const QString &name);
    static Solution *defaultSolution();

    Solution(const QString &name, const QString &rootPath);
    Solution(const QString &name, const SolutionDescriptor &);
    virtual ~Solution();

    QString name() const;

    void dump();

    enum Type {
                None     = 0x0000,
                _Project  = 0x0001,
                Absolute = 0x0008,
                Wildcard = 0x0010,

                All = 0x0019
              };
    QString filesystemBuildPath(const QString &name,
                                const SolutionFile &relative) const;
    SolutionFile buildFile(const QString &name,
                           const SolutionFile &relative) const;
    SolutionFile file(const QString &name,
                      Type, // All types
                      const SolutionFile &relative) const;
    SolutionFiles files(const QString &name,
                        Type, // All types
                        const SolutionFile &relative) const;
    QStringList paths(const QString &name,
                      Type,
                      const SolutionFile &relative) const;

    SolutionFiles _paths(const QString &name,
                         Type,
                         const SolutionFile &relative) const;

    QStringList subPaths(const QString &name,
                         Type,
                         const SolutionFile &relative) const;
    QStringList filesystemPaths(const QString &name,
                                Type,
                                const SolutionFile &relative) const;

private:
    bool checkNameForSolution(const QString &name, QString &solution, QString &path) const;
    SolutionFiles filesRecur(const PathIterator &, const PathGlob &, int) const;
    SolutionFiles _pathsRecur(const PathIterator &, const PathGlob &, int) const;
    QStringList pathsRecur(const PathIterator &, const PathGlob &, int) const;

public:

    enum FileType { Project, Generated, Existing };
    SolutionFile findFile(const QString &,
                          FileType = Project,
                          const SolutionFile & = SolutionFile()) const;
    QStringList pathMappings(const QString &);
    QStringList paths(const QString &,
                      const SolutionFile & = SolutionFile()) const;
    QStringList files(const QString &) const;

    QList<SolutionFile> commonIncludes() const;
    QList<SolutionFile> blankIncludes() const;
    QList<SolutionFile> disabledIncludes() const;
    QList<SolutionFile> defaultIncludes() const;

    SolutionFile realToSolution(const QString &);
    SolutionFile fuzzyRealToSolution(const QString &);

    void addPath(const QString &, const QString &);

private:
    QString m_name;

    SolutionFile findFile(const QString &,
                         FileType,
                         const SolutionFile &,
                         bool fallback) const;

    SolutionFile findAbsFile(const QString &, const SolutionFile &, bool exist) const;

    void dump(int);

    friend class PathIterator;
    QList<SolutionDir *> m_paths;

    static Solution *m_defaultSolution;
    static QHash<QString, Solution *> m_solutions;

    void primeIncludes() const;
    mutable bool m_includesPrimed;
    mutable QList<SolutionFile> m_commonIncludes;
    mutable QList<SolutionFile> m_blankIncludes;
    mutable QList<SolutionFile> m_disabledIncludes;
    mutable QList<SolutionFile> m_defaultIncludes;
    mutable QMutex m_lock;
};

class QMakeObject;
class SolutionProject
{
public:
    static SolutionProject fromNode(const QString &, Solution * = 0);
    static SolutionProject fromNode(const QString &, Project *);
    static SolutionProject fromSolutionFile(const QString &, Solution * = 0);

    SolutionProject();
    SolutionProject(const SolutionProject &other);
    SolutionProject &operator=(const SolutionProject &other);

    bool isValid() const;

    bool isSubproject(const SolutionProject &) const;

    bool fileMode() const;
    void setFileMode(bool);

    /* The reference node name */
    QString node() const;
    /* The node path */
    QString nodePath() const;
    /* The filename within the node path */
    QString fileName() const;
    /* The project file */
    SolutionFile projectFile() const;

    QStringList subProjects() const;
    SolutionProject subProject(const QString &) const;

    Project *project() const;
    Solution *solution() const;

private:
    SolutionProject(Solution *, const QString &node);

    static void splitNodeName(const QString &node, QString &solution,
                              QString &project);
    static QString simplifyNodeName(const QString &node);

    // Passed to constructor
    Solution *m_solution;
    mutable QString m_nodePath;
    mutable QString m_fileName;

    mutable QString m_node;

    void primeFile() const;

    void prime() const;
    bool search(const QString &) const;

    mutable bool m_filemode;
    mutable bool m_primed;
    mutable QStringList m_subProjects;

    mutable bool m_filePrimed;
    mutable SolutionFile m_file;

};

class SolutionDir
{
public:
    SolutionDir(const QString &location, const QString &path);

    const QString &location() const;
    const QString &path() const;

    bool findFile(const QString &req, QString &rv) const;
    QStringList paths(const QString &req) const;
    QStringList files(const QString &req) const;

    QString mappedPath(const QString &req) const;
private:

    QString m_location;
    QString m_path;
};

class PathGlob
{
public:
    PathGlob();
    PathGlob(const QString &);

    int depth() const;
    bool match(const QString &, int depth) const;
    bool isConst(int depth) const;
    QByteArray glob(int depth) const;

private:
    PathGlob(const PathGlob &);
    PathGlob &operator=(const PathGlob &);

    QList<QByteArray> words;
    QList<bool> consts;
};

#endif
