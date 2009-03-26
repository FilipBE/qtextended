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
#include <qcontentsortcriteria.h>
#include <qtopianamespace.h>
#include <QtDebug>

class QContentSortCriteriaPrivate : public QSharedData
{
public:
    struct Sort
    {
        Sort( QContentSortCriteria::Attribute attribute, const QString scope, Qt::SortOrder order )
            : attribute( attribute ), scope( scope ), order( order )
        {
        }

        bool operator ==( const Sort &other )
        {
            return attribute == other.attribute && scope == other.scope && order == other.order;
        }

        QContentSortCriteria::Attribute attribute;
        QString scope;
        Qt::SortOrder order;
    };

    QList< Sort > sorts;

    static int compare( const Sort &sort, const QContent &left, const QContent &right );
};

int QContentSortCriteriaPrivate::compare( const Sort &sort, const QContent &left, const QContent &right )
{
    switch( sort.attribute )
    {
    case QContentSortCriteria::Name:
        return QString::localeAwareCompare( Qtopia::dehyphenate( left.name() ), Qtopia::dehyphenate( right.name() ) );
    case QContentSortCriteria::MimeType:
        return QString::compare( left.type(), right.type() );
    case QContentSortCriteria::Property:
        {
            QString group = sort.scope.section( QLatin1Char( '/' ), 0, 1 );
            QString key = sort.scope.section( QLatin1Char( '/' ), 1, 1 );

            return QString::compare( left.property( key, group ), right.property( key, group ) );
        }
    case QContentSortCriteria::FileName:
        return QString::compare( left.fileName(), right.fileName() );
    case QContentSortCriteria::LastUpdated:
        {
            if( left.lastUpdated() < right.lastUpdated() )
                return -1;
            else if( left.lastUpdated() > right.lastUpdated() )
                return 1;
            else
                return 0;
        }
    default:
        return 0;
    }
}

Q_GLOBAL_STATIC_WITH_ARGS(
        QSharedDataPointer<QContentSortCriteriaPrivate>,nullQContentSortCriteria,
        (new QContentSortCriteriaPrivate) );

/*!
    \class QContentSortCriteria
    \inpublicgroup QtBaseModule

    \brief The QContentSortCriteria class defines the attributes and order a QContentSet should be sorted by.

    The sorting criteria is an ordered list of QContent attributes and sort orders which can be used to compare two
    QContents.  When a comparison is made between QContents each attribute in the sorting criteria is compared in
    order until one that is not equal is found and the result of that comparison is returned.  If all comparisons
    are equal the sorting criteria will fall back to the user visible name, and then the content id in ascending order
    to make a comparison.  A comparison with a null QContent will always return the null QContent as the lesser one.

    When sorting by a property it is also necessary to provide an attribute scope to specify the group and key of the
    property that are sorted on.  The scope should be a string of the form \c [group]/[key].  Alternatively if the
    property belongs to the QContent::Property enumeration that may be used instead of an attribute.

    Using the sort criteria a content set containing music sorted by artist, album, and track number could be created as below:
    \code
        QContentSet music;

        music.setCriteria( QContentFilter::mimeType( "audio/mpeg" ) );

        QContentSortCriteria sort;

        sort.addSort( QContentSortCriteria::Property, "none/Artist" );
        sort.addSort( QContentSortCriteria::Property, "none/Album" );
        sort.addSort( QContentSortCriteria::Property, "none/Track" );

        music.setSortCriteria( sort );
    \endcode
*/

/*!
    \enum QContentSortCriteria::Attribute

    The Attribute enumeration defines an attribute of a QContent that a comparison should be made on.

    \value Name The user visible \l{QContent::name()}{name} of the content.
    \value MimeType The \l{QContent::type()}{type} of the content.
    \value Property A \l{QContent::property()}{property} of the content.  The group and key for the property must be supplied as an scope of the sort as a string of the form \c [group]/[key]
    \value FileName The \l{QContent::fileName()}{file name} of the content.
    \value LastUpdated The time and date the content record was \l{QContent::lastUpdated()}{last updated}.
*/

/*!
    Constructs a null sort criteria.
*/
QContentSortCriteria::QContentSortCriteria()
    : d( *nullQContentSortCriteria() )
{
}

/*!
    Constructs a sort criteria that sorts a set by an \a attribute in the given sort \a order.
*/
QContentSortCriteria::QContentSortCriteria( Attribute attribute, Qt::SortOrder order )
{
    d = new QContentSortCriteriaPrivate;

    addSort( attribute, order );
}

/*!
    Constructs a sort criteria that sorts a set by an \a attribute with a \a scope in the given sort \a order.
 */
QContentSortCriteria::QContentSortCriteria( Attribute attribute, const QString &scope, Qt::SortOrder order )
{
    d = new QContentSortCriteriaPrivate;

    addSort( attribute, scope, order );
}

/*!
    Constructs a sort criteria that sorts a set by \a property in the given sort \a order.
*/
QContentSortCriteria::QContentSortCriteria( QContent::Property property, Qt::SortOrder order )
{
    addSort( property, order );
}

/*!
    Constructs a copy of the sort criteria \a other.
*/
QContentSortCriteria::QContentSortCriteria( const QContentSortCriteria &other )
    : d( other.d )
{
}

/*!
    Destroys a sort criteria.
*/
QContentSortCriteria::~QContentSortCriteria()
{
}

/*!
    Assigns the value of \a other to a sort criteria.
*/
QContentSortCriteria &QContentSortCriteria::operator =( const QContentSortCriteria &other )
{
    d = other.d;

    return *this;
}

/*!
    Compares \a other to a sort criteria.  Returns true if it is equal; false otherwise.
*/
bool QContentSortCriteria::operator ==( const QContentSortCriteria &other ) const
{
    if( d == other.d )
        return true;
    else
        return d->sorts == other.d->sorts;
}

/*!
    Compares \a other to a sort criteria.  Returns true if it is not equal; false otherwise.
 */
bool QContentSortCriteria::operator !=( const QContentSortCriteria &other ) const
{
    if( d == other.d )
        return false;
    else
        return d->sorts != other.d->sorts;
}

/*!
    Adds an \a attribute to sort by in the given \a order to the end of the sort criteria.
*/
void QContentSortCriteria::addSort( Attribute attribute, Qt::SortOrder order )
{
    d->sorts.append( QContentSortCriteriaPrivate::Sort( attribute, QString(), order ) );
}

/*!
    Adds an \a attribute with a \a scope to sort by in the given \a order to the end of the sort criteria.
 */
void QContentSortCriteria::addSort( Attribute attribute, const QString &scope, Qt::SortOrder order )
{
    d->sorts.append( QContentSortCriteriaPrivate::Sort( attribute, scope, order ) );
}

/*!
    Adds a \a property to sort by in the given \a order to the end of the sort criteria.
 */
void QContentSortCriteria::addSort( QContent::Property property, Qt::SortOrder order )
{
    d->sorts.append( QContentSortCriteriaPrivate::Sort( Property, QLatin1String( "none/" ) + QContent::propertyKey( property ), order ) );
}

/*!
    Removes all sorts from the sorting criteria.
*/
void QContentSortCriteria::clear()
{
    d->sorts.clear();
}

/*!
    Returns the number of attributes the sorting criteria sorts on.
*/
int QContentSortCriteria::sortCount() const
{
    return d->sorts.count();
}

/*!
    Returns the attribute sorted by at the given \a index.
*/
QContentSortCriteria::Attribute QContentSortCriteria::attribute( int index ) const
{
    return d->sorts.at( index ).attribute;
}

/*!
    Returns the scope of the attribute sorted by at the given \a index.
 */
QString QContentSortCriteria::scope( int index ) const
{
    return d->sorts.at( index ).scope;
}

/*!
    Returns the order the attribute at the given \a index is sorted by.
*/
Qt::SortOrder QContentSortCriteria::order( int index ) const
{
    return d->sorts.at( index ).order;
}

/*!
    Compares the \a left and \a right contents using the sorting criteria and returns a negative integer
    if the left is less than the right, a positive integer if the left is greater than the right, and zero
    if they are equivalent.
*/
int QContentSortCriteria::compare( const QContent &left, const QContent &right ) const
{
    int comparison = 0;

    foreach( const QContentSortCriteriaPrivate::Sort sort, d->sorts )
        if( (comparison = QContentSortCriteriaPrivate::compare( sort, left, right )) != 0 )
            return sort.order == Qt::AscendingOrder
                    ? comparison
                    : -comparison;

    if( (comparison = QString::localeAwareCompare( left.name(), right.name() )) == 0 )
    {
        if( left.id().second < right.id().second )
            return -1;
        else if( left.id().second > right.id().second )
            return 1;
    }

    return comparison;
}

/*!
    Compares the \a left and \a right contents using the sorting criteria and returns true if the left
    is less than the right and false otherwise.
*/
bool QContentSortCriteria::lessThan( const QContent &left, const QContent &right ) const
{
    int comparison = 0;

    foreach( const QContentSortCriteriaPrivate::Sort sort, d->sorts )
        if( (comparison = QContentSortCriteriaPrivate::compare( sort, left, right )) != 0 )
            return sort.order == Qt::AscendingOrder
                    ? (comparison < 0)
                    : (comparison > 0);

    if( (comparison = QString::localeAwareCompare( left.name(), right.name() )) != 0 )
        return comparison < 0;
    else
        return left.id().second < right.id().second;
}

/*!
    Compares the \a left and \a right contents using the sorting criteria and returns true if the left
    is greater than the right and false otherwise.
*/
bool QContentSortCriteria::greaterThan( const QContent &left, const QContent &right ) const
{
    int comparison = 0;

    foreach( const QContentSortCriteriaPrivate::Sort sort, d->sorts )
        if( (comparison = QContentSortCriteriaPrivate::compare( sort, left, right )) != 0 )
            return sort.order == Qt::AscendingOrder
                    ? (comparison > 0)
                    : (comparison < 0);

    if( (comparison = QString::localeAwareCompare( Qtopia::dehyphenate( left.name() ), Qtopia::dehyphenate( right.name() ) )) != 0 )
        return comparison > 0;
    else
        return left.id().second > right.id().second;
}

/*!
    \fn QContentSortCriteria::serialize(Stream &stream) const

    \internal
*/
template <typename Stream> void QContentSortCriteria::serialize(Stream &stream) const
{
    stream << d->sorts.count();

    foreach( const QContentSortCriteriaPrivate::Sort &sort, d->sorts )
    {
        stream << sort.attribute;
        stream << sort.scope;
        stream << sort.order;
    }
}

/*!
    \fn QContentSortCriteria::deserialize(Stream &stream)

    \internal
 */
template <typename Stream> void QContentSortCriteria::deserialize(Stream &stream)
{
    d->sorts.clear();

    int size;
    Attribute attribute;
    QString scope;
    int order;

    stream >> size;

    for( int i = 0; i < size; i++ )
    {
        stream >> attribute;
        stream >> scope;
        stream >> order;

        addSort( attribute, scope, static_cast< Qt::SortOrder >( order ) );
    }
}

Q_IMPLEMENT_USER_METATYPE(QContentSortCriteria);
Q_IMPLEMENT_USER_METATYPE_ENUM(QContentSortCriteria::Attribute);


