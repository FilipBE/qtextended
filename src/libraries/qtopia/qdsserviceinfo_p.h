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

#ifndef QDSSERVICEINFO_P_H
#define QDSSERVICEINFO_P_H

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

// Qt includes
#include <QString>
#include <QStringList>

// ============================================================================
//
//  QDSServiceInfoPrivate
//
// ============================================================================

class QDSServiceInfoPrivate
{
public:
    QDSServiceInfoPrivate();
    QDSServiceInfoPrivate( const QDSServiceInfoPrivate& other );
    QDSServiceInfoPrivate( const QString& name,
                           const QString& service );

    void processSettings();
    bool supportsDataType( const QStringList& supported, const QString& type );
    bool correctQtopiaServiceDescription();

    // Data members
    QString             mService;
    QString             mName;
    QString             mUiName;
    QStringList         mRequestDataTypes;
    QStringList         mResponseDataTypes;
    QStringList         mAttributes;
    QStringList         mDepends;
    QString             mDescription;
    QString             mIcon;
    bool                mProcessed;
    bool                mValid;
};

#endif
