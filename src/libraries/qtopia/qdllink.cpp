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

// Local includes
#include "qdllink.h"

// Qtopia includes
#include <QDSAction>
#include <QDSData>
#include <QDSServices>
#include <qtopialog.h>

// Qt includes
#include <QList>
#include <QDataStream>

// Macros
Q_IMPLEMENT_USER_METATYPE(QDLLink);
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QDLLink>);

// ============================================================================
//
// Constants
//
// ============================================================================

static const QString    QDLLINK_MIMETYPE        =   "x-link/x-qdllink"; // No tr
static const QString    QDLLINK_LIST_MIMETYPE   =   "x-link/x-qlist-qdsdata"; // No tr

// ============================================================================
//
// QDLLinkPrivate
//
// ============================================================================

class QDLLinkPrivate
{
public:
    QDLLinkPrivate();

    QString mService;
    QByteArray mData;
    QString mDescription;
    QString mIcon;
    bool mBroken;
};

QDLLinkPrivate::QDLLinkPrivate()
:   mService(),
    mData(),
    mDescription(),
    mIcon(),
    mBroken( true )
{
}

// ============================================================================
//
// QDLLink
//
// ============================================================================

/*!
    \class QDLLink
    \inpublicgroup QtBaseModule

    \brief The QDLLink class fully describes a QDL link.

    The QDLLink class contains all the information for a QDL link. It allows
    the link to be activated on the source, contains a description
    and icon for the presentation of a link, and a flag to determine if the
    link is broken.

    A QDL link is activated by the QDLLink::activate() method. The
    application specific data required to activate the link is then transfered
    to the QDL source through the Qt Extended service.

    When the QDL source needs to delete the linked data
    QDLLink::setBroken() should be called to inform clients that link
    activation will not succeed. The client can then decide how, or
    if broken links should be displayed.

    \ingroup ipc
*/

/*!
    Constructs a Null QDLLink

    \sa isNull()
*/
QDLLink::QDLLink()
:   d( 0 )
{
    d = new QDLLinkPrivate();
}

/*!
    Constructs a QDLLink with \a description and an \a icon name.
    The Qt Extended service \a service contains a QDS service for activating the
    QDLLink on the QDL source and application specific data for the
    activation is stored in \a data.
*/
QDLLink::QDLLink( const QString &service,
                  const QByteArray &data,
                  const QString &description,
                  const QString &icon )
:   d( 0 )
{
    d = new QDLLinkPrivate();
    d->mService = service;
    d->mData = data;
    d->mDescription = description;
    d->mIcon = icon;
    d->mBroken = false;
}

/*!
    Constructs a QDLLink by deep copying \a other.
*/
QDLLink::QDLLink( const QDLLink &other )
:   d( 0 )
{
    d = new QDLLinkPrivate();
    d->mService = other.service();
    d->mData = other.data();
    d->mDescription = other.description();
    d->mIcon = other.icon();
    d->mBroken = other.isBroken();
}

/*!
    Constructs a QDLLink from the data in \a dataObject
*/
QDLLink::QDLLink( const QDSData& dataObject )
:   d( 0 )
{
    d = new QDLLinkPrivate();
    if ( dataObject.type().id() == mimeType().id() ) {
        QByteArray array = dataObject.data();
        {
            QDataStream ds( &array, QIODevice::ReadOnly );
            ds >> *this;
        }
    }
}

/*!
    Destroy a QDL Link object.
*/
QDLLink::~QDLLink()
{
    delete d;
    d= 0;
}

/*!
    Deep copies \a other.
*/
QDLLink& QDLLink::operator=( const QDLLink &other )
{
    d->mService = other.service();
    d->mData = other.data();
    d->mDescription = other.description();
    d->mIcon = other.icon();
    d->mBroken = other.isBroken();

    return *this;
}

/*!
    Returns the MIME type used for QDLLinks stored in QDSData objects.

    \sa listMimeType()
*/
QMimeType QDLLink::mimeType()
{
    return QMimeType( QDLLINK_MIMETYPE );
}

/*!
    Returns the MIME type used for a list of QDLLinks stored in QDSData
    objects.

    \sa mimeType()
*/
QMimeType QDLLink::listMimeType()
{
    return QMimeType( QDLLINK_LIST_MIMETYPE );
}

/*!
    Returns true if the link is null; otherwise returns false.
*/
bool QDLLink::isNull() const
{
    return d->mService.isEmpty() || d->mData.isEmpty();
}

/*!
    Returns true if the link is broken; otherwise returns false. This usually occurs when the QDL data
    source deletes the linked data item.

    \sa setBroken()
*/
bool QDLLink::isBroken() const
{
    return d->mBroken;
}

/*!
    Returns the Qt Extended service which contains the QDS service to activate the
    link.
*/
QString QDLLink::service() const
{
    return d->mService;
}

/*!
    Returns the application specific data used to activate the link on the QDL
    source.

    \sa setData()
*/
QByteArray QDLLink::data() const
{
    return d->mData;
}

/*!
    Returns the description of the link.

    \sa setDescription()
*/
QString QDLLink::description() const
{
    return d->mDescription;
}

/*!
    Returns the icon name for the link.

    \sa setIcon()
*/
QString QDLLink::icon() const
{
    return d->mIcon;
}

/*!
    Converts the link to a QDSData object. The object is only stored temporarily,
    if prelonged storage is required QDSData::store() should be called on the
    returned object.
*/
QDSData QDLLink::toQDSData() const
{
    QByteArray data;
    {
        QDataStream ds( &data, QIODevice::WriteOnly );
        ds << *this;
    }

    return QDSData( data, mimeType() );
}

/*!
    Activates the link on the QDL source.
*/
void QDLLink::activate() const
{
    if ( isNull() )
        return;
    if ( isBroken() ) {
        qLog(DataLinking) << "QDLLink::activate() - link is broken, "
                          << "can't activate";
        return;
    }

    // Find the QDS service that activates the link
    QStringList attributes;
    attributes.append( "activate" ); // No tr
    attributes.append( "QDL" );
    QDSServices activationServices( QDLLink::mimeType().id(),
                                    "",
                                    attributes,
                                    service() );

    if ( activationServices.count() == 0 ) {
        qLog(DataLinking) << "QDLLink::activate() - Service doesn't"
                          << " provide a QDL activation service";
        return;
    }

    // Create a QDS action with the first activation service
    QDSAction action( activationServices[0] );
    int res = action.exec( toQDSData() );
    if ( res == QDSAction::Error ) {
        qLog(DataLinking) << "QDLLink::activate() - An error occured"
                          << " during QDL activiation";
    } else if ( res == QDSAction::CompleteData ) {
        qLog(DataLinking) << "QDLLink::activate() - QDL activiation"
                          << " returned data";
    } else if ( res == QDSAction::Invalid ) {
        qLog(DataLinking) << "QDLLink::activate() - QDL activiation"
                          << " had an invalid response";
    }
}

/*!
    Sets the Qt Extended service which contains the QDS service for link activation to
    \a service.
*/
void QDLLink::setService( const QString &service )
{
    d->mService = service;
}

/*!
    Sets the application specific data for link activation to \a data.

    \sa data()
*/
void QDLLink::setData( const QByteArray &data )
{
    d->mData = data;
}

/*!
    Sets the link description to \a description.

    \sa description()
*/
void QDLLink::setDescription( const QString &description )
{
    d->mDescription = description;
}

/*!
    Sets the link icon name to \a icon.

    \sa icon()
*/
void QDLLink::setIcon( const QString &icon )
{
    d->mIcon = icon;
}

/*!
    Set the link's broken state to \a broken.

    \sa isBroken()
*/
void QDLLink::setBroken( const bool broken )
{
    d->mBroken = broken;
}

/*!
    \fn void QDLLink::deserialize(Stream &value)

    \internal

    Deserializes the QDLLink instance out to a template type \c{Stream}
    \a stream.
 */
template <typename Stream> void QDLLink::deserialize(Stream &stream)
{
    QString service;
    QByteArray data;
    QString description;
    QString icon;
    bool broken;

    stream >> service >> data >> description >> icon >> broken;

    setService( service );
    setData( data );
    setDescription( description );
    setIcon( icon );
    setBroken( broken );
}

/*!
    \fn void QDLLink::serialize(Stream &value) const

    \internal

    Serializes the QDLLink instance out to a template type \c{Stream}
    \a stream.
 */
template <typename Stream> void QDLLink::serialize(Stream &stream) const
{
    stream << service() << data() << description() << icon() << isBroken();
}

