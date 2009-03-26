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

#ifndef OBEXQUOTESERVER_H
#define OBEXQUOTESERVER_H

#include <QHostAddress>

class QTcpServer;

class ObexQuoteServer : public QObject
{
    Q_OBJECT
public:
    ObexQuoteServer(QObject *parent = 0);

    bool run();
    QHostAddress serverAddress() const;
    quint16 serverPort();

private slots:
    void newConnection();

private:
    QTcpServer *m_tcpServer;
};

#endif
