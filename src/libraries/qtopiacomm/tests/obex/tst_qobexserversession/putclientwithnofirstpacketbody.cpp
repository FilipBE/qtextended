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

//QTEST_SKIP_TEST_DOC

#include "putclientwithnofirstpacketbody.h"

#include <QObexHeader>
#include "qobexheader_p.h"
#include "qobexsocket_p.h"
#include <QObject>

#include <openobex/obex.h>

#include <QIODevice>
#include <QTest>
#include <shared/qtopiaunittest.h>

/*
    This class creates an OBEX client that will send a Put request that
    does not contain any body data in the first request packet. This helps
    to test that an OBEX server implementation can distinguish between

        1) a Put request without body data in the first packet, and
        2) a Put-Delete request (which has no body data at all).

    When a Sony Ericsson P900 sends a Put request, it does not send any
    body data in the first request packet.

    This duplicates a lot of code from QObexClient since it's just like an
    ordinary client but sends data in a slightly different way.
*/

static int BUF_SIZE = 1024;


// 248 bytes is just enough to fill up a packet so that OpenOBEX will not 
// send us an OBEX_EV_STREAMEMPTY (because we don't want to send any body 
// data in the first packet). You can't put in too much either, otherwise 
// it won't fit in one packet.
const int PutClientWithNoFirstPacketBody::MAX_HEADER_SIZE = 248;

static void openobex_callback(obex_t *handle, obex_object_t *obj, int mode,
                              int event, int obex_cmd, int obex_rsp)
{
    PutClientWithNoFirstPacketBody *conn =
            static_cast<PutClientWithNoFirstPacketBody *>(OBEX_GetUserData(handle));
    conn->connectionEvent(obj, mode, event, obex_cmd, obex_rsp);
}


PutClientWithNoFirstPacketBody::PutClientWithNoFirstPacketBody(QIODevice *device, QObject *parent)
    : QObject(parent),
      m_socket(new QObexSocket(device, this)),
      m_buf(0)
{
    m_handle = static_cast<obex_t *>(m_socket->handle());
    Q_ASSERT(m_handle != 0);

    OBEX_SetUserCallBack(m_handle, openobex_callback, 0);
    OBEX_SetUserData(m_handle, this);
}

PutClientWithNoFirstPacketBody::~PutClientWithNoFirstPacketBody()
{
    m_socket->resetObexHandle();

    if (m_buf) {
        delete[] m_buf;
        m_buf = 0;
    }
}

void PutClientWithNoFirstPacketBody::sendPutRequest(const QObexHeader &header, QIODevice *device)
{
    m_device = device;
    m_packetsSent = 0;

    bool b;
    int r;

    if (!device->isOpen()) {
        b = device->open(QIODevice::ReadOnly);
        Q_ASSERT(b);
    }

    if (m_buf)
        delete[] m_buf;
    m_buf = new char[1024];

    obex_object_t *obj = OBEX_ObjectNew(m_handle, OBEX_CMD_PUT);
    obex_headerdata_t hv;

    b = QObexHeaderPrivate::writeOpenObexHeaders(m_handle, obj, true, header);
    Q_ASSERT(b);

    // Don't send any data yet, just indicate we want to stream, so
    // we get the OBEX_EV_STREAMEMPTY event.
    r = OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, 0,
                         OBEX_FL_STREAM_START);
    Q_ASSERT(r >= 0);

    r = OBEX_Request(m_handle, obj);
    Q_ASSERT(r >= 0);
}

void PutClientWithNoFirstPacketBody::connectionEvent(obex_object_t *obj, int /*mode*/, int event, int /*obex_cmd*/, int obex_rsp)
{
    switch (event) {
        case OBEX_EV_PROGRESS:
            qLog(Autotest) << "PutClientWithNoFirstPacketBody: OBEX_EV_PROGRESS";
            m_packetsSent++;
            break;
        case OBEX_EV_REQDONE:
            qLog(Autotest) << "PutClientWithNoFirstPacketBody: OBEX_EV_REQDONE";
            if (obex_rsp == OBEX_RSP_SUCCESS) {
                m_packetsSent++;
                emit done(false);
            } else {
                emit done(true);
            }
            break;
        case OBEX_EV_LINKERR:
            qLog(Autotest) << "PutClientWithNoFirstPacketBody: OBEX_EV_LINKERR";
            emit done(true);
            break;
        case OBEX_EV_STREAMEMPTY:
        {
            qLog(Autotest) << "PutClientWithNoFirstPacketBody: OBEX_EV_STREAMEMPTY";
            obex_headerdata_t hv;
            int r;

            int len = m_device->read(m_buf, BUF_SIZE);
            qLog(Autotest) << "PutClientWithNoFirstPacketBody: read" << len;
            Q_ASSERT(len >= 0);
            if (len > 0) {
                hv.bs = reinterpret_cast<const uint8_t*>(m_buf);
                r = OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, len,
                                        OBEX_FL_STREAM_DATA);
            } else {
                hv.bs = 0;
                r = OBEX_ObjectAddHeader(m_handle, obj, OBEX_HDR_BODY, hv, 0,
                                        OBEX_FL_STREAM_DATAEND);
            }
            Q_ASSERT(r >= 0);
            break;
        }
        default:
            qLog(Autotest) << "PutClientWithNoFirstPacketBody not handling event" << event;
            break;
    }
}
