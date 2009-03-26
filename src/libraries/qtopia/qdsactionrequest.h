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

#ifndef QDSACTIONREQUEST_H
#define QDSACTIONREQUEST_H

// Qt includes
#include <QObject>

// Qtopia includes
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSActionRequestPrivate;
class QDSServiceInfo;
class QDSData;

// ============================================================================
//
//  QDSActionRequest
//
// ============================================================================

class QTOPIA_EXPORT QDSActionRequest : public QObject
{
    Q_OBJECT

public:

    explicit QDSActionRequest( QObject* parent = 0 );
    QDSActionRequest( const QDSActionRequest& other );

    QDSActionRequest( const QDSServiceInfo& serviceInfo,
                      const QString& channel,
                      QObject* parent = 0 );

    QDSActionRequest( const QDSServiceInfo& serviceInfo,
                      const QDSData& requestData,
                      const QString& channel,
                      const QByteArray& auxiliary = QByteArray(),
                      QObject* parent = 0 );

    ~QDSActionRequest();

    const QDSActionRequest& operator=( const QDSActionRequest& other );

    const QDSServiceInfo& serviceInfo() const;
    bool isValid() const;
    bool isComplete() const;

    const QDSData& requestData() const;
    const QDSData& responseData() const;
    const QByteArray& auxiliaryData() const;
    QString errorMessage() const;

    bool respond();
    bool respond( const QDSData& responseData );
    bool respond( const QString& error );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDSActionRequestPrivate* d;
};

Q_DECLARE_USER_METATYPE( QDSActionRequest );

#endif
