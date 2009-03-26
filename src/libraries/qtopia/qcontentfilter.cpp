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

#include "qcontentfilter.h"
#include <QRegExp>
#include <QMultiHash>
#include "drmcontent_p.h"
#include <QtDebug>
#include <QDir>
#include <QMimeType>
#include <QCategoryFilter>
#include "qcontentstore_p.h"

class QContentFilterPrivate : public QSharedData
{
public:
    QContentFilter::Operand operand;

    bool negated;

    QMultiHash< QContentFilter::FilterType, QString > arguments;

    QList< QContentFilter > subFilters;
};

// See qcontentfilter.qdoc for class documentation

/*!
    \enum QContentFilter::Operand
    Represents the operand used to combine a group of filters.
    \value NoOperand The operand for an invalid filter or a filter with a single argument and no sub filters.
    \value And The filter passes the intersection of all its arguments and sub filters.
    \value Or The filter passes the union of all its arguments and sub filters.
*/

/*!
    \enum QContentFilter::FilterType

    Identifies the QContent property that a filter argument is compared with.

    \value Location The root path of the storage media or mount the content is stored on.
    \value Role The value of QContent::role(); Application, Document or Data.
    \value MimeType The value of QContent::type(); audio/mpeg, image/gif, etc.
    \value Directory The directory the content is located in.
    \value Category The ID of a category assigned to the content.
    \value DRM The \l{QContent::DrmState}{DRM state} of the content.
    \value Property The value of a content \l{QContent::property()}{property}.  Arguments are of the form \c [group]/[key]/[value].
    \value Name Filters on the the value of QContent::name().
    \value FileName Filters on the file name of content excluding the path.
    \value QtopiaType QtopiaType has been deprecated. Please use \c Role instead.
    \value Synthetic The Synthetic filter type has been renamed to \c Property.
    \value Unknown Invalid filter argument.
*/

 /*!
    Constructs an invalid \c QContentFilter which will not pass any content.
*/
QContentFilter::QContentFilter()
    : d( 0 )
{
}

/*!
    Constructs a copy of the filter \a other.
*/
QContentFilter::QContentFilter( const QContentFilter &other )
    : d( other.d )
{
}

/*!
    Constructs a filter with the FilterType \a type and the value to match \a argument.
*/
QContentFilter::QContentFilter( FilterType type, const QString &argument )
{
    d = new QContentFilterPrivate;

    d->operand = NoOperand;
    d->negated = false;
    d->arguments.insert( type, argument );
}

/*!
    Constructs a filter which passes content whose property \a property matches \a value.
*/
QContentFilter::QContentFilter( QContent::Property property, const QString &value )
{
    d = new QContentFilterPrivate;

    QString argument = QLatin1String( "none/" ) + QContent::propertyKey( property );

    if( !value.isEmpty() )
        argument += '/' + value;

    d->operand = NoOperand;
    d->negated = false;
    d->arguments.insert( Synthetic, argument );
}

/*!
    Constructs a filter that passes content with the given content \a role.
*/
QContentFilter::QContentFilter( QContent::Role role )
    : d( 0 )
{
    QString argument;

    switch( role )
    {
    case QContent::UnknownUsage:
        argument = QString();
        break;
    case QContent::Document:
        argument = QLatin1String( "Document" );
        break;
    case QContent::Data:
        argument = QLatin1String( "Data" );
        break;
    case QContent::Application:
        argument = QLatin1String( "Application" );
        break;
    case QContent::Folder:
        argument = QLatin1String( "Folder" );
    }

    if( !argument.isEmpty() )
    {
        d = new QContentFilterPrivate;

        d->operand = NoOperand;
        d->negated = false;

        d->arguments.insert( QtopiaType, argument );
    }
}

/*!
    Constructs a filter that passes content with the given \a mime type.
*/
QContentFilter::QContentFilter( const QMimeType &mime )
{
    d = new QContentFilterPrivate;

    d->operand = NoOperand;
    d->negated = false;
    d->arguments.insert( MimeType, mime.id() );
}

/*!
    Constructs a filter that passes content matching the given category \a filter.
*/
QContentFilter::QContentFilter( const QCategoryFilter &filter )
{
    if( filter.acceptAll() )
    {
        d = 0;
    }
    else if( filter.acceptUnfiledOnly() )
    {
        d = new QContentFilterPrivate;

        d->operand = NoOperand;
        d->negated = false;
        d->arguments.insert( Category, QLatin1String( "Unfiled" ) );
    }
    else
    {
        d = new QContentFilterPrivate;

        d->operand = filter.requiredCategories().count() > 1 ? And : NoOperand;
        d->negated = false;

        foreach( QString argument, filter.requiredCategories() )
            d->arguments.insert( Category, argument );
    }
}

/*!
    Destroys a \c QContentFilter.
*/
QContentFilter::~QContentFilter()
{
}

/*!
    Assigns \a other to this \c QContentFilter.
*/
QContentFilter &QContentFilter::operator =( const QContentFilter &other )
{
    d = other.d;

    return *this;
}

/*!
    Creates a negated copy of a\c  QContentFilter.

    The new filter will pass all content not passed by the existing \c QContentFilter.

    \sa negated()
*/
QContentFilter QContentFilter::operator ~() const
{
    if( !d )
        return QContentFilter();

    QContentFilter newFilter = *this;

    newFilter.d->negated = !d->negated;

    return newFilter;
}

/*!
    Returns a new \c QContentFilter which will pass the intersection of this filter
    and \a other.

    Combining an invalid QContentFilter and a valid one will return the valid QContentFilter.

    \sa And, operator&=()
*/
QContentFilter QContentFilter::operator &( const QContentFilter &other ) const
{
    if( !d )
        return other;

    if( !other.d )
        return *this;

    QContentFilter newFilter;

    newFilter.d = new QContentFilterPrivate;

    newFilter.d->negated = false;
    newFilter.d->operand = And;

    if( d->operand != Or && !d->negated && other.d->operand != Or && !other.d->negated )
    {
        newFilter.d->subFilters = d->subFilters + other.d->subFilters;
        newFilter.d->arguments  = d->arguments  + other.d->arguments;
    }
    else
    {
        newFilter.d->subFilters.append( *this );
        newFilter.d->subFilters.append( other );
    }

    return newFilter;
}

/*!
    Creates a new \c QContentFilter which will pass the union of this filter
    and \a other.

    Combining an invalid QContentFilter and a valid one will return the valid QContentFilter.

    \sa Or, operator|=()
 */
QContentFilter QContentFilter::operator |( const QContentFilter &other ) const
{
    if( !d )
        return other;

    if( !other.d )
        return *this;

    QContentFilter newFilter;

    newFilter.d = new QContentFilterPrivate;

    newFilter.d->negated = false;
    newFilter.d->operand = Or;

    if( d->operand != And && !d->negated && other.d->operand != And && !other.d->negated )
    {
        newFilter.d->subFilters = d->subFilters + other.d->subFilters;
        newFilter.d->arguments  = d->arguments  + other.d->arguments;
    }
    else
    {
        newFilter.d->subFilters.append( *this );
        newFilter.d->subFilters.append( other );
    }

    return newFilter;
}

/*!
    Restricts this \c QContentFilter to the intersection of it and another \c QContentFilter \a other.

    If the QContentFilter is invalid, it will be assigned \a other.

    \sa And, operator&()
*/
QContentFilter &QContentFilter::operator &=( const QContentFilter &other )
{
    *this = *this & other;

    return *this;
}

/*!
    Restricts this \c QContentFilter to the union of it and another \c QContentFilter \a other.

    If the QContentFilter is invalid, it will be assigned \a other.

    \sa Or, operator|()
 */
QContentFilter &QContentFilter::operator |=( const QContentFilter &other )
{
    *this = *this | other;

    return *this;
}

/*!
    Compares a QContentFilter to \a other.  Returns true if they are equal and false otherwise.
*/
bool QContentFilter::operator ==( const QContentFilter &other ) const
{
    if( !d && !other.d )
        return true;
    if( (!d && other.d) || (d && !other.d) )
        return false;
    else
    {
        return d->operand    == other.d->operand   &&
               d->negated    == other.d->negated   &&
               d->arguments  == other.d->arguments &&
               d->subFilters == other.d->subFilters;
    }
}

/*!
    Compares a QContentFilter to \a other.  Returns true if they are not equal and false otherwise.
 */
bool QContentFilter::operator !=( const QContentFilter &other ) const
{
    if( !d && !other.d )
        return false;
    if( (!d && other.d) || (d && !other.d) )
        return true;
    else
    {
        return d->operand    != other.d->operand   ||
               d->negated    != other.d->negated   ||
               d->arguments  != other.d->arguments ||
               d->subFilters != other.d->subFilters;
    }
}

/*!
    Constructs a content filter that filters for content with a \l{QContent::type()}{mime type} value that
    matches \a mimeType.
*/
QContentFilter QContentFilter::mimeType( const QString &mimeType )
{
    return QContentFilter( MimeType, mimeType );
}

/*!
    Constructs a content filter that filters for content belonging to the category with the id \a categoryId.
*/
QContentFilter QContentFilter::category( const QString &categoryId )
{
    return QContentFilter( Category, categoryId );
}

/*!
    Constructs a content filter that filters for content with a \l{QContent::name()}{name} value that matches \a name.
*/
QContentFilter QContentFilter::name( const QString &name )
{
    return QContentFilter( Name, name );
}

/*!
    Constructs a content filter that filters for content with a \l{QContent::fileName()}{file name} value that matches
    \a fileName.
*/
QContentFilter QContentFilter::fileName( const QString &fileName )
{
    return QContentFilter( FileName, fileName );
}

/*!
    Constructs a content filter that filters for content with the given \l{QContent::property()}{property} \a group and
    \a key that matches \a value.
 */
QContentFilter QContentFilter::property( const QString &group, const QString &key, const QString &value )
{
    return QContentFilter( Synthetic, group + QLatin1Char( '/' ) + key + QLatin1Char( '/' ) + value );
}

/*!
    Constructs a content filter that filters for content with the given \l{QContent::property()}{property} \a key and
    no group that matches \a value.
 */
QContentFilter QContentFilter::property( const QString &key, const QString &value )
{
    return QContentFilter( Synthetic, QLatin1String( "none/" ) + key + QLatin1Char( '/' ) + value );
}

/*!
    Returns the operand used to combine the filter arguments and sub filters.

    \sa types(), arguments(), subFilters()
*/
QContentFilter::Operand QContentFilter::operand() const
{
    return d ? d->operand : NoOperand;
}

/*!
    Returns true if the filter has been negated.

    \sa operator~()
*/
bool QContentFilter::negated() const
{
    return d ? d->negated : false;
}

/*!
    Returns a list of filter types common to the arguments the filter is composed of, not including sub filters.

    \sa arguments(), subFilters()
*/
QList< QContentFilter::FilterType > QContentFilter::types() const
{
    return d ? d->arguments.uniqueKeys() : QList< FilterType >();
}

/*!
    Returns the filter arguments of FilterType \a type, not including sub filters.

    \sa types(), subFilters()
*/
QStringList QContentFilter::arguments( FilterType type ) const
{
    return d ? QStringList(d->arguments.values( type )) : QStringList();
}

/*!
    Returns any sub filters the the filter is composed of.

    \sa types(), arguments()
*/
QList< QContentFilter > QContentFilter::subFilters() const
{
    return d ? d->subFilters : QList< QContentFilter >();
}

/*!
    Returns true if the filter is valid. A \c QContentFilter constructed with no
    arguments is invalid.

    \sa clear()
*/
bool QContentFilter::isValid() const
{
    return d != 0;
}

/*!
    Clears the contents of a filter, invalidating it.

    \sa isValid()
*/
void QContentFilter::clear()
{
    d = 0;
}

/*!
    Returns true if a QContent object \a content passes the filter; otherwise returns false.
*/
bool QContentFilter::test( const QContent &content ) const
{
    if( !d )
        return false;

    bool pass;

    if( d->operand == Or || d->operand == NoOperand )
    {
        pass = false;

        QMultiHash< FilterType, QString >::const_iterator it, end;

        for( it = d->arguments.begin(), end = d->arguments.end(); it != end; it++ )
            if( test( content, it.key(), it.value() ) ){
                pass = true; break; }

        if( !pass )
            foreach( QContentFilter subFilter, d->subFilters )
                if( subFilter.test( content ) ){
                    pass = true; break; }
    }
    else
    {
        pass = true;

        QMultiHash< FilterType, QString >::const_iterator it, end;

        for( it = d->arguments.begin(), end = d->arguments.end(); it != end; it++ )
            if( !test( content, it.key(), it.value() ) ){
                pass = false; break; }

        if( pass )
            foreach( QContentFilter subFilter, d->subFilters )
                if( !subFilter.test( content ) ){
                    pass = false; break; }
    }

    return d->negated ? !pass : pass;
}

/*!
    Returns a list of filter arguments of FilterType \a type that can be used to further narrow the matches returned by a QContentFilter.

    The \a scope is used to optionally restrict matches to a sub group of arguments matching the filter. The scope of a
    MimeType filter is the mime major type, so for example the \c image scope will return arguments like \c image/jpeg,
    and \c image/png. For Category filters scope simply refers to the category scope, and for Synthetic filters it is
    the property group and key in the form \c [group]/[key].
*/
QStringList QContentFilter::argumentMatches( FilterType type, const QString &scope ) const
{
    return QContentStore::instance()->filterMatches( *this, type, scope );
}

/*!
    Returns true if the QContent object \a content passes the filter \a argument of FilterType \a type.
*/
bool QContentFilter::test( const QContent &content, QContentFilter::FilterType type, const QString &argument )
{
    switch( type )
    {
        case MimeType:
            return QRegExp( argument, Qt::CaseInsensitive, QRegExp::Wildcard ).exactMatch( content.type() );
        case Location:
            return content.fileName() == argument || content.fileName().startsWith( argument + '/' );
        case Directory:
        {
            QRegExp exp( argument, Qt::CaseSensitive, QRegExp::Wildcard  );

            return exp.exactMatch( content.fileName() );
        }
        case Role:
            return ( content.role() == QContent::Application && argument == QLatin1String("Application") ) ||
                   ( content.role() == QContent::Document    && argument == QLatin1String("Document"   ) ) ||
                   ( content.role() == QContent::Data        && argument == QLatin1String("Data"       ) ) ||
                   ( content.role() == QContent::Folder      && argument == QLatin1String("Folder"     ) );
        case Category:
            return content.categories().contains( argument );
        case Synthetic:
            return QRegExp( argument.section( "/", 1 ), Qt::CaseInsensitive, QRegExp::Wildcard )
                    .exactMatch( argument.section( "/", 0, 0 ) );
        case DRM:
            return content.drmState() == QContent::Protected && argument == QLatin1String( "Protected" );
        case FileName:
            return QRegExp( argument, Qt::CaseSensitive, QRegExp::Wildcard )
                    .exactMatch( content.fileName().section( "/", -1 ) );
        case Name:
            return QRegExp( argument, Qt::CaseInsensitive, QRegExp::Wildcard )
                    .exactMatch( content.name() );
        default:
            return true;
    }
}

/*!
    \fn QContentFilter::serialize(Stream &stream) const
    \internal
*/
template <typename Stream> void QContentFilter::serialize(Stream &stream) const
{
    if( d )
    {
        stream << d->negated;
        stream << d->operand;
        stream << d->arguments;
        stream << d->subFilters;
    }
    else
    {
        stream << false;
        stream << NoOperand;
        stream << QMultiHash< QContentFilter::FilterType, QString >();
        stream << QList< QContentFilter >();
    }
}

/*!
    \fn QContentFilter::deserialize(Stream &stream)
    \internal
 */
template <typename Stream> void QContentFilter::deserialize(Stream &stream)
{
    bool negated;
    QContentFilter::Operand operand;
    QMultiHash< QContentFilter::FilterType, QString > arguments;
    QList< QContentFilter > subFilters;

    stream >> negated;
    stream >> operand;
    stream >> arguments;
    stream >> subFilters;

    if( !arguments.isEmpty() || !subFilters.isEmpty() )
    {
        d = new QContentFilterPrivate;

        d->negated    = negated;
        d->operand    = operand;
        d->arguments  = arguments;
        d->subFilters = subFilters;
    }
    else
        d = 0;
}

/*!
    Writes a description of a content \a filter to a \a debug stream.

    \internal
*/
QDebug operator <<( QDebug debug, const QContentFilter &filter )
{
    if( !filter.isValid() )
    {
        debug << "()";

        return debug;
    }

    if( filter.negated() )
        debug << "~(";
    else
        debug << "(";

    QContentFilter::Operand operand = QContentFilter::NoOperand;

    foreach( QContentFilter::FilterType type, filter.types() )
    {
        foreach( QString argument, filter.arguments( type ) )
        {
            if( operand == QContentFilter::And )
                debug << "&";
            else if( operand == QContentFilter::Or )
                debug << "|";

            switch( type )
            {
            case QContentFilter::Location:
                debug << "Location:";
                break;
            case QContentFilter::Role:
                debug << "Role:";
                break;
            case QContentFilter::MimeType:
                debug << "MimeType:";
                break;
            case QContentFilter::Directory:
                debug << "Directory:";
                break;
            case QContentFilter::Category:
                debug << "Category:";
                break;
            case QContentFilter::DRM:
                debug << "DRM:";
                break;
            case QContentFilter::Synthetic:
                debug << "Synthetic:";
                break;
            case QContentFilter::FileName:
                debug << "FileName:";
                break;
            case QContentFilter::Name:
                debug << "Name:";
            break;
            case QContentFilter::Unknown:
                debug << "Unknown:";
            }

            debug << argument.toLatin1().constData();

            operand = filter.operand();
        }
    }

    foreach( QContentFilter f, filter.subFilters() )
    {
        if( operand == QContentFilter::And )
            debug << "&";
        else if( operand == QContentFilter::Or )
            debug << "|";

        debug << f;

        operand = filter.operand();
    }

    debug << ")";

    return debug;
}
Q_IMPLEMENT_USER_METATYPE(QContentFilter)
Q_IMPLEMENT_USER_METATYPE_ENUM(QContentFilter::Operand)
Q_IMPLEMENT_USER_METATYPE_ENUM(QContentFilter::FilterType)

