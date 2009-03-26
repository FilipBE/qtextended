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
#include "qmimetypedata_p.h"
#include "drmcontent_p.h"
#include <QApplication>
#include <QtDebug>

Q_GLOBAL_STATIC_WITH_ARGS(QIcon,unknownDocumentIcon,(QLatin1String(":image/qpe/UnknownDocument")));


Q_GLOBAL_STATIC_WITH_ARGS(QIcon,validDrmUnknownDocumentIcon,(DrmContentPrivate::createIcon(
                *unknownDocumentIcon(),
                qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ),
                qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ),
                true )));

Q_GLOBAL_STATIC_WITH_ARGS(QIcon,invalidDrmUnknownDocumentIcon,(DrmContentPrivate::createIcon(
                *unknownDocumentIcon(),
                qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ),
                qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ),
                false )));

class QMimeTypeDataPrivate : public QSharedData
{
public:

    struct AppData
    {
        QContent application;
        QString iconFile;
        QIcon icon;
        QIcon validDrmIcon;
        QIcon invalidDrmIcon;
        QDrmRights::Permission permission;
        bool iconLoaded;
        bool validDrmIconLoaded;
        bool invalidDrmIconLoaded;
    };

    QMimeTypeDataPrivate()
    {
    }

    ~QMimeTypeDataPrivate()
    {
        qDeleteAll( applicationData.values() );
    }

    QString id;

    QContentList applications;

    QContent defaultApplication;

    QMap< QContentId, AppData * > applicationData;

    void loadIcon( AppData *data ) const;
    void loadValidDrmIcon( AppData *data ) const;
    void loadInvalidDrmIcon( AppData *data ) const;
};


void QMimeTypeDataPrivate::loadIcon( AppData *data ) const
{
    if( !data->iconFile.isEmpty() )
        data->icon = QIcon( QLatin1String( ":icon/" ) + data->iconFile );

    data->iconLoaded = true;
}

void QMimeTypeDataPrivate::loadValidDrmIcon( AppData *data ) const
{
    if( !data->icon.isNull() )
        data->validDrmIcon = DrmContentPrivate::createIcon(
                data->icon,
                qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ),
                qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ),
                true );

    data->validDrmIconLoaded = true;
}

void QMimeTypeDataPrivate::loadInvalidDrmIcon( AppData *data ) const
{
    if( !data->icon.isNull() )
        data->invalidDrmIcon = DrmContentPrivate::createIcon(
                data->icon,
                qApp->style()->pixelMetric( QStyle::PM_ListViewIconSize ),
                qApp->style()->pixelMetric( QStyle::PM_LargeIconSize ),
                false );

    data->invalidDrmIconLoaded = true;
}

Q_GLOBAL_STATIC_WITH_ARGS(QSharedDataPointer<QMimeTypeDataPrivate>,nullQMimeTypeDataPrivate,(new QMimeTypeDataPrivate));

QMimeTypeData::QMimeTypeData()
{
    d = *nullQMimeTypeDataPrivate();
}

QMimeTypeData::QMimeTypeData( const QString &id )
{
    if( !id.isEmpty() )
    {
        d = new QMimeTypeDataPrivate;

        d->id = id;
    }
    else
        d = *nullQMimeTypeDataPrivate();
}

QMimeTypeData::QMimeTypeData( const QMimeTypeData &other )
    : d( other.d )
{
}

QMimeTypeData::~QMimeTypeData()
{
}

QMimeTypeData &QMimeTypeData::operator =( const QMimeTypeData &other )
{
    d = other.d;

    return *this;
}

bool QMimeTypeData::operator ==( const QMimeTypeData &other )
{
    return d->id == other.d->id;
}

QString QMimeTypeData::id() const
{
    return d->id;
}

QContentList QMimeTypeData::applications() const
{
    return d->applications;
}

QContent QMimeTypeData::defaultApplication() const
{
    return d->defaultApplication;
}

QIcon QMimeTypeData::icon( const QContent &application ) const
{
    if( d->applicationData.contains( application.id() ) )
    {
        const QMimeTypeDataPrivate::AppData *data = d->applicationData.value( application.id() );

        if( !data->iconLoaded )
            d->loadIcon( const_cast< QMimeTypeDataPrivate::AppData * >( data ) );

        return !data->icon.isNull() ? data->icon : data->application.icon();
    }
    else
        return *unknownDocumentIcon();
}


QIcon QMimeTypeData::validDrmIcon( const QContent &application ) const
{
    if( d->applicationData.contains( application.id() ) )
    {
        const QMimeTypeDataPrivate::AppData *data = d->applicationData.value( application.id() );

        if( !data->iconLoaded )
            d->loadIcon( const_cast< QMimeTypeDataPrivate::AppData * >( data ) );

        if( !data->validDrmIconLoaded )
            d->loadValidDrmIcon( const_cast< QMimeTypeDataPrivate::AppData * >( data ) );

        return !data->validDrmIcon.isNull() ? data->validDrmIcon : data->application.icon();
    }
    else
        return *validDrmUnknownDocumentIcon();
}

QIcon QMimeTypeData::invalidDrmIcon( const QContent &application ) const
{
    if( d->applicationData.contains( application.id() ) )
    {
        const QMimeTypeDataPrivate::AppData *data = d->applicationData.value( application.id() );

        if( !data->iconLoaded )
            d->loadIcon( const_cast< QMimeTypeDataPrivate::AppData * >( data ) );

        if( !data->invalidDrmIconLoaded )
            d->loadInvalidDrmIcon( const_cast< QMimeTypeDataPrivate::AppData * >( data ) );

        return !data->invalidDrmIcon.isNull() ? data->invalidDrmIcon : data->application.icon();
    }
    else
        return *invalidDrmUnknownDocumentIcon();
}

QDrmRights::Permission QMimeTypeData::permission( const QContent &application ) const
{
    if( d->applicationData.contains( application.id() ) )
    {
        const QMimeTypeDataPrivate::AppData *data = d->applicationData.value( application.id() );

        return data->permission;
    }
    else
        return QDrmRights::Unrestricted; 
}

void QMimeTypeData::addApplication( const QContent &application, const QString &iconFile, QDrmRights::Permission permission )
{
    if( application.id() != QContent::InvalidId && !d->applicationData.contains( application.id() ) )
    {
        QMimeTypeDataPrivate::AppData *data = new QMimeTypeDataPrivate::AppData;

        data->application = application;
        data->iconFile = iconFile;
        data->permission = permission;
        data->iconLoaded = false;
        data->validDrmIconLoaded = false;
        data->invalidDrmIconLoaded = false;

        d->applicationData.insert( application.id(), data );
        d->applications.append( application );

        if(d->defaultApplication.id() == QContent::InvalidId)
            setDefaultApplication(application);
    }
}

void QMimeTypeData::removeApplication( const QContent &application )
{
    if(d->applicationData.contains( application.id() ))
    {
        delete d->applicationData.take(application.id());
        d->applications.removeAll(application);
    }
}

void QMimeTypeData::setDefaultApplication( const QContent &application )
{
    if(application.id() != QContent::InvalidId)
        d->defaultApplication = application;
}

template <typename Stream> void QMimeTypeData::serialize(Stream &stream) const
{
    stream << d->id;
    stream << d->defaultApplication.id();
    stream << d->applicationData.count();

    QList< QContentId > keys = d->applicationData.keys();

    foreach( QContentId contentId, keys )
    {
        QMimeTypeDataPrivate::AppData *data = d->applicationData.value( contentId );

        stream << contentId;
        stream << data->iconFile;
        stream << data->permission;
    }
}

template <typename Stream> void QMimeTypeData::deserialize(Stream &stream)
{
    qDeleteAll( d->applicationData.values() );

    d->applicationData.clear();

    stream >> d->id;

    {
        QContentId contentId;

        stream >> contentId;

        d->defaultApplication = QContent( contentId );
    }

    int count;

    stream >> count;

    for( int i = 0; i < count; i++ )
    {
        QContentId contentId;
        QString iconFile;
        QDrmRights::Permission permission;

        stream >> contentId;
        stream >> iconFile;
        stream >> permission;

        addApplication( QContent( contentId ), iconFile, permission );
    }
}

Q_IMPLEMENT_USER_METATYPE(QMimeTypeData);
