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

#ifndef QDL_H
#define QDL_H

// Qtopia includes
#include <qtopianamespace.h>

// Qt includes
#include <QList>

// Forward class declarations
class QDataStream;
class QDLClient;
class QObject;

// ============================================================================
//
// functions
//
// ============================================================================

QTOPIA_EXPORT QDataStream& operator>>( QDataStream& stream,
                                       QList<QDLClient *>& clientList );

QTOPIA_EXPORT QDataStream& operator<<( QDataStream& stream,
                                       const QList<QDLClient *>& clientList );

// ============================================================================
//
// QDL
//
// ============================================================================

class QTOPIA_EXPORT QDL
{
public:
    static QList<QDLClient *> clients( QObject *parent );

    static void saveLinks( QString &str, QList<QDLClient *> clientList );
    static void loadLinks( const QString &str, QList<QDLClient *> clientList );
    static void releaseLinks( const QString& str );

    static void activateLink( const QString &href,
                              const QList<QDLClient *> &clientList );

    static const QString ANCHOR_HREF_PREFIX;

    static const QString CLIENT_DATA_KEY;
    static const QString SOURCE_DATA_KEY;
};

#endif
