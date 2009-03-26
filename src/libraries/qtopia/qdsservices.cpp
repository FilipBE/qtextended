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
#include "qdsservices.h"
#include "qdsserviceinfo.h"

#include <qtopialog.h>

// QT includes
#include <QSettings>

// ============================================================================
//
//  QDSServices
//
// ============================================================================

/*!
    \class QDSServices
    \inpublicgroup QtBaseModule

    \brief The QDSServices class provides a filtered list of available Qt Extended Data Sharing (QDS) services.

    QDSServices can used to search for available QDS services. The search can
    be filtered on a combination of request data types, response data types, 
    attributes, and Qt Extended service names.

    For example if you wanted to find a QDS service which could convert a jpeg
    image into a bitmap image you would do the following:

    \code
    QDSServices service( "image/jpeg", "image/bmp" )

    if ( service.count() == 0 )
        qWarning() << "No jpeg to bitmap image conversion service available";
    \endcode

    The data types can include a wildcard to match across multiple types, i.e.
    for a request data type filter of \c "text*" the search would find services
    which had a request data type of \c "text/plain", \c "text/html", and so on.
    A null data type is used to identify no request or response data.

    Qt Extended service names may also use wildcards, e.g. \c "MyApp*", but attributes
    do not support wildcards.

    \sa QDSServiceInfo, {Qt Extended Data Sharing (QDS)}

    \ingroup ipc
*/

/*!
    Finds all QDS services available on the device which have a request data type of
    \a requestDataType, have a response data type of \a responseDataType (both case
    insensitive), contain all the attributes listed in \a attributes (case insensitive),
    and use Qt Extended services which match \a service (case sensitive).
*/
QDSServices::QDSServices( const QString& requestDataType,
                          const QString& responseDataType,
                          const QStringList& attributes,
                          const QString& service )
{
    QString qdsDir = Qtopia::qtopiaDir() + QLatin1String("etc/qds");
    QDir files( qdsDir, service, QDir::Unsorted, QDir::Files );
    QStringList filesList = files.entryList();

    qLog(DataSharing) << "Looking for services which match"
        << requestDataType << responseDataType << attributes << service
        << "in directory" << qdsDir << "(files:" << filesList << ")";

    if ( filesList.count() == 0 )
        return;


    // Find QDS services in each
    foreach ( QString const& qdsService, filesList ) {
        processQdsServiceFile( qdsService,
                               requestDataType,
                               responseDataType,
                               attributes );
    }
}

/*!
    Finds and returns the first QDS service with name \a name (case sensitive) in
    the list. If no such service exists an invalid QDSServiceInfo is returned.
*/
QDSServiceInfo QDSServices::findFirst( const QString& name )
{
    foreach (QDSServiceInfo serviceInfo, *this) {
        if ( serviceInfo.name() == name )
            return serviceInfo;
    }

    return QDSServiceInfo();
}


/*!
    \internal
*/
void QDSServices::processQdsServiceFile( const QString& service,
                                         const QString& requestDataTypeFilter,
                                         const QString& responseDataTypeFilter,
                                         const QStringList& attributesFilter )
{
    QString serviceFileName = Qtopia::qtopiaDir() + QLatin1String("etc/qds/") + service;
    QSettings serviceFile( serviceFileName, QSettings::IniFormat );
    QStringList qdsServices = serviceFile.childGroups();
    foreach ( QString const& name, qdsServices ) {
        if ( name == QLatin1String("QDSInformation") ) //the group QDSInformation is reserved
            continue;
        if ( name == QLatin1String("Translation") ) //skip translations
            continue;

        QDSServiceInfo serviceInfo( name, service );

        bool valid = serviceInfo.isValid();
        bool requestMatch = valid && passTypeFilter( serviceInfo, requestDataTypeFilter, Request );
        bool responseMatch = requestMatch && passTypeFilter( serviceInfo, responseDataTypeFilter, Response );
        bool finalMatch = responseMatch && passAttributesFilter( serviceInfo, attributesFilter );

        qLog(DataSharing) << "Service" << QString("%1::%2").arg(service).arg(name)
            << ": valid" << valid << ", request" << requestMatch
            << ", response" << responseMatch << ", attributes" << finalMatch;

        if (finalMatch) {
            append( serviceInfo );
        }
    }
}

bool QDSServices::passTypeFilter( const QDSServiceInfo& serviceInfo,
                                  const QString& typeFilter,
                                  const Mode mode )
{
    if ( typeFilter == "*" )
        return true;

    if ( mode == Request )
        return serviceInfo.supportsRequestDataTypeOrWild( typeFilter );
    else if ( mode == Response )
        return serviceInfo.supportsResponseDataTypeOrWild( typeFilter );

    return false;
}

bool QDSServices::passAttributesFilter( const QDSServiceInfo& serviceInfo,
                                        const QStringList& attributesFilter )
{
    const QStringList& attributes = serviceInfo.attributes();

    foreach( QString attribute, attributesFilter ) {
        if ( !attributes.contains( attribute, Qt::CaseInsensitive ) ) {
            return false;
        }
    }

    return true;
}
