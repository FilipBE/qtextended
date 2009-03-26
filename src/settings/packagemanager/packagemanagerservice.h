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

#ifndef PACKAGEMANAGERSERVICE_H
#define PACKAGEMANAGERSERVICE_H

#include <QHttp>
#include <QBuffer>
#include <QFile>
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QtopiaAbstractService>
#include "installcontrol.h"
#include "md5file.h"

class PackageServiceInstaller;
class PackageView;
class QDSActionRequest;
class InstalledPackageScanner;
class QUrl;
class QTextEdit;

class PackageManagerService : public QtopiaAbstractService
{
    Q_OBJECT
public slots:
    void installPackageConfirm( const QString &url );
    void installPackage( const QDSActionRequest &request );

private slots:
    void installFinished();
private:
    PackageManagerService( PackageView *parent );

    PackageServiceInstaller *m_installer;
    PackageView *m_packageView;
    QStringList m_pendingUrls;
    QStringList m_pendingDescriptors;
    friend class PackageView;
};

class ServicePackageDetails;

class PackageServiceInstaller : public QDialog, public ErrorReporter
{
    Q_OBJECT
public:
    PackageServiceInstaller( PackageView *parent, Qt::WindowFlags flags = 0 );

    bool installActive() const{ return m_installActive; }


public slots:
    void installPackage( const QString &url );
    void installPackage( const QByteArray &descriptor );

    virtual void accept();
    virtual void reject();

private slots:
    void confirmInstall( const InstallControl::PackageInfo &package );
    void installPendingPackage();
    void headerDownloadDone( bool error );
    void packageDownloadDone( bool error );

    void updateHeaderProgress( int done, int total );
    void updatePackageProgress( int done, int total );

private:
    QHttp m_http;
    QBuffer m_headerBuffer;
    Md5File m_packageFile;
    InstallControl::PackageInfo m_pendingPackage;
    InstallControl m_installer;
    InstalledPackageScanner *m_scanner;

    QTextEdit *m_progressTextEdit;
    QProgressBar *m_progressBar;
    ServicePackageDetails *m_packageDetails;
    bool m_installActive;
    PackageView *m_packageView;

    int m_expectedPackageSize;
    int m_maxDescriptorSize;
    QUrl *currentUrl;

    void doReportError( const QString &error, const QString &detailedError );
};

#endif
