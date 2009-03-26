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

#ifndef BSCIDRMAGENTSERVICE_H
#define BSCIDRMAGENTSERVICE_H

#include <qdrmcontentplugin.h>
#include <QtopiaAbstractService>
#include <stdlib.h>
#include <bsci.h>
#include <qtopiaglobal.h>
#include <QQueue>
#include <QPair>

class QDSActionRequest;
class BSciDrmAgentServicePrivate;

class BSciDrmAgentService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    BSciDrmAgentService( QObject *parent = 0 );
    virtual ~BSciDrmAgentService();

public slots:
    void handleProtectedRightsObject( const QDSActionRequest &request );
    void handleXmlRightsObject( const QDSActionRequest &request );
    void handleWbXmlRightsObject( const QDSActionRequest &request );
    void handleRoapTrigger( const QDSActionRequest &request );
    void handleRoapPdu( const QDSActionRequest &request );

    void convertMessage( const QDSActionRequest &request );;
private:
    void handleXmlRightsObject( const QByteArray &object );
    void handleWbXmlRightsObject( const QByteArray &object );
    void handleRoapTrigger( const QByteArray &trigger );
    void handleProtectedRightsObject( const QByteArray &object );
    void handleRoapPdu( const QByteArray &pdu );

    QByteArray convertMessage( const QByteArray &message );
};


#endif
