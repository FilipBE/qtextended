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

#ifndef QDSSERVICEINFO_H
#define QDSSERVICEINFO_H

// Qtopia includes
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QMimeType>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QDSServiceInfoPrivate;
class QDSServices;
class QString;

// ============================================================================
//
//  QDSServiceInfo
//
// ============================================================================

class QTOPIA_EXPORT QDSServiceInfo
{
    friend class QDSServices;

public:
    QDSServiceInfo();
    QDSServiceInfo( const QDSServiceInfo& other );
    QDSServiceInfo( const QString& name,
                    const QString& service );

    ~QDSServiceInfo();

    // Operators
    const QDSServiceInfo& operator=( const QDSServiceInfo& other );
    bool operator==( const QDSServiceInfo& other ) const;
    bool operator!=( const QDSServiceInfo& other ) const;

    // Access
    bool isValid() const;
    bool isAvailable() const;
    QString serviceId() const;
    QString serviceName() const;
    QString name() const;
    QStringList requestDataTypes() const;
    bool supportsRequestDataType(
        const QMimeType& type = QMimeType( QString() ) ) const;
    QStringList responseDataTypes() const;
    bool supportsResponseDataType(
        const QMimeType& type = QMimeType( QString() ) ) const;
    QStringList attributes() const;
    QString description() const;
    QString icon() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
private:

    bool supportsRequestDataTypeOrWild( const QString& type = QString() ) const;
    bool supportsResponseDataTypeOrWild( const QString& type = QString() ) const;

    QDSServiceInfoPrivate* d;
};

Q_DECLARE_USER_METATYPE( QDSServiceInfo );

#endif
