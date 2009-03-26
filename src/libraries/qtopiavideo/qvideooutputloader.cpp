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


#include "qvideooutputloader.h"
#include "qabstractvideooutput.h"
#include "qvideooutputfactory.h"

#include <QApplication>
#include <QPluginManager>
#include <QStringList>
#include <QDebug>

class QVideoOutputLoaderPrivate
{
public:
    QVideoOutputLoaderPrivate()
        :pluginManager(0)
    {
    }

    QList<QVideoOutputFactory*> videoOutputs;
    QPluginManager* pluginManager;
    bool isLoaded;
    static QVideoOutputLoader *instance;
};

QVideoOutputLoader* QVideoOutputLoaderPrivate::instance = 0;


QVideoOutputLoader::QVideoOutputLoader( QObject *parent )
    :QObject(parent), d( new QVideoOutputLoaderPrivate )
{
}

QVideoOutputLoader::~QVideoOutputLoader()
{
    unload();
    delete d;
}

QVideoOutputLoader* QVideoOutputLoader::instance()
{
    if ( QVideoOutputLoaderPrivate::instance == 0 )
        QVideoOutputLoaderPrivate::instance = new QVideoOutputLoader( qApp );

    return QVideoOutputLoaderPrivate::instance;
}

void QVideoOutputLoader::load()
{
    if ( isLoaded() )
        return;

    d->pluginManager = new QPluginManager("videooutput");
    QStringList engines = d->pluginManager->list();

    //load directpaintervideooutput only if all the other engines failed
    if ( engines.contains("directpaintervideooutput") ) {
        engines.removeAll( "directpaintervideooutput" );
        engines.append( "directpaintervideooutput" );
    }

    foreach( QString engineName, engines ) {
        QObject *instance = d->pluginManager->instance(engineName);
        if ( !instance )
            qWarning() << "QVideoOutputLoader: loading the plugin instance failed";

        QVideoOutputFactory *factory = qobject_cast<QVideoOutputFactory*>(instance);

        if ( factory )
            d->videoOutputs << factory;
        else {
            qWarning() << "QVideoOutputLoader: plugin instance is not QVideoOutputFactory";
            delete instance;
        }
    }
}

QAbstractVideoOutput* QVideoOutputLoader::create( QScreen *screen,
                                                  const QVideoFormatList& expectedFormats,
                                                  QObject *parent )
{
    load();

    if ( expectedFormats.isEmpty() ) {
        foreach( QVideoOutputFactory *factory, videoOutputFactories() ) {
            QAbstractVideoOutput *res = factory->create( screen, parent );
            if ( res )
                return res;
        }
    } else {
        //put factories to QMap to have them sorted in preferred order
        QMap<int,QVideoOutputFactory*> factories;
        QSet<QVideoFrame::PixelFormat> expectedFormatsSet = expectedFormats.toSet();

        foreach( QVideoOutputFactory *factory, videoOutputFactories() ) {
            QSet<QVideoFrame::PixelFormat> preferredFormats = factory->preferredFormats().toSet();
            QSet<QVideoFrame::PixelFormat> supportedFormats = factory->supportedFormats().toSet();

            // using negative scores to have most preferred factories processed first
            int score = 0;
            score -= ( supportedFormats & expectedFormatsSet ).count();
            score -= ( preferredFormats & expectedFormatsSet ).count()*10;

            factories.insert( score, factory );
        }

        foreach( QVideoOutputFactory *factory, factories.values() ) {
            QAbstractVideoOutput *res = factory->create( screen, parent );
            if ( res )
                return res;
        }
    }

    return new QNullVideoOutput( screen, parent );
}

void QVideoOutputLoader::unload()
{
    qDeleteAll( d->videoOutputs );
    d->videoOutputs.clear();
    delete d->pluginManager;
    d->pluginManager = 0;
}

bool QVideoOutputLoader::isLoaded() const
{
    return d->pluginManager != 0;
}

QList<QVideoOutputFactory*> const& QVideoOutputLoader::videoOutputFactories()
{
    load();
    return d->videoOutputs;
}

