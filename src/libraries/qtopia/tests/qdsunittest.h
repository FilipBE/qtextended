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

#ifndef QDSUNITTEST_H
#define QDSUNITTEST_H

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QCoreApplication>

static QString test_dir;

void rmtree(QString const &path)
{
    if (QFileInfo(path).isDir()) {
        foreach (QString s, QDir(path).entryList(QDir::AllEntries|QDir::NoDotAndDotDot)) {
            rmtree( path + "/" + s );
        }
        if (!QDir("/").rmdir(path))
            qWarning("Couldn't remove dir %s", qPrintable(path));
    } else {
        if (!QFile::remove(path))
            qWarning("Couldn't remove %s", qPrintable(path));
    }
}


void destroyTestDir()
{
    if (!test_dir.isEmpty())
        rmtree(test_dir);
}

#ifndef Q_QDOC
namespace Qtopia {
QString qtopiaDir()
{
    if (test_dir.isEmpty()) {
        {
            QTemporaryFile tf(QDir::tempPath() + "/tst_qdsaction");
            if (!tf.open()) qFatal("Can't make temp dir");
            test_dir = tf.fileName();
        }
        QDir("/").mkpath(test_dir + "/services");
        if (!test_dir.endsWith("/")) test_dir += "/";
        qAddPostRoutine(destroyTestDir);
    }

    return test_dir;
}
};
#endif

#endif

