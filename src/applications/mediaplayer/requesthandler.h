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

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "servicerequest.h"
#include <qtopiaglobal.h>

class RequestHandler
{
public:
    RequestHandler( RequestHandler* successor = 0 );
    virtual ~RequestHandler() {};

    virtual void execute( ServiceRequest* request );

protected:
    RequestHandler* head() const { return m_head; }

private:
    void propagateHead( RequestHandler* handler );

    RequestHandler *m_successor;
    RequestHandler *m_head;
};

#endif
