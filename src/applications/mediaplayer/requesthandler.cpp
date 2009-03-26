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

#include "requesthandler.h"

#include <qtopialog.h>

#include <QtCore>

/*!
    \class RequestHandler
    \inpublicgroup QtMediaModule
    \internal
*/

RequestHandler::RequestHandler( RequestHandler* successor )
    : m_successor( successor )
{
    propagateHead( this );
}

/*!
    \fn RequestHandler::execute( ServiceRequest* request )
    \internal
*/
void RequestHandler::execute( ServiceRequest* request )
{
    // If successor exists, pass request to successor
    // Otherwise, use generic handler
    if( m_successor ) {
        m_successor->execute( request );
    } else {
        switch( request->type() )
        {
        case ServiceRequest::TriggerSlot:
            {
            TriggerSlotRequest *req = (TriggerSlotRequest*)request;
            QMetaObject::invokeMethod( req->receiver(), req->member() );
            }
            break;
        case ServiceRequest::Compound:
            {
            CompoundRequest* req = (CompoundRequest*)request;
            foreach( ServiceRequest* request, req->requests() ) {
                head()->execute( request->clone() );
            }
            break;
            }
        default:
            //REPORT_ERROR( ERR_UNSUPPORTED );
            qLog(Media) << "Service request type" << request->type();
            break;
        }

        delete request;
    }
}

void RequestHandler::propagateHead( RequestHandler* handler )
{
   m_head = handler;

    if( m_successor ) {
        m_successor->propagateHead( handler );
    }
}
