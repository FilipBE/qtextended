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

#include "qdrmrights.h"

Q_IMPLEMENT_USER_METATYPE_ENUM( QDrmRights::Permission );
Q_IMPLEMENT_USER_METATYPE_ENUM( QDrmRights::Permissions );
Q_IMPLEMENT_USER_METATYPE_ENUM( QDrmRights::Status );

/*!
    \class QDrmRights
    \inpublicgroup QtBaseModule

    \brief The QDrmRights class describes the status and constraints associated with a permission for DRM protected content.

    A drm rights object represents the rights associated with an item of content for a permission.  It is composed of a
    permission, the current status of the rights, and a list of constraints imposed on the consumption of the content.

    \ingroup drm
 */

 /*!
    \enum QDrmRights::Permission
    Represents an action permission is being sought for.

    Render permissions:
    \value Play Playback permission for media files.
    \value Display Display permission for images and documents.
    \value Execute Execute permission for applications.
    \value Print Print permission for images and documents.
    \value Export Export data to another DRM format.  OMA DRM v2 only.
    Secondary permissions:
    \value Distribute Move content to another device.
    \value Preview Permission to render a preview version of the content.
    \value Automated Content may be used in automated renderings such as ring tones.
    \value BrowseContents Permission to browse the contents of a container.
    Additional states:
    \value Unrestricted Permission to open content for any use.  This will never be granted for DRM protected content.
    \value NoPermissions The content cannot be rendered with any permission or distributed.
    \value InvalidPermission Uninitalized permission state.
*/

/*!
    \enum QDrmRights::Status
    Represents the status of the rights.

    \value Invalid Indicates there are no valid rights associated with the content the rights were requested for.
    \value Valid Indicates the content the rights were requested for are valid and the content can be accessed.
    \value ValidInFuture Indicates there are rights associated with the content the rights were requested but they will not be valid until some later date.
 */

class QDrmRightsConstraintPrivate : public QSharedData
{
public:
    QString name;
    QVariant value;
    QList< QPair< QString, QVariant > > attributes;
};

/*!
    \class QDrmRights::Constraint
    \inpublicgroup QtBaseModule

    \brief The Constraint class describes a constraint placed on the usage of DRM content.

    A constraint is typically a simple name value pair where the name describes the type of constraint
    the value represents.  A constraint on the the number of times a content item may be rendered may have
    the name 'Allowed uses' and a value of 5.

    Additionally a constraint may be assigned any number of name/value attributes to convey more complex information.
    For example a constraint restricting the duration content can be accessed may include the duration as the value
    of the constraint and include the times the duration started and will end as attributes.

    \ingroup drm
*/

/*!
    Constructs an empty constraint.
*/
QDrmRights::Constraint::Constraint()
{
}

/*!
    Constructs a constraint with the name \a name and value \a value.
*/
QDrmRights::Constraint::Constraint( const QString &name, const QVariant &value )
{
    d = new QDrmRightsConstraintPrivate;

    d->name = name;
    d->value = value;
}

/*!
    Constructs a constraint with the name \a name, value \a value and attributes \a attributes.
 */
QDrmRights::Constraint::Constraint( const QString &name, const QVariant &value, const QList< QPair< QString, QVariant > > &attributes )
{
    d = new QDrmRightsConstraintPrivate;

    d->name = name;
    d->value = value;
    d->attributes = attributes;
}

/*!
    Constructs a copy of the constraint \a other.
*/
QDrmRights::Constraint::Constraint( const Constraint &other )
{
    *this = other;
}

/*!
    Destroys a constraint.
*/
QDrmRights::Constraint::~Constraint()
{
}

/*!
    Assigns the value of \a other to a constraint.
*/
QDrmRights::Constraint &QDrmRights::Constraint::operator =( const Constraint &other )
{
    d = other.d;

    return *this;
}

/*!
    Returns the name or type of the constraint.
*/
QString QDrmRights::Constraint::name() const
{
    return d ? d->name : QString();
}

/*!
    Returns the value of the constraint.
*/
QVariant QDrmRights::Constraint::value() const
{
    return d ? d->value : QVariant();
}

/*!
    Returns the number of attributes the constraint has.
*/
int QDrmRights::Constraint::attributeCount() const
{
    return d ? d->attributes.count() : 0;
}

/*!
    Returns the name or type of the attribute at the index \a index.
*/
QString QDrmRights::Constraint::attributeName( int index ) const
{
    return d ? d->attributes[ index ].first : QString();
}

/*!
    Returns the value of the attribute at the index \a index.
*/
QVariant QDrmRights::Constraint::attributeValue( int index ) const
{
    return d ? d->attributes[ index ].second : QVariant();
}

/*!
    \fn QDrmRights::Constraint::serialize(Stream &stream) const

    Writes a QDrmRights::Constraint object to a \a stream.

    \internal
 */
template <typename Stream> void QDrmRights::Constraint::serialize(Stream &stream) const
{
    if( d != 0 )
    {
        stream << d->name;
        stream << d->value;
        stream << d->attributes;
    }
    else
    {
        stream << QString();
        stream << QVariant();
        stream << QList< QPair< QString, QVariant > >();
    }
}

/*!
    \fn QDrmRights::Constraint::deserialize(Stream &stream )

    Reads a QDrmRights::Constriant object from a \a stream.

    \internal
 */
template <typename Stream> void QDrmRights::Constraint::deserialize(Stream &stream)
{
    d = new QDrmRightsConstraintPrivate;

    stream >> d->name;
    stream >> d->value;
    stream >> d->attributes;
}

/*!
    \typedef QDrmRights::ConstraintList

    Synonym for \c{QList< QDrmRights::Constraint >}
*/

/*!
    \typedef QDrmRights::Permissions

    Synonym for \c{QFlags< QDrmRights::Permission >}
*/

class QDrmRightsPrivate : public QSharedData
{
public:
    QDrmRightsPrivate()
        : permission( QDrmRights::Unrestricted )
        , status( QDrmRights::Valid )
    {
    }

    QDrmRights::Permission permission;
    QDrmRights::Status status;
    QDrmRights::ConstraintList constraints;
};

/*!
    Constructs a QDrmRights object.

    The inital status of the rights object is Unprotected.
*/
QDrmRights::QDrmRights()
    : d( 0 )
{
}

/*!
    Constructs a drm rights object with given \a permission, \a status and \a constraints.
*/
QDrmRights::QDrmRights( Permission permission, Status status, const ConstraintList &constraints )
{
    d = new QDrmRightsPrivate;

    d->permission  = permission;
    d->status      = status;
    d->constraints = constraints;
}

/*!
    Construct of copy of \a other.
*/
QDrmRights::QDrmRights( const QDrmRights &other )
{
    *this = other;
}

/*!
    Destroys a QDrmRights object.
*/
QDrmRights::~QDrmRights()
{
}

/*!
    Assigns the value of \a other to a QDrmRights object.
*/
QDrmRights &QDrmRights::operator =( const QDrmRights &other )
{
    d = other.d;

    return *this;
}

/*!
    Returns the permission rights information is available for.
 */
QDrmRights::Permission QDrmRights::permission() const
{
    return d ? d->permission : Unrestricted;
}

/*!
    Returns the status of the rights.
*/
QDrmRights::Status QDrmRights::status() const
{
    return d ? d->status : Valid;
}

/*!
    Returns a list of constraints imposed by the rights.
*/
QDrmRights::ConstraintList QDrmRights::constraints() const
{
    return d ? d->constraints : ConstraintList();
}

/*!
    Returns a string representation of a DRM \a permission.
*/
QString QDrmRights::toString( Permission permission )
{
    switch( permission )
    {
    case Play:       return QObject::tr( "Play license"          );
    case Display:    return QObject::tr( "Display license"       );
    case Execute:    return QObject::tr( "Execute license"       );
    case Print:      return QObject::tr( "Print license"         );
    case Export:     return QObject::tr( "Export license"        );
    case Distribute: return QObject::tr( "Distribution permission"  );
    case Preview:    return QObject::tr( "Preview permission"       );
    default:
        return QString();
    }
}

/*!
    Returns a string representation of a DRM \a permission for a \a status.
 */
QString QDrmRights::toString( Permission permission, Status status )
{
    QString validity;

    switch( status )
    {
        case Invalid:
            validity = QObject::tr( "Invalid", "Used in context of licences as adjective -> Invalid print license"      );
            break;
        case Valid:
            validity = QObject::tr( "Valid", "Used in context of licences as adjective -> Invalid print license"        );
            break;
        case ValidInFuture:
            validity = QObject::tr( "Future", "Used in context of licences as adjective-> Invalid print license" );
    }

    switch( permission )
    {
        case Play:       return QObject::tr( "%1 play license", "%1=Invalid/Valid/Future" ).arg( validity );
        case Display:    return QObject::tr( "%1 display license" , "%1=Invalid/Valid/Future"        ).arg( validity );
        case Execute:    return QObject::tr( "%1 execute license"  , "%1=Invalid/Valid/Future"       ).arg( validity );
        case Print:      return QObject::tr( "%1 print license" , "%1=Invalid/Valid/Future"          ).arg( validity );
        case Export:     return QObject::tr( "%1 export license" , "%1=Invalid/Valid/Future"         ).arg( validity );
        case Distribute: return QObject::tr( "%1 distribution license" , "%1=Invalid/Valid/Future").arg( validity );
        case Preview:    return QObject::tr( "%1 preview license" , "%1=Invalid/Valid/Future"     ).arg( validity );
        default:
            return QString();
    }
}

/*!
    \fn QDrmRights::serialize(Stream &stream) const

    Writes a QDrmRights object to a \a stream.

    \internal
*/
template <typename Stream> void QDrmRights::serialize(Stream &stream) const
{
    stream << permission();
    stream << status();
    stream << constraints();
}

/*!
    \fn QDrmRights::deserialize(Stream &stream )

    Reads a QDrmRights object from a \a stream.

    \internal
*/
template <typename Stream> void QDrmRights::deserialize(Stream &stream)
{
    d = new QDrmRightsPrivate;

    stream >> d->permission;
    stream >> d->status;
    stream >> d->constraints;
}

Q_IMPLEMENT_USER_METATYPE( QDrmRights );
Q_IMPLEMENT_USER_METATYPE( QDrmRights::Constraint );
Q_IMPLEMENT_USER_METATYPE_TYPEDEF( QDrmRightsConstraintList, QDrmRights::ConstraintList );
