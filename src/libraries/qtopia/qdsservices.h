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

#ifndef QDSSERVICES_H
#define QDSSERVICES_H

// Local includes
#include "qdsserviceinfo.h"

// Qt includes
#include <QList>

// Qtopia includes
#include <qtopianamespace.h>

// ============================================================================
//
//  Forward class declarations
//
// ============================================================================

class QMimeType;
class QString;

// ============================================================================
//
//  QDSServices
//
// ============================================================================

class QTOPIA_EXPORT QDSServices : public QList<QDSServiceInfo>
{
public:
    explicit QDSServices( const QString& requestDataType = QString( "*" ),
                          const QString& responseDataType = QString( "*" ),
                          const QStringList& attributes = QStringList(),
                          const QString& service = QString( "*" ) );

    QDSServiceInfo findFirst( const QString& name );

private:
    enum Mode { Request, Response };

    void processQdsServiceFile( const QString& service,
                                const QString& requestDataTypeFilter,
                                const QString& responseDataTypeFilter,
                                const QStringList& attributesFilter );
    bool passTypeFilter( const QDSServiceInfo& serviceInfo,
                         const QString& typeFilter,
                         const Mode mode );
    bool passAttributesFilter( const QDSServiceInfo& serviceInfo,
                               const QStringList& attributesFilter );

};

#endif
