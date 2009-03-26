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
#include "qds_p.h"
#include "qdsserviceinfo.h"
#include "qdsserviceinfo_p.h"

// Qt includes
#include <QRegExp>
#include <QDebug>

// Qtopia includes
#include <QMimeType>
#include <QtopiaFeatures>
#include <QTranslatableSettings>
#include <QtopiaService>

// ============================================================================
//
//  QDSServicePrivateInfo
//
// ============================================================================

QDSServiceInfoPrivate::QDSServiceInfoPrivate()
:   mService(),
    mName(),
    mUiName(),
    mRequestDataTypes(),
    mResponseDataTypes(),
    mAttributes(),
    mDepends(),
    mDescription(),
    mIcon(),
    mProcessed( false ),
    mValid( false )
{
}

QDSServiceInfoPrivate::QDSServiceInfoPrivate(
    const QDSServiceInfoPrivate& other )
:   mService( other.mService ),
    mName( other.mName ),
    mUiName( other.mUiName ),
    mRequestDataTypes( other.mRequestDataTypes ),
    mResponseDataTypes( other.mResponseDataTypes ),
    mAttributes( other.mAttributes ),
    mDepends( other.mDepends ),
    mDescription( other.mDescription ),
    mIcon( other.mIcon ),
    mProcessed( false ),
    mValid( false )
{
}

QDSServiceInfoPrivate::QDSServiceInfoPrivate( const QString& name,
                                              const QString& service )
:   mService( service ),
    mName( name ),
    mUiName(),
    mRequestDataTypes(),
    mResponseDataTypes(),
    mAttributes(),
    mDepends(),
    mDescription(),
    mIcon(),
    mProcessed( false ),
    mValid( false )
{
}

void QDSServiceInfoPrivate::processSettings()
{
    // First check that we have a name and service defined
    if ( mService.isEmpty() || mName.isEmpty() ) {
        mValid = false;
        mProcessed = true;
        return;
    }

    // Generate the filename of the services file, and check it exists
    QString filename = Qtopia::qtopiaDir() + "etc/qds/" + mService;
    QFileInfo file( filename);
    if ( !file.exists() ) {
        mValid = false;
        mProcessed = true;
        return;
    }

    // Check that the Qtopia service file has an action for this QDS service
    if(!correctQtopiaServiceDescription()) {
        mValid = false;
        mProcessed = true;
        return;
    }

    // Make sure the settings file has a group for the QDS service specified
    // by name
    QTranslatableSettings settings( filename, QSettings::IniFormat );
    if ( !settings.childGroups().contains( mName ) ) {
        mValid = false;
        mProcessed = true;
        return;
    }
    //obtain ui name of this QDS Service
    settings.beginGroup( QLatin1String("QDSInformation") );
    mUiName = settings.value( QLatin1String("Name") ).toString(); // No tr
    settings.endGroup();

    // Now obtain the additional information about the QDS service
    settings.beginGroup( mName );

    QString requestTypes = settings.value( "RequestDataType" ).toString(); // No tr
    if ( !requestTypes.isEmpty() )
        mRequestDataTypes = requestTypes.split( ";" );

    QString responseTypes = settings.value( "ResponseDataType" ).toString(); // No tr
    if ( !responseTypes.isEmpty() )
        mResponseDataTypes = responseTypes.split( ";" );

    QString attributes = settings.value( "Attributes" ).toString(); // No tr
    mAttributes = attributes.split( ";" );

    QString depends = settings.value( "Depends" ).toString(); // No tr
    if ( !depends.isEmpty() )
        mDepends = depends.split( ";" );

    mDescription = settings.value( "Description" ).toString(); // No tr

    mIcon = settings.value( "Icon" ).toString(); // No tr

    settings.endGroup();

    // Set the flags
    mValid = true;
    mProcessed = true;
}

bool QDSServiceInfoPrivate::correctQtopiaServiceDescription()
{
    if ( mName == "Translation" )
        return true;

    QString serviceDFile = QtopiaService::config( mService );
    if ( serviceDFile.isEmpty() ) {
        qWarning() << "No service description for" << mService;
        return false;
    }

    static QString lastDFile;
    static QString lastActions;

    QString actions;
    if (lastDFile == serviceDFile) {
        actions = lastActions;
    } else {
        QTranslatableSettings settings( serviceDFile, QSettings::IniFormat );
        settings.beginGroup( "Service" );
        actions = settings.value( "Actions" ).toString();
        settings.endGroup();
        // Cache last result, because we're likely to be called again soon.
        lastActions = actions;
        lastDFile = serviceDFile;
    }
    if ( !actions.contains( mName + "(QDSActionRequest)" ) ) {
        qWarning() << "No action for"
                   << mName + "(QDSActionRequest)"
                   << "provided in"
                   << serviceDFile;
        return false;
    }

    return true;
}

bool QDSServiceInfoPrivate::supportsDataType( const QStringList& supported,
                                              const QString& type )
{
    if ( type.isEmpty() )
        if ( supported.count() == 0 )
            return true;

    bool wildcard = false;
    QString dtype = type.toLower();
    if ( type.right( 1 ) == "*" ) {
        wildcard = true;
        dtype = dtype.left( dtype.count() - 1 );
    }

    foreach ( QString stype, supported ) {
        if (stype.contains('*')) {
            // stype contains a wildcard - use slow regexp
            QRegExp regx( stype );
            regx.setPatternSyntax( QRegExp::Wildcard );
            regx.setCaseSensitivity( Qt::CaseInsensitive );
            if ( regx.exactMatch( dtype ) )
                return true;
            else if ( wildcard && stype.toLower().startsWith( dtype.toLower() ) )
                return true;
        } else {
            // no wildcard in stype - do quick comparison.
            if ( wildcard && stype.toLower().startsWith( dtype.toLower() ) )
                return true;
            else if (stype.toLower() == dtype)
                return true;
        }
    }

    return false;
}

// ============================================================================
//
//  QDSServiceInfo
//
// ============================================================================

/*!
    \class QDSServiceInfo
    \inpublicgroup QtBaseModule

    \brief The QDSServiceInfo class encapsulates the description of a Qt Extended Data Sharing (QDS) service

    Each QDS service is described by:

    \list
        \o QDS service name
        \o Service class
        \o Supported request data MIME types (if any)
        \o Supported response data MIME types (if any)
        \o A list of dependent QtopiaFeatures (if any)
        \o A list of attributes (optional)
        \o A description (optional)
        \o An icon (optional)
    \endlist

    Each QDS service must have a unique combination of QDS service name and Qt Extended service. The QDS service name must only contain alphanumeric characters.

    The description for a QDS service is obtained from the service file
    \c{<Qt Extended Runtime Prefix>/etc/qds/<qtopia_service>}. Each QDS service
    in a Qt Extended service should be included in the one service file, and should
    follow the form below:

    \code
    [Translation]
    File=QtopiaServices
    Context=Contacts
    [setContactImage]
    RequestDataType="image/x-qpixmap"
    ResponseDataType=
    Attributes="Set picture;Upload"
    Depends=
    Description[]=This service sets a contacts image
    Icon=AddressBook
    [beamVCard]
    RequestDataType="text/x-vcard"
    ResponseDataType=
    Attributes="beam;ir;infrared;send"
    Depends="Infrared"
    Description[]=Beam VCard
    Icon=beam
    \endcode

    A QDS service can support multiple data types for both the request and
    response data. The types can be listed individually in the service file, and
    wildcards can be used. This is demonstrated in the service file below:

    \code
    [Translation]
    File=QtopiaServices
    Context=Contacts
    [setContactImage]
    RequestDataType="image*"
    ResponseDataType=
    Attributes="Set picture;Upload"
    Depends=
    Description[]=This service sets a contacts image
    Icon=AddressBook
    [beamVCard]
    RequestDataType="text/x-vcard;text/plain;text/html"
    ResponseDataType=
    Attributes="beam;ir;infrared;send"
    Depends="Infrared"
    Description[]=Beam VCard
    Icon=beam
    \endcode

    The QDS service should also be included in the actions section of the
    Qt Extended service description, for the QDS services list above, the
    \c{<Qt Extended Runtime Prefix>/services/Contacts.service} file would look
    like:

    \code
    [Translation]
    File=QtopiaServices
    Context=Contacts
    [Service]
    Actions = "setContactImage(QString);beamVCard(QDSActionRequest)"
    Icon = addressbook/AddressBook
    Name[]=Contacts
    \endcode

    \sa QDSAction, QDSActionRequest, QDSServices, {Qt Extended Data Sharing (QDS)}

    \ingroup ipc
*/

/*!
    Constructs an empty QDSServiceInfo object.
*/
QDSServiceInfo::QDSServiceInfo()
:   d( 0 )
{
    d = new QDSServiceInfoPrivate();
}

/*!
    Constructs a deep copy of \a{other}.
*/
QDSServiceInfo::QDSServiceInfo( const QDSServiceInfo& other )
:   d( 0 )
{
    d = new QDSServiceInfoPrivate( *( other.d ) );
}

/*!
    Constructs a QDSServiceInfo object for the QDS service \a name
    and the Qt Extended service \a service.
*/
QDSServiceInfo::QDSServiceInfo( const QString& name,
                                const QString& service )
:   d( 0 )
{
    d = new QDSServiceInfoPrivate( name, service );
}

/*!
    Destroys the QDSServiceInfo object.
*/
QDSServiceInfo::~QDSServiceInfo()
{
    delete d;
}

/*!
    Makes a deep copy of \a other and assigns it to this QDSServiceInfo object.
    Returns a reference to this QDSServiceInfo object.
*/
const QDSServiceInfo& QDSServiceInfo::operator=( const QDSServiceInfo& other )
{
    d->mService = other.d->mService;
    d->mName = other.d->mName;
    d->mRequestDataTypes = other.d->mRequestDataTypes;
    d->mResponseDataTypes = other.d->mResponseDataTypes;
    d->mAttributes = other.d->mAttributes;
    d->mDepends = other.d->mDepends;
    d->mDescription = other.d->mDescription;
    d->mIcon = other.d->mIcon;
    d->mProcessed = other.d->mProcessed;
    d->mValid = other.d->mValid;

    return *this;
}

/*!
    Returns true if \a other describes the same QDS service; otherwise returns false.
*/
bool QDSServiceInfo::operator==( const QDSServiceInfo& other ) const
{
    if ( ( d->mService == other.serviceId() ) &&
         ( d->mName == other.name() ) ) {
        return true;
    }

    return false;
}

/*!
    Returns true if \a other doesn't describe the same QDS service; otherwise returns false.
*/
bool QDSServiceInfo::operator!=( const QDSServiceInfo& other ) const
{
    return operator==( other );
}

/*!
    Returns true if the QDSServiceInfo object describes a valid QDS service; 
    otherwise returns false. For this to be true a correctly formatted service file is required.
*/
bool QDSServiceInfo::isValid() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mValid;
}

/*!
    Returns true if the QDS service is available; otherwise returns false. The QDS service is available
    if all dependent features are currently available in QtopiaFeatures.
    A QDS service must be available for a QDSAction request to be successful.
*/
bool QDSServiceInfo::isAvailable() const
{
    if ( d->mDepends.count() == 0 )
        return true;

    foreach( QString feature, d->mDepends ) {
        if ( !feature.isEmpty() && !QtopiaFeatures::hasFeature( feature ) )
            return false;
    }

    return true;
}

/*!
    Returns the QDS service ID.
*/
QString QDSServiceInfo::serviceId() const
{
    return d->mService;
}

/*!
    Returns the translated name of the QDS service.
*/
QString QDSServiceInfo::serviceName() const
{
    return d->mUiName;
}

/*!
    Returns the name of the service.
*/
QString QDSServiceInfo::name() const
{
    return d->mName;
}

/*!
    \fn QStringList QDSServiceInfo::requestDataTypes() const
    Returns the IDs for supported request data types.

    \sa supportsRequestDataType(), responseDataTypes()
*/
QStringList QDSServiceInfo::requestDataTypes() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mRequestDataTypes;
}

/*!
    \fn bool QDSServiceInfo::supportsRequestDataType( const QMimeType& type ) const
    Returns true if the described service supports request data of \a type.

    \sa requestDataTypes(), supportsResponseDataType()
*/
bool QDSServiceInfo::supportsRequestDataType( const QMimeType& type ) const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->supportsDataType( d->mRequestDataTypes, type.id() );
}

/*!
    \internal
*/
bool QDSServiceInfo::supportsRequestDataTypeOrWild(
    const QString& type ) const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->supportsDataType( d->mRequestDataTypes, type );
}


/*!
    \fn QStringList QDSServiceInfo::responseDataTypes() const
    Returns the IDs for supported response data types.

    \sa supportsResponseDataType(), requestDataTypes()
*/
QStringList QDSServiceInfo::responseDataTypes() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mResponseDataTypes;
}

/*!
    \fn bool QDSServiceInfo::supportsResponseDataType( const QMimeType& type ) const
    Returns true if the described service supports response data of \a type.

    \sa responseDataTypes(), supportsRequestDataType()
*/
bool QDSServiceInfo::supportsResponseDataType( const QMimeType& type ) const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->supportsDataType( d->mResponseDataTypes, type.id() );
}

/*!
    \internal
*/
bool QDSServiceInfo::supportsResponseDataTypeOrWild(
    const QString& type ) const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->supportsDataType( d->mResponseDataTypes, type );
}

/*!
    Returns the attributes of the service.
*/
QStringList QDSServiceInfo::attributes() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mAttributes;
}

/*!
    Returns the description of the service.
*/
QString QDSServiceInfo::description() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mDescription;
}

/*!
    Returns the icon of the service.
*/
QString QDSServiceInfo::icon() const
{
    if ( !d->mProcessed )
        d->processSettings();

    return d->mIcon;
}

/*!
    \fn void QDSServiceInfo::deserialize(Stream &value)

    \internal

    Deserializes the QDSServiceInfo instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSServiceInfo::deserialize(Stream &stream)
{
    QString name;
    stream >> name;

    QString service;
    stream >> service;

    delete d;
    d = new QDSServiceInfoPrivate( name, service );
}

/*!
    \fn void QDSServiceInfo::serialize(Stream &value) const

    \internal

    Serializes the QDSServiceInfo instance out to a template
    type \c{Stream} \a stream.
 */

template <typename Stream> void QDSServiceInfo::serialize(Stream &stream) const
{
    // Service info
    stream << name();
    stream << serviceId();
}

// Macros
Q_IMPLEMENT_USER_METATYPE(QDSServiceInfo);
