/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QFILEINFOGATHERER_H
#define QFILEINFOGATHERER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qfilesystemwatcher.h>
#include <qfileiconprovider.h>
#include <qpair.h>
#include <qdatetime.h>
#include <qstack.h>
#include <qdir.h>

QT_BEGIN_NAMESPACE

class QExtendedInformation {
public:
    enum Type { Dir, File, System };

    QExtendedInformation() : size(0), fileType(System), isHidden(false),
                             isSymLink(false), caseSensitive(true) {}

    qint64 size;
    QString displayType;
    QIcon icon;
    QDateTime lastModified;
    QFile::Permissions permissions;
    Type fileType;
    bool isHidden : 1;
    bool isSymLink : 1;
    bool caseSensitive : 1;

    inline bool isDir() { return fileType == Dir; }
    inline bool isFile() { return fileType == File; }
    inline bool isSystem() { return fileType == System; }

    bool operator ==(const QExtendedInformation &fileInfo) const {
       return fileInfo.size == size
       && fileInfo.displayType == displayType
       && fileInfo.lastModified == lastModified
       && fileInfo.permissions == permissions
       && fileInfo.fileType == fileType
       && fileInfo.isHidden == isHidden
       && fileInfo.isSymLink == isSymLink
       && fileInfo.caseSensitive == caseSensitive;
    }
    void operator =(const QExtendedInformation &fileInfo) {
        size = fileInfo.size;
        displayType = fileInfo.displayType;
        icon = fileInfo.icon;
        lastModified = fileInfo.lastModified;
        permissions = fileInfo.permissions;
        fileType = fileInfo.fileType;
        isHidden = fileInfo.isHidden;
        isSymLink = fileInfo.isSymLink;
        caseSensitive = fileInfo.caseSensitive;
    }
};

class QFileIconProvider;

#ifndef QT_NO_FILESYSTEMMODEL

class Q_AUTOTEST_EXPORT QFileInfoGatherer : public QThread
{
Q_OBJECT

Q_SIGNALS:
    void updates(const QString &directory, const QList<QPair<QString, QFileInfo> > &updates);
    void newListOfFiles(const QString &directory, const QStringList &listOfFiles) const;
    void nameResolved(const QString &fileName, const QString &resolvedName) const;

public:
    QFileInfoGatherer(QObject *parent = 0);
    ~QFileInfoGatherer();

    void clear();
    QExtendedInformation getInfo(const QFileInfo &info) const;

public Q_SLOTS:
    void list(const QString &directoryPath);
    void fetchExtendedInformation(const QString &path, const QStringList &files);
    void updateFile(const QString &path);
    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;
    void setIconProvider(QFileIconProvider *provider);
    QFileIconProvider *iconProvider() const;

protected:
    void run();
    void getFileInfos(const QString &path, const QStringList &files);

private:
    void fetch(const QFileInfo &info, QTime &base, bool &firstTime, QList<QPair<QString, QFileInfo> > &updatedFiles, const QString &path);
    QString translateDriveName(const QFileInfo &drive) const;
    QFile::Permissions translatePermissions(const QFileInfo &fileInfo) const;

    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    QStack<QString> path;
    QStack<QStringList> files;

#ifndef QT_NO_FILESYSTEMWATCHER
    QFileSystemWatcher *watcher;
#endif
    bool m_resolveSymlinks;
    QFileIconProvider *m_iconProvider;
    QFileIconProvider defaultProvider;
#ifndef Q_OS_WIN
    uint userId;
    uint groupId;
#endif
};
#endif // QT_NO_FILESYSTEMMODEL


QT_END_NAMESPACE
#endif // QFILEINFOGATHERER_H

