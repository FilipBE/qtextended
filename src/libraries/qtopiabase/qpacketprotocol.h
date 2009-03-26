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

#ifndef QPACKETPROTOCOL_H
#define QPACKETPROTOCOL_H

#include <QObject>
#include <QDataStream>

#include "qtopiailglobal.h"

class QIODevice;
class QBuffer;
class QPacket;
class QPacketAutoSend;
class QPacketProtocolPrivate;

class QTOPIAIL_EXPORT QPacketProtocol : public QObject
{
Q_OBJECT
public:
    explicit QPacketProtocol(QIODevice * dev, QObject * parent = 0);
    virtual ~QPacketProtocol();

    qint32 maximumPacketSize() const;
    qint32 setMaximumPacketSize(qint32);

    QPacketAutoSend send();
    void send(const QPacket &);

    qint64 packetsAvailable() const;
    QPacket read();

    void clear();

    QIODevice * device();

signals:
    void readyRead();
    void invalidPacket();
    void packetWritten();

private:
    QPacketProtocolPrivate * d;
};


class QTOPIAIL_EXPORT QPacket : public QDataStream
{
public:
    QPacket();
    QPacket(const QPacket &);
    virtual ~QPacket();

    void clear();
    bool isEmpty() const;

protected:
    friend class QPacketProtocol;
    QPacket(const QByteArray & ba);
    QByteArray b;
    QBuffer * buf;
};

class QTOPIAIL_EXPORT QPacketAutoSend : public QPacket
{
public:
    virtual ~QPacketAutoSend();

private:
    friend class QPacketProtocol;
    QPacketAutoSend(QPacketProtocol *);
    QPacketProtocol * p;
};

#endif
