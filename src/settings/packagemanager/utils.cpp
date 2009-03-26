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

#include "utils.h"

#include <QFileSystem>
#include <Qtopia>
#include <QRegExp>
#include <math.h>
#include <QProcess>
#include <QDebug>
#include <sys/vfs.h>

namespace SizeUtils
{
#ifndef PACKAGEMANAGER_UTILS_TEST
/*  \internal
    \a path must refer to an existing
    file or directory */
    qulonglong availableSpace( QString path )
    {
        struct statfs stats;
        statfs( path.toLocal8Bit(), &stats);
        qulonglong bavail = (qulonglong)stats.f_bavail;
        qulonglong bsize = (qulonglong)stats.f_bsize;

        return bavail * bsize;
    }

#endif

    bool isSufficientSpace( const InstallControl::PackageInfo &pkgInfo,
            ErrorReporter *reporter)
    {
        bool ok;
        //qpk downloadSize is always in bytes, see mkPackages script
        qulonglong pkgFileSize = (qulonglong)pkgInfo.downloadSize.toLongLong( &ok );

        QString installedSizeStr = pkgInfo.installedSize;
        qulonglong installedSize =  parseInstalledSize(installedSizeStr);

        qulonglong reqSize = reqInstallSpace(pkgInfo);

        qulonglong availableInstallSpace =
            availableSpace(Qtopia::packagePath());

        QString simpleError;
        QString detailedError;

        if (!ok)
        {
            simpleError =  QObject::tr("Invalid package download size supplied, contact package supplier");
            detailedError ="SizeUtils::isSufficientSpace- Invalid package size supplied: %1";
            detailedError = detailedError.arg(pkgInfo.downloadSize);
            goto size_error;
        }

        if (installedSize  < 1)
        {
            simpleError =  QObject::tr( "Package did not supply valid installed size, contact package supplier" );
            detailedError = "SizeUtils::isSufficientSpace:- Package supplied invalid size %1";
            detailedError = detailedError.arg(installedSizeStr);
            goto size_error;
        }

        if (pkgFileSize >= installedSize)
        {
            simpleError =  QObject::tr("Invalid package, contact package supplier");
            detailedError = "SizeUtils::isSufficientSpace():- download file size >= installed size, "
                "download size = %1, installed size = %2 ";
            detailedError = detailedError.arg(pkgFileSize).arg(installedSize);
            goto size_error;
        }

        if ( availableInstallSpace < reqSize)
        {
            QStorageMetaInfo * storage = QStorageMetaInfo::instance();
            const QFileSystem * fs = storage->fileSystemOf(Qtopia::packagePath());

            simpleError =  QObject::tr("Not enough space to install package, free up %1 from %2",
                            "%1=size eg 2MB, %2=filesystem");
            simpleError = simpleError.arg( getSizeString(reqSize) ).arg(fs->name());
            detailedError = "SizeUtils::isSufficientSpace:- Insufficient space to install package "
                "to %1, required space= %2, available space =%3";
            detailedError = detailedError.arg(Qtopia::packagePath())
                                        .arg(reqSize).arg(availableInstallSpace);
            goto size_error;
        }

        return true;

size_error:
        if (reporter)
            reporter->reportError(simpleError, detailedError);
        return false;
    }

    qulonglong reqInstallSpace(const InstallControl::PackageInfo &pkgInfo)
    {
        //The required install spacee is the extracted size plus the size of
        //the data.tar.gz(approx. qpk's size) and control file.
        //All these will for a short period of time
        //exist together on the filesystem.
        //It is expected the control file is no larger than 10kb
        return parseInstalledSize(pkgInfo.installedSize)
                + pkgInfo.downloadSize.toLongLong() + 10 * 1024;
    }



    //get a human readable string representing the size
    //eg 10MB, 200KB etc
    QString getSizeString( qulonglong size )
    {
        double s = size;
        double scaledSize = s;
        QString suffix;
        bool skip = false;
        if ( s < 0 ) s = 0;
        if ( s < 1024 ) {
            suffix = QObject::tr("B","bytes");
            scaledSize = s;
            skip = true;
        }
        s /= 1024;
        if ( skip == false && s < 1024 ) {
            suffix = QObject::tr("KB","kilobytes");
            scaledSize = s;
            skip = true;
        }
        s /= 1024;
        if ( skip == false && s < 1024 ) {
            suffix = QObject::tr("MB","megabytes");
            scaledSize = s;
            skip = true;
        }
        s /= 1024;
        if ( skip == false && s < 1024 ) {
            suffix = QObject::tr("GB","gigabytes");
            scaledSize = s;
            skip = true;
        }
        return QString().sprintf("%0.2f",scaledSize ) + suffix;

    }

    //Converts a string eg 2M into the equivalent number of bytes
    //returns 0 if installedSize parameter is invalid.
    qulonglong parseInstalledSize( QString installedSize )
    {
        QRegExp rx("^(\\d+\\.?\\d+)([km]?)$");
        installedSize = installedSize.trimmed().toLower();
        long multiplier;
        if ( rx.indexIn(installedSize) !=0 )
            return 0;

        QStringList captures = rx.capturedTexts();
        if ( !captures.count() == 3 )
            return 0;

        if ( captures[2].isEmpty()  )
            multiplier = 1;
        else if ( captures[2] == "k" )
            multiplier = 1024;
        else if ( captures[2] == "m" )
            multiplier = 1024 * 1024;
        else
            return 0;

        bool ok = false;
        double d = captures[1].toDouble( &ok );
        if ( !ok )
            return 0;
        double rounded = round( d );
        if ( floor(d) == rounded )
            rounded = rounded + 0.5;
        return (qulonglong)(rounded * multiplier) ;
    }

    qulonglong usedSpace(QString path)
    {

        static qulonglong bsize = 0;
        if (bsize == 0) {
            struct statfs stats;
            statfs( Qtopia::packagePath().toLocal8Bit(), &stats);
            bsize = (qulonglong)stats.f_bsize;
        }

        QDir dir(path);
        if (!dir.exists()) {
            return 0;
        }

        qulonglong space = 0;
        foreach( QFileInfo fi, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files
                                                | QDir::Dirs | QDir::System)){
            if (fi.isSymLink()){
                space += bsize;
                continue;
            }

            if ( fi.isDir() )
                space += usedSpace(fi.canonicalFilePath());
            space +=(qulonglong)(ceil((double)fi.size() / bsize) * bsize);
        }
        return space;
    }
}
bool LidsUtils::isLidsEnabled()
{
    return QFile::exists("/proc/sys/lids/locks");
}


/*
    Returns true if the maximum number of lids rules has been exceeded
*/
/*
    Implementation note: having a single number as the maximum number of
    lids rules is major simplification of how lids works.  Lids has
    limits on the number of subjects, number of objects, number of
    protected inodes.  For simplicity sake we use a single (configuration)
    value based on constants in the lids patch:
    CONFIG_LIDS_MAX_INODE
    CONFIG_LIDS_MAX_SACL
    CONFIG_LIDS_MAX_OACL
*/
bool LidsUtils::maxRulesExceeded()
{
    static int maxLidsRules=-1;

    if (maxLidsRules == -1)
    {
        QSettings conf( "Trolltech", "PackageManager" );
        conf.beginGroup("Configuration");
        bool b;
        maxLidsRules = conf.value("MaxLidsRules").toInt(&b);
        if (!b || maxLidsRules < 0)
        {
            maxLidsRules = 512;
            qWarning()  << "Invalid/Missing MaxLidsRules field in PackageManager settings file, "
                        << "using default value of " << maxLidsRules;
        }
    }

    QProcess lidsconf;
    lidsconf.start("lidsconf -L");
    if (lidsconf.waitForFinished())
    {
        QStringList output(QString(lidsconf.readAll()).split("\n"));
        if(!output.contains ("Killed"))
        {
            if ((output.count() - 5) > maxLidsRules)
                return true;
            else
                return false;
        }
    }

    //execution shouldn't get here
    //but return true to be on the "safe side"
    //(and not allow extra packages to be installed)
    return true;
}

namespace ScriptRunner
{
    void runScript(const QString &cmd)
    {
        QProcess process;
        QEventLoop eventLoop;
        QObject::connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),
                    &eventLoop, SLOT(quit()));
        QObject::connect(&process, SIGNAL(error(QProcess::ProcessError)),
                    &eventLoop, SLOT(quit()));
        process.start(cmd);
        eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
    }
}
