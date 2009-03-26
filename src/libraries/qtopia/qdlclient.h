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

#ifndef QDLCLIENT_H
#define QDLCLIENT_H

// Local includes
#include "qdllink.h"

// Qtopia includes
#include <qtopianamespace.h>

// Qt includes
#include <QObject>
#include <QMap>
#include <QByteArray>

// Forward class declarations
class QDLClientPrivate;
class QDSData;
class QDSServiceInfo;

// ============================================================================
//
// QDLClient
//
// ============================================================================

class QTOPIA_EXPORT QDLClient : public QObject
{
    Q_OBJECT

public:
    QDLClient( QObject *parent, const QString& name );
    virtual ~QDLClient();

    // Access
    void saveLinks( QDataStream& stream ) const;
    QDLLink link( const int linkId ) const;
    QList<int> linkIds() const;
    QString hint() const;
    QString linkAnchorText( const int linkId, const bool noIcon = false ) const;
    bool validLinkId( const int linkId ) const;

    // Modification
    void setHint( const QString& hint );
    virtual void loadLinks( QDataStream& stream );
    virtual int addLink( QDSData& link );
    virtual void setLink( const int linkId, const QDLLink& link );
    virtual void removeLink( const int linkId );
    void breakLink( const int linkId, const bool broken = true );

public slots:
    void clear();
    void requestLinks( QWidget* parent );
    void requestLinks( const QDSServiceInfo& qdlService );
    void activateLink( const int linkId );
    virtual void verifyLinks();

private:
    QDLClientPrivate* d;
};

#endif
