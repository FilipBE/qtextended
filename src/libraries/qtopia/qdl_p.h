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

#ifndef QDL_P_H
#define QDL_P_H

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

// Forward class declarations
class QDLLink;
class QString;

// ============================================================================
//
// QDLPrivate
//
// ============================================================================

class QDLPrivate
{
public:
    static QString linkAnchorText( const QString& clientName,
                                   const int linkId,
                                   const QDLLink& link,
                                   const bool noIcon = false );

    static int indexOfQDLAnchor( const QString& text,
                                 const int startPos,
                                 QString& anchor );

    static int indexOfQDLAnchor( const QString& text,
                                 const QString& clientName,
                                 const int linkId,
                                 const int startPos,
                                 QString& anchor );

    static QString anchorToHref( const QString& anchor );

    static bool decodeHref( const QString& href,
                            QString& clientName,
                            int& linkId );
};

#endif
