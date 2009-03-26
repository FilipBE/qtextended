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

#ifndef QMEMORYFILE_P_H
#define QMEMORYFILE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qglobal.h>
#include <qstring.h>
#include <qtopiaglobal.h>

class QMemoryFileData;

class QTOPIA_AUTOTEST_EXPORT QMemoryFile
{
public:

    // Documented in qmemoryfile.cpp
    enum Flags {
        Write           = 0x00000001,
        Shared          = 0x00000002,
        Create          = 0x00000004,
    };

    explicit QMemoryFile(const QString &fileName, int flags=-1, uint size=0);
    ~QMemoryFile();

    bool isShared() {return (flags & QMemoryFile::Shared) != 0;}
    bool isWritable() { return (flags & QMemoryFile::Write) != 0;}
    uint size() { return  length;}
    char *data() { return block; }

private:
    QMemoryFileData *openData (const QString &fileName, int flags, uint size);
    void closeData(QMemoryFileData *memoryFile);

protected:
    char* block;
    uint length;
    uint flags;
    QMemoryFileData* d;
};

#endif
