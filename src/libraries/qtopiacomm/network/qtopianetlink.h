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

#ifndef QTOPIANETLINK_H
#define QTOPIANETLINK_H

#include <QObject>

#include <qtopiaglobal.h>

class QtopiaNetlinkPrivate;
class QTOPIACOMM_EXPORT QtopiaNetlink : public QObject
{
    Q_OBJECT
public:
    enum Protocol
    {
        Route = 0x0001,      //see rtnetlink(7), rtnetlink(3)
        KernelObjectEvent = 0x0002
    };

    Q_DECLARE_FLAGS(Protocols, Protocol);

    enum RouteNotification
    {
        NewAddress = 0,
        DelAddress = 1,

        NewRoute = 99,
        DelRoute = 100,
    };


    QtopiaNetlink( QtopiaNetlink::Protocols protocol, QObject* parent = 0 );
    ~QtopiaNetlink();

    static bool supports( Protocol protocol );
signals:
    void routeNotification( QtopiaNetlink::RouteNotification msg );
    void kernelUEventNotification( const QByteArray& message );

private:
    Q_PRIVATE_SLOT( d, void _q_readUEventMessage(int) );
private:
    QtopiaNetlinkPrivate* d;
};

#endif
