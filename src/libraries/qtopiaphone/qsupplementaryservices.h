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

#ifndef QSUPPLEMENTARYSERVICES_H
#define QSUPPLEMENTARYSERVICES_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QSupplementaryServices : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(OutgoingNotification IncomingNotification UnstructuredAction)
public:
    explicit QSupplementaryServices( const QString& service = QString(),
                                     QObject *parent = 0,
                                     QCommInterface::Mode mode = Client );
    ~QSupplementaryServices();

    enum OutgoingNotification
    {
        MO_UnconditionalForwardingActive,
        MO_ConditionalForwardingActive,
        MO_Forwarded,
        MO_Waiting,
        MO_ClosedUserGroup,
        MO_OutgoingCallsBarred,
        MO_IncomingCallsBarred,
        MO_CallerIdSuppressionRejected,
        MO_Deflected
    };

    enum IncomingNotification
    {
        MT_Forwarded,
        MT_ClosedUserGroup,
        MT_Hold,
        MT_Retrieved,
        MT_MultipartyEntered,
        MT_HoldReleased,
        MT_ForwardCheck,
        MT_Alerting,
        MT_ExplicitTransfer,
        MT_Deflected,
        MT_AdditionalIncomingForwarded
    };

    enum UnstructuredAction
    {
        NoFurtherActionRequired,
        FurtherActionRequired,
        TerminatedByNetwork,
        OtherLocalClientResponded,
        OperationNotSupported,
        NetworkTimeout
    };

public slots:
    virtual void cancelUnstructuredSession();
    virtual void sendUnstructuredData( const QString& data );
    virtual void sendSupplementaryServiceData( const QString& data );

signals:
    void outgoingNotification
        ( QSupplementaryServices::OutgoingNotification type,
          int groupIndex );
    void incomingNotification
        ( QSupplementaryServices::IncomingNotification type,
          int groupIndex, const QString& number );
    void unstructuredNotification
        ( QSupplementaryServices::UnstructuredAction action,
          const QString& data );
    void unstructuredResult( QTelephony::Result result );
    void supplementaryServiceResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QSupplementaryServices::OutgoingNotification)
Q_DECLARE_USER_METATYPE_ENUM(QSupplementaryServices::IncomingNotification)
Q_DECLARE_USER_METATYPE_ENUM(QSupplementaryServices::UnstructuredAction)

#endif
