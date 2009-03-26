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

#include <qcontentproperties.h>
#include "contentpluginmanager_p.h"

class QContentPropertiesEngineSharedNull : public QContentPropertiesEngine
{
public:
    QContentPropertiesEngineSharedNull(){}
    virtual bool isReadOnly() const{ return true; }
    virtual QStringList keys() const{ return QStringList(); }
    virtual QStringList supportedKeys() const{ return QStringList(); }
    virtual QVariant value( const QString &, const QVariant &defaultValue ) const{ return defaultValue; }
    virtual bool setValue( const QString &, const QVariant & ){ return false; }
    virtual bool contains( const QString & ) const{ return false; }
    virtual bool remove( const QString & ){ return false; }
};

Q_GLOBAL_STATIC(QContentPropertiesEngineSharedNull,q_sharedNullQContentPropertiesEngine);

class QContentPropertiesPrivate
{
public:
    QContentPropertiesEngine *engine;
    QContent content;
};

/*!
    \class QContentProperties
    \inpublicgroup QtBaseModule
    \brief The QContentProperties class allows the meta-data stored in content to be read and edited.

    Many content formats include meta-data fields which are used to store descriptive information
    about the content.  QContentProperties provides an interface to access these properties directly
    in a generic manner.

    Properties are identified by string keys, for any given content a list of keys which have
    associated values can be queried using the keys() function, and a list of all possible
    keys for the content can be gained from the supportedKeys() function.  Common properties are
    documented in the QContent::Property enumeration.

    Unlike QContent::property() and QContent::setProperty() QContentProperties accesses data
    embedded in the content itself and not the database which means the changes will be
    persisted wherever the content is moved.  Any properties normally extracted by the content
    system will be updated in the database as well.  Note the reverse does not occur, changing
    a property in the database with QContent::setProperty() will not modify properties embedded
    in the content.

    \sa QContent::Property, QContent::propertyKey(), QContent::property(), QContent::setProperty()

    \ingroup content

    \preliminary
*/

/*!
    Constructs a new \a content properties editor.
*/
QContentProperties::QContentProperties( const QContent &content )
    : d( new QContentPropertiesPrivate )
{
    d->content = content;

    d->engine = QContentFactory::createPropertiesEngine( d->content );

    if( !d->engine )
        d->engine = q_sharedNullQContentPropertiesEngine();
}

/*!
    Destroys a content properities editor.
*/
QContentProperties::~QContentProperties()
{
    if( d->engine != q_sharedNullQContentPropertiesEngine() )
        delete d->engine;

    delete d;
}

/*!
    Returns the content whose properties are being accessed.
*/
QContent QContentProperties::content() const
{
    return d->content;
}

/*!
    Returns true if the content properties are read-only.  Properties may be read-only because the underlying file is read-only
    or the plug-in doesn't support writing properties.
*/
bool QContentProperties::isReadOnly() const
{
    return d->engine->isReadOnly();
}

/*!
    Returns a list of all the keys the content has properties for.
*/
QStringList QContentProperties::keys() const
{
    return d->engine->keys();
}

/*!
    Returns a list of all property keys explicitly supported for the content's type.  If arbitrary strings keys are supported
    this will return an empty list.
*/
QStringList QContentProperties::supportedKeys() const
{
    return d->engine->supportedKeys();
}

/*!
    Returns the value of the content property identified by \a key.  If the content doesn't contain the property \a defaultValue is returned instead.
*/
QVariant QContentProperties::value( const QString &key, const QVariant &defaultValue ) const
{
    return d->engine->value( key, defaultValue );
}

/*!
    Sets the value of the content property identified by \a key to \a value.  Returns true if the value was assigned and false otherwise.
*/
bool QContentProperties::setValue( const QString &key, const QVariant &value )
{
    return d->engine->setValue( key, value );
}

/*!
    Returns true if the content contains a property for the given \a key, and false otherwise.
*/
bool QContentProperties::contains( const QString &key ) const
{
    return d->engine->contains( key );
}

/*!
    Removes the property with the given \a key from the content.  Returns true if the property was removed and false otherwise.
*/
bool QContentProperties::remove( const QString &key )
{
    return d->engine->remove( key );
}

/*!
    \class QContentPropertiesEngine
    \inpublicgroup QtBaseModule
    \brief The QContentPropertiesEngine class provides an abstraction for accessing content properties.

    The QContentProperties class uses QContentPropertiesEngine internally to implement meta-data
    access for different content types.

    Engines are created by instances of QContentPropertiesPlugin which are automatically loaded by
    QContentProperties.

    \sa QContentProperties, QContentPropertiesPlugin

    \ingroup content

    \preliminary
*/

/*!
    Destroys a QContentPropertiesEngine.
*/
QContentPropertiesEngine::~QContentPropertiesEngine()
{
}

/*!
    \fn bool QContentPropertiesEngine::isReadOnly() const

    Returns true if the content properties are read-only.  Properties may be read-only because the underlying file is read-only
    or the plug-in doesn't support writing properties.
*/

/*!
    \fn QStringList QContentPropertiesEngine::keys() const

    Returns a list of all the keys the content has properties for.
*/

/*!
    \fn QStringList QContentPropertiesEngine::supportedKeys() const

    Returns a list of all property keys explicitly supported for the content's type.  If arbitrary strings keys are supported
    this will return an empty list.
*/

/*!
    \fn QVariant QContentPropertiesEngine::value( const QString &key, const QVariant &defaultValue ) const

    Returns the value of the content property identified by \a key.  If the content doesn't contain the property \a defaultValue is returned instead.
*/

/*!
    \fn bool QContentPropertiesEngine::setValue( const QString &key, const QVariant &value )

    Sets the value of the content property identified by \a key to \a value.  Returns true if the value was assigned and false otherwise.
*/

/*!
    \fn bool QContentPropertiesEngine::contains( const QString &key ) const

    Returns true if the content contains a property for the given \a key, and false otherwise.
*/

/*!
    \fn bool QContentPropertiesEngine::remove( const QString &key )

    Removes the property with the given \a key from the content.  Returns true if the property was removed and false otherwise.
*/
