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

#ifndef PUTCLIENTWITHNOFIRSTPACKETBODY_H
#define PUTCLIENTWITHNOFIRSTPACKETBODY_H

#include <QObject>

class QIODevice;
class QObexSocket;
class QObexHeader;
typedef void* obex_t;
typedef void* obex_object_t;

class PutClientWithNoFirstPacketBody : public QObject
{
    Q_OBJECT
public:
    PutClientWithNoFirstPacketBody(QIODevice *device, QObject *parent = 0);
    ~PutClientWithNoFirstPacketBody();

    void sendPutRequest(const QObexHeader &header, QIODevice *device);

    void connectionEvent(obex_object_t *obj, int mode, int event,
                         int obex_cmd, int obex_rsp);

    inline int sentPacketsCount() { return m_packetsSent; }

    static const int MAX_HEADER_SIZE;

signals:
    void done(bool error);

private:
    QObexSocket *m_socket;
    obex_t *m_handle;
    QIODevice *m_device;
    char *m_buf;
    int m_packetsSent;
};

#endif
