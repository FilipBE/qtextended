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

#include <QBuffer>
#include <QSettings>
#include <QString>

#include "httpfetcher.h"
#include "packageinformationreader.h"
#include "packageversion.h"
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qtopiabase/version.h>

#define DEFAULT_MAX_PACKAGES 100
#define MAX_PACKAGES_LIST_BYTES 50000
#define MAX_FILE_SIZE_BYTES 4194304

HttpFetcher::HttpFetcher( const QString &iurl, QObject *parent )
    : QThread( parent )
    , http( 0 )
    , packageData( 0 )
    , httpRequestAborted( false )
    , url( iurl )
    , fileSize( 0 )
    , curProgValue( 0 )
{
    pkgController = qobject_cast<AbstractPackageController*>(parent);
}

HttpFetcher::~HttpFetcher()
{
    delete packageData;
    delete http;
}

const int HttpFetcher::initProgress = 10;
const int HttpFetcher::maxProgress = 100;

void HttpFetcher::run()
{
    QString fetchFile;

    // if no file set, assume fetching package list
    fetchFile = file.isEmpty() ? AbstractPackageController::PACKAGE_SUMMARY_FILE : file;

    if(url.path().endsWith("/"))
        url.setPath(url.path() + fetchFile);
    else
        url.setPath(url.path() + "/" + fetchFile);

    if ( http == 0 ) http = new QHttp();
    connect( this, SIGNAL(httpAbort()),
            http, SLOT(abort()) );

    QString query;
    HttpFileReceiver *hr;
    Md5File *md5File = 0;
    QDir packagePathDir( Qtopia::packagePath() );
    if ( file.isEmpty() ) // getting package list
    {
        if ( packageData == 0 ) packageData = new QBuffer();
        hr = new HttpInfoReceiver;
        connect( packageData, SIGNAL(bytesWritten(qint64)),
                hr, SLOT(packageDataWritten(qint64)));
    }
    else                  // getting a file
    {
        md5File = new Md5File( InstallControl::downloadedPkgPath() );
        if ( md5File->exists() )
            md5File->remove();
        packageData = md5File;
        hr = new HttpFileReceiver;
        hr->fileSize = fileSize;
    }
    hr->fetcher = this;

    connect( http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            hr, SLOT(readResponseHeader(QHttpResponseHeader)));
    connect( http, SIGNAL(dataReadProgress(int,int)),
            hr, SLOT(updateDataReadProgress(int,int)));
    connect( http, SIGNAL(requestFinished(int,bool)),
            hr, SLOT(httpRequestFinished(int,bool)));

    http->setHost( url.host() , url.port(80));
    packageData->open( QIODevice::WriteOnly );
    httpGetId = http->get( url.path() + query, packageData );

    // show some progress now that we've set everything up
    curProgValue = initProgress;
    emit progressValue( initProgress );

    // run threads event loop
    exec();

    packageData->close();

    if( md5File )
        md5Sum = md5File->md5Sum();

    delete packageData;
    packageData = 0;
    delete http;
    http = 0;
    delete hr;
}

void HttpFetcher::cancel( const QString &errorReason )
{
    error = errorReason;
    httpRequestAborted = true;
    emit httpAbort();
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpFileReceiver implementation
/////

HttpFileReceiver::HttpFileReceiver( QObject *p )
    : QObject( p )
    , fileSize( 0 )
{
}

HttpFileReceiver::~HttpFileReceiver()
{
}

void HttpFileReceiver::httpRequestFinished(int requestId, bool error)
{
    qLog(Package) << "requestId" << requestId << "finished" << ( error ? "in error" : "ok" );

    if ( requestId != fetcher->httpGetId )
        return;

    emit fetcher->progressValue( HttpFetcher::maxProgress );

    if ( error && !fetcher->httpRequestAborted )
    {
        fetcher->error = QLatin1String("HttpFileReceiver::httpRequestFinished:- http error: ") 
                                    + fetcher->http->errorString();
        fetcher->httpRequestAborted = true;
    }
    // terminate the threads loop
    if ( fetcher->httpRequestAborted )
    {
        fetcher->exit(1);
    }
    else
        fetcher->quit();
}

void HttpFileReceiver::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    if ( responseHeader.statusCode() >= 400 )
    {
        fetcher->httpRequestAborted = true;
        QString detailedError( "HttpFileReceiver::readReasponseHeader:- status code: %1 "
                               "reason phrase: %2" );
        detailedError = detailedError.arg( responseHeader.statusCode() )
                                     .arg( responseHeader.reasonPhrase() );
        fetcher->error = detailedError; 
    }
}

void HttpFileReceiver::updateDataReadProgress(int bytesRead, int totalBytes)
{
    qLog(Package) << "update data read - bytes read:" << bytesRead << " total:" << totalBytes;
    if ( fetcher->httpRequestAborted )
        return;

    int run = HttpFetcher::maxProgress - HttpFetcher::initProgress;
    if ( totalBytes != 0 )
    {
        fetcher->curProgValue = HttpFetcher::initProgress +
                                ( bytesRead * ( run / totalBytes ));
        if ( fileSize && bytesRead > fileSize )
        {
            qWarning( "Bytes read %d overflowed expected file size %d\n",
                    bytesRead, fileSize );
            fetcher->cancel(tr("Downloaded file exceeds expected file size"));
        }
    }

    emit fetcher->progressValue( fetcher->curProgValue );
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpInfoReceiver implementation
/////

HttpInfoReceiver::HttpInfoReceiver( QObject *p )
    : HttpFileReceiver( p )
    , maxPackagesList( DEFAULT_MAX_PACKAGES )
{
    reader = new PackageInformationReader;
    connect( reader, SIGNAL(packageComplete()),
            this, SLOT(packageComplete()));

    QSettings pkgManagerConf( "Trolltech", "PackageManager" );
    QStringList pkgManagerConfList = pkgManagerConf.childGroups();
    if ( pkgManagerConfList.contains( QLatin1String( "Configuration" )))
    {
        pkgManagerConf.beginGroup( QLatin1String( "Configuration" ));
        if ( pkgManagerConf.contains( QLatin1String( "MaxPackagesList" )))
            maxPackagesList = pkgManagerConf.value( QLatin1String( "MaxPackagesList" )).toInt();

        maxPackagesListSize = pkgManagerConf.value( QLatin1String( "MaxPackagesListSize" ),
                                        QVariant(MAX_PACKAGES_LIST_BYTES) ).toInt();

        pkgManagerConf.endGroup();
    }
    else
    {
        maxPackagesListSize = MAX_PACKAGES_LIST_BYTES;
    }

}

HttpInfoReceiver::~HttpInfoReceiver()
{
    delete reader;
}

void HttpInfoReceiver::packageComplete()
{
    NetworkPackageController *npc = qobject_cast<NetworkPackageController*>(fetcher->pkgController);
    Q_ASSERT( npc );

     reader->updateInstalledSize();
    //filter out packages based on whether they are sxe compatible
#ifdef QT_NO_SXE
    if ( reader->package().type.toLower() == "sxe-only" )
    {
        qLog(Package) <<"The Package: " << reader->package().name << " will only run on an sxe configured qtopia, has been filtered out";
        return;
    }
#endif

    if ( npc->numberPackages() < maxPackagesList )
    {
        //filter out packages according to device
        if ( DeviceUtil::checkDeviceLists( QLatin1String(QTOPIA_COMPATIBLE_DEVICES), reader->package().devices) )
        {
            qLog(Package) << reader->package().name << "is device compatible";

            //filter out packages which are not compatible with this version of qtopia
            if ( VersionUtil::checkVersionLists(QLatin1String(QTOPIA_COMPATIBLE_VERSIONS), reader->package().qtopiaVersion ) )
                npc->addPackage( reader->package() );
            else
                qLog(Package) << "The Package:" << reader->package().name << "is not version compatible, has been filtered out";
        }
        else
        {
            qLog(Package) << "The Package: "<< reader->package().name << "is not device compatible, has been filtered out";
        }
    }
    else
    {
        // TODO: after string freeze this should display user visible error
        qWarning( "Overflowed package list limit %d\n", maxPackagesList );
        fetcher->cancel(tr("Too many packages available to display from server"));
    }
}

void HttpInfoReceiver::packageDataWritten( qint64 bytes )
{
    if ( fetcher->httpRequestAborted )
        return;
    fetcher->packageData->close();
    fetcher->packageData->open( QIODevice::ReadOnly );
    while ( true )
    {
        lineString += fetcher->packageData->readLine( bytes );
        if ( !lineString.contains( "\n" ))
        {
            goto out_recv_data;
        }
        reader->readLine( lineString );
        lineString = "";
    }
out_recv_data:
    fetcher->packageData->close();

    //if downloading a package list clear buffer
    QBuffer *buf = qobject_cast<QBuffer *>(fetcher->packageData);
    if ( buf )
       buf->buffer().clear();
    fetcher->packageData->open( QIODevice::WriteOnly );
}


void HttpInfoReceiver::updateDataReadProgress(int bytesRead, int totalBytes)
{
    if ( totalBytes > maxPackagesListSize )
    {
        fetcher->cancel( QString("Packages.list exceeds maximum size of %1 bytes.")
                        .arg( maxPackagesListSize ) );
        return;
    }
    HttpFileReceiver::updateDataReadProgress( bytesRead, totalBytes );
}
