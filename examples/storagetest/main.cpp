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

#include <QtopiaApplication>
#include <QStorageMetaInfo>
#include <QDebug>

int main( int argc, char **argv)
{
    QtopiaApplication app(argc, argv);
    QFileSystemFilter fsf;
    foreach ( QFileSystem *fs, QStorageMetaInfo::instance()->fileSystems(&fsf) ) {
        qWarning() << "Name : " << fs->name();
        qWarning() << "Disk : " << fs->disk();
        qWarning() << "Path : " << fs->path();
        qWarning() << "Options : " << fs->options();
        qWarning() << "Available blocks : " << fs->availBlocks();
        qWarning() << "Total blocks : " << fs->totalBlocks();
        qWarning() << "Block size : " << fs->blockSize();
        qWarning() << "Is removable : " << fs->isRemovable();
        qWarning() << "Is writeable : " << fs->isWritable();
        qWarning() << "Contains documents : " << fs->documents();
    }
    return 0;
}
