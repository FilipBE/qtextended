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
#ifndef BSCIDRM_H
#define BSCIDRM_H

#include <stdlib.h>
#include <stdio.h>
#include "bsci.h"
#include "bsciMMI.h"
#include "bsciLibMgmt.h"
#include "bsciLicMgmt.h"
#include "bsciContentAccess.h"
#include "bsciContentMgmt.h"
#include <qdrmrights.h>


class BSciDrm
{
public:
    static void initialiseAgent( const QString &id, SBSciCallbacks *callbacks );

    static void releaseAgent();

    static EPermission transformPermission( QDrmRights::Permission permissionType );

    static QString formatInterval( const SBSciDuration &duration );

    static QDrmRights constraints( QDrmRights::Permission permission, ERightsStatus status, SBSciConstraints *constraints );

    static bool isStateful( const SBSciConstraints &constraints );

    static const char *getError( int error );

    static void printError( int error, const QString &method, const QString &filePath );

    static void printError( int error, const QString &method );

    static bool hasRights( const QString &dcfFilePath, QDrmRights::Permission permission );

    static QString getMetaData( const QString &filePath, enum EMetaData item );

    static qint64 getContentSize( const QString &dcfFilePath );

    static QString getPreviewUri( const QString &dcfFilePath );

    static QStringList convertTextArray( const SBSciTextArray &text );

    static const QByteArray formatPath( const QString &filePath );

    static void *context;
};

#endif
