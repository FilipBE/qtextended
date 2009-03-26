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

#ifndef INSTALLEDPACKAGESCANNER_H
#define INSTALLEDPACKAGESCANNER_H

#include <QThread>
#include <QStringList>

#include "installcontrol.h"

class AbstractPackageController;
class QEventLoop;

class InstalledPackageScanner : public QThread
{
    Q_OBJECT
public:
    static const int initProgress;
    static const int maxProgress;

    InstalledPackageScanner( QObject *parent );
    virtual ~InstalledPackageScanner();
    virtual void run();
    void setLocations( const QStringList & );
    bool wasAborted() const { return aborted; }
    QString getError() const { return error; }

    static bool isPackageEnabled( const InstallControl::PackageInfo &pkgInfo );
    static bool isPackageInstalled( const InstallControl::PackageInfo &pkgInfo );
public slots:
    void cancel();
signals:
    void progressValue( int );
    void newPackage( InstallControl::PackageInfo * );
private:
    static InstallControl::PackageInfo scan( const QString & );
    AbstractPackageController *pkgController;
    QEventLoop *eventLoop;
    QStringList locations;
    bool aborted;
    QString error;
};

#endif
