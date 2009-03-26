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

#ifndef QDSACTION_H
#define QDSACTION_H

// Local includes
#include "qdsdata.h"

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>
#include <QIcon>

// Qtopia includes
#include <qtopianamespace.h>
#include <QUniqueId>
#include <qtopiaglobal.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSActionPrivate;
class QDSServiceInfo;
class QDataStream;

// ============================================================================
//
//  QDSAction
//
// ============================================================================

class QTOPIA_EXPORT QDSAction : public QObject
{
    Q_OBJECT

public:
    explicit QDSAction( QObject* parent = 0 );
    QDSAction( const QDSAction& other );
    QDSAction( const QString& name,
               const QString& service,
               QObject* parent = 0 );
    explicit QDSAction( const QDSServiceInfo& serviceInfo,
                        QObject* parent = 0 );

    ~QDSAction();

    const QDSAction& operator=( const QDSAction& other );

    // Enumerations
    enum ResponseCode { Invalid, Complete, CompleteData, Error };

    // Access methods
    bool isValid() const;
    bool isAvailable() const;
    bool isActive() const;

    QUniqueId id() const;
    const QDSServiceInfo& serviceInfo() const;
    QDSData responseData() const;
    QString errorMessage() const;

    // Action invocation
    bool invoke();
    bool invoke( const QDSData &requestData,
                 const QByteArray& auxiliary = QByteArray() );
    int exec();
    int exec( const QDSData &requestData,
              const QByteArray& auxiliary = QByteArray() );

signals:
    void response( const QUniqueId& actionId );
    void response( const QUniqueId& actionId, const QDSData& responseData );
    void error( const QUniqueId& actionId, const QString& message );

private:
    QDSActionPrivate* d;
};

#endif
