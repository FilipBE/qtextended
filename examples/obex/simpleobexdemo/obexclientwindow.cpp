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

#include "obexclientwindow.h"

#include <QObexClientSession>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

ObexClientWindow::ObexClientWindow(const QHostAddress &serverAddress, quint16 serverPort, QWidget *parent, Qt::WFlags f)
    : QMainWindow(parent, f),
      m_socket(new QTcpSocket(this)),
      m_obexClient(0)
{
    setupUi();

    connect(m_socket, SIGNAL(connected()), SLOT(socketConnected()));
    m_socket->connectToHost(serverAddress, serverPort);
}

void ObexClientWindow::socketConnected()
{
    m_obexClient = new QObexClientSession(m_socket, this);
    connect(m_obexClient, SIGNAL(readyRead()), SLOT(socketReadyRead()));

    m_obexClient->connect();
}

void ObexClientWindow::buttonClicked()
{
    m_obexClient->get(QObexHeader());
}

void ObexClientWindow::socketReadyRead()
{
    m_textEdit->setText(m_obexClient->readAll());
}

void ObexClientWindow::setupUi()
{
    m_textEdit = new QTextEdit;
    QPushButton *button = new QPushButton(tr("Get a Dilbert quote!"));
    connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));

    QWidget *mainWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_textEdit);
    layout->addWidget(button);
    mainWidget->setLayout(layout);
    setCentralWidget(mainWidget);
}

#include "obexclientwindow.moc"
