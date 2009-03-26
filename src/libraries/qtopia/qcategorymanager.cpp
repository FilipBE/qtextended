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
#include "qcategorymanager.h"
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#else
#include <qtopianamespace.h>
#endif
#include <qtopiasql.h>
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#include "qcategorystore_p.h"

#include <QSettings>
#include <QMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <limits.h>

class QCategoryFilterData : public QSharedData
{
public:
    QCategoryFilterData() : type(QCategoryFilter::All) {}

    QList<QString> required;
    QCategoryFilter::FilterType type;
};

/*!
  \class QCategoryFilter
    \inpublicgroup QtBaseModule

  \ingroup categories
  \brief The QCategoryFilter class allows consistent filtering of records or objects
  that have a set of categories assigned.

  QCategoryFilter is used to represent selected categories. Unlike a list of category
  ids, QCategoryFilter handles the \c All and \c Unfiled
  cases so it allows unambiguous behaviour when new categories are added to the system.
  It is most useful when filtering items by category to create a QCategoryFilter and use
  the \l accepted() function.

  \sa Categories
*/

/*!
  \enum QCategoryFilter::FilterType

  \value List Only accepts category sets that contain all of the required categories.
  \value Unfiled Only accepts empty category sets.
  \value All Accepts all category sets.
*/

/*!
  Constructs a QCategoryFilter object.  By default this object will accept
  all category sets.
*/
QCategoryFilter::QCategoryFilter()
{
    d = new QCategoryFilterData();
}

/*!
  Constructs a QCategoryFilter object of type \a t.

  \sa FilterType
*/
QCategoryFilter::QCategoryFilter(FilterType t)
{
    d = new QCategoryFilterData();
    d->type = t;
}

/*!
  Constructs a QCategoryFilter object that only accepts category sets containing
  all of the category ids listed in \a set. The type is set to \l QCategoryFilter::List.
*/
QCategoryFilter::QCategoryFilter(const QList<QString>&set)
{
    d = new QCategoryFilterData();
    d->type = List;
    d->required = set;
}

/*!
  Constructs a QCategoryFilter object that only accepts category sets containing category id \a c.
  The type is set to \l QCategoryFilter::List.
*/
QCategoryFilter::QCategoryFilter(const QString &c)
{
    d = new QCategoryFilterData();
    d->type = List;
    d->required.append(c);
}

/*!
  Constructs a QCategoryFilter object as a copy of \a other.
*/
QCategoryFilter::QCategoryFilter(const QCategoryFilter &other)
{
    d = other.d;
}

/*!
  Destroys a QCategoryFilter object.
*/
QCategoryFilter::~QCategoryFilter() {}

/*!
  Assigns the QCategoryFilter object to be a copy of \a other.
*/
QCategoryFilter &QCategoryFilter::operator=(const QCategoryFilter &other)
{
    d = other.d;
    return *this;
}

/*!
  Returns true if the set of categories described by \a list is accepted by the QCategoryFilter
  object.  Otherwise returns false.
*/
bool QCategoryFilter::accepted(const QList<QString> &list) const
{
    if (d->type == Unfiled)
        return (list.count() == 0);
    if (d->type == All)
        return true;

    foreach(QString cat, d->required) {
        if (!list.contains(cat))
            return false;
    }
    return true;
}

/*!
  Returns true if the QCategoryFilterObject accepts all category sets.
  Otherwise returns false.
  \sa QCategoryFilter::requiredCategories()
*/
bool QCategoryFilter::acceptAll() const
{
    return d->type == All;
}

/*!
  Returns true if the QCategoryFilterObject only accepts empty category sets.
  Otherwise returns false.
*/
bool QCategoryFilter::acceptUnfiledOnly() const
{
    return d->type == Unfiled;
}

/*!
  Returns the list of categories that must be in a set for the set to be accepted.
  Note that this only returns a list of the QCategoryFilter object is of type
  \l QCategoryFilter::List. To get the list of categories corresponding to
  \l QCategoryFilter::All you must create a QCategoryManager.

  \code
    QStringList categories;
    if ( filter.acceptAll() )
        categories = QCategoryManager(scope).categoryIds();
    else
        categories = filter.requiredCategories(); // returns empty list for Unfiled
  \endcode
*/
QList<QString> QCategoryFilter::requiredCategories() const
{
    if (d->type == List)
        return d->required;
    return QList<QString>();
}

static const QString QCategoryFilter_AcceptAll = "AcceptAll";// no tr
static const QString QCategoryFilter_RequireEmpty = "RequireEmpty";// no tr
static const QString QCategoryFilter_RequireList = "RequireList";// no tr

/*!
  Writes the QCategoryFilter to field \a key in \a c.

  \code
    QCategoryFilter filter("mycategory");
    QSettings settings("mycompany", "myapp");
    filter.writeConfig(settings,"filter");
  \endcode

  \sa readConfig()
*/
void QCategoryFilter::writeConfig(QSettings &c, const QString &key) const
{
    QString value;
    // version
    value += "0000 ";
    // type
    switch(d->type) {
        case All:
            value += QCategoryFilter_AcceptAll;
            break;
        case Unfiled:
            value += QCategoryFilter_RequireEmpty;
            break;
        case List:
            value += QCategoryFilter_RequireList;
            value += " " + QStringList(d->required).join(" ");
            break;
    }
    c.setValue(key, value);
}

/*!
  Reads the QCategoryFilter from field \a key in \a c.

  \code
    QSettings settings("mycompany", "myapp");
    QCategoryFilter filter;
    filter.readConfig(settings,"filter");
  \endcode

  \sa writeConfig()
*/
void QCategoryFilter::readConfig(const QSettings &c, const QString &key)
{
    QString value = c.value(key).toString();
    if (value.left(4) == "0000") {
        // latest version.
        value = value.mid(5);
        if (value.left(QCategoryFilter_AcceptAll.length()) == QCategoryFilter_AcceptAll) {
            d->type = All;
            return;
        }
        if (value.left(QCategoryFilter_RequireEmpty.length()) == QCategoryFilter_RequireEmpty) {
            d->type = Unfiled;
            return;
        }
        if (value.left(QCategoryFilter_RequireList.length()) == QCategoryFilter_RequireList) {
            d->type = List;
            value = value.mid(QCategoryFilter_RequireList.length()+1);
            d->required = value.split(" ", QString::SkipEmptyParts);
            return;
        }
    }
}

/*!
  Returns a translated string that briefly describes the QCategoryFilter object.
  If specified, \a scope specifies the scope in which to search for the label.

  This is useful when displaying a string describing the categories the QCategoryFilter
  allows.

  \code
    currentCategoryLabel->setText( catetoryFilter->label() );
  \endcode
*/
QString QCategoryFilter::label(const QString &scope) const
{
    if (d->type == Unfiled)
        return QCategoryManager::unfiledLabel();
    if (d->type == All || d->required.count() == 0)
        return QCategoryManager::allLabel();

    if (d->required.count() == 1) {
        QCategoryManager cat(scope);
        return cat.label(d->required[0]);
    }

    return QCategoryManager::multiLabel();
}

/*!
  Returns true if the QCategoryFilter object is equivilent to \a other.
  Otherwise returns false.
*/
bool QCategoryFilter::operator==(const QCategoryFilter &other) const
{
    if (other.d->type != d->type)
        return false;
    return other.d->required == d->required;
}

/*!
  Returns true if the QCategoryFilter object is not equivilent to \a other.
  Otherwise returns false.
*/
bool QCategoryFilter::operator!=(const QCategoryFilter &other) const
{
    return !( *this == other );
}

/* Must be self loading/self reloading. */

class QCategoryManagerData
{
public:
    QCategoryManagerData(const QString &s)
        : scope(s), categoriesLoaded( false ){}

    const QString scope;
    QMap< QString, QCategoryData > categories;
    bool categoriesLoaded;

    void loadCategories()
    {
        categories = QCategoryStore::instance()->scopeCategories( scope );

        categoriesLoaded = true;
    }

    QCategoryData category( const QString &id ) const
    {
        if( !categoriesLoaded )
            const_cast< QCategoryManagerData * >( this )->loadCategories();

        QCategoryData category;

        if( categories.contains( id ) )
            category = categories.value( id );
        else
            category = QCategoryStore::instance()->categoryFromId( id );

        return category;
    }
};

/*!
  \class QCategoryManager
    \inpublicgroup QtBaseModule

  \ingroup categories
  \brief The QCategoryManager class provides a set of functions to create, modify, and remove categories.

  \sa Categories
*/

/*!
  \fn void QCategoryManager::categoriesChanged()

  This signal is emitted when any changes are made to categories.
*/

/*!
  Constructs a QCategoryManager object with parent \a parent.
  It will only see categories in the global scope.
*/
QCategoryManager::QCategoryManager(QObject *parent)
    : QObject(parent)
{
    d = new QCategoryManagerData(QString());

    connect( QCategoryStore::instance(), SIGNAL(categoriesChanged()), this, SLOT(reloadCategories()) );
}

/*!
  Constructs a QCategoryManager object with parent \a parent.
  If \a scope is null then only categories in the global scope will be seen.
  Otherwise both global categories and categories restricted to \a scope will be seen.
*/
QCategoryManager::QCategoryManager(const QString &scope, QObject *parent)
    : QObject(parent)
{
    d = new QCategoryManagerData(scope);

    connect( QCategoryStore::instance(), SIGNAL(categoriesChanged()), this, SLOT(reloadCategories()) );
}

/*!
  Destroys a QCategoryManager object.
*/
QCategoryManager::~QCategoryManager()
{
    delete d;
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the display label for the category id.  Otherwise returns a
  null string.

  User categories have a display label set by the user. System categories
  have a string that is translated to obtain the display label.

  \sa {User Categories}, {System Categories}
*/
QString QCategoryManager::label(const QString &id) const
{
    return d->category( id ).label();
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the icon for the category id.  Otherwise returns a
  null icon.
*/
QIcon QCategoryManager::icon(const QString &id) const
{
    return d->category( id ).icon();
}

/*!
  If there is a category id \a id in the scope of the QCategoryManager
  returns the icon filename for the category id.  Otherwise returns an
  empty string.
 */
QString QCategoryManager::iconFile(const QString &id) const
{
    return d->category( id ).iconFile();
}

/*!
  If there is a category id \a id in the scope of the QCategoryManger
  returns the ringtone filename for the category id. Otherwise returns an
  empty string.
  */
QString QCategoryManager::ringTone(const QString &id) const
{
    return d->category( id ).ringTone();
}

/*!
  Returns a list containing the translated label for each
  category id in the list \a l that is in the scope of the QCategoryManager.
  The list returned will have a count smaller than the list if id's \a l
  if one or more of the ids in the list \a l are not in the scope of the QCategoryManager.
  \sa label()
*/
QList<QString> QCategoryManager::labels(const QList<QString> &l) const
{
    QList<QString> r;
    foreach(QString i, l) {
        if (contains(i))
            r.append(label(i));
    }
    return r;
}

/*!
  Returns the translated label for the empty set of categories.  Also known as unfiled
  due to the set not yet having any categories assigned.
*/
QString QCategoryManager::unfiledLabel()
{
    return tr("Unfiled");
}

/*!
  Returns the translated label for the set of all categories.
*/
QString QCategoryManager::allLabel()
{
    return tr("All");
}

/*!
  Returns a translated label for a set containing two or more categories.
*/
QString QCategoryManager::multiLabel()
{
    return tr("(Multi) ...");
}

/*!
  Returns true if the category id \a id is in the global scope.
*/
bool QCategoryManager::isGlobal(const QString &id) const
{
    return d->category( id ).isGlobal();
}

/*!
  Returns true if the category identified by \a id is a system category.
  Otherwise returns false.
  \sa {System Categories}
*/
bool QCategoryManager::isSystem(const QString &id) const
{
    return d->category( id ).isSystem();
}

/*!
  \obsolete
  Sets the category with category id \a id to be a system category. Returns true on success; otherwise returns false.
  Doing this will make the category read only and undeleteable. Use with care.

  System categories should be created using the \l ensureSystemCategory() function.
*/
bool QCategoryManager::setSystem(const QString &id)
{
    d->categoriesLoaded = false;

    return QCategoryStore::instance()->setSystemCategory( id );
}

/*!
  Sets the category with category id \a id to the global scope if \a global is true or to the scope of
  QCategoryManager otherwise. Returns true on success. Returns false if the category does
  not exist, if \a global is false and QCategoryManager does not have a scope or if there
  is a database failure.
*/
bool QCategoryManager::setGlobal(const QString &id, bool global)
{
    if( isGlobal( id ) != global )
    {
        d->categoriesLoaded = false;

        return QCategoryStore::instance()->setCategoryScope( id, global ? QString() : d->scope );
    }
    else
        return true;
}

/*!
  Creates a new category with the user-supplied label \a trLabel and icon \a icon.
  The category is created in the scope of the QCategoryManager unless \a forceGlobal
  is true, when it will be created in the global scope.

  Returns the id of the new category if the new category is successfully added.
  Otherwise returns the null string.

  Note that this function is not suitable for applications wishing to create categories
  programmatically. Instead, a system category should be created, using
  the \l ensureSystemCategory() function.

  \sa {User Categories}, {System Categories}
*/
QString QCategoryManager::add( const QString &trLabel, const QString &icon, bool forceGlobal )
{
    if (trLabel == unfiledLabel())
        return QString();

    QString id=trLabel;
    if ( id.isEmpty() ) {
        id = "empty";
    }
    // Prepend user. to the id to avoid conflicts with system category ids
    id = QString("user.%1").arg(id);

    if ( !QCategoryStore::instance()->categoryExists(id) )
        if ( addCategory(id, trLabel, icon, forceGlobal) )
            return id;
        else
            return QString();

    // The id is already in use... try to generate a new one
    QString key;
    for ( int i = 0; i < INT_MAX; i++ ) {
        key = QString("%1_%2").arg(id).arg(i);
        if ( !QCategoryStore::instance()->categoryExists(key) ) {
            if ( addCategory(key, trLabel, icon, forceGlobal) )
                return key;
            else
                return QString();
        }
    }

    return QString();
}

/*!
  \obsolete
  Creates a new category with category id \a id, the user-supplied label \a trLabel and icon \a icon
  in the scope of the QCategoryManager. If \a forceGlobal is true, when the category is created it will be
  created as a global category (ie with no scope). If the scope the class was created with is
  null the category will be global also. If \a isSystem is set to true, will set the system
  flag on the category, and will make it unmodifiable, and unremovable.

  For creating categories on behalf of the user use the \l add() function. For creating system
  categories use the \l ensureSystemCategory() function.

  Returns true if the new category is successfully added.  Otherwise returns false.
*/
bool QCategoryManager::addCategory( const QString &id, const QString &trLabel, const QString &icon, bool forceGlobal, bool isSystem )
{
    if (id.isEmpty() || trLabel.isEmpty() || trLabel == unfiledLabel() || exists(id))
        return false;

    d->categoriesLoaded = false;

    return QCategoryStore::instance()->addCategory( id, forceGlobal ? QString() : d->scope, trLabel, icon, isSystem );
}

/*!
  Creates a new system category with category id \a id, translatable label \a trLabel and icon \a icon
  in the scope of the QCategoryManager. If \a forceGlobal is true or the QCategoryManager has no scope
  the category will be created in the global scope.

  Note that \a id must be unique. If the id already exists and the existing category does not match the
  arguments the existing category is removed and re-created using the arguments.

  Returns true if the new category is successfully added or a matching system category already exists.
  Otherwise returns false.

  Note that applications wishing to create categories on behalf of the user should use the \l add() function.

  \sa {User Categories}, {System Categories}
*/
bool QCategoryManager::ensureSystemCategory( const QString &id, const QString &trLabel, const QString &icon, bool forceGlobal )
{
    // You cannot create a system category id starting with "user." as that is reserved for user categories
    if (id.isEmpty() || trLabel.isEmpty() || id.startsWith("user.") || trLabel == unfiledLabel())
        return false;

    if( !d->categoriesLoaded )
        d->loadCategories();

    QCategoryData data = d->categories.value( id );

    if ( !data.isNull() ) {
        // Simple case, the system category already exists as described by the arguments
        if ( data.isSystem() && trLabel == data.label() && icon == data.iconFile() && forceGlobal == data.isGlobal() ) {
            return true;
        } else if ( !QCategoryStore::instance()->removeCategory( id ) ) {
            return false;
        } else {
            d->categoriesLoaded = false;
        }
    }

    return QCategoryStore::instance()->addCategory( id, forceGlobal ? QString() : d->scope, trLabel, icon, true );
}

/*!
  Attempts to remove the category with category id \a id as long as the category is either
  global or in the scope of the QCategoryManager object.
  Returns true If the category is successfully removed.
  Otherwise returns false.

  Note that this will always fail if \a id is a system category.
*/
bool QCategoryManager::remove( const QString &id )
{
    if( !d->categoriesLoaded )
        d->loadCategories();

    QCategoryData data = d->categories.value( id );

    if( !data.isNull() && !data.isSystem() && QCategoryStore::instance()->removeCategory( id ) )
    {
        d->categoriesLoaded = false;

        return true;
    }
    else
        return false;
}

/*!
  Attempts to rename the category with category id \a id to have the translated label \a trLabel.
  Returns true If the category is successfully renamed.
  Otherwise returns false.

  Note that this will always fail if \a id is a system category.
*/
bool QCategoryManager::setLabel( const QString &id, const QString &trLabel )
{
    if (trLabel != unfiledLabel() && QCategoryStore::instance()->setCategoryLabel(id, trLabel)) {
        if( d->categories.contains( id ) )
            d->categoriesLoaded = false;

        return true;
    } else {
        return false;
    }
}

/*!
  Attempts to set the icon for the category with category id \a id to \a icon.
  Returns true If the category icon is changed successfully.
  Otherwise returns false.

  Note that this will always fail if \a id is a system category.
*/
bool QCategoryManager::setIcon( const QString &id, const QString &icon )
{
    if( QCategoryStore::instance()->setCategoryIcon( id, icon ) )
    {
        if( d->categories.contains( id ) )
            d->categoriesLoaded = false;

        return true;
    }
    else
        return false;
}

/*!
  Attempts to set the ringtone for the category with category id \a id to \a fileName.
  Returns true if the category ringtone is changed successfully. Otherwise return false.
*/
bool QCategoryManager::setRingTone( const QString &id, const QString &fileName )
{
    if ( QCategoryStore::instance()->setCategoryRingTone( id, fileName ) )
    {
        if ( d->categories.contains( id ) )
            d->categoriesLoaded = true;

        return false;
    }
    else
        return false;
}

/*!
  Returns true if there is a category in the global scope or the scope of
  QCategoryManager with category id \a id.  Otherwise returns false.
*/
bool QCategoryManager::contains(const QString &id) const
{
    if( !d->categoriesLoaded )
        const_cast< QCategoryManagerData * >( d )->loadCategories();

    return d->categories.contains( id );
}

/*!
    Returns true if there is a category in any scope with category id \a id.
*/
bool QCategoryManager::exists( const QString &id ) const
{
    return id == QLatin1String("Unfiled")
        || contains(id)
        || QCategoryStore::instance()->categoryExists(id);
}

/*!
  Returns true if there is a category in the global scope or the scope of
  QCategoryManager with text \a label.  Otherwise returns false.
  Set \a forceGlobal to true to limit the search to categories in the global scope.

  Note that this searches on the text stored which may not match what is returned by
  \l label() for system categories.
 */
bool QCategoryManager::containsLabel(const QString &label, bool forceGlobal) const
{
    if( !d->categoriesLoaded )
        const_cast< QCategoryManagerData * >( d )->loadCategories();

    foreach( QString id, d->categories.keys() )
    {
        QCategoryData category = d->categories.value( id );

        if( category.label() == label && (!forceGlobal || category.isGlobal()) )
            return true;
    }

    return false;
}

/*!
  Returns the category id for a category with text \a label in the global scope or the
  scope of QCategoryManager. Otherwise returns an empty string.

  Note that this searches on the text stored which may not match what is returned by
  \l label() for system categories.
 */
QString QCategoryManager::idForLabel(const QString &label) const
{
    if( !d->categoriesLoaded )
        const_cast< QCategoryManagerData * >( d )->loadCategories();

    foreach( QString id, d->categories.keys() )
    {
        if( d->categories.value( id ).label() == label )
            return id;
    }

    return QString();
}

/*!
  Returns the set of category ids that QCategoryManager can see.
*/
QList<QString> QCategoryManager::categoryIds() const
{
    if( !d->categoriesLoaded )
        const_cast< QCategoryManagerData * >( d )->loadCategories();

    return d->categories.keys();
}

/*!
  \internal
  Emit the categoriesChanged() signal.
*/
void QCategoryManager::reloadCategories()
{
    d->categoriesLoaded = false;

    emit categoriesChanged();
}

