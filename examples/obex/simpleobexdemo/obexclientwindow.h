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

#ifndef OBEXCLIENTWINDOW_H
#define OBEXCLIENTWINDOW_H

#include <QMainWindow>

class QObexClientSession;
class QTcpSocket;
class QTextEdit;
class QHostAddress;

class ObexClientWindow : public QMainWindow
{
    Q_OBJECT
public:
    ObexClientWindow(const QHostAddress &serverAddress,
                     quint16 serverPort,
                     QWidget *parent = 0,
                     Qt::WFlags f = 0);

private slots:
    void socketConnected();
    void buttonClicked();
    void socketReadyRead();

private:
    QTcpSocket *m_socket;
    QObexClientSession *m_obexClient;
    QTextEdit *m_textEdit;

    void setupUi();
};

#endif
