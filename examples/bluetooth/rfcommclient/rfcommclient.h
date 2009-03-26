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

#ifndef RFCOMMCLIENT_H
#define RFCOMMCLIENT_H

#include <QMainWindow>

class QWaitWidget;
class QAction;
class QBluetoothRfcommSocket;
class QTextEdit;
class QLineEdit;

class RfcommClient : public QMainWindow
{
    Q_OBJECT

public:
    RfcommClient(QWidget *parent = 0, Qt::WFlags f = 0);
    ~RfcommClient();

public slots:
    void cancelConnect();

private slots:
    void connectSocket();
    void disconnectSocket();

    void socketConnected();
    void socketDisconnected();

    void connectFailed();

    void serverReplied();
    void newUserText();

private:
    QTextEdit *logArea;
    QLineEdit *userEntry;
    QWaitWidget *waiter;
    bool canceled;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *sendAction;
    QBluetoothRfcommSocket *socket;
};

#endif
