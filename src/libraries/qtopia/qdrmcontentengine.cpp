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

#include "qdrmcontentengine_p.h"
#include "drmcontent_p.h"
#include <qtopialog.h>
#include <qtopianamespace.h>

/*!
    \class QDrmContentEngine
    \inpublicgroup QtBaseModule

    \brief QDrmContentEngine is the default content engine for DRM protected content.

    \internal
*/

/*!
    Constructs a new unpopulated QDrmContentEngine.
*/
QDrmContentEngine::QDrmContentEngine()
    : QFSContentEngine( QLatin1String( "*/*" ) )
{
}

/*!
    Constructs a new QDrmContentPrivate where \a engineType is the engine mime type of an inheriting engine.
*/
QDrmContentEngine::QDrmContentEngine( const QString &engineType )
    : QFSContentEngine( engineType )
{
}


/*!
    Destroys a QDrmContentEngine.
*/
QDrmContentEngine::~QDrmContentEngine()
{
}

/*!
    \reimp
*/
QContentEngine *QDrmContentEngine::createCopy() const
{
    QDrmContentEngine *engine = new QDrmContentEngine;

    engine->copy( *this );
    engine->setId( id() );

    return engine;
}

/*!
    \reimp
*/
QIODevice *QDrmContentEngine::open( QIODevice::OpenMode mode )
{
    QDrmContentPlugin *plugin = DrmContentPrivate::plugin( fileName() );

    QDrmContentLicense *license = plugin ? plugin->license( fileName() ) : 0;

    QIODevice *io = license ? plugin->createDecoder( fileName(), license->permission() ) : 0;

    if( !(io && io->open( mode )) )
    {
        delete io;

        io = 0;
    }

    return io;
}

/*!
    \reimp
*/
bool QDrmContentEngine::execute( const QStringList &arguments ) const
{
    if( role() == QContent::Application )
    {
        qLog(DocAPI) << "QDrmContentEngine::execute" << fileName() << arguments;

        Qtopia::execute( fileName(), arguments.count() ? arguments[0] : QString() );

        return true;
    }
    else
    {
        QContent app = mimeType().application();

        if( app.isValid() && const_cast< QDrmContentEngine * >( this )->activate( mimeType().permission(), 0 ) )
        {
            app.execute( QStringList() << arguments << fileName() );

            return true;
        }
    }

    return false;
}

/*!
    \reimp
*/
QDrmRights::Permissions QDrmContentEngine::queryPermissions()
{
    return DrmContentPrivate::permissions( fileName() );
}

/*!
    \reimp
*/
QDrmRights QDrmContentEngine::rights( QDrmRights::Permission permission ) const
{
    return DrmContentPrivate::getRights( fileName(), permission );
}

/*!
    \reimp
*/
bool QDrmContentEngine::canActivate() const
{
    return DrmContentPrivate::canActivate( fileName() );
}

/*!
    \reimp
*/
bool QDrmContentEngine::activate( QDrmRights::Permission permission, QWidget *parent )
{
    return DrmContentPrivate::activate( fileName(), permission, parent );
}

/*!
    \reimp
*/
bool QDrmContentEngine::reactivate( QDrmRights::Permission permission, QWidget *parent )
{
    DrmContentPrivate::reactivate( fileName(), permission, parent );

    return true;
}

/*!
    \reimp
*/
QDrmContentLicense *QDrmContentEngine::requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    return DrmContentPrivate::requestContentLicense( QContent( this ), permission, options );
}
