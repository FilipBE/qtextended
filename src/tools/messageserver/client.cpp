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


#include "client.h"

#include <QMailMessageServer>

Client::Client(QObject* parent)
    : QObject(parent)
{
}

Client::~Client()
{
}

void Client::setAccount(const QMailAccountId &)
{
}

QMailAccountId Client::account() const
{
    return QMailAccountId();
}

void Client::foldersOnly(bool)
{
}

void Client::headersOnly(bool, int)
{
}

void Client::newConnection()
{
}

void Client::closeConnection()
{
}

void Client::setSelectedMails(const SelectionMap&)
{
}

bool Client::hasDeleteImmediately() const
{
    return false;
}

void Client::deleteImmediately(const QString&)
{
}

void Client::checkForNewMessages()
{
}

void Client::resetNewMailCount()
{
}

void Client::cancelTransfer()
{
}

TransmittingClient::TransmittingClient(QObject* parent)
    : Client(parent)
{
}

int TransmittingClient::addMail(const QMailMessage &)
{
    return QMailMessageServer::ErrNoError;
}

