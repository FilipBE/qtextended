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

#include "drmbrowser.h"


#include <QSoftMenuBar>
#include <QDrmContentPlugin>
#include <QPluginManager>
#include <QtopiaApplication>
#include <qdsdata.h>
#include <qdsservices.h>
#include <qdsserviceinfo.h>
#include <qdsaction.h>
#include <QTabWidget>
#include <QVBoxLayout>

Q_GLOBAL_STATIC_WITH_ARGS(QPluginManager, pluginManager, (QLatin1String("drmagent")))

/*!
    \class DrmBrowser
    \inpublicgroup QtDrmModule
    \internal
    The DrmBrowser processes received DRM content and messages. It also displays a list
    of DRM protected content on the file system and the current rights for that content.
*/

/*!
    Creates a new drm browser object defined by flags \a f and attachs it to \a parent.
*/
DrmBrowser::DrmBrowser( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{

    QPluginManager *manager = pluginManager();

    QStringList pluginNames = manager->list();

    foreach( QString pluginName, pluginNames )
    {
        QObject *object = manager->instance( pluginName );

        QDrmAgentPlugin *agent = qobject_cast< QDrmAgentPlugin * >( object );

        if( agent )
            agent->createService( this );
    }

    if( static_cast< QtopiaApplication * >( qApp )->willKeepRunning() )
        initView();
}

/*!
    \internal
    Destroys a DrmBrowser object.
*/
DrmBrowser::~DrmBrowser()
{
}

/*!
    \internal
    Initializes the DrmBrowser layout and fills a list of DRM protected content.
*/
void DrmBrowser::initView()
{
    setWindowTitle( tr( "Licenses" ) );

    QSoftMenuBar::menuFor( this );

    QTabWidget *tabWidget = new QTabWidget( this );

    QPluginManager *manager = pluginManager();

    QStringList pluginNames = manager->list();

    foreach( QString pluginName, pluginNames )
    {
        QObject *object = manager->instance( pluginName );

        QDrmAgentPlugin *agent = qobject_cast< QDrmAgentPlugin * >( object );

        if( agent )
            foreach( QWidget *widget, agent->createManagerWidgets( this ) )
                tabWidget->addTab( widget, widget->windowIcon(), widget->windowTitle() );
    }

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( tabWidget );
}


/*!
    Processes a DRM message or rights file at \a fileName.
    Message files (.dm) are converted to .dcf files and deleted from the file system.
    Rights files are installed if there is a corresponding content file and then deleted from the file system.
*/
void DrmBrowser::setDocument( const QString &fileName )
{
    QContent content( QContent::execToContent( fileName ) );

    QMimeType type = content.isNull() ? QMimeType::fromFileName( fileName ) : QMimeType::fromId( content.type() );

    execService( fileName, type, QLatin1String( "handle" ) ) || execService( fileName, type, QLatin1String( "x-wap.application:drm.ou" ) );
}

bool DrmBrowser::execService( const QString &fileName, const QMimeType &type, const QString &hint )
{
    QDSServices services( type.id(), QString(), QStringList() << hint );

    if( !services.isEmpty() )
    {
        QFile file( fileName );

        QDSServiceInfo service = services.first();

        QDSAction action( service );

        action.exec( QDSData( file, type ) );

        return true;
    }
    else
        return false;
}
