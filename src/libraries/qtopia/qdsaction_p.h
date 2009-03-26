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

#ifndef QDSACTION_P_H
#define QDSACTION_P_H

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
#include "qdsaction.h"
#include "qdsserviceinfo.h"
#include "qdsdata.h"

// Qt includes
#include <QObject>
#include <QUuid>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSData;
class QTimer;
class QUuid;
class QEventLoop;
class QtopiaIpcAdaptor;

// ============================================================================
//
//  QDSActionPrivate
//
// ============================================================================

class QTOPIA_AUTOTEST_EXPORT QDSActionPrivate : public QObject
{
    Q_OBJECT

public:
    QDSActionPrivate();
    QDSActionPrivate( const QDSActionPrivate& other );
    QDSActionPrivate( const QString& name,
                      const QString& service );
    QDSActionPrivate( const QDSServiceInfo& serviceInfo );
    ~QDSActionPrivate();

    // Methods
    bool requestActive();
    void emitRequest();
    void emitRequest( const QDSData& data, const QByteArray& auxiliary );
    void reset();
    void connectToAction( QDSAction* action );

    // Data members
    QUniqueId      mId;
    QDSServiceInfo      mServiceInfo;
    QtopiaIpcAdaptor*   mResponseChannel;
    QTimer*             mTimer;
    QEventLoop*         mEventLoop;
    QDSData             mResponseData;
    QString             mErrorMsg;
    int                 mResponseCode;

    static QUniqueIdGenerator mIdGen;

private slots:
    void heartbeatSlot();
    void responseSlot();
    void responseSlot( const QDSData& responseData );
    void requestTimeoutSlot();
    void errorSlot( const QString& message );

signals:
    void response( const QUniqueId& actionId );
    void response( const QUniqueId& actionId, const QDSData& responseData );
    void error( const QUniqueId& actionId, const QString& message );

private:
    void connectToChannel();
    void disconnectFromChannel();
    void startTimer();

    QString responseChannel();
};

#endif
