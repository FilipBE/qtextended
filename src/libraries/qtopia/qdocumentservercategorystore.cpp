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
#include "qdocumentservercategorystore_p.h"
#include "qdocumentserverchannel_p.h"
#include <QtDebug>

class QDocumentServerCategoryStorePrivate : public QDocumentServerClient
{
    Q_OBJECT
public:
    QDocumentServerCategoryStorePrivate( QObject *parent )
        : QDocumentServerClient( "QCategoryStoreServer", parent )
    {
    }

protected slots:

    virtual void invokeSignal( const QDocumentServerMessage &message )
    {
        if( message.signature() == "categoriesChanged()" )
            emit categoriesChanged();
    }

signals:
    void categoriesChanged();
};

QDocumentServerCategoryStore::QDocumentServerCategoryStore( QObject *parent )
    : QCategoryStore( parent )
{
    d = new QDocumentServerCategoryStorePrivate( this );

    connect( d, SIGNAL(categoriesChanged()), this, SIGNAL(categoriesChanged()) );

    for( int i = 0; i < 5 && !d->connect(); i++ )
        qWarning() << "Could not connect to category store server";
}

QDocumentServerCategoryStore::~QDocumentServerCategoryStore()
{
}

bool QDocumentServerCategoryStore::addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem )
{
    QDocumentServerMessage response = d->callWithArgumentList( "addCategory(QString,QString,QString,QString,bool)",
            QVariantList()  << categoryId << scope << label << icon << isSystem );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerCategoryStore::categoryExists( const QString &categoryId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "categoryExists(QString)",
            QVariantList() << categoryId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

QCategoryData QDocumentServerCategoryStore::categoryFromId( const QString &categoryId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "categoryFromId(QString)",
            QVariantList() << categoryId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QCategoryData >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QCategoryData();
}

QCategoryDataMap QDocumentServerCategoryStore::scopeCategories( const QString &scope )
{
    QDocumentServerMessage response = d->callWithArgumentList( "scopeCategories(QString)",
            QVariantList() << scope );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< QCategoryDataMap >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return QMap< QString, QCategoryData >();
}

bool QDocumentServerCategoryStore::removeCategory( const QString &categoryId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "removeCategory(QString)",
            QVariantList() << categoryId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerCategoryStore::setCategoryScope( const QString &categoryId, const QString &scope )
{
    QDocumentServerMessage response = d->callWithArgumentList( "setCategoryScope(QString,QString)",
            QVariantList() << categoryId << scope );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerCategoryStore::setCategoryIcon( const QString &categoryId, const QString &icon )
{
    QDocumentServerMessage response = d->callWithArgumentList( "setCategoryIcon(QString,QString)",
            QVariantList() << categoryId << icon );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerCategoryStore::setCategoryRingTone( const QString &categoryId, const QString &fileName )
{
    QDocumentServerMessage response = d->callWithArgumentList( "setCategoryRingTone(QString,QString)",
            QVariantList() << categoryId << fileName );

    if ( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >(response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }
        return false;
}

bool QDocumentServerCategoryStore::setCategoryLabel( const QString &categoryId, const QString &label )
{
    QDocumentServerMessage response = d->callWithArgumentList( "setCategoryLabel(QString,QString)",
            QVariantList() << categoryId << label );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

bool QDocumentServerCategoryStore::setSystemCategory( const QString &categoryId )
{
    QDocumentServerMessage response = d->callWithArgumentList( "setSystemCategory(QString)",
            QVariantList() << categoryId );

    if( response.type() == QDocumentServerMessage::ReplyMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        return qvariant_cast< bool >( response.arguments().first() );
    }
    else if( response.type() == QDocumentServerMessage::ErrorMessage )
    {
        Q_ASSERT( response.arguments().count() == 1 );

        setErrorString( qvariant_cast< QString >( response.arguments().first() ) );
    }

    return false;
}

#include "qdocumentservercategorystore.moc"
