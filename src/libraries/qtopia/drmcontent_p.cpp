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

#include "drmcontent_p.h"
#include <qtopialog.h>
#include <qthumbnail.h>
#include <qcontentfilter.h>
#include <QPainter>
#include <QThread>
#include <QPixmapCache>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif

DrmContentPluginManager::DrmContentPluginManager()
    : m_agentManager( 0 )
    , m_contentManager( 0 )
{
}

DrmContentPluginManager::~DrmContentPluginManager()
{
    qDeleteAll( m_plugins );

    delete m_agentManager;
    delete m_contentManager;
}

QDrmContentPlugin *DrmContentPluginManager::plugin( const QString &filePath ) const
{
    foreach( QDrmContentPlugin *plugin, m_plugins )
        foreach( QString key, plugin->keys() )
        if( filePath.endsWith( '.' + key ) || filePath.contains( '.' + key + QDir::separator() ) )
            return plugin;

    return 0;
}

QList< QDrmContentPlugin * > DrmContentPluginManager::plugins() const
{
    return m_plugins;
}

DrmContentPluginManager *DrmContentPluginManager::instance()
{
    static DrmContentPluginManager *instance = 0;

    if( !instance )
        instance = new DrmContentPluginManager;

    return instance;
}

void DrmContentPluginManager::load()
{
    if( m_agentManager )
        return;

    m_agentManager = new QPluginManager( QLatin1String( "drmagent" ) );

    QStringList pluginNames = m_agentManager->list();

    foreach( QString pluginName, pluginNames )
    {
        qLog(DRMAgent) << "Load Plugin" << pluginName;

        QObject *object = m_agentManager->instance( pluginName );

        QDrmAgentPlugin *agent = qobject_cast< QDrmAgentPlugin * >( object );

        if( agent )
        {
            QDrmContentPlugin *plugin = agent->createDrmContentPlugin();

            QStringList keys = plugin->keys();

            foreach( QString key, keys )
            {
                qLog(DRMAgent) << "Key" << key;

                if( !m_pluginMap.contains( key ) )
                    m_pluginMap[ key ] = plugin;
                else
                    qWarning() << "Multiple DRM plugins with a common key";
            }

            m_plugins.append( plugin );
        }
    }

    m_contentManager = new QPluginManager( QLatin1String( "drmcontent" ) );

    pluginNames = m_contentManager->list();

    foreach( QString pluginName, pluginNames )
    {
        qLog(DRMAgent) << "Load Plugin" << pluginName;

        QObject *object = m_contentManager->instance( pluginName );

        QDrmContentPlugin *plugin = qobject_cast< QDrmContentPlugin * >( object );

        if( plugin )
        {
            QStringList keys = plugin->keys();

            foreach( QString key, keys )
            {
                qLog(DRMAgent) << "Key" << key;

                if( !m_pluginMap.contains( key ) )
                    m_pluginMap[ key ] = plugin;
                else
                    qWarning() << "Multiple DRM plugins with a common key";
            }
        }
    }
}

/*!
    \class DrmContentPrivate
    \inpublicgroup QtBaseModule

    \internal
*/

QIcon DrmContentPrivate::createIcon( const QIcon &baseIcon, int smallSize, int bigSize, bool validRights )
{
    QIcon icon;

    icon.addPixmap( compositeDrmIcon( baseIcon.pixmap(smallSize, smallSize), smallSize, validRights ) );
    icon.addPixmap( compositeDrmIcon( baseIcon.pixmap(bigSize, bigSize), bigSize, validRights ) );

    return icon;
}

QPixmap DrmContentPrivate::drmIcon( int size )
{
    QPixmap overlay;
#ifndef QTOPIA_NO_CACHE_PIXMAPS
    QString id = QLatin1String("_QPE_Global_Drm_lock_") + QString::number( size );
    if( !QPixmapCache::find( id, overlay ) )
    {
#endif
        QString path = size <= 16 ? QLatin1String(":image/drm/Drm_lock_16") : QLatin1String(":image/drm/Drm_lock");

        overlay = QPixmap( path ).scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

#ifndef QTOPIA_NO_CACHE_PIXMAPS
        QPixmapCache::insert( id, overlay );
    }
#endif
    return overlay;
}

QPixmap DrmContentPrivate::drmIconInvalid( int size )
{
    QPixmap overlay;
#ifndef QTOPIA_NO_CACHE_PIXMAPS
    QString id = QLatin1String("_QPE_Global_Drm_lock_invalid_") + QString::number( size );
    if( !QPixmapCache::find( id, overlay ) )
    {
#endif
        QString path = size <= 16 ? QLatin1String(":image/drm/Drm_lock_invalid_16") : QLatin1String(":image/drm/Drm_lock_invalid");

        overlay = QPixmap( path ).scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

#ifndef QTOPIA_NO_CACHE_PIXMAPS
        QPixmapCache::insert( id, overlay );
    }
#endif
    return overlay;
}

QPixmap DrmContentPrivate::compositeDrmIcon( const QPixmap &base, int size, bool validRights )
{
    QPixmap pm = base;
    QPixmap overlay = validRights ? drmIcon( size ) : drmIconInvalid( size );

    if( !pm.isNull() && ! overlay.isNull() )
    {
        QPainter painter( &pm );

        QSize diff = base.size() - overlay.size();

        painter.drawPixmap( diff.width(), diff.height(), overlay );
    }
    return pm;
}



bool DrmContentPrivate::isProtected( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->isProtected( filePath ) : false;
}

QDrmRights::Permissions DrmContentPrivate::permissions( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->permissions( filePath ) : QDrmRights::Permissions();
}

QDrmRights DrmContentPrivate::getRights( const QString &filePath, QDrmRights::Permission permission )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->getRights( filePath, permission ) : QDrmRights();
}

QDrmContentLicense *DrmContentPrivate::requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    QDrmContentPlugin *p = plugin( content.fileName() );

    return p ? p->requestContentLicense( content, permission, options ) : 0;
}

bool DrmContentPrivate::activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    if( content.permissions() == QDrmRights::Unrestricted )
        return true;
    else if( (content.permissions() & permission) == permission )
        return true;
    else
    {
        QDrmContentPlugin *p = plugin( content.fileName() );

        return p ? p->activate( content, permission, focus ) : false;
    }
}

void DrmContentPrivate::reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    QDrmContentPlugin *p = plugin( content.fileName() );

    if( p )
        p->reactivate( content, permission, focus );
}

QIODevice *DrmContentPrivate::createDecoder( const QString &filePath, QDrmRights::Permission permission )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->createDecoder( filePath, permission ) : 0;
}

bool DrmContentPrivate::canActivate( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->canActivate( filePath ) : false;
}

void DrmContentPrivate::activate( const QContent &content, QWidget *focus )
{
    QDrmContentPlugin *p = plugin( content.fileName() );

    if( p )
        p->activate( content, focus );
}

bool DrmContentPrivate::deleteFile( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->deleteFile( filePath ) : false;
}

qint64 DrmContentPrivate::unencryptedSize( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->unencryptedSize( filePath ) : 0;
}

bool DrmContentPrivate::installContent( const QString &filePath, QContent *content )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->installContent( filePath, content ) : false;
}

bool DrmContentPrivate::updateContent( QContent *content )
{
    QDrmContentPlugin *p = plugin( content->fileName() );

    return p ? p->updateContent( content ) : false;
}

QPixmap DrmContentPrivate::thumbnail( const QString &filePath, const QSize &size, Qt::AspectRatioMode mode )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->thumbnail( filePath, size, mode ) : QPixmap();
}


QDrmContentPlugin *DrmContentPrivate::plugin( const QString &filePath )
{
    return DrmContentPluginManager::instance()->plugin( filePath );
}


