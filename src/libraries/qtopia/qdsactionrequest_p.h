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

#ifndef QDSACTIONREQUEST_P_H
#define QDSACTIONREQUEST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Local includes
#include "qdsserviceinfo.h"
#include "qdsdata.h"

// Qt includes
#include <QObject>

// ============================================================================
//
//  QDSHeartBeat
//
// ============================================================================

class QDSHeartBeat : public QObject
{
    Q_OBJECT
public:
    explicit QDSHeartBeat( QObject* parent = 0 );
    explicit QDSHeartBeat( const QString& channel, QObject* parent = 0 );
    QDSHeartBeat( const QDSHeartBeat& other );

    const QDSHeartBeat& operator=( const QDSHeartBeat& other );

private slots:
    void beat();

private:
    QString mChannel;
    QTimer* mTimer;
};

// ============================================================================
//
//  QDSActionRequest
//
// ============================================================================

class QDSActionRequestPrivate : public QObject
{
    Q_OBJECT

public:

    QDSActionRequestPrivate();
    QDSActionRequestPrivate( const QDSActionRequestPrivate& other );
    QDSActionRequestPrivate( const QDSServiceInfo& serviceInfo,
                             const QDSData& requestData,
                             const QByteArray& auxiliary,
                             const QString& channel );
    ~QDSActionRequestPrivate();

    QDSServiceInfo mServiceInfo;
    QDSData        mRequestData;
    QDSData        mResponseData;
    QByteArray     mAuxData;
    QString        mChannel;
    bool           mComplete;
    QString        mErrorMessage;

    QDSHeartBeat mHeartBeat;

    void emitResponse();
    void emitError( const QString& error );
};

#endif
