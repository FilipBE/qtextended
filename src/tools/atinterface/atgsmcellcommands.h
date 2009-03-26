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

#ifndef ATGSMCELLCOMMANDS_H
#define ATGSMCELLCOMMANDS_H

#include <QObject>

#include <QPreferredNetworkOperators>
#include <QCallForwarding>
#include <QCallSettings>
#include <QPhoneBook>
#include <QPhoneRfFunctionality>
#include <QPhoneBook>
#include <QPair>
#include <QCallBarring>
#include <QSupplementaryServices>
#include <QContact>

struct ContactRecord
{
public:
    QUniqueId id;
    QContact::PhoneType type;
};

class AtCommands;
class QAdviceOfCharge;
class QContactModel;
class QSimGenericAccess;
class QPinManager;

class AtGsmCellCommands : public QObject
{
    Q_OBJECT

public:
    AtGsmCellCommands(AtCommands * parent);
    ~AtGsmCellCommands();

    QPhoneRfFunctionality *phonerf;
    bool settingPhoneRf;

public slots:
    void atcacm( const QString& params );
    void atcamm( const QString& params );
    void atcaoc( const QString& params );
    void atccfc( const QString& params );
    void atccwe( const QString& params );
    void atclck( const QString& params );
    void atclir( const QString& params );
    void atcolp( const QString& params );
    void atcopn( const QString& params );
    void atcpbf( const QString& params );
    void atcpbr( const QString& params );
    void atcpbs( const QString& params );
    void atcpbw( const QString& params );
    void atcpls( const QString& params );
    void atcpol( const QString& params );
    void atcpuc( const QString& params );
    void atcpwd( const QString& params );
    void atcsim( const QString& params );
    void atcsmp( const QString& params );
    void atcsms( const QString& params );
    void atcusd( const QString& params );

private:
    void writeMemoryPhoneBookEntry( bool isDeletion, uint index, const QString& number = QString(), const QString& text = QString() );

private slots:
    void incomingSupplementaryServicesNotification( 
            QSupplementaryServices::IncomingNotification type,
            int groupIndex, const QString & number );
    void outgoingSupplementaryServicesNotification( 
            QSupplementaryServices::OutgoingNotification type, int groupIndex );
    void unstructuredSupplementaryServicesNotification(
            QSupplementaryServices::UnstructuredAction action, const QString & data );
    void unstructuredSupplementaryServicesResult( QTelephony::Result result );
    void setPinLockStatusResult( const QString& type, bool valid );
    void pinLockStatus( const QString& type, bool enable );
    void changePinResult( const QString& type, bool valid );
    void setBarringStatusResult( QTelephony::Result result );
    void barringStatus( QCallBarring::BarringType type, QTelephony::CallClass cls );
    void changeBarringPasswordResult( QTelephony::Result result );
    void callerIdRestriction( QCallSettings::CallerIdRestriction clir,
            QCallSettings::CallerIdRestrictionStatus status );
    void setCallerIdRestrictionResult( QTelephony::Result );
    void currentCallMeter( int value, bool explicitRequest );
    void accumulatedCallMeter( int value );
    void resetAccumulatedCallMeterResult( QTelephony::Result result );
    void accumulatedCallMeterMaximum( int value );
    void setAccumulatedCallMeterMaximumResult( QTelephony::Result result );
    void pricePerUnit( const QString& currency, const QString& unitPrice );
    void setPricePerUnitResult( QTelephony::Result result );
    void callMeterMaximumWarning();
    void operatorNames(
            const QList<QPreferredNetworkOperators::NameInfo>& names );
    void preferredOperators(
            QPreferredNetworkOperators::List list, 
            const QList<QPreferredNetworkOperators::Info>& opers );
    void writePreferredOperatorResult( QTelephony::Result result );
    void setForwardingResult( QCallForwarding::Reason, QTelephony::Result );
    void forwardingStatus( QCallForwarding::Reason, const QList<QCallForwarding::Status>& );
    void connectedIdPresentation( QCallSettings::PresentationStatus status );
    void simGenericAccessResponse( const QString & reqid, 
            QTelephony::Result result, const QByteArray & data );
    void setLevelResult( QTelephony::Result result );
    void phoneBookLimits( const QString& store, const QPhoneBookLimits& limits );
    void phoneBookEntries( const QString& store, const QList<QPhoneBookEntry>& entries );
    void initializeMemoryPhoneBook();

private:

    AtCommands *atc;

    // supplementary services
    QSupplementaryServices *supplementaryServices;
    bool sendingUnstructuredServiceData;
    QCallForwarding *callForwarding;
    int settingCallForwardingReason;
    int requestingCallForwardingStatusReason;
    QPinManager *pinManager;
    QString settingPinLockType;
    QString requestingPinLockType;
    QString changingPinType;
    QCallBarring *callBarring;
    bool settingBarringStatus;
    QCallBarring::BarringType requestingBarringStatusType;
    bool settingBarringPassword;
    QCallSettings *callSettings;
    bool requestingCallWaiting;
    bool settingCallWaiting;
    bool requestingConnectedIdPresentation;
    bool requestingCallerIdRestriction;
    bool settingCallerIdRestriction;
    QAdviceOfCharge *adviceOfCharge;
    bool requestingCurrentCallMeter;
    bool requestingAccumulatedCallMeter;
    bool resettingAccumulatedCallMeter;
    bool requestingAccumulatedCallMeterMaximum;
    bool settingAccumulatedCallMeterMaximum;
    bool requestingPricePerUnit;
    bool settingPricePerUnit;

    // operator list related
    QPreferredNetworkOperators *prefNetOps;
    bool requestingOperatorNames;
    int requestingPreferredOperatorsFromList;
    bool testingPreferredOperators;
    bool settingPreferredOperator;

    // sim access related
    QSimGenericAccess *sga;
    QString simRequestId;

    // phone book related
    QPhoneBook* phoneBook;
    bool settingFixedDialingState;
    bool requestingFixedDialingState;
    QPair<uint,uint> phoneBookIndex;
    QPhoneBookEntry entryToWrite;
    bool phoneBookQueried;
    QByteArray limitsReqBy;
    QContactModel* contactModel;
    QHash<int, ContactRecord> contactIndices;
    int availableMemoryContacts;
    QString pbSearchText;

};

#endif
