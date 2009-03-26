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

#include <qpreferrednetworkoperators.h>

/*!
    \class QPreferredNetworkOperators
    \inpublicgroup QtTelephonyModule

    \brief The QPreferredNetworkOperators class provides information about preferred network operators.
    \ingroup telephony

    The preferred operator list is stored on the SIM card and used by the modem
    to select the operator most preferred by the user.

    The currently active network operator can be retrieved and set using
    the QNetworkRegistration class.

    \sa QCommInterface
*/

/*!
    \enum QPreferredNetworkOperators::List
    Selected preferred operator list from 3GPP TS 27.007.

    \value Current Retrieve the current preferred operator list.
    \value UserControlled User-controlled preferred operator list.
    \value OperatorControlled Operator-controlled preferred operator list.
    \value HPLMN HPLMN selector.
*/

/*!
    \class QPreferredNetworkOperators::Info
    \inpublicgroup QtTelephonyModule

    \brief The Info class specifies information about a preferred network operator on the SIM.

    The \c index field indicates
    the position within the list (1 indicates the most-preferred operator).
    The \c format field indicates the 3GPP TS 27.007 format of the operator
    name (0 = long alphabetic, 1 = short alphabetic, 2 = numeric).
    The \c id field indicates the numeric operator code, if known.
    The \c name field indicates the alphabetic operator name, if known.
    The \c technologies field indicates the list of operator technologies
    that are preferred (\c GSM, \c GSMCompact, \c UTRAN, etc).
*/

/*!
    \class QPreferredNetworkOperators::NameInfo
    \inpublicgroup QtTelephonyModule

    \brief The NameInfo class specifies information about an operator that was retrieved from the SIM.

    The \c name field is the name of the operator, and \c id is its numeric identifier.
*/

/*!
    Construct a new preferred network operators object for \a service and
    attach it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports preferred network operators.  If there is more
    than one service that supports preferred network operators, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QPreferredNetworkOperators objects for each.

    \sa QCommServiceManager::supports()
*/
QPreferredNetworkOperators::QPreferredNetworkOperators
            ( const QString& service, QObject *parent,
              QCommInterface::Mode mode )
    : QCommInterface( "QPreferredNetworkOperators", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this preferred operator handling object.
*/
QPreferredNetworkOperators::~QPreferredNetworkOperators()
{
}

/*!
    Resolve numeric identifiers in \a opers according to \a names.
    Any item within \a opers that has a format of 2 (numeric) will
    be resolved and the correct name written to the corresponding
    item in the return list.  If a numeric identifier cannot be
    resolved, then the \c name field will be set to the string form
    of the identifier.

    \sa requestOperatorNames(), requestPreferredOperators()
*/
QList<QPreferredNetworkOperators::Info> QPreferredNetworkOperators::resolveNames
            ( const QList<QPreferredNetworkOperators::Info>& opers,
              const QList<QPreferredNetworkOperators::NameInfo>& names )
{
    QList<QPreferredNetworkOperators::Info> newOpers;
    QList<QPreferredNetworkOperators::Info>::ConstIterator it;
    for ( it = opers.begin(); it != opers.end(); ++it ) {
        if ( (*it).format == 2 ) {
            QPreferredNetworkOperators::Info info = *it;
            QList<QPreferredNetworkOperators::NameInfo>::ConstIterator nit;
            for ( nit = names.begin(); nit != names.end(); ++nit ) {
                if ( (*it).id == (*nit).id ) {
                    info.name = (*nit).name;
                    break;
                }
            }
            if ( info.name.isEmpty() ) {
                info.name = QString::number( (*it).id );
            }
            newOpers += info;
        } else {
            // The operator is already alphabetic, so leave it as-is.
            newOpers += *it;
        }
    }
    return newOpers;
}

/*!
    Request the operator name list from the SIM.  The service will respond
    by emitting the operatorNames() signal.

    \sa operatorNames()
*/
void QPreferredNetworkOperators::requestOperatorNames()
{
    invoke( SLOT(requestOperatorNames()) );
}

/*!
    Request a specific \a list of preferred operators from the SIM.
    The service will respond by emitting the preferredOperators() signal.

    If \a list is QPreferredNetworkOperators::Current, then the current
    preferred operator list on the SIM will be returned.

    \sa preferredOperators()
*/
void QPreferredNetworkOperators::requestPreferredOperators
        ( QPreferredNetworkOperators::List list )
{
    invoke( SLOT(requestPreferredOperators(QPreferredNetworkOperators::List)),
            qVariantFromValue( list ) );
}

/*!
    Write the preferred operator information in \a oper to \a list and
    make \a list the active preferred operator list.  The
    writePreferredOperatorResult() signal will be emitted once
    the request completes.

    \sa writePreferredOperatorResult()
*/
void QPreferredNetworkOperators::writePreferredOperator
        ( QPreferredNetworkOperators::List list,
          const QPreferredNetworkOperators::Info & oper )
{
    invoke( SLOT(writePreferredOperator(QPreferredNetworkOperators::List,QPreferredNetworkOperators::Info)),
            qVariantFromValue( list ),
            qVariantFromValue( oper ) );
}

/*!
    \fn void QPreferredNetworkOperators::operatorNames( const QList<QPreferredNetworkOperators::NameInfo>& names )

    Signal that is emitted in response to a requestOperatorNames() request.
    The \a names parameter will contain the list of all names and numeric
    identifiers.

    \sa requestOperatorNames()
*/

/*!
    \fn void QPreferredNetworkOperators::preferredOperators( QPreferredNetworkOperators::List list, const QList<QPreferredNetworkOperators::Info>& opers )

    Signal that is emitted in response to a requestPreferredOperators()
    request.  The \a list parameter indicates the list that was requested,
    and \a opers will contain the list.

    If the requested list was QPreferredNetworkOperators::Current in the call
    to requestPreferredOperators(), then the \a list parameter to this signal
    will indicate the actual current list.

    It is normally necessary to call resolveNames() to convert \a opers
    into a list suitable for display to a user.

    \sa requestPreferredOperators()
*/

/*!
    \fn void QPreferredNetworkOperators::writePreferredOperatorResult( QTelephony::Result result );

    Signal that is emitted when a writePreferredOperator() request completes,
    indicating whether the request succeeded or failed according to \a result.

    \sa writePreferredOperator()
*/

/*!
    \internal
    \fn void QPreferredNetworkOperators::Info::serialize(Stream &stream) const
*/
template <typename Stream>
        void QPreferredNetworkOperators::Info::serialize(Stream &stream) const
{
    stream << index;
    stream << format;
    stream << id;
    stream << name;
    stream << technologies;
}

/*!
    \internal
    \fn void QPreferredNetworkOperators::Info::deserialize(Stream &stream)
*/
template <typename Stream>
        void QPreferredNetworkOperators::Info::deserialize(Stream &stream)
{
    stream >> index;
    stream >> format;
    stream >> id;
    stream >> name;
    stream >> technologies;
}

/*!
    \internal
    \fn void QPreferredNetworkOperators::NameInfo::serialize(Stream &stream) const
*/
template <typename Stream>
    void QPreferredNetworkOperators::NameInfo::serialize(Stream &stream) const
{
    stream << name;
    stream << id;
}

/*!
    \internal
    \fn void QPreferredNetworkOperators::NameInfo::deserialize(Stream &stream)
*/
template <typename Stream>
    void QPreferredNetworkOperators::NameInfo::deserialize(Stream &stream)
{
    stream >> name;
    stream >> id;
}

Q_IMPLEMENT_USER_METATYPE_ENUM(QPreferredNetworkOperators::List)
Q_IMPLEMENT_USER_METATYPE(QPreferredNetworkOperators::Info)
Q_IMPLEMENT_USER_METATYPE(QPreferredNetworkOperators::NameInfo)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::NameInfo>)
Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<QPreferredNetworkOperators::Info>)
