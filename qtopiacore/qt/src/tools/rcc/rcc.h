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

#ifndef RCC_H
#define RCC_H

#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QHash>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

struct RCCFileInfo;
class QIODevice;
class RCCResourceLibrary;
class QTextStream;

class RCCBuilder
{
public:
    RCCBuilder();

    void initializeLibrary(RCCResourceLibrary &lib) const;
    bool processResourceFile(const QStringList &filenamesIn, const QString &filenameOut, bool list) const;
    bool processResourceFile(const QStringList &filenamesIn, const QString &filenameOut, bool list, QIODevice &errorDevice) const;


public:
    bool writeBinary;
    QString initName;
    bool verbose;
    int compressLevel;
    int compressThreshold;
    bool useNameSpace;
    QString resourceRoot;
};

class RCCResourceLibrary
{
    RCCResourceLibrary(const RCCResourceLibrary &);
    RCCResourceLibrary &operator=(const RCCResourceLibrary &);

public:
    RCCResourceLibrary();
    ~RCCResourceLibrary();

    bool output(QIODevice &out, QIODevice &errorDevice);

    bool readFiles(bool ignoreErrors, QIODevice &errorDevice);

    enum Format { Binary, C_Code };
    inline void setFormat(Format f) { mFormat = f; }
    inline Format format() const { return mFormat; }

    inline void setInputFiles(QStringList files) { mFileNames = files; }
    inline QStringList inputFiles() const { return mFileNames; }

    QStringList dataFiles() const;

    // Return a map of resource identifier (':/newPrefix/images/p1.png') to file.
    typedef QHash<QString, QString> ResourceDataFileMap;
    ResourceDataFileMap resourceDataFileMap() const;

    inline void setVerbose(bool b) { mVerbose = b; }
    inline bool verbose() const { return mVerbose; }

    inline void setInitName(const QString &n) { mInitName = n; }
    inline QString initName() const { return mInitName; }

    inline void setCompressLevel(int c) { mCompressLevel = c; }
    inline int compressLevel() const { return mCompressLevel; }

    inline void setCompressThreshold(int t) { mCompressThreshold = t; }
    inline int compressThreshold() const { return mCompressThreshold; }

    inline void setResourceRoot(const QString &str) { mResourceRoot = str; }
    inline QString resourceRoot() const { return mResourceRoot; }
    
    inline void setUseNameSpace(bool v) { mUseNameSpace = v; }
    inline bool useNameSpace() const { return mUseNameSpace; }
    
    QStringList failedResources() const { return mFailedResources; }

private:
    struct Strings {
        Strings();
        const QString TAG_RCC;
        const QString TAG_RESOURCE;
        const QString TAG_FILE;
        const QString ATTRIBUTE_LANG;
        const QString ATTRIBUTE_PREFIX;
        const QString ATTRIBUTE_ALIAS;
        const QString ATTRIBUTE_THRESHOLD;
        const QString ATTRIBUTE_COMPRESS;
    };
    const Strings m_strings;
    RCCFileInfo *root;
    void reset();
    bool addFile(const QString &alias, const RCCFileInfo &file);
    bool interpretResourceFile(QIODevice *inputDevice, const QString &file,
        QString currentPath = QString(), bool ignoreErrors = false);
    bool writeHeader(QIODevice &out);
    bool writeDataBlobs(QIODevice &out);
    bool writeDataNames(QIODevice &out);
    bool writeDataStructure(QIODevice &out);
    bool writeInitializer(QIODevice &out);

    QStringList mFileNames;
    QString mResourceRoot, mInitName;
    Format mFormat;
    bool mVerbose;
    int mCompressLevel;
    int mCompressThreshold;
    int mTreeOffset, mNamesOffset, mDataOffset;
    bool mUseNameSpace;
    QStringList mFailedResources;
    QIODevice *m_errorDevice;
};

QT_END_NAMESPACE

#endif // RCC_H
