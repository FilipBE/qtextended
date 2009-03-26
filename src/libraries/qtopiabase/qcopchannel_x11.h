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

#ifndef QCOPCHANNEL_X11_H
#define QCOPCHANNEL_X11_H

#include <qobject.h>
#include <qtopiaglobal.h>

#if defined(Q_WS_X11)

class QCopChannelPrivate;
class QCopX11Client;
class QCopX11Server;

class QTOPIABASE_EXPORT QCopChannel : public QObject
{
    Q_OBJECT
public:
    explicit QCopChannel(const QString& channel, QObject *parent=0);
    virtual ~QCopChannel();

    QString channel() const;

    static bool isRegistered(const QString&  channel);
    static bool send(const QString& channel, const QString& msg);
    static bool send(const QString& channel, const QString& msg,
                      const QByteArray &data);

    static bool flush();

    static void sendLocally( const QString& ch, const QString& msg,
                               const QByteArray &data);
    static void reregisterAll();

    virtual void receive(const QString& msg, const QByteArray &data);

Q_SIGNALS:
    void received(const QString& msg, const QByteArray &data);

private:
    void init(const QString& channel);

    // server side
    static void registerChannel(const QString& ch, QCopX11Client *cl);
    static void requestRegistered(const QString& ch, QCopX11Client *cl);
    static void detach(QCopX11Client *cl);
    static void detach(const QString& ch, QCopX11Client *cl);
    static void answer(QCopX11Client *cl, const QString& ch,
                        const QString& msg, const QByteArray &data);
    // client side
    QCopChannelPrivate* d;

    friend class QCopX11Server;
    friend class QCopX11Client;
};

class QTOPIABASE_EXPORT QCopServer : public QObject
{
    Q_OBJECT
public:
    QCopServer(QObject *parent);
    ~QCopServer();

    static QCopServer *instance();

signals:
    void newChannel(const QString& channel);
    void removedChannel(const QString& channel);

private:
    QCopX11Server *server;

    friend class QCopChannel;
};

#endif // Q_WS_X11

#endif
