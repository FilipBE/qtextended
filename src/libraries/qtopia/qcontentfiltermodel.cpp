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

#include "qcontentfiltermodel.h"
#include <QtDebug>
#include <qcategorymanager.h>
#include <QStorageMetaInfo>
#include <QObject>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <qtopiaapplication.h>

/*!
    \enum QContentFilterModel::TemplateOption

    Provides optional features to be used when generating lists.

    \value NoTemplateOptions Don't display any additional list items or check boxes.
    \value ForceAll Always display a select all option at the start of a list.
    \value SelectAll Display a select all option at the start of a list if there is more than one other item in the list.
    \value CheckList The list is a checklist.
    \value AndCheckedFilters When determining the selected filter filters are and'ed together.  By default filters are or'ed.
    \value ShowEmptyLabels Forces labels to be shown irregardless of whether they match any content in the database.
*/

/*!
    \typedef QContentFilterModel::TemplateOptions

    Synonym for QFlags< QContentFilterModel::TemplateOption >
*/

/*
    Data for label or list item in a template.
*/
class TemplateItem
{
public:
    TemplateItem()
        : filterType( QContentFilter::Unknown )
        , checkedLabel( false )
    {
    }

    QContentFilterModel::Template target;
    QString title;
    QContentFilter::FilterType filterType;
    QString filterScope;
    QContentFilter filter;
    QStringList checkedList;
    bool checkedLabel;
};

/*
    Private class for QContentFilterModel::Template.
*/
class QContentFilterModelTemplatePrivate : public QSharedData
{
public:
    QContentFilterModelTemplatePrivate()
        : options( QContentFilterModel::SelectAll )
    {
    }

    QList< TemplateItem > items;
    QContentFilter filter;
    QContentFilterModel::TemplateOptions options;
};

/*!
    \class QContentFilterModel::Template
    \inpublicgroup QtBaseModule

    \brief The Template class enables the construction of complex QContentFilterModels with fixed entries and multiple lists.

    A basic content filter model consists of just a flat list of content filters of a common type. A content filter model
    template adds options to extend these lists to include fixed labels, and multiple lists based on different criteria.

    Templates also allows list items to link to child templates to create hierachial filter models where the contents
    of each child list is filtered by the parent filter item.

    \sa QContentFilterModel

  \ingroup content
*/

/*!
    Constructs a new empty Template.
*/
QContentFilterModel::Template::Template()
{
    d = new QContentFilterModelTemplatePrivate;
}

/*!
    Constructs a new Template for a list of filters with the FilterType \a type and specialization \a scope.

    By default no template \a options will be set.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
*/
QContentFilterModel::Template::Template( QContentFilter::FilterType type, const QString &scope, TemplateOptions options, const QStringList &checked )
{
    d = new QContentFilterModelTemplatePrivate;

    d->options = options;

    addList( type, scope, checked );
}

/*!
    Constructs a new Template for a list of filters for selecting from the property \a property.

    By default no template \a options will be set.

    The properties listed in the \a checked list will be checked by default.
*/
QContentFilterModel::Template::Template( QContent::Property property, TemplateOptions options, const QStringList &checked )
{
    d = new QContentFilterModelTemplatePrivate;

    d->options = options;

    addList( property, checked );
}

/*!
    Constructs a new Template for a list of filters with the FilterType \a type and specialization \a scope.

    Each filter in the list will parent a sub list defined by the Template \a target.

    By default no template \a options will be set.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
*/
QContentFilterModel::Template::Template( const Template &target, QContentFilter::FilterType type, const QString &scope, TemplateOptions options, const QStringList &checked )
{
    d = new QContentFilterModelTemplatePrivate;

    d->options = options;

    addList( target, type, scope, checked );
}

/*!
    Constructs a new Template for a list of filters for selecting from the property \a property.

    Each filter in the list will parent a sub list defined by the Template \a target.

    By default no template \a options will be set.

    The properties listed in the \a checked list will be checked by default.
*/
QContentFilterModel::Template::Template( const Template &target, QContent::Property property, TemplateOptions options, const QStringList &checked )
{
    d = new QContentFilterModelTemplatePrivate;

    d->options = options;

    addList( target, property, checked );
}

/*!
    Creates a copy of \a other.
*/
QContentFilterModel::Template::Template( const Template &other )
{
    *this = other;
}

/*!
    Destroys a Template.
*/
QContentFilterModel::Template::~Template()
{
}

/*!
    Assigns \a other a Template.
*/
QContentFilterModel::Template &QContentFilterModel::Template::operator =( const Template &other )
{
    d = other.d;

    return *this;
}

/*!
    Returns true if the template is valid.
*/
bool QContentFilterModel::Template::isValid() const
{
    return !d->items.isEmpty();
}

/*!
    Returns the template options.
*/
QContentFilterModel::TemplateOptions QContentFilterModel::Template::options() const
{
    return d->options;
}

/*!
    Sets the template \a options.
*/
void QContentFilterModel::Template::setOptions( TemplateOptions options )
{
    d->options = options;
}

/*!
    Adds a label with the text \a title to the template, if \a checked is true the label will be checked by default.
*/
void QContentFilterModel::Template::addLabel( const QString &title, bool checked )
{
    TemplateItem item;

    item.title = title;
    item.checkedLabel = checked;

    d->items.append( item );
}

/*!
    Adds a label with the text \a title which selects the QContentFilter \a filter to the template, if \a checked is
    true the label will be checked by default.
*/
void QContentFilterModel::Template::addLabel( const QString &title, const QContentFilter &filter, bool checked )
{
    TemplateItem item;

    item.title  = title;
    item.filter = filter;
    item.checkedLabel = checked;

    d->items.append( item );
}

/*!
    Adds a label with the text \a title to the template, if \a checked is true the label will be checked by default.

    The label will parent a sub list defined by the Template \a target.

*/
void QContentFilterModel::Template::addLabel( const Template &target, const QString &title, bool checked )
{
    TemplateItem item;

    item.target = target;
    item.title  = title;
    item.checkedLabel = checked;

    d->items.append( item );
}

/*!
    Adds a label with the text \a title which selects the QContentFilter \a filter to the template, if \a checked is
    true the label will be checked by default.

    The label will parent a sub list defined by the Template \a target.
*/
void QContentFilterModel::Template::addLabel( const Template &target, const QString &title, const QContentFilter &filter, bool checked )
{
    TemplateItem item;

    item.target       = target;
    item.title        = title;
    item.filter       = filter;
    item.checkedLabel = checked;

    d->items.append( item );
}

/*!
    Adds a list of filters of type \a type within the given \a scope to the template.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
*/
void QContentFilterModel::Template::addList( QContentFilter::FilterType type, const QString &scope, const QStringList &checked )
{
    TemplateItem item;

    item.filterType    = type;
    item.filterScope = scope;
    item.checkedList   = checked;

    d->items.append( item );
}

/*!
    Adds a list of filters of type \a type within the given \a scope to the template.

    Each filter in the list will parent a sub list defined by the Template \a target.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const Template &target, QContentFilter::FilterType type, const QString &scope, const QStringList &checked )
{
    TemplateItem item;

    item.target        = target;
    item.filterType    = type;
    item.filterScope = scope;
    item.checkedList   = checked;

    d->items.append( item );
}

/*!
    Adds a list of filters of type \a type within the given \a scope and filtered by \a filter to the template.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const QContentFilter &filter, QContentFilter::FilterType type, const QString &scope, const QStringList &checked )
{
    TemplateItem item;

    item.filterType    = type;
    item.filterScope = scope;
    item.filter        = filter;
    item.checkedList   = checked;

    d->items.append( item );
}

/*!
    Adds a list of filters of type \a type within the given \a scope and filtered by \a filter to the template.

    Each filter in the list will parent a sub list defined by the Template \a target.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const Template &target, const QContentFilter &filter, QContentFilter::FilterType type, const QString &scope, const QStringList &checked )
{
    TemplateItem item;

    item.target        = target;
    item.filterType    = type;
    item.filterScope = scope;
    item.filter        = filter;
    item.checkedList   = checked;

    d->items.append( item );
}

/*!
    Adds a list of filters based on the property \a property to the template.

    The properties listed in the \a checked list will be checked by default.
*/
void QContentFilterModel::Template::addList( QContent::Property property, const QStringList &checked )
{
    addList( QContentFilter::Synthetic, QLatin1String( "none/" ) + QContent::propertyKey( property ), checked );
}

/*!
    Adds a list of filters based on the property \a property to the template.

    Each filter in the list will parent a sub list defined by the Template \a target.

    The properties listed in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const Template &target, QContent::Property property, const QStringList &checked )
{
    addList( target, QContentFilter::Synthetic, QLatin1String( "none/" ) + QContent::propertyKey( property ), checked );
}

/*!
    Adds a list of filters filtered by \a filter and based on the property \a property to the template.

    The properties listed in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const QContentFilter &filter, QContent::Property property, const QStringList &checked )
{
    addList( filter, QContentFilter::Synthetic, QLatin1String( "none/" ) + QContent::propertyKey( property ), checked );
}

/*!
    Adds a list of filters filtered by \a filter and based on the property \a property to the template.

    Each filter in the list will parent a sub list defined by the Template \a target.

    The properties listed in the \a checked list will be checked by default.
 */
void QContentFilterModel::Template::addList( const Template &target, const QContentFilter &filter, QContent::Property property, const QStringList &checked )
{
    addList( target, filter, QContentFilter::Synthetic, QLatin1String( "none/" ) + QContent::propertyKey( property ), checked );
}

/*!
    Returns the filter applied to all items in the template.
*/
QContentFilter QContentFilterModel::Template::filter() const
{
    return d ? d->filter : QContentFilter();
}

/*!
    Sets the \a filter applied to all items in the template.
 */
void QContentFilterModel::Template::setFilter( const QContentFilter &filter )
{
    if( !d )
        d = new QContentFilterModelTemplatePrivate;

    d->filter = filter;
}

class FilterGroup
{
public:
    FilterGroup( const QContentFilterModel::Template &target, const QContentFilter &filter );
    virtual ~FilterGroup();

    virtual QString label( int index ) const = 0;
    virtual QIcon icon( int index ) const;
    virtual bool checked( int index ) const = 0;
    virtual void setChecked( int index, bool checked ) = 0;
    virtual QContentFilter filter( int index ) const = 0;
    virtual QContentFilterModelPrivate *child( int index ) const = 0;
    virtual void setChild( int index, QContentFilterModelPrivate *child ) = 0;
    virtual int count() const = 0;

    virtual void refresh( QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset ) = 0;

    QContentFilterModel::Template target() const;
    QContentFilter filter() const;

private:
    QContentFilterModel::Template m_target;
    QContentFilter m_filter;
};

class QContentFilterModelPrivate
{
public:
    enum Selection
    {
        Unknown,
        None,
        All,
        Partial
    };

    QContentFilterModelPrivate( QContentFilterModelPrivate *parent = 0 );
    QContentFilterModelPrivate( const QContentFilterModel::Template &pageTemplate, const QContentFilter &filter, QContentFilterModelPrivate *parent = 0 );
    virtual ~QContentFilterModelPrivate();

    QString label( int index ) const;
    QIcon icon( int index ) const;
    bool checked( int index ) const;
    bool setChecked( int index, bool checked );
    QContentFilter filter( int index ) const;
    QContentFilterModelPrivate *child( int index ) const;
    void setChild( int index, QContentFilterModelPrivate *child );
    QContentFilterModel::Template target( int index ) const;
    int count() const;

    int childIndex( const QContentFilterModelPrivate *child ) const;

    void invalidateSelection();
    void clearSelection();
    Selection selection() const;
    Selection selection( int index ) const;

    QContentFilter filter( int group, int index );
    int count( int group ) const;

    QContentFilterModel::Template pageTemplate() const;
    QContentFilter filter() const;

    void setPageTemplate( const QContentFilterModel::Template &pageTemplate );
    void setFilter( const QContentFilter &filter );

    QContentFilterModelPrivate *parent() const;

    bool dirty() const;
    bool selectAll() const;

    void repopulate();

    void refresh( QContentFilterModel *model );

    QContentFilter checkedFilter() const;

    void beginRemoveRows( QContentFilterModel *model, int start, int end );
    void endRemoveRows( QContentFilterModel *model );
    void beginInsertRows( QContentFilterModel *model, int start, int end );
    void endInsertRows( QContentFilterModel *model );

private:
    QList< FilterGroup * > m_groups;
    QContentFilterModel::Template m_template;
    QContentFilter m_filter;
    QContentFilterModelPrivate *m_parent;
    Selection m_selection;
    int m_count;
    bool m_dirty;
    bool m_selectAll;
};

struct FilterListItem
{
    FilterListItem( QString label, QString filterBody )
        : label( label )
        , filterBody( filterBody )
        , child( 0 )
        , checked( false )
    {
    }

    FilterListItem( QString label, QString filterBody, QIcon icon )
        : label( label )
        , filterBody( filterBody )
        , icon( icon )
        , child( 0 )
        , checked( false )
    {
    }

    ~FilterListItem()
    {
        if( child )
            delete child;
    }

    QString label;
    QString filterBody;
    QIcon icon;
    QContentFilterModelPrivate *child;
    bool checked;
};


class ItemConstructor
{
public:
    virtual ~ItemConstructor(){}

    virtual FilterListItem *create( const QString &filter ) const
    {
        return new FilterListItem( filter, filter );
    }
};


class SyntheticItemConstructor : public ItemConstructor
{
public:
    FilterListItem *create( const QString &filter ) const
    {
        QString name = filter.section( '/', 2 );

        if( name.isEmpty() )
            name = QObject::tr( "Unknown" );

        return new FilterListItem( name, filter );
    }
};

class MimeItemConstructor : public ItemConstructor
{
public:
    FilterListItem *create( const QString &filter ) const
    {
        return new FilterListItem( filter.section( '/', -1 ), filter );
    }
};

class CategoryItemConstructor : public ItemConstructor
{
public:
    CategoryItemConstructor( const QString &scope )
    : categories( scope )
    {
    }

    FilterListItem *create( const QString &filter ) const
    {
        return new FilterListItem( categories.label( filter.section( '/', -1 ) ), filter, categories.icon( filter.section( '/', -1 ) ) );
    }

private:
    QCategoryManager categories;
};


class FilterList : public FilterGroup
{
public:
    FilterList( QContentFilter::FilterType type, const QString &scope, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, const QStringList &checked );
    virtual ~FilterList();

    virtual QString label( int index ) const;
    virtual QIcon icon( int index ) const;
    virtual bool checked( int index ) const;
    virtual void setChecked( int index, bool checked );
    virtual QContentFilter filter( int index ) const;
    virtual QContentFilterModelPrivate *child( int index ) const;
    virtual void setChild( int index, QContentFilterModelPrivate *child );
    virtual int count() const;

    virtual void refresh( QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset );

private:
    void init( const ItemConstructor &constructor, const QStringList &filters, const QStringList &checked );

    void refresh( const ItemConstructor &constructor, const QStringList &filters, QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset );

    QContentFilter::FilterType m_type;
    QString m_scope;
    QList< FilterListItem * > m_items;
};

class FilterLabel : public FilterGroup
{
public:
    FilterLabel( const QString &label, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, bool showEmpty, bool checked );
    FilterLabel( const QString &label, const QIcon &icon, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, bool showEmpty, bool checked );
    virtual ~FilterLabel();

    virtual QString label( int index ) const;
    virtual QIcon icon( int index ) const;
    virtual bool checked( int index ) const;
    virtual void setChecked( int index, bool checked );
    virtual QContentFilter filter( int index ) const;
    virtual QContentFilterModelPrivate *child( int index ) const;
    virtual void setChild( int index, QContentFilterModelPrivate *child );
    virtual int count() const;

    virtual void refresh( QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset );

private:
    QString m_label;
    QIcon m_icon;
    QContentFilterModelPrivate *m_child;
    bool m_checked;
    bool m_visible;
    bool m_showEmpty;
};


FilterGroup::FilterGroup( const QContentFilterModel::Template &target, const QContentFilter &filter )
    : m_target( target )
    , m_filter( filter )
{
}

FilterGroup::~FilterGroup()
{
}

QIcon FilterGroup::icon( int index ) const
{
    Q_UNUSED( index );

    return QIcon();
}

QContentFilterModel::Template FilterGroup::target() const
{
    return m_target;
}

QContentFilter FilterGroup::filter() const
{
    return m_filter;
}

FilterList::FilterList( QContentFilter::FilterType type, const QString &scope, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, const QStringList &checked )
    : FilterGroup( target, filter )
    , m_type( type )
    , m_scope( scope )
{
    if( type == QContentFilter::Unknown )
        return;

    QContentFilter f = parentFilter & filter;

    QStringList filters = f.argumentMatches( type, scope );

    if( filters.count() == 0  && !(type == QContentFilter::Category && scope.isEmpty() ) )
        return;

    switch( type )
    {
    case QContentFilter::Synthetic:
        init( SyntheticItemConstructor(), filters, checked );
        break;
    case QContentFilter::MimeType:
    case QContentFilter::Directory:
    case QContentFilter::Location:
        init( MimeItemConstructor(), filters, checked );
        break;
    case QContentFilter::Category: {
        CategoryItemConstructor cic( m_scope );
        init( cic, filters, checked );
        break;
    }
    default:
        init( ItemConstructor(), filters, checked );
    }
}

FilterList::~FilterList()
{
    qDeleteAll( m_items );
}

QString FilterList::label( int index ) const
{
    return m_items[ index ]->label;
}

QIcon FilterList::icon( int index ) const
{
    return m_items[ index ]->icon;
}

bool FilterList::checked( int index ) const
{
    return m_items[ index ]->checked;
}

void FilterList::setChecked( int index, bool checked )
{
    m_items[ index ]->checked = checked;
}

QContentFilter FilterList::filter( int index ) const
{
    return QContentFilter( m_type, m_items[ index ]->filterBody );
}

QContentFilterModelPrivate *FilterList::child( int index ) const
{
    return m_items[ index ]->child;
}

void FilterList::setChild( int index, QContentFilterModelPrivate *child )
{
    FilterListItem *item = m_items[ index ];

    if( item->child )
        delete item->child;

    item->child = child;
}

int FilterList::count() const
{
    return m_items.count();
}

void FilterList::init( const ItemConstructor &constructor, const QStringList &filters, const QStringList &checked )
{
    if( m_type == QContentFilter::Category && m_scope.isEmpty() ) // First item in global category list is 'Unfiled'
    {
        m_items.append( new FilterListItem( QCategoryManager::unfiledLabel(), QLatin1String( "Unfiled" ) ) );

        if( checked.contains( m_items.last()->filterBody ) )
            m_items.last()->checked = true;
    }

    foreach( QString filter, filters )
    {
        m_items.append( constructor.create( filter ) );

        if( checked.contains( m_items.last()->filterBody ) )
            m_items.last()->checked = true;
    }
}

void FilterList::refresh( QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset )
{
    if( m_type == QContentFilter::Unknown )
        return;

    QContentFilter f = parent->filter() & FilterGroup::filter();

    QStringList filters = f.argumentMatches( m_type, m_scope );

    switch( m_type )
    {
    case QContentFilter::Synthetic:
        refresh( SyntheticItemConstructor(), filters, model, parent, offset );
        break;
    case QContentFilter::MimeType:
    case QContentFilter::Directory:
    case QContentFilter::Location:
        refresh( MimeItemConstructor(), filters, model, parent, offset );
        break;
    case QContentFilter::Category: {
        CategoryItemConstructor cic( m_scope );
        refresh( cic, filters, model, parent, offset );
        break;
    }
    default:
        refresh( ItemConstructor(), filters, model, parent, offset );
    }
}

void FilterList::refresh( const ItemConstructor &constructor, const QStringList &filters, QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset )
{
    int itemIndex = 0;
    int filterIndex = 0;

    if( m_type == QContentFilter::Category && m_scope.isEmpty() ) // First item in global category list is 'Unfiled'
        itemIndex = 1;

    while( itemIndex < m_items.count() || filterIndex < filters.count() )
    {
        if( itemIndex == m_items.count() )
        {
            int start = itemIndex + offset;
            int end   = itemIndex + filters.count() - 1 - filterIndex + offset;

            parent->beginInsertRows( model, start, end );

            for( ; filterIndex < filters.count(); filterIndex++ )
                m_items.append( constructor.create( filters[ filterIndex ] ) );

            parent->endInsertRows( model );

            break;
        }

        if( itemIndex < m_items.count() &&
            filterIndex < filters.count() &&
            m_items[ itemIndex ]->filterBody == filters[ filterIndex ] )
        {
            itemIndex++;
            filterIndex++;
            continue;
        }

        int lastMatch = filters.indexOf( m_items[ itemIndex ]->filterBody, filterIndex + 1 );

        if( lastMatch != -1 )
        {
            int start = itemIndex + offset;
            int end   = itemIndex + lastMatch - 1 - filterIndex + offset;

            parent->beginInsertRows( model, start, end );

            for( ; filterIndex < lastMatch; filterIndex++ )
                m_items.insert( itemIndex++, constructor.create( filters[ filterIndex ] ) );

            parent->endInsertRows( model );

            continue;
        }

        lastMatch = itemIndex;

        while( ++lastMatch < m_items.count() && m_items[ lastMatch ]->filterBody != filters[ filterIndex ] );

        int start = itemIndex + offset;
        int end   = lastMatch - 1 + offset;

        parent->beginRemoveRows( model, start, end );

        while( lastMatch-- != itemIndex )
            delete m_items.takeAt( lastMatch );

        parent->endRemoveRows( model );
    }
}

FilterLabel::FilterLabel( const QString &label, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, bool showEmpty, bool checked )
    : FilterGroup( target, filter )
    , m_label( label )
    , m_child( 0 )
    , m_checked( checked )
    , m_showEmpty( showEmpty )
{
    m_visible = showEmpty || QContentSet::count( parentFilter & filter ) > 0;
}

FilterLabel::FilterLabel( const QString &label, const QIcon &icon, const QContentFilter &parentFilter, const QContentFilterModel::Template &target, const QContentFilter &filter, bool showEmpty, bool checked )
    : FilterGroup( target, filter )
    , m_label( label )
    , m_icon( icon )
    , m_child( 0 )
    , m_checked( checked )
    , m_showEmpty( showEmpty )
{
    m_visible = showEmpty || QContentSet::count( parentFilter & filter ) > 0;
}

FilterLabel::~FilterLabel()
{
    if( m_child )
        delete m_child;
}

QString FilterLabel::label( int index ) const
{
    Q_UNUSED( index );

    return m_label;
}

QIcon FilterLabel::icon( int index ) const
{
    Q_UNUSED( index );

    return m_icon;
}

bool FilterLabel::checked( int index ) const
{
    Q_UNUSED( index );

    return m_checked && m_visible;
}

void FilterLabel::setChecked( int index, bool checked )
{
    Q_UNUSED( index );

    m_checked = checked;
}

QContentFilter FilterLabel::filter( int index ) const
{
    Q_UNUSED( index );

    return QContentFilter();
}

QContentFilterModelPrivate *FilterLabel::child( int index ) const
{
    Q_UNUSED( index );

    return m_child;
}

void FilterLabel::setChild( int index, QContentFilterModelPrivate *child )
{
    Q_UNUSED( index );

    if( m_child )
        delete m_child;

    m_child = child;
}

int FilterLabel::count() const
{
    return m_visible ? 1 : 0;
}

void FilterLabel::refresh( QContentFilterModel *model, QContentFilterModelPrivate *parent, int offset )
{
    Q_UNUSED( offset );
    Q_UNUSED( parent );

    bool visible = m_showEmpty || QContentSet::count( parent->filter() & FilterGroup::filter() ) > 0;

    if( m_visible && !visible )
    {
        parent->beginRemoveRows( model, offset, offset );

        m_visible = false;

        if( m_child )
        {
            delete m_child;

            m_child = 0;
        }

        parent->endRemoveRows( model );
    }
    else if( !m_visible && visible )
    {
        parent->beginInsertRows( model, offset, offset );

        m_visible = true;

        parent->endInsertRows( model );
    }
    else if( visible && m_child )
        m_child->refresh( model );
}

QContentFilterModelPrivate::QContentFilterModelPrivate( QContentFilterModelPrivate *parent )
    : m_parent( parent )
    , m_selection( Unknown )
    , m_dirty( true )
    , m_selectAll( false )
{
}

QContentFilterModelPrivate::QContentFilterModelPrivate( const QContentFilterModel::Template &pageTemplate, const QContentFilter &filter, QContentFilterModelPrivate *parent )
    : m_template( pageTemplate )
    , m_filter( filter )
    , m_parent( parent )
    , m_selection( Unknown )
    , m_dirty( true )
    , m_selectAll( false )
{
    repopulate();
}

QContentFilterModelPrivate::~QContentFilterModelPrivate()
{
    qDeleteAll( m_groups );
}

QString QContentFilterModelPrivate::label( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
            return QObject::tr( "All" );
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->label( index );
        else
            index -= group->count();
    }

    return QString();
}

QIcon QContentFilterModelPrivate::icon( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
            return QIcon();
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->icon( index );
        else
            index -= group->count();
    }

    return QIcon();
}

bool QContentFilterModelPrivate::checked( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
        {
            if( m_parent )
            {
                int index = m_parent->childIndex( this );

                return m_parent->checked( index );
            }
            else
                return selection() == None;
        }
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->checked( index );
        else
            index -= group->count();
    }

    return false;
}

bool QContentFilterModelPrivate::setChecked( int index, bool check )
{
    if( m_selectAll )
    {
        if( index == 0 )
        {
            if( m_parent )
            {
                int index = m_parent->childIndex( this );

                return m_parent->setChecked( index, check );
            }
            else if( check & selection() != None )
            {
                clearSelection();

                return true;
            }
            else
                return false;
        }
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
        {
            group->setChecked( index, check );

            if( group->child( index ) )
                group->child( index )->clearSelection();

            invalidateSelection();

            return true;
        }
        else
            index -= group->count();
    }

    return false;
}

QContentFilter QContentFilterModelPrivate::filter( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
            return QContentFilter();
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->filter() & group->filter( index );
        else
            index -= group->count();
    }

    return QContentFilter();
}

QContentFilterModelPrivate *QContentFilterModelPrivate::child( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
            return 0;
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->child( index );
        else
            index -= group->count();
    }

    return 0;
}

void QContentFilterModelPrivate::setChild( int index, QContentFilterModelPrivate *child )
{
    if( m_selectAll )
    {
        if( index == 0 )
            return;
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
        {
            return group->setChild( index, child );

            return;
        }
        else
            index -= group->count();
    }
}

int QContentFilterModelPrivate::childIndex( const QContentFilterModelPrivate *child ) const
{
    int index = m_selectAll ? 1 : 0;

    foreach( const FilterGroup *group, m_groups )
    {
        for( int i = 0; i < group->count(); i++ )
        {
            if( group->child( i ) == child )
                return index;
            else
                index++;
        }
    }

    return -1;
}

QContentFilterModel::Template QContentFilterModelPrivate::target( int index ) const
{
    if( m_selectAll )
    {
        if( index == 0 )
            return QContentFilterModel::Template();
        else
            index--;
    }

    foreach( FilterGroup *group, m_groups )
    {
        if( index < group->count() )
            return group->target();
        else
            index -= group->count();
    }

    return QContentFilterModel::Template();
}

int QContentFilterModelPrivate::count() const
{
    return m_count;
}

QContentFilter QContentFilterModelPrivate::filter( int group, int index )
{
    return m_groups[ group ]->filter( index );
}

int QContentFilterModelPrivate::count( int group ) const
{
    return m_groups[ group ]->count();
}

QContentFilterModel::Template QContentFilterModelPrivate::pageTemplate() const
{
    return m_template;
}

QContentFilter QContentFilterModelPrivate::filter() const
{
    return m_filter;
}

void QContentFilterModelPrivate::setPageTemplate( const QContentFilterModel::Template &pageTemplate )
{
    m_dirty    = true;
    m_template = pageTemplate;
}

void QContentFilterModelPrivate::setFilter( const QContentFilter &filter )
{
    m_dirty  = true;
    m_filter = filter;
}

bool QContentFilterModelPrivate::dirty() const
{
    return m_dirty;
}

bool QContentFilterModelPrivate::selectAll() const
{
    return m_selectAll;
}

void QContentFilterModelPrivate::repopulate()
{
    qDeleteAll( m_groups );

    m_groups.clear();
    m_count = 0;
    m_selectAll = false;

    const QContentFilterModel::Template temp = m_template;

    if( !temp.isValid() )
        return;

    bool emptyLabels = temp.options() & QContentFilterModel::ShowEmptyLabels;
    bool checkList = temp.options() & QContentFilterModel::CheckList;

    foreach( const TemplateItem &item, temp.d->items )
    {
        FilterGroup *group;

        if( item.filterType == QContentFilter::Unknown )
            group = new FilterLabel(
                    item.title,
                    m_filter & temp.filter(),
                    item.target,
                    item.filter,
                    emptyLabels,
                    item.checkedLabel && checkList );
        else
            group = new FilterList(
                    item.filterType,
                    item.filterScope,
                    m_filter & temp.filter(),
                    item.target,
                    item.filter,
                    checkList ? item.checkedList : QStringList() );

        m_groups.append( group );

        m_count += group->count();
    }

    if( temp.d->options & QContentFilterModel::ForceAll || ( temp.d->options & QContentFilterModel::SelectAll && m_count > 1 ) )
    {
        m_selectAll = true;

        m_count ++;
    }

    m_dirty = false;
}

void QContentFilterModelPrivate::refresh( QContentFilterModel *model )
{
    int offset = m_selectAll ? 1 : 0;

    foreach( FilterGroup *group, m_groups )
    {
        group->refresh( model, this, offset );

        offset += group->count();
    }

    const QContentFilterModel::Template temp = m_template;

    bool newAll = temp.d->options & QContentFilterModel::ForceAll ||
            ( temp.d->options & QContentFilterModel::SelectAll && m_count > ( m_selectAll ? 2 : 1 ) );

    if( newAll && !m_selectAll )
    {
        beginInsertRows( model, 0, 0 );

        m_selectAll = true;

        endInsertRows( model );
    }
    else if( !newAll && m_selectAll )
    {
        beginRemoveRows( model, 0, 0 );

        m_selectAll = false;

        endRemoveRows( model );
    }
}

void QContentFilterModelPrivate::beginRemoveRows( QContentFilterModel *model, int start, int end )
{
    model->beginRemoveRows( this, start, end );

    m_count -= (end - start) + 1;
}

void QContentFilterModelPrivate::endRemoveRows( QContentFilterModel *model )
{
    model->endRemoveRows();
}

void QContentFilterModelPrivate::beginInsertRows( QContentFilterModel *model, int start, int end )
{
    model->beginInsertRows( this, start, end );

    m_count += (end - start) + 1;
}

void QContentFilterModelPrivate::endInsertRows( QContentFilterModel *model )
{
    model->endInsertRows();
}


QContentFilterModelPrivate *QContentFilterModelPrivate::parent() const
{
    return m_parent;
}

QContentFilter QContentFilterModelPrivate::checkedFilter() const
{
    QContentFilter filter;

    foreach( FilterGroup *group, m_groups )
    {
        QContentFilter groupFilter;

        bool childChecked = false;

        for( int i = 0; i < group->count(); i++ )
        {
            QContentFilter childFilter = group->child( i ) ? group->child( i )->checkedFilter() : QContentFilter();

            if( childFilter.isValid() || group->checked( i ) )
            {
                childChecked = true;

                if( m_template.options() & QContentFilterModel::AndCheckedFilters )
                    groupFilter &= group->filter( i ) & childFilter;
                else
                    groupFilter |= group->filter( i ) & childFilter;
            }
        }

        if( childChecked )
        {
            if( m_template.options() & QContentFilterModel::AndCheckedFilters )
                filter &= group->filter() & groupFilter;
            else
                filter |= group->filter() & groupFilter;
        }
    }

    return filter;
}

void QContentFilterModelPrivate::invalidateSelection()
{
    m_selection = Unknown;

    if( m_parent )
        m_parent->invalidateSelection();
}

void QContentFilterModelPrivate::clearSelection()
{
    foreach( FilterGroup *group, m_groups )
    {
        for( int i = 0; i < group->count(); i++ )
        {
            group->setChecked( i, false );

            if( group->child( i ) )
                group->child( i )->clearSelection();
        }
    }

    m_selection = None;

    if( m_parent )
        m_parent->invalidateSelection();
}

QContentFilterModelPrivate::Selection QContentFilterModelPrivate::selection() const
{
    if( m_selection == Unknown )
    {
        Selection selection = Unknown;

        foreach( const FilterGroup *group, m_groups )
        {
            for( int i = 0; i < group->count() && selection != Partial; i++ )
            {
                Selection childSelection = None;

                if( group->checked( i ) )
                    childSelection =  All;
                else if( group->child( i ) )
                    childSelection = group->child( i )->selection();

                if( childSelection == All && m_template.options() & QContentFilterModel::AndCheckedFilters )
                    selection = Partial;
                else if( selection == Unknown )
                    selection = childSelection;
                else if( selection != childSelection )
                    selection = Partial;
            }
        }

        if( selection == Unknown )
            selection = None;

        const_cast< QContentFilterModelPrivate * >( this )->m_selection = selection;
    }

    return m_selection;
}

QContentFilterModelPrivate::Selection QContentFilterModelPrivate::selection( int index ) const
{
    if( m_selectAll && index == 0 )
    {
        if( m_parent )
        {
            Selection s = m_parent->selection( m_parent->childIndex( this ) );

            return s == Partial ? None : s;
        }
        else
            return selection() == Partial ? None : All;
    }
    else if( child( index  ) )
    {
        Selection s = child( index )->selection();

        if( s == None )
            return checked( index ) ? All : None;
        else
            return s;
    }
    else
        return checked( index ) ? All : None;
}


/*!
    \class QContentFilterModel
    \inpublicgroup QtBaseModule

    \brief The QContentFilterModel class defines a model for displaying and selecting possible content filters.

    A basic content filter model is simply a list of content filters of common type which match content passed
    by a base filter. The type of filter listed in the model is specified using the \l QContentFilter::FilterType
    enumeration with some types requiring an additional sub-type.  A base filter may be applied to a content filter
    model to restrict the displayed filters to those which also pass content passed by the base filter.

    \section1 File path lists

    A content filter model constructed with the \l QContentFilter::Location type and no sub-type will list locations
    filters for the root document paths containing content which matches the base filter.  If a directory is passed
    as a sub-type it will list any sub-directories which contain content (including content in sub-directories) matching
    the base filter.

    The \l QContentFilter::Directory filter type will list directory filters for sub-directories of the directory given in
    the sub-type which contain content matching the base filter.  Listed directories will contain matching content
    in the immediate directory, not a sub-directory.  If no directory is given in the sub-type then no filters will be
    listed.

    \section1 Mime type lists

    A list of mime types can be obtained by constructing the content filter model with the \l QContentFilter::MimeType filter
    type.  If a mime major type is given as the the sub-type then only mime-types belonging to that major type will be listed,
    otherwise all mime types within the scope of the base filter will be listed.

    A content filter model listing all available mime types:
    \code
        QContentFilterModel *mimeTypes = new QContentFilterModel( QContentFilter::MimeType );
    \endcode
    Or just the image mime types:
    \code
        QContentFilterModel imageTypes = new QContentFilterModel( QContentFilter::MimeType, "image" );
    \endcode

    \section1 Category lists

    Category filter may be listed using the \l QContentFilter::Category filter type where the sub-type specifies the scope
    of the category filters, if no sub-type is given categories belonging to the global scope will be listed.

    A content filter model containing global categories:
    \code
        QContentFilterModel *categories = new QContentFilterModel( QContentFilter::Category );
    \endcode
    Categories in the "Documents" scope:
    \code
        QContentFilterModel *categories = new QContentFilterModel( QContentFilter::Category, "Documents" );
    \endcode

    \section1 Property lists

    Content filter models which list content property values can be constructed using the \l QContentFilter::Synthetic
    filter type with the property group and key in the sub-type, or using the \l QContent::Property enumeration.  The
    property group and key used in the sub-type should be concatenated in the form "[group]/[key]", if the property does
    not belong to a group the sub-type should be of the form "none/[key]".

    A content filter model listing all artists:
    \code
        QContentFilterModel *artists = new QContentFilterModel( QContentFilterModel::Synthetic, "none/Artist" );
    \endcode
    The same filter model using the QContent::Property enumeration.
    \code
        QContentFilterModel *artists = new QContentFilterModel( QContent::Artist );
    \endcode

    \section1 Templates

    Using templates more complex filter models can be created, templates extend on the filter lists by including static labels,
    concatenation of multiple filter lists, and nested lists.

    \section2 Labels

    Labels allow items to be explicitly added to a list, they may be used to select a static filter or a nested list.  A label
    at a minimum consists of a user visible string usually combined with a a filter or a target sub-template and may include an
    icon. Labels are not sorted they appear in the order they are appended to a list.  By default if the filter for a label does
    not match any known content the label will not be displayed, this can be disabled by setting the
    \l QContentFilterModel::ShowEmptyLabels template option.

    A simple template using labels to select an Application or Document role filter:
    \code
    QContentFilterModel::Template role;
    role.addLabel( tr( "Applications" ), QContentFilter( QContent::Role ) );
    role.addLabel( tr( "Documents" ), QContentFilter( QContent::Document ) );
    \endcode

    \section2 Nested templates

    Templates may be nested to construct a tree model of content filters, templates are nested by specifying a target template
    when adding a list or label to another template.  The contents of a nested template will be filtered by the filters of the
    its parent items and the model's base filter and the filter returned for an index in a tree model will be the filter at
    the index and'ed with the filters of its parents but not the model's base filter.

    A content filter model using a nested template to display artists with albums by an artist as children:
    \code
    QContentFilterModel::Template album( QContent::Album );
    QContentFilterModel::Template artist( album, QContent::Artist );

    QContentFilterModel *model = new QContentFilterModel();

    model->setModelTemplate( album );
    \endcode

    \section1 Multiple selection

    Content filter models may be either single or multiple selection, to make a model multiple selection the
    \l QContentFilterModel::CheckList option should be set in the model template, which will allow list items
    to be checked.  The check list option only applies to list items created from the template it was set in and
    not in any children, in order for all levels of a tree filter model to be able to be checked all the templates must
    have the check list option set.  By default the checked filters are combined using the OR operand, setting the
    \l QContentFilterModel::AndCheckedFilters template option will cause the AND operand to be used instead.

    If the \l QContentFilterModel::CheckList option is set a list of default filter arguments may be supplied when adding a
    list to a template, if the argument of a filter in that list matches one of the default arguments then that filter will
    will be checked when the list is first populated.

    A default select all option may be included in a content filter model by setting either the
    \l QContentFilterModel::SelectAll or \l QContentFilterModel::SelectAll template options in the model template.
    This will include an option with the text 'All' as the first item in a list defined by a template, if SelectAll is set
    then the 'All' item will only be displayed if there are two or more items in the list, if ForceAll is set the 'All' item
    will be shown irregardless of the number of item in a list.  If the 'All' item is appears in the root list of a model it
    will select a null filter and if in a checklist will be automatically checked if no other item is.  If it is in a nested
    list then it will be select its parent item in single selection list or mimic its parent's check state in a multiple
    selection list.

    A template for a checklist of mime types with a select all option:
    \code
    QContentFilterModel::Template mimes(
        QContentFilter::MimeType, QString(), QContentFilterModel::SelectAll | QContentFilterModel::CheckList );
    \endcode

    \sa QContentFilter, QContentFilterSelector, QContentFilterDialog

  \ingroup content
*/

/*!
    Creates a new QContentFilterModel with the given \a parent.
*/
QContentFilterModel::QContentFilterModel( QObject *parent )
    : QAbstractItemModel( parent )
{
    d = new QContentFilterModelPrivate;

    connect( qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
             this, SLOT  (contentChanged(QContentIdList,QContent::ChangeType)) );
}

/*!
    Creates a new content filter model parented to \a parent which lists filters for the property
    \a property according to the given template \a options.

    The properties listed in the \a checked list will be checked by default.
*/
QContentFilterModel::QContentFilterModel( QContent::Property property, TemplateOptions options, const QStringList &checked, QObject *parent )
    : QAbstractItemModel( parent )
{
    d = new QContentFilterModelPrivate;

    setModelTemplate( Template( property, options, checked ) );

    connect( qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
             this, SLOT  (contentChanged(QContentIdList,QContent::ChangeType)) );
}

/*!
    Creates a new content filter model parented to \a parent which lists filters of type \a type within the given
    \a scope according to the template \a options.

    Any filters with an argument that matches one in the \a checked list will be checked by default.
*/
QContentFilterModel::QContentFilterModel( QContentFilter::FilterType type, const QString &scope, TemplateOptions options, const QStringList &checked, QObject *parent )
    : QAbstractItemModel( parent )
{
    d = new QContentFilterModelPrivate;

    setModelTemplate( Template( type, scope, options, checked ) );

    connect( qApp, SIGNAL(contentChanged(QContentIdList,QContent::ChangeType)),
             this, SLOT  (contentChanged(QContentIdList,QContent::ChangeType)) );
}

/*!
    Destroys a QContentFilterModel.
*/
QContentFilterModel::~QContentFilterModel()
{
    delete d;
}

/*!
    Returns the base filter of the model.  Only filters which return content passed
    by the base filter will be listed in the view.
*/
QContentFilter QContentFilterModel::baseFilter() const
{
    return d->filter();
}

/*!
    Sets the base filter of the model to \a filter.  Only filters which return content passed
    by the base filter will be listed in the view.
 */
void QContentFilterModel::setBaseFilter( const QContentFilter &filter )
{
    d->setFilter( filter );

    reset();
}

/*!
    Returns the template which the model is constructed from.
*/
QContentFilterModel::Template QContentFilterModel::modelTemplate() const
{
    return d->pageTemplate();
}

/*!
    Sets the template the model is constructed from to \a modelTemplate.
*/
void QContentFilterModel::setModelTemplate( const Template &modelTemplate )
{
    d->setPageTemplate( modelTemplate );

    reset();
}

/*!
    Returns a filter built from the the checked items that are children of \a parent.
*/
QContentFilter QContentFilterModel::checkedFilter( const QModelIndex &parent ) const
{
    QContentFilterModelPrivate *page
        = parent.isValid()
        ? static_cast< QContentFilterModelPrivate * >( parent.internalPointer() )->child( parent.row() )
        : d;

    return page ? filter( parent ) & page->checkedFilter() : QContentFilter();
}

/*!
    Returns the QContentFilter at \a index.  The returned filter will be and'ed combination
    of the filter at the index and it's parents, not including the models base filter.
*/
QContentFilter QContentFilterModel::filter( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        QContentFilterModelPrivate *page = static_cast< QContentFilterModelPrivate * >( index.internalPointer() );

        if( !page->count() )
            return QContentFilter();

        return filter( index.parent() ) & page->pageTemplate().filter() & page->filter( index.row() );
    }

    return QContentFilter();
}

/*!
    \reimp
 */
int QContentFilterModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );

    return 1;
}

/*!
    \reimp
 */
QVariant QContentFilterModel::data( const QModelIndex &index, int role ) const
{
    if( !index.isValid() )
        return QVariant();

    QContentFilterModelPrivate *page = static_cast< QContentFilterModelPrivate * >( index.internalPointer() );

    if( role == Qt::DisplayRole )
        return page->count() ? page->label( index.row() ) : tr( "No matches found" );
    if( role == Qt::DecorationRole && page->count() )
        return page->icon( index.row() );
    else if( role == Qt::CheckStateRole && page->count() )
    {
        if( page->pageTemplate().options() & CheckList )
        {
            switch( page->selection( index.row() ) )
            {
            case QContentFilterModelPrivate::All:
                return Qt::Checked;
            case QContentFilterModelPrivate::Partial:
                return Qt::PartiallyChecked;
            case QContentFilterModelPrivate::None:
                return Qt::Unchecked;
            case QContentFilterModelPrivate::Unknown:
                break;
            }
        }
    }

    return QVariant();
}

/*!
    \reimp
 */
bool QContentFilterModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if( index.isValid() && role == Qt::CheckStateRole )
    {
        QContentFilterModelPrivate *page = static_cast< QContentFilterModelPrivate * >( index.internalPointer() );

        if( !page->count() )
            return false;

        QContentFilterModelPrivate *parent = page->parent();
        int childIndex = parent ? parent->childIndex( page ) : -1;
        bool priorCheck = childIndex != -1 ? parent->checked( childIndex ) : false;

        QContentFilterModelPrivate::Selection priorSelection = page->selection();

        if( page->setChecked( index.row(), value == Qt::Checked ) )
        {
            if( page->selection() != priorSelection || priorCheck != ( childIndex != -1 && parent->checked( childIndex ) ) )
                emit dataChanged( QModelIndex(), QModelIndex() );
            else
                emit dataChanged( index, index );

            return true;
        }
    }

    return false;
}

/*!
    \reimp
 */
Qt::ItemFlags QContentFilterModel::flags( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        QContentFilterModelPrivate *page = static_cast< QContentFilterModelPrivate * >( index.internalPointer() );

        if( page->count() && page->pageTemplate().options() & CheckList )
            return QAbstractItemModel::flags( index ) | Qt::ItemIsUserCheckable;
    }

    return QAbstractItemModel::flags( index );
}

/*!
    \reimp
 */
QModelIndex QContentFilterModel::index( int row, int column, const QModelIndex &parent ) const
{
    QContentFilterModelPrivate *page = parent.isValid() ? child( parent ) : d;

    if( row >= 0 && row < rowCount( parent ) )
        return createIndex( row, column, page );

    return QModelIndex();
}

/*!
    \reimp
 */
QModelIndex QContentFilterModel::parent( const QModelIndex &index ) const
{
    if( index.isValid() )
    {
        QContentFilterModelPrivate *page = static_cast< QContentFilterModelPrivate * >( index.internalPointer() );

        if( page->parent() )
            return createIndex( page->parent()->childIndex( page ), 0, page->parent() );
    }

    return QModelIndex();
}

/*!
    \reimp
 */
int QContentFilterModel::rowCount( const QModelIndex &parent ) const
{
    QContentFilterModelPrivate *page = parent.isValid() ? child( parent ) : d;

    if( page && page->dirty() )
        page->repopulate();

    return page && page->count() ? page->count() : 1;
}

/*!
    \reimp
*/
bool QContentFilterModel::hasChildren( const QModelIndex &parent ) const
{
    if( !parent.isValid() )
        return true;

    QContentFilterModelPrivate *page =  static_cast< QContentFilterModelPrivate * >( parent.internalPointer() );

    return page && page->count() ? page->target( parent.row() ).isValid() : false;
}

QContentFilterModelPrivate *QContentFilterModel::child( const QModelIndex &parent ) const
{
    QContentFilterModelPrivate *parentPage = static_cast< QContentFilterModelPrivate * >( parent.internalPointer() );

    QContentFilterModelPrivate *page = parentPage->child( parent.row() );

    if( !page && parentPage->target( parent.row() ).isValid() )
    {
        page = new QContentFilterModelPrivate(
                    parentPage->target( parent.row() ),
                    parentPage->filter() & parentPage->filter( parent.row() ),
                    parentPage );

        parentPage->setChild( parent.row(), page );
    }

    return page;
}

void QContentFilterModel::contentChanged( const QContentIdList &ids, QContent::ChangeType type )
{
    Q_UNUSED( ids  );
    Q_UNUSED( type );

    d->refresh( this );
}

void QContentFilterModel::beginRemoveRows( QContentFilterModelPrivate *parent, int start, int end )
{
    if( parent->parent() )
        QAbstractItemModel::beginRemoveRows(
                createIndex( parent->parent()->childIndex( parent ), 0, parent->parent() ), start, end );
    else
        QAbstractItemModel::beginRemoveRows( QModelIndex(), start, end );
}

void QContentFilterModel::beginInsertRows( QContentFilterModelPrivate *parent, int start, int end )
{
    if( parent->parent() )
        QAbstractItemModel::beginInsertRows(
                createIndex( parent->parent()->childIndex( parent ), 0, parent->parent() ), start, end );
    else
        QAbstractItemModel::beginInsertRows( QModelIndex(), start, end );
}
