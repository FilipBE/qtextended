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

#include "packageinformationreader.h"
#include "packagecontroller.h"

#include <QFile>
#include <QTextStream>
#include <qtopialog.h>
#include <Qtopia>
#include <sys/vfs.h>

/**
  Base constructor
*/
PackageInformationReader::PackageInformationReader( InstallControl::PackageInfo::Source src )
    : isError( false )
    , hasContent( false )
    , wasSizeUpdated( false )
    , accumulatingFullDesc( false )
    , source( src )
{
    reset();
}

/**
  Take a stream onto a control file and return the decoded
  package information.

  Set isError == true if the information format is wrong.
*/
PackageInformationReader::PackageInformationReader( QTextStream &ts,
                                                    InstallControl::PackageInfo::Source src )
    : isError( false )
    , hasContent( false )
    , wasSizeUpdated( false )
    , accumulatingFullDesc( false )
    , source( src )
{
    reset();
    while (!ts.atEnd())
    {
        QString line = ts.readLine();
        readLine( line );
    }
    updateInstalledSize();

    if ( !pkg.isComplete(source, &error) )
        isError = true;
}

/**
  Take a filename of a control file and return the decoded
  package information.

  Set isError == true if the information format is wrong.
*/
PackageInformationReader::PackageInformationReader( const QString& fName,
                                                    InstallControl::PackageInfo::Source src )
    : isError( false )
    , hasContent( false )
    , wasSizeUpdated( false )
    , accumulatingFullDesc( false )
    , source( src )
{
    reset();
    QString fileName( fName );
    // open file for reading
    QFile file(fileName);

    if (!file.exists()) {
        //assumption that control file has md5 in it's full filename, use as default name
        int pos = fileName.lastIndexOf( "/" );
        pkg.name = fileName.mid( pos + 1, 32 ); //32 chars is length of md5 digest
        pkg.description = fileName + " does not exist";
        isError = true;
        qLog( Package ) << pkg.description;
        return;
    }

    file.open(QIODevice::ReadOnly);

    QTextStream textStream(&file);

    while (!textStream.atEnd()) {
        QString line = textStream.readLine();

        readLine( line );
    }
    if ( !pkg.isComplete(source, &error) )
        isError = true;

    updateInstalledSize();
}

void PackageInformationReader::reset()
{
    pkg.name = QString::null;
    pkg.description = QString::null;
    pkg.fullDescription = QString::null;
    pkg.downloadSize = QString::null;
    pkg.section = QString::null;
    pkg.domain = QString::null;
    pkg.packageFile = QString::null;
    pkg.version = QString::null;
    pkg.trust = QString::null;
    pkg.files.clear(); //pkg.files is deprecated
    pkg.url = QString();
    pkg.qtopiaVersion = QString::null;
    pkg.devices = QString::null;
    pkg.installedSize = QString::null;
    pkg.fileCount = QString::null;
    pkg.type = QString::null;
    error = QString::null;
    isError = false;
    hasContent = false;
    wasSizeUpdated = false;
}

void PackageInformationReader::checkCompleted()
{
    if ( hasContent && pkg.isComplete(source) )
    {
        emit packageComplete();
        reset();
    }
}

/**
  Read a line of package information eg:
  \code
      Package: FoobarBaz
  \endcode
*/
void PackageInformationReader::readLine( const QString &line )
{
    if ( line.isEmpty() )
    {
        checkCompleted();
        return;
    }
    bool isDescContinuation = line.startsWith( " " );
    QString lineStr = line.trimmed();
    if ( lineStr.length() == 0 )
    {
        checkCompleted();
        return;
    }
    int colon = line.indexOf(':');
    if ( colon == -1 )
    {
        pkg.name = "corrupted";  // NO TR
        error = "No colon in package information"; // NO TR
        isError = true;
        return;
    }
    colon += 2;
    if ( isDescContinuation && accumulatingFullDesc )
    {
        pkg.fullDescription += "\n";
        pkg.fullDescription += line;
        return;
    }
    accumulatingFullDesc = false;
    if ( lineStr.startsWith( QLatin1String( "Package:" )))
    {
        checkCompleted();
        pkg.name = lineStr.mid( colon ).trimmed();
        if ( !pkg.name.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Description:" ), Qt::CaseInsensitive ))
    {
        pkg.description = lineStr.mid( colon ).trimmed();
        if ( !pkg.description.isEmpty() ) hasContent = true;
        accumulatingFullDesc = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Size:" ), Qt::CaseInsensitive ))
    {
        pkg.downloadSize = lineStr.mid( colon ).trimmed();
        if ( !pkg.description.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Section:" ), Qt::CaseInsensitive ))
    {
        pkg.section = lineStr.mid( colon ).trimmed();
        if ( !pkg.section.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Domain:" ), Qt::CaseInsensitive ))
    {
        pkg.domain = lineStr.mid( colon ).trimmed();
#ifndef QT_NO_SXE
        if ( !DomainInfo::isDomainValid( pkg.domain ) )
            pkg.domain = DomainInfo::defaultDomain();
#endif
        if ( !pkg.domain.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Filename:" ), Qt::CaseInsensitive ))
    {
        pkg.packageFile = lineStr.mid( colon ).trimmed();
        if ( !pkg.packageFile.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "MD5Sum:" ), Qt::CaseInsensitive ))
    {
        pkg.md5Sum = lineStr.mid( colon ).trimmed();
        if ( !pkg.md5Sum.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Trust:" ), Qt::CaseInsensitive ))
    {
        pkg.trust = lineStr.mid( colon ).trimmed();
        if ( !pkg.trust.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Version:" ), Qt::CaseInsensitive ))
    {
        pkg.version = lineStr.mid( colon ).trimmed();
        if ( !pkg.version.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Files:" ), Qt::CaseInsensitive ))
    {//Files field is deprecated
        QString fileList = lineStr.mid( colon ).trimmed();
        pkg.files = fileList.split( QLatin1String( " " ), QString::SkipEmptyParts );
        if ( !pkg.files.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "URL:" ), Qt::CaseInsensitive ))
    {
        pkg.url = lineStr.mid( colon ).trimmed();
        if ( !pkg.url.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "QtopiaVersion:" ), Qt::CaseInsensitive ))
    {
        pkg.qtopiaVersion = lineStr.mid( colon ).trimmed();
        if ( !pkg.qtopiaVersion.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Devices:" ), Qt::CaseInsensitive ))
    {
        pkg.devices = lineStr.mid( colon ).trimmed();
        if ( !pkg.devices.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Installed-Size: " ), Qt::CaseInsensitive ))
    {
        pkg.installedSize = lineStr.mid( colon ).trimmed();
        if ( !pkg.installedSize.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "File-Count: " ), Qt::CaseInsensitive ))
    {
        pkg.fileCount = lineStr.mid( colon ).trimmed();
        if ( !pkg.fileCount.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Type: " ), Qt::CaseInsensitive ))
    {
        pkg.type = lineStr.mid( colon ).trimmed();
        if ( !pkg.type.isEmpty() ) hasContent = true;
    }
    else
    {
        // legacy/irrelevant fields not currently an error
    }
}

/*!
    \internal
    Update the installed size based on the filecount

    Implementation note:
    The Installed-Size field of the control file only shows the apparent size of a package.
    The actual disk size will be different based upon the file system block size.
    Since we  don't know the file system block size during package creation, we include the number of files
    and directories in the file count of the control file.  We can then use this to update the installed size
    so it reflects what is actually needed on the device taking into account a worst case scenario of
    every file using up a(n) (extra) near empty block.
*/
void PackageInformationReader::updateInstalledSize()
{
    if (wasSizeUpdated)
        return;

    //re-calculate installed size based on file count
    int fileCount = pkg.fileCount.toInt();
    if ( fileCount > 0 )
    {
        //if there is a fileCount then, the installed
        //size must be have already been expressed as bytes
        //as opposed to Kb of Mb
        qulonglong size= pkg.installedSize.toLong();
        struct statfs stats;
        statfs( Qtopia::packagePath().toLatin1(), &stats);
        qulonglong blockSize = (qulonglong)stats.f_bsize;
        size = size + fileCount * blockSize;
        pkg.installedSize =  QString::number(size);
    }
    wasSizeUpdated = true;
}

