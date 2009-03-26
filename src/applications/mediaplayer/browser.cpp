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

#include "browser.h"

/*!
    \class PropertyBrowser
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn PropertyBrowser::PropertyBrowser( Property property, const QContentFilter& filter, QObject* parent )
    \internal
*/
PropertyBrowser::PropertyBrowser( Property property, const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_property( property ), m_filter( filter ), dataInitialised( false )
{
    switch( m_property )
    {
    case Artist:
        m_unknownValue = tr( "Unknown Artist" );
        break;
    case Album:
        m_unknownValue = tr( "Unknown Album" );
        break;
    case Genre:
        m_unknownValue = tr( "Unknown Genre" );
        break;
    default:
        m_unknownValue = tr( "Unknown" );
    }

    //populateData();
}

/*!
    \fn QString PropertyBrowser::filterString( const QModelIndex& index ) const
    \internal
*/
QString PropertyBrowser::filterString( const QModelIndex& index ) const
{
    const_cast<PropertyBrowser *>(this)->populateData();
    return filterKey( m_property ) + m_values[index.row()];
}

/*!
    \fn QContentFilter PropertyBrowser::filter() const
    \internal
*/
QContentFilter PropertyBrowser::filter() const
{
    return m_filter;
}

/*!
    \fn void PropertyBrowser::setFilter( const QContentFilter& filter )
    \internal
*/
void PropertyBrowser::setFilter( const QContentFilter& filter )
{
    m_filter = filter;

    populateData(true);
}

/*!
    \fn int PropertyBrowser::rowCount( const QModelIndex& ) const
    \internal
*/
int PropertyBrowser::rowCount( const QModelIndex& ) const
{
    const_cast<PropertyBrowser *>(this)->populateData();
    return m_values.count();
}

/*!
    \fn QVariant PropertyBrowser::data( const QModelIndex& index, int role ) const
    \internal
*/
QVariant PropertyBrowser::data( const QModelIndex& index, int role ) const
{
    if( role == Qt::DisplayRole ) {
        const_cast<PropertyBrowser *>(this)->populateData();
        QString value = m_values[index.row()];

        return value.isEmpty() ? m_unknownValue : value;
    }

    return QVariant();
}

/*!
    \fn QString PropertyBrowser::filterKey( Property property )
    \internal
*/
QString PropertyBrowser::filterKey( Property property )
{
    QString ret;

    switch( property )
    {
    case Artist:
        ret = "none/Artist/";
        break;
    case Album:
        ret = "none/Album/";
        break;
    case Genre:
        ret = "none/Genre/";
        break;
    default:
        // Unsupported property
        break;
    }

    return ret;
}

/*!
    \internal
*/
void PropertyBrowser::populateData(bool force)
{
    if(dataInitialised == true && force == false)
        return;
    m_values.clear();

    QString key = filterKey( m_property );
    QStringList args = m_filter.argumentMatches( QContentFilter::Synthetic, key );

    int keylen = key.length();
    foreach( QString arg, args ) {
        m_values.append( arg.mid( keylen ) );
    }
    dataInitialised = true;
}

/*!
    \class ContentBrowser
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn ContentBrowser::ContentBrowser( const QContentFilter& filter, QObject* parent )
    \internal
*/
ContentBrowser::ContentBrowser( const QContentFilter& filter, QObject* parent )
    : BrowserMenu( parent ), m_set( filter )
{
    m_setmodel = new QContentSetModel( &m_set );
}

/*!
    \fn ContentBrowser::~ContentBrowser()
    \internal
*/
ContentBrowser::~ContentBrowser()
{
    delete m_setmodel;
}

/*!
    \fn void ContentBrowser::setSortOrder( const QStringList& list )
    \internal
*/
void ContentBrowser::setSortOrder( const QStringList& list )
{
    m_set.setSortOrder( list );
}

/*!
    \fn QContent ContentBrowser::content( const QModelIndex& index ) const
    \internal
*/
QContent ContentBrowser::content( const QModelIndex& index ) const
{
    return m_setmodel->content( index );
}

/*!
    \fn QContentFilter ContentBrowser::filter() const
    \internal
*/
QContentFilter ContentBrowser::filter() const
{
    return m_set.filter();
}

/*!
    \fn void ContentBrowser::setFilter( const QContentFilter& filter )
    \internal
*/
void ContentBrowser::setFilter( const QContentFilter& filter )
{
    m_set.setCriteria( filter );
}

/*!
    \fn int ContentBrowser::rowCount( const QModelIndex& index ) const
    \internal
*/
int ContentBrowser::rowCount( const QModelIndex& index ) const
{
    return m_setmodel->rowCount( index );
}

/*!
    \fn QVariant ContentBrowser::data( const QModelIndex& index, int role ) const
    \internal
*/
QVariant ContentBrowser::data( const QModelIndex& index, int role ) const
{
    // ### FIXME
    if( role != Qt::DecorationRole ) {
        return m_setmodel->data( index, role );
    }

    return QVariant();
}

