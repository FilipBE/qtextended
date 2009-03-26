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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>

#include <qtopialog.h>
#include <qtopianamespace.h>

#include <qsxepolicy.h>
#include <qpackageregistry.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Uncomment this to allow a permissive system
// #define PERMISSIVE 1

// This is _much_ quicker than using QFileInfo.
static time_t fileModified(const QString &filename)
{
    struct stat statbuf;
    if (stat(filename.toLocal8Bit(), &statbuf) == 0)
        return statbuf.st_mtime;

    return ((time_t)-1);
}

/*!
  \class SXEPolicyManager
    \inpublicgroup QtPkgManagementModule

  \brief The SXEPolicyManager class provides a management interface for Qt Extended Safe Execution Environment policies

  In order for Qtopia \l {Safe Execution Environment (SXE)} programs to function they must request services and
  other functionality from other components of Qtopia.  However to provide
  a level of security for the SXE, requests must only be actioned for those
  programs which are authorised.

  The SXEPolicy manager provides an interface to the Qt Extended system policy
  files which record which programs are authorised for what service requests.

  In the context of SXE Policy, the word \i{service request} is intended to
  be more general than \l {Services} {Qt Extended Services}.
  Policy does cover these services, but it also is designed to work with
  any type of inter-process communication which maybe described by a string name.

  In the context of SXE Policy, a \i{program} is a number of (at least one)
  binary executable files, and support files that are installed onto the
  storage media available to the Qt Extended device.  Any binary installed with
  the program or executing on its behalf is identified by that program's
  unique Program Identity, a number between 1 and 255 inclusive.  Program
  Identity 0 is reserved for the Qt Extended server.

  SXE Policies provide a list of profile names for each Program Identity.
  Each profile name stands for a list of service request names.  Some examples
  of service request names are:
  \list
    \o openURL(QString url)
    \o removeEvent(PimEvent)
    \o mtabChanged()
  \endlist

  The profile names are documented in the
  \l {Safe Execution Environment (SXE)}{SXE Documentation} but as an example
  the \c openURL(QString) request is in the \i {web profile}, and the
  \c removeEvent(PimEvent) request is in the \i {admin profile}; and the
  \c mtabChanged() request is in the \i comm profile.

  Profiles serve as a level of indirection to lessen the impact of changes
  in the service names or structures.

  To use the SXEPolicyManager class first obtain the Program Identity, then call
  findPolicy() to get a list of the profile names authorized for the program.
  In attempting to decide whether to action a request these profile names may be
  used in application program logic:
  \code
    QStringList profs = SXEPolicyManager::getInstance()->findPolicy( progId );
    if ( profs.contains( "web" ))
        processWebRequest();
  \endcode

  To find if a particular service request name is allowed call findRequest() to
  see if a profile authorizes a particular request.

  A program may be authorized under zero, one or more than one profile.
  A request will generally be only included in one profile, but there is nothing
  to prevent it being present in more than one.

  \sa QPackageRegistry
*/

/*!
  Construct a new SXEPolicyManager.  Client code should never construct
  call this, instead use the getInstance() method
*/
SXEPolicyManager::SXEPolicyManager()
    : policyCache()
    , checkDate( true )
{
    readProfiles();
}

/*!
  Destruct a SXEPolicyManager.
*/
SXEPolicyManager::~SXEPolicyManager()
{
}

/*!
  Return an pointer to this processes SXEPolicyManager instance.
*/
SXEPolicyManager *SXEPolicyManager::getInstance()
{
    static SXEPolicyManager theInstance;
    return &theInstance;
}

/**
  \internal
  return eg the number 123 from "[123]\n" or -1 if doesnt parse
*/
static int parseNumberInSquareBrackets( const QString &s )
{
    int i = 1;       // start after the '['
    unsigned int n = 0;
    while ( s[i] != '\0' && s[i] != QLatin1Char(']') )
    {
        if ( s[i] >= '0' && s[i] <= '9' )
            n = n * 10 + ( s[i].unicode() - '0' );
        else
            return -1;
        ++i;
    }
    return n;
}

/*!
  Given the \a progId return a list of the profile names which that
  program is authorised to access.  The information is read from the
  Qt Extended SXE policy file [qt_prefix]/etc/sxe.policy.  A caching algorithm
  is used to lessen the number of file accesses required for
  recurring lookups.  The cache is checked for freshness against this
  files last modify time, as the Qt Extended installer may have changed
  it since it was last accessed.
*/
QStringList SXEPolicyManager::findPolicy( unsigned char progId )
{
    static QString policyFileName;

    if ( policyFileName.isNull() )
        policyFileName = QPackageRegistry::getInstance()->sxeConfPath() +
            "/" + QPackageRegistry::policyFileName;

    QMutexLocker lock( &policyMutex );

    // XXX Use notifcation from package installer rather than stat file.
    static time_t freshness = 0;
    time_t lastModified = fileModified( policyFileName );
    if ( lastModified != freshness ) {
        policyCache.clear();
        freshness = lastModified;
    } else {
        QStringList *cachedPolicy = policyCache.object(progId);
        if (cachedPolicy)
            return *cachedPolicy;
    }

    bool cacheNeedsPriming = policyCache.isEmpty();
    QFile pf( policyFileName );
    if ( ! pf.open( QFile::ReadOnly ))
    {
        qWarning( "Could not open policy file %s!", qPrintable(policyFileName) );
        return QStringList();
    }
#ifndef SXE_INSTALLER
    Qtopia::lockFile( pf );
#endif
    QTextStream ts( &pf );
    unsigned int line = 1;
    QStringList list;
    QStringList theFoundList;
    QString buf;
    unsigned char progbuf = 255;
    int num;
    while( !ts.atEnd() )
    {
        buf = ts.readLine();
        if ( buf[0] == QLatin1Char('[') )
        {
            if (( num = parseNumberInSquareBrackets( buf )) == -1 )
            {
                qWarning( "%s: bad number in square brackets at line %d",
                        qPrintable( policyFileName ), line );
                return QStringList();
            }
            // if the cache needs priming and not the first time thru the loop
            // cache the one we just finished with before we clear it; but make
            // sure we dont keep "new"'ing if the cache is nearly full
            if ( cacheNeedsPriming && ( list.count() > 0 ) &&
                    ( policyCache.totalCost() < ( policyCache.maxCost() - 10 )))
                policyCache.insert( progbuf, new QStringList( list ), list.count() );
            if ( progbuf == progId )
            {
                // always cache the hits
                policyCache.insert( progbuf, new QStringList( list ), list.count() );
                theFoundList = list;
            }
            progbuf = (unsigned char)num;
            list.clear();
        }
        else
        {
#ifndef SXE_INSTALLER
            // ##### TODO - handle filtering policy
            int idx = buf.indexOf(QLatin1Char('{'));
            if (idx >= 0)
                list << buf.left(idx);
            else
#endif
                list << buf;
        }
        line++;
    }
    if ( progbuf == progId )
    {
        // always cache the hits
        policyCache.insert( progbuf, new QStringList( list ), list.count() );
        theFoundList = list;
    }

    return theFoundList;
    // QFile pf destructs off the stack and closes file, releasing lock
}

/*!
  Given a \a request return the profile name containing that request.  If
  the request exists in multiple profiles, this method will return a random
  profile name which contains that request.  If the request is not found,
  a null string is returned.

  If the list \a prof is non-empty, just those profiles are searched, and a
  profile name from amongst them returned if the request is found.  If the
  \a prof list is empty (the default) all profiles are searched.

  The meaning of not found (null string return) is defined by the caller.
  For a secure system \i{not found} equals deny.

  Note that for a worst case, ie no \a prof is provided, and the messages
  request is not found this method will first check its internal cache and
  then scan the entire file on storage before determining the \i{not found}
  result.  For this reason the profile lists should be carefully chosen so
  that common messages do not cause a worst case.

  To avoid worst cases two synthetic profiles "allow" and "deny" should be
  used to \i{white-list} and \i{black-list} common service requests.

  The search order is
  \table
      \header \o item searched \o conditions
      \row
        \o cache \o if in cache AND if either the \a prof is empty, OR \a prof
            contains the cached result (counting "deny" or "allow" as listed
            in \a prof)
      \row
        \o \a prof  \o if non-empty
      \row
        \o "deny"   \o if exists
      \row
        \o "allow"  \o if exists
      \row
        \o sxe.profile on disk   \o always on fall-thru
  \endtable

  Note that a cache freshness check is not performed since the sxe.profiles
  file is not altered during run-time.
*/
QString SXEPolicyManager::findRequest( QString request, QStringList prof )
{
    QMultiHash<QString,QString>::const_iterator it = requestHash.find(request);
    if (it != requestHash.end()) {
        if (prof.isEmpty()) {
            return *it;
        }
        while (it != requestHash.end() && it.key() == request) {
            const QString &value = *it;
            if (value == QLatin1String("deny")
                || value == QLatin1String("allow")
                || prof.contains(value))
            {
                return value;
            }
            ++it;
        }
    }

    if ( !prof.isEmpty() )
    {
        // will only look for things in prof, so add "deny" and "allow"
#ifdef PERMISSIVE
        prof << QLatin1String("allow") << QLatin1String("deny");
#else
        prof << QLatin1String("deny") << QLatin1String("allow");
#endif
    }

    QString buf = checkWildcards( request, prof );
    if ( !buf.isEmpty() ) {
        return buf;
    }

    return QString();
}

bool SXEPolicyManager::readProfiles()
{
#ifdef SXE_INSTALLER
    QString policyFileName = QPackageRegistry::getQtopiaDir() + "etc/sxe.profiles";
#else
    QString policyFileName = Qtopia::qtopiaDir() + "etc/sxe.profiles";
#endif
    QFile profs( policyFileName );
    QTextStream ts( &profs );
    if ( ! profs.open( QFile::ReadOnly ))
    {
#ifndef SXE_INSTALLER
        qWarning( "Could not open policy file %s!", policyFileName.toLocal8Bit().constData() );
#endif
        return false;
    }

    unsigned int line = 1;
    QString currentProf;
    while( !ts.atEnd() )
    {
        QString buf = ts.readLine().trimmed();
        if (buf.isEmpty())
        {
            ++line;
            continue;
        }
        else if ( buf[0] == QLatin1Char('[') )
        {
            int rhb = buf.indexOf( QLatin1Char(']') );
            if ( rhb == -1 )
            {
                qWarning( "Bad profile name in square brackets at line %d", line );
                return false;
            }
            currentProf = buf.mid( 1, rhb - 1 );
        }
        else if ( buf[0] != QLatin1Char('#') )
        {
            if ( buf.endsWith( QLatin1Char('*') ))
            {
                buf.chop(1);
                wildcards.insertMulti(currentProf, buf);
            }
            else
            {
                requestHash.insert(buf, currentProf);
            }
        }
        line++;
    }

    return true;
}

QString SXEPolicyManager::checkWildcards( const QString &request, const QStringList &profs )
{
    QMap<QString,QString>::Iterator it = wildcards.begin();
    while ( it != wildcards.end() )
    {
        if ( request.startsWith( it.value() ) && profs.contains( it.key() ))
            return it.key();
        ++it;
    }
    return QString();
}

/*!
  Receive a time-out signal to reset the checkDate flag.  This flag prevents
  the date check mechanism from DoS'ing the out-of-date code.
*/
void SXEPolicyManager::resetDateCheck()
{
    checkDate = true;
}

/*!
  This slot is for receiving signals from a QTransportAuth object alerting
  the presence of an incoming message for authentication against policy.
  The QTransportAuth should already have taken care of confirming the
  identity of message originator, and validity of the message transport.

  Note that any number of authorisers can connect to this slot.  If any one
  of them sets the permit Status bits to Deny, then the request will be
  denied regardless of other authorisers.  In other words all must either
  provide Allow status (or leave the status unchanged) for the request
  to pass.

  The QTransportAuth::Data item \a d is the connection data representing
  the source of the \a req.
*/
void SXEPolicyManager::policyCheck( QTransportAuth::Data &d, const QString &req )
{
    QString request = req;
#ifndef SXE_INSTALLER
    qLog(SXE) << "Policy Check program" << d.progId << ", pid" << d.processId
        << "- status " << QTransportAuth::errorString( d ) << " - request"
            << ( request.isEmpty() ? "empty" : request );
#endif
    if ( request.isEmpty() ) return;   // nothing to do

    if ( request.endsWith("_fragment_") )
            request.chop( 10 );//size of "_fragment_"

#ifdef PERMISSIVE
    if ( d.status == QTransportAuth::Allow ) return;
#else
    if ( d.status == QTransportAuth::Deny ) return;
#endif

    QStringList profiles = findPolicy( d.progId );

    qLog(SXE) << "\tprofiles awarded to program" << d.progId << "are" << profiles;

    QString foundIn = findRequest( request, profiles );
    if ( foundIn.isEmpty() )
    {
        // the request was not found at all in the searched policies
        qLog(SXE) << "\trequest" << request << "not found in above profiles";
        d.status = ( d.status & QTransportAuth::ErrMask ) | QTransportAuth::Deny;
    }
    else if ( foundIn == QLatin1String("deny") )
    {
        // the request was blacklisted
        qLog(SXE) << "\trequest" << request << "blacklisted";
        d.status = ( d.status & QTransportAuth::ErrMask ) | QTransportAuth::Deny;
    }
    else if ( foundIn == QLatin1String("allow") )
    {
        // the request was whitelisted
        qLog(SXE) << "\trequest" << request << "whitelisted";
        d.status = ( d.status & QTransportAuth::ErrMask ) | QTransportAuth::Allow;
    }
    else if ( profiles.contains( foundIn ))
    {
        qLog(SXE) << "\trequest" << request << "found in above profiles";
        d.status = ( d.status & QTransportAuth::ErrMask ) | QTransportAuth::Allow;
    }
    else
    {
        qLog(SXE) << "\trequest" << request << "found in domain not awarded:" << foundIn;
        d.status = ( d.status & QTransportAuth::ErrMask ) | QTransportAuth::Deny;
    }
}
