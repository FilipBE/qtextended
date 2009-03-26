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

#include "packageversion.h"
#include <qstringlist.h>
#include <qtopianamespace.h>
#include <qdebug.h>
#include <qtopialog.h>

const long Version::MAJOR_MAG = 1000000;
const long Version::MINOR_MAG = 1000;
const long Version::PATCH_MAG = 1;

/*!
  \internal
  \class Version
    \inpublicgroup QtPkgManagementModule
  \brief Represents a version of the form "a.b.c" which allows operator comparisons with other Version objects.

  If an invalid version string is supplied the version becomes 0.0.0 and all operator comparisons
  will return false.
*/

/*!
  Constructs a Version given \a version as a QString
*/
Version::Version( const QString &version ):m_version(0)
{
    int thisV = 0;
    int thisD = 0;
    int thisPtr = 0;
    int i = 0;
    long magnitudes[NUM_VERSION_PARTS] = { MAJOR_MAG, MINOR_MAG, PATCH_MAG };

    if ( thisPtr > 0 )
        thisPtr--;

    while ( i < NUM_VERSION_PARTS  )
    {
        while ( thisPtr < version.count() && ( thisD = version[thisPtr++].digitValue() ) != -1 )
        {
            thisV = thisV * 10 + thisD;
        }

        if( version[ thisPtr - 1 ].isLetter() )
        {
            qWarning( "Version::Version:- letter found, version string=%s", qPrintable( version) );
            m_version = 0;
            break;
        }

        m_version += thisV * magnitudes[i];
        thisV = 0;
        i++;
    }
}


/*!
  \internal
  Returns true if this version is less than \a other,
  otherwise returns false.
*/
bool Version::operator< ( const Version& other ) const
{
    return bothValid(other) ? m_version < other.m_version : false ;
}

/*!
  \internal
  Returns true if this version is equal to \a other,
  otherwise returns false.
*/
bool Version::operator== ( const Version &other ) const
{
    return bothValid(other) ? m_version == other.m_version : false;
}


/*!
  \internal
  Returns true if this version is less than or equal to \a other,
  otherwise returns false.
*/
bool Version::operator<=( const Version &other ) const
{
    return bothValid(other) ? ( (*this == other) || (*this < other) ) : false;
}

/*!
  \internal
  Returns true if this version is greater than \a other,
  otherwise returns false.
*/
bool Version::operator > ( const Version& other ) const
{
    return bothValid(other) ? (m_version > other.m_version) : false;
}

/*!
  \internal
  Returns true if this version is greater than or equal to \a other,
  otherwise returns false.
*/
bool Version::operator >= ( const Version& other ) const
{
    return bothValid(other) ? m_version >= other.m_version : false;
}


/*!
  \internal
  Returns a string representation of the Version
*/
QString Version::toString() const
{
    return QString::number( maj() ) + "." + QString::number( min() ) + "."
            + QString::number( patch() );
}

/*!
  \internal
  \fn int Version::maj() const

  Returns a the major number of the Version. i.e. the x of x.b.c
*/


/*!
  \internal
  \fn int Version::min() const

  Returns a the minor number of the Version. i.e. the x of a.x.c
*/

/*!
  \internal
  \fn int Version::patch() const

  Returns a the patch number of the Version. i.e. the x of a.b.x
*/

/*!
  \internal
  Returns true if this Version and \a other are each valid versions, otherwise
  return false.  A valid Version does not evaluate "0.0.0"
*/
bool Version::bothValid( const Version &other) const
{
    return m_version != 0 && other.m_version != 0;
}

/*!
  \internal
  \class VersionItem
    \inpublicgroup QtPkgManagementModule
  \brief The VersionItem class provides an abstract interface to version item classes.

  The VersionItem class defines a standard interface to implement version items.
  There are two types of concrete version items, single items such as 4.2.3 and
  range item's such as 4.2-4.3.  It is not expected that there will be any more
  types of version items in future.

  The primary reason to use the VersionItem class is to generically
  be able to see if two version items are compatible without knowing the
  what the version item actually is i.e. whether it is a \l Single or \l Range.

  VersionItems should only be constructed via the createVersionItem() and
  createVersionItemList() functions, which accept a version or list of
  versions in string format.

  \sa Single, Range
*/

/*!
  \internal
  Constructs a VersionItem
*/
VersionItem::VersionItem( const QString &str)
{ Q_UNUSED(str) };


/*!
  \internal
  Destroys the VersionItem
*/
VersionItem::~VersionItem() {};


/*!
  \internal
  \fn bool VersionItem::isCompatibleWith( const VersionItem &other ) const
  Returns true if this version item and \a other are compatible with
  each other.  Compatible is determined by whether they overlap

  e.g. 4.2.2 overlaps with 4.2.2, 4.2.2 overlaps with 4.1-4.3.
*/


/*!
  \internal
  \fn bool VersionItem::isCompatibleWith( const Single &other) const
  Returns true if this Version Item is compatible the Single defined by \a other.
  Otherwise returns false.
*/

/*!
  \internal
  \fn bool VersionItem::isCompatibleWith( const Range &other) const
  Returns true if this Version Item is compatible the Range defined by \a other.
  Otherwise returns false.
*/

/*!
  \internal
  \fn QString VersionItem::toString() const
  Returns a string representing this Version Item.
*/


/*!
  \internal
  \class Single
    \inpublicgroup QtPkgManagementModule
  \brief The Single class represents a version of the form "a.b.c" and allows
         testing of compatability with other \l VersionItems.

   The Single class is a concrete implementation of VersionItem, the class
   is not intended to be used directly.  See \l VersionItem.

  \sa VersionItem
*/

/*!
  \internal
  Constructs a Single given \a str
*/
Single::Single( const QString &str ):VersionItem(str), m_version(str)
{
}

/*!
  \internal
  Returns true if this Single is compatible with \a other
  Since this is of type Single, this will call the overloaded version
*/

bool Single::isCompatibleWith( const VersionItem &other ) const
{
    return other.isCompatibleWith(*this);
}

/*!
  \internal
  Returns true if this Single is the same as \a other.
  Otherwise returns false.
*/
bool Single::isCompatibleWith( const Single &other ) const
{
    return  m_version == other.ver();
}

/*!
  \internal
  Returns true if this single version is within the range defined by \a other.
  Otherwise returns false.
*/
bool Single::isCompatibleWith( const Range &other ) const
{
    return ( other.minVer() <= m_version && m_version <= other.maxVer() );

}

/*!
  \internal
  \fn Version Single::ver() const
   Returns a Version object which is equivalent to this Single
*/

/*!
  \internal
  \fn QString Single::toString() const
   Returns a string representation of this Single
*/

/*!
  \internal
  \class Range
    \inpublicgroup QtPkgManagementModule
  \brief The Range class represents a version, typically of the form "a.b.c-x.y.z" and allows
         testing of compatability with other \l VersionItems.

   The Range class is a concrete implementation of VersionItem, the class
   is not intended to be used directly.  See \l VersionItem.

   The minimum and maximum ranges need not have 3 places. The following ranges
   are acceptable e.g 4-5, 4.2.3-4.5, 4-4.2.3.

  \sa VersionItem
*/

/*!
  \internal
  Constructs a Range given \a str
*/
Range::Range( const QString &str ) :VersionItem(str),m_min("0"), m_max("0")
{
    if ( !str.contains("-")  )
        return;
    QStringList strList = str.split("-");
    if ( strList.size() != 2 )
       return;

    m_min = Version( strList[0] );
    m_max = Version( strList[1] );
    if ( m_max <= m_min )
    {
        m_min = Version( "0" );
        m_max = Version( "0" );
    }
}

/*!
  \internal
  Returns true if this range version is compatible with \a other.
  Since this is of type Range, this will forward the call to the
  right overload.
*/
bool Range::isCompatibleWith( const VersionItem &other ) const
{
    return other.isCompatibleWith(*this);
}

/*!
  \internal
  Returns true if the single version \a other lies within the range
  represented by this Range, otherwise returns false.
*/
bool Range::isCompatibleWith( const Single &other ) const
{
   return ( m_min <= other.ver() && other.ver() <= m_max );
}

/*!
  \internal
  Returns true if the range's represented by this and \a other overlap,
  otherwise returns false.

  The overlap may be partial such as 4.0-5.0 and 4.5-5.5
  or complete such as 4-5 and 3-6.
*/
bool Range::isCompatibleWith( const Range &other ) const
{
    return ( other.minVer() <= m_min && m_min <= other.maxVer() )
        || ( other.minVer() <= m_max && m_max <= other.maxVer() )
        || ( m_min <= other.minVer() && other.minVer() <= m_max )
        || ( m_min <= other.maxVer() && other.maxVer() <= m_max );
}

/*
  \internal
  \fn Version Range::maxVer() const
   Returns a Version object which is equivalent to the upper version of
   this Range.
*/

/*
  \internal
  \fn Version Range::minVer() const
   Returns a Version object which is equivalent to the lower version of
   this Range.
*/

/*
  \internal
  \fn QString Range::toString() const
   Returns a string representation of this Range
*/

/*!
  \internal
  Creates a VersionItem given \a str.  \a str may be of the form
  of a single version "a.b.c" or a range such "a.b.c-x.y.z" or "a-x"
*/
VersionItem* VersionItem::createVersionItem( const QString &str )
{
    QStringList strList;
    if ( str.contains("-") )
    {
        strList = str.split("-");
        return new Range( str );
    }
    else
    {
        return new Single( str );
    }
}

/*!
  \internal
  Creates a list of VersionItems given a single string \a verList which represents
  all the version items.

  An example \a verList is "4.2.2,4.3-4.4", would produce two VersionItems
  one a Single and the other a Range.
  \bold {Note:} Ensure that all item's in the QList are destroyed when the list
  is no longer needed to avoid memory leakage.
*/
QList<VersionItem*> VersionItem::createVersionItemList( const QString &verList )
{
    QList<VersionItem *> ret;
    foreach( QString verStr, verList.split(",") )
    {
        ret.append( VersionItem::createVersionItem( verStr ) );
    }
    return ret;
}


/*!
  \internal
  Returns true if there is overlap of any two version items listed between the version lists
  \a verListStr1 and \a verListStr2, which are in string format.
*/
bool VersionUtil::checkVersionLists( const QString &verListStr1, const QString &verListStr2 )
{
    QList<VersionItem*> verList1 = VersionItem::createVersionItemList( verListStr1 );
    QList<VersionItem*> verList2 = VersionItem::createVersionItemList( verListStr2 );

    foreach( VersionItem *verItem1, verList1 )
    {
        foreach ( VersionItem *verItem2, verList2  )
            if ( verItem2->isCompatibleWith( *verItem1 ) )
                return true;
    }

   while( !verList1.isEmpty() )
        delete verList1.takeFirst();

    while( !verList2.isEmpty() )
        delete verList2.takeFirst();

    return false;
}
/*!
  \internal
  Returns true if any of the devices listed in \a devList1 and \a devList2 are common
  to both.  This means that the providers of the two lists are device compatable.

  The parameters are comma delimited lists of devices.  If any of the lists is set to
  Unknown or a list is empty, true is returned.
*/
bool DeviceUtil::checkDeviceLists( const QString &devList1, const QString &devList2 )
{

    qLog(Package) << "DeviceUtil::checkDevicelists list 1: " << devList1 << " list2: " << devList2;
    //if device type is specifically set to Unknown or not set at all let it it pass this check
    if ( devList1 == QLatin1String("Unknown") || devList2 == QLatin1String("Unknown") 
         || devList1 == QString::null || devList2 == QString::null )
        return true; 

    QStringList list1 = devList1.toLower().split(",");
    QStringList list2 = devList2.toLower().split(",");
    foreach( QString device, list1 )
    {
        if ( list2.contains(device) )
            return true;
    }
    return false;
}

/*!
  \internal
  Returns list of devices compatible with this device.  By convnetion, this device
  should be listed first.
*/
QString DeviceUtil::compatibleDevices()
{
    return QTOPIA_COMPATIBLE_DEVICES;
}


