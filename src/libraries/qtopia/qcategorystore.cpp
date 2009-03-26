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
#include "qsqlcategorystore_p.h"
#include <QtGlobal>

#ifndef QTOPIA_CONTENT_INSTALLER
#include <QtopiaApplication>
#include "qdocumentservercategorystore_p.h"
#include <QThreadStorage>
#endif

class QCategoryDataPrivate : public QSharedData
{
public:
    QCategoryDataPrivate(){ ref.ref(); }
    QCategoryDataPrivate( const QCategoryDataPrivate &other ) : QSharedData( other ){}
    QString label;
    QString iconFile;
    QString ringTone;
    QIcon icon;
    QCategoryData::Flags flags;
};

Q_GLOBAL_STATIC(QCategoryDataPrivate,nullQCategoryDataPrivate);

/*!
    \class QCategoryData
    \inpublicgroup QtBaseModule

    \brief The QCategoryData class holds the attributes of a single category.

    \internal
*/

/*!
    \enum QCategoryData::Flag
    Flags representing boolean properties of a category.
    \value Global Indicates a category is in the global scope.
    \value System Indicates a category is a system category.
    \value IconLoaded Indicates the icon for the category has been loaded.
*/

/*!
    \typedef QCategoryData::Flags

    Synonym for \c{QFlags< Flag >}.
*/

/*! 
    Constructs a null QCategoryData object.
*/
QCategoryData::QCategoryData()
    : d( nullQCategoryDataPrivate() )
{
}

/*!
    Constructs a QCategoryData with the label \a label, icon file \a iconFile and flags \a flags.
*/
QCategoryData::QCategoryData( const QString &label, const QString &iconFile, const QString &ringTone, Flags flags )
    : d( nullQCategoryDataPrivate() )
{
    d.detach();

    d->label = label;
    d->iconFile = iconFile;
    d->ringTone = ringTone;
    d->flags = flags & ~IconLoaded;
}

/*!
    Constructs a copy of the QCategoryData \a other.
 */
QCategoryData::QCategoryData( const QCategoryData &other )
    : d( other.d )
{
}

/*!
    Destroys a QCategoryData.
 */
QCategoryData::~QCategoryData()
{
}

QCategoryData &QCategoryData::operator =( const QCategoryData &other )
{
    d = other.d;

    return *this;
}

bool QCategoryData::operator ==( const QCategoryData &other ) const
{
    return d == other.d;
}

/*!
    Returns the categories user visible name.
 */
const QString &QCategoryData::label() const
{
    return d->label;
}

/*!
    Returns the path of the categories icon.
 */
const QString &QCategoryData::iconFile() const
{
    return d->iconFile;
}

/*!
    Returns the path of the category's ringtone.
*/
const QString &QCategoryData::ringTone() const
{
    return d->ringTone;
}

/*!
    Returns the categories icon.
 */
const QIcon &QCategoryData::icon()
{
    if( !(d->flags & IconLoaded) )
        const_cast< QCategoryData * >( this )->loadIcon();

    return d->icon;
}
/*!
    Returns true if the category is a global category.
 */
bool QCategoryData::isGlobal() const
{
    return d->flags & Global;
}

/*!
    Returns true if the category is a system category.
*/
bool QCategoryData::isSystem() const
{
     return d->flags & System;
}

/*!
    Returns true if the category data is uninitialized.
*/
bool QCategoryData::isNull() const
{
    return d.constData() == nullQCategoryDataPrivate();
}

template <typename Stream> void QCategoryData::serialize(Stream &stream) const
{
    stream << d->label;
    stream << d->iconFile;
    stream << d->ringTone;
    stream << (d->flags &~ IconLoaded);
}

template <typename Stream> void QCategoryData::deserialize(Stream &stream)
{
    d.detach();

    stream >> d->label;
    stream >> d->iconFile;
    stream >> d->ringTone;
    stream >> d->flags;

    d->icon = QIcon();
    d->flags &= ~IconLoaded;
}

void QCategoryData::loadIcon()
{
    if (!d->iconFile.isEmpty())
        d->icon = QIcon( QLatin1String( ":icon/" ) + d->iconFile );

    d->flags |= IconLoaded;
}

Q_IMPLEMENT_USER_METATYPE(QCategoryData);
Q_IMPLEMENT_USER_METATYPE_ENUM(QCategoryData::Flags);
Q_IMPLEMENT_USER_METATYPE_TYPEDEF(QCategoryDataMap,QCategoryDataMap);

#ifndef QTOPIA_CONTENT_INSTALLER
Q_GLOBAL_STATIC( QThreadStorage< QCategoryStore * >, categoryStores );
#endif

/*!
    \class QCategoryStore
    \inpublicgroup QtBaseModule

    \brief The QCategoryStore class provides an interface to a category store.

    \internal
*/

/*!
    \fn QCategoryStore::addCategory( const QString &categoryId, const QString &scope, const QString &label, const QString &icon, bool isSystem )

    Adds a category with the id \a categoryId, scope \a scope, label \a label, and icon \a icon to the store.

    If \a isSystem is true the category will be made a system category.
*/

/*!
    \fn QCategoryStore::categoryExists( const QString &categoryId )

    Returns true if the category with the id \a categoryId exists.
 */

/*!
    \fn QCategoryStore::categoryFromId( const QString &categoryId )

    Returns the category with the id \a categoryId.
 */

/*!
    \fn QCategoryStore::scopeCategories( const QString &scope )

    Returns all the categories in the scope \a scope or the global scope.
 */

/*!
    \fn QCategoryStore::removeCategory( const QString &categoryId )

    Removes the category with the id \a categoryId from the store.
 */

/*!
    \fn QCategoryStore::setCategoryScope( const QString &categoryId, const QString &scope )

    Sets the \a scope of the category with the id \a categoryId.
 */

/*!
    \fn QCategoryStore::setCategoryIcon( const QString &categoryId, const QString &icon )

    Sets the \a icon for the category with the id \a categoryId.
 */

/*!
    \fn QCategoryStore::setCategoryRingTone( const QString &categoryId, const QString &fileName )

    Sets the \a fileName for the category with the id \a categoryId.
*/

/*!
    \fn QCategoryStore::setCategoryLabel( const QString &categoryId, const QString &label )

    Sets the \a label for the category with the id \a categoryId.
 */

/*!
    \fn QCategoryStore::setSystemCategory( const QString &categoryId )

    Sets the category with the id \a categoryId to a system category.
*/

QCategoryStore::QCategoryStore( QObject *parent )
    : QObject( parent )
{
}

/*!
    Destroys a category store.
*/
QCategoryStore::~QCategoryStore()
{
}

/*!
    Returns a pointer to a static instance of QCategoryStore.
*/
QCategoryStore *QCategoryStore::instance()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if( !categoryStores()->hasLocalData() )
{
    if( QContent::documentSystemConnection() == QContent::DocumentSystemDirect )
        categoryStores()->setLocalData( new QSqlCategoryStore );
    else
        categoryStores()->setLocalData(  new QDocumentServerCategoryStore );
}

    return categoryStores()->localData();
#else
    static QSqlCategoryStore *instance = 0;

    if( !instance )
        instance = new QSqlCategoryStore;

    return instance;
#endif
}

QString QCategoryStore::errorString() const
{
    return m_errorString;
}

void QCategoryStore::setErrorString( const QString &errorString )
{
    m_errorString = errorString;
}
