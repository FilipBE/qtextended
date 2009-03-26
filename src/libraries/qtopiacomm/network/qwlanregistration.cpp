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

#include "qwlanregistration.h"

#ifndef NO_WIRELESS_LAN
#include <QtopiaNetwork>
#include <qtopialog.h>

#include <QHash>
#include <QSettings>


class QWlanRegistrationPrivate
{
public:
    QWlanRegistrationPrivate()
    {
    }
    
    QString configHandle;
};

/*!
  \class QWlanRegistration
    \inpublicgroup QtBaseModule

  \internal
  \preliminary
  \brief The QWlanRegistration class provides additional information about 
  WLAN network devices.

  This class is an extention to QNetworkDevice and QNetworkConnection as it reports WLAN specific information
  in addition to the parameter provided by the named two classes. Each QWlanRegistration
  is associated to one QNetworkDevice of type \c{QtopiaNetwork::WirelessLAN} and there are as
  many QNetworkConnection objects as the list returned by knownEssids() has entries.
  \ingroup io

  \sa QNetworkConnection, QNetworkDevice
  */


/*!
  Constructs a WLAN registration object with the given \a parent.
  The \a hashedHandle is the hash value of
  the WLAN network device handle. The object will be created in client mode if
  \a mode is Client, or server mode otherwise.
 
  The following example demonstrates how to create a QWlanRegistration: 
  \code
        QStringList wlanDevices = QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::WirelessLAN );
        foreach( QString wlanHandle, wlanDevices ) {
            QWlanRegistration* wlanReg = new QWlanRegistration( QString::number(qHash( wlanHandle )) );
        }
  \endcode

  \sa QCommInterface
  */
QWlanRegistration::QWlanRegistration( const QString& hashedHandle,
                        QObject* parent, QAbstractIpcInterface::Mode mode )
    : QCommInterface( "QWlanRegistration", hashedHandle, parent, mode )
{
    dptr = new QWlanRegistrationPrivate();
    proxy( SIGNAL(accessPointChanged()) );      
}

/*!
  Destructs an object of this type.
  */
QWlanRegistration::~QWlanRegistration()
{
    delete dptr;
    dptr = 0;
}

/*!
  \fn void QWlanRegistration::accessPointChanged()

  This signal is emitted when the current access point changes. Such a change could be triggered
  by a logon/logoff as well as the wlan automatically reconnects to a different WLAN due to enabled 
  roaming. 

  Note that this signal is not emitted when the MAC address changes only. This could happen 
  when the user is moving within a WLAN network which is supported by several access points.

*/

/*!
  Returns the ESSID of the current WLAN access point. If the associated network device
  is not online this function returns an empty string.
  */
QString QWlanRegistration::currentESSID() const
{
    return value("currentESSID").toString();
}

/*!
  Returns a list of all ESSIDs which are known to the WLAN device. 
  A WLAN device becomes known if the user has a configuration for 
  a particular WLAN. New WLAN's are either added via the standard device
  configuration dialog or as a result of active WLAN scans.

  The order of ESSIDs in the returned list indicates their priority 
  (first item being the most important ESSID). the priority is of relevance 
  when active roaming is enabled.
  */
QStringList QWlanRegistration::knownESSIDs() const
{
    //find the corresponding 
    if ( dptr->configHandle.isEmpty() ) 
    { 
        QStringList wlanDevices = QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::WirelessLAN );
        QString hash;
        foreach ( QString h, wlanDevices ) {
            hash = QString::number( qHash( h ) );
            if ( hash == service() ) 
            {
                dptr->configHandle = h;
                break;
            }
        }
    }
    if ( dptr->configHandle.isEmpty() ) {
        qLog(Network) << "Could not find WLAN configuration despite existing WLAN registration";
        return QStringList();
    }

    QStringList result;
    QSettings cfg( dptr->configHandle, QSettings::IniFormat );
    int size = cfg.beginReadArray( QLatin1String("WirelessNetworks") );
    for ( int i = 0; i < size ; i++ ) 
    {
        cfg.setArrayIndex( i );
        result.append( cfg.value("ESSID").toString() ); 
    }
    cfg.endArray();
    return result;
}
#endif //NO_WIRELESS_LAN    
