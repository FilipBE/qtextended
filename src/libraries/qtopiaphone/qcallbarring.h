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

#ifndef QCALLBARRING_H
#define QCALLBARRING_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallBarring : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(BarringType)
public:
    explicit QCallBarring( const QString& service = QString::null,
                           QObject *parent = 0,
                           QCommInterface::Mode mode = Client );
    ~QCallBarring();

    enum BarringType
    {
        OutgoingAll,
        OutgoingInternational,
        OutgoingInternationalExceptHome,
        IncomingAll,
        IncomingWhenRoaming,
        IncomingNonTA,
        IncomingNonMT,
        IncomingNonSIM,
        IncomingNonMemory,
        AllBarringServices,
        AllOutgoingBarringServices,
        AllIncomingBarringServices
    };

public slots:
    virtual void requestBarringStatus( QCallBarring::BarringType type );
    virtual void setBarringStatus( QCallBarring::BarringType type,
                                   const QString& password,
                                   QTelephony::CallClass cls,
                                   bool lock );
    virtual void unlockAll( const QString& password );
    virtual void unlockAllIncoming( const QString& password );
    virtual void unlockAllOutgoing( const QString& password );
    virtual void changeBarringPassword( QCallBarring::BarringType type,
                                        const QString& oldPassword,
                                        const QString& newPassword );

signals:
    void barringStatus( QCallBarring::BarringType type,
                        QTelephony::CallClass cls );
    void setBarringStatusResult( QTelephony::Result result );
    void unlockResult( QTelephony::Result result );
    void changeBarringPasswordResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallBarring::BarringType)

#endif
