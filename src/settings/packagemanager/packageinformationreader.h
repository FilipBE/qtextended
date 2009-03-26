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

#ifndef PACKAGEINFORMATIONREADER_H
#define PACKAGEINFORMATIONREADER_H

#include <QString>
#include <QTextStream>

#include "installcontrol.h"

class PackageInformationReader : public QObject
{
    Q_OBJECT
public:

    PackageInformationReader(const QString& fileName, InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList);
    PackageInformationReader(QTextStream &ts, InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList);
    PackageInformationReader(InstallControl::PackageInfo::Source source = InstallControl::PackageInfo::PkgList);

    void readLine( const QString & );
    void reset();

    QString name() const { return pkg.name; }
    QString description() const { return pkg.description; }
    QString section() const { return pkg.section; }
    QString domain() const { return pkg.domain; }
    QString trust() const { return pkg.trust; }
    QString url() const { return pkg.url; }
    QString md5Sum() const { return pkg.md5Sum; }
    QString qtopiaVersion() const { return pkg.qtopiaVersion; }
    QString devices() const { return pkg.devices; }
    QString installedSize() const { return pkg.installedSize; }
    QString fileCount() const { return pkg.fileCount; }
    QString type() const { return pkg.type; }


    const InstallControl::PackageInfo &package() const { return pkg; }
    bool getIsError() const { return isError; }
    QString getError() const { return error; }
    void updateInstalledSize();

signals:
    void packageComplete();

private:
    QString error;
    bool isError;
    bool hasContent;
    bool wasSizeUpdated;

    bool accumulatingFullDesc;
    InstallControl::PackageInfo pkg;
    InstallControl::PackageInfo::Source source;

    void checkCompleted();
};

#endif
