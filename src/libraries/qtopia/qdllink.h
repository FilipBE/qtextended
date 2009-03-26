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

#ifndef QDLLINK_H
#define QDLLINK_H

// Qtopia includes
#include <qtopianamespace.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QMimeType>

// Forward class declarations
class QDLLinkPrivate;
class QDSData;

// ============================================================================
//
// QDLLink
//
// ============================================================================

class QTOPIA_EXPORT QDLLink
{
public:
    QDLLink();
    QDLLink( const QString &service,
             const QByteArray &data,
             const QString &description,
             const QString &icon );
    QDLLink( const QDLLink &other );
    explicit QDLLink( const QDSData& dataObject );

    ~QDLLink();

    // Operators
    QDLLink &operator=( const QDLLink &other );

    // Access
    static QMimeType mimeType();
    static QMimeType listMimeType();
    bool isNull() const;
    bool isBroken() const;

    QString service() const;
    QByteArray data() const;
    QString description() const;
    QString icon() const;
    QDSData toQDSData() const;

    void activate() const;

    // Modification
    void setService( const QString &service );
    void setData( const QByteArray &data );
    void setDescription( const QString &description );
    void setIcon( const QString &icon );
    void setBroken( const bool broken = true );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDLLinkPrivate* d;
};

// Macros
Q_DECLARE_USER_METATYPE(QDLLink)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QDLLink>)

#endif
