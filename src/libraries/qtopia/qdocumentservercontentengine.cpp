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
#include "qdocumentservercontentengine_p.h"
#include "qdocumentservercontentstore_p.h"
#include "drmcontent_p.h"

QDocumentServerContentEngine::QDocumentServerContentEngine( const QString &engineType )
    : QContentEngine( engineType )
{
}

QDrmRights QDocumentServerContentEngine::rights( QDrmRights::Permission permission ) const
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentRights( id(), permission );
}

QContentEngine *QDocumentServerContentEngine::copyTo( const QString &newPath )
{
    Q_UNUSED( newPath );

    return 0;
}

bool QDocumentServerContentEngine::moveTo( const QString &newPath )
{
    Q_UNUSED( newPath );

    return false;
}

bool QDocumentServerContentEngine::rename(const QString &name)
{
    Q_UNUSED(name);

    return false;
}

bool QDocumentServerContentEngine::execute( const QStringList &arguments ) const
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->executeContent( id(), arguments );
}

bool QDocumentServerContentEngine::canActivate() const
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::canActivate( fileName() );
    }
    else
    {
        return false;
    }
}

bool QDocumentServerContentEngine::activate( QDrmRights::Permission permission, QWidget *parent )
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::activate( fileName(), permission, parent );
    }
    else
    {
        return true;
    }
}

bool QDocumentServerContentEngine::reactivate( QDrmRights::Permission permission, QWidget *parent )
{
    if( drmState() == QContent::Protected )
    {
        DrmContentPrivate::reactivate( fileName(), permission, parent );
    }

    return true;
}

QDrmContentLicense *QDocumentServerContentEngine::requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::requestContentLicense( QContent( this ), permission, options );
    }
    else
        return 0;
}

bool QDocumentServerContentEngine::remove()
{
    return false;
}

QIODevice *QDocumentServerContentEngine::open( QIODevice::OpenMode mode )
{
    QFile *file = new QFile( fileName() );

    if( file->open( mode ) )
    {
        return file;
    }
    else
    {
        setError( file->errorString() );

        delete file;

        return 0;
    }
}

QContentEngine *QDocumentServerContentEngine::createCopy() const
{
    QDocumentServerContentEngine *copy = new QDocumentServerContentEngine( engineType() );

    copy->setId( id() );
    copy->copy( *this );

    return copy;
}

bool QDocumentServerContentEngine::isOutOfDate() const
{
    return false;
}

QDrmRights::Permissions QDocumentServerContentEngine::queryPermissions()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentPermissions( id() );
}

qint64 QDocumentServerContentEngine::querySize()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentSize( id() );
}

bool QDocumentServerContentEngine::queryValidity()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentValid( id() );
}

