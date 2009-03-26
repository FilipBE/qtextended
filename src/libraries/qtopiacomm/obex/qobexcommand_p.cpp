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
#include "qobexcommand_p.h"
#include <qobexheader.h>

#include <QIODevice>

QObexCommand::QObexCommand(QObex::Request req, const QObexHeader &header,
        const QByteArray &data)
    : m_req(req),
      m_header(header),
      m_setPathFlags(0)
{
    m_isba = true;
    m_data.data = new QByteArray(data);

    m_id = nextId();
}

QObexCommand::QObexCommand(QObex::Request req, const QObexHeader &header,
                           QIODevice *device)
    : m_req(req),
      m_header(header),
      m_setPathFlags(0)
{
    m_isba = false;
    m_data.device = device;

    m_id = nextId();
}

QObexCommand::~QObexCommand()
{
    if (m_isba)
        delete m_data.data;
}

QAtomicInt QObexCommand::idCounter(1);

int QObexCommand::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSetOrdered(id, id + 1))
            break;
    }
    return id;
}
