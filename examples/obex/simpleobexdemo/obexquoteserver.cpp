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

#include "obexquoteserver.h"

#include <QObexServerSession>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QDateTime>

static QList<QByteArray> getDilbertQuotes()
{
    QList<QByteArray> quotes;
    quotes << "I love deadlines. I especially like the whooshing sound they make as they go flying by."
            << "I can only please one person per day. Today is not your day. Tomorrow is not looking good either."
            << "On the keyboard of life, always keep one finger on the escape key."
            << "Never argue with idiots. They drag you down to their level, and then beat you with experience."
            << "A pat on the back is only a few inches from a kick in the butt."
            << "Don't be irreplaceable. If you can't be replaced, you can't be promoted."
            << "Eat one live toad the first thing in the morning and nothing worse will happen to you the rest of the day."
            << "If it wasn't for the last minute, nothing would get done."
            << "When you don't know what to do, walk fast and look worried."
            << "Only the mediocre are at their best all the time."
            << "There's a fine line between genius and insanity. I have erased this line."
            << "If at first you don't succeed... skydiving isn't for you."
            << "At work, the authority of a person is inversely proportional to the number of pens that person is carrying.";
    return quotes;
}
static const QList<QByteArray> DILBERT_QUOTES = getDilbertQuotes();


class ObexQuoteServerSession : public QObexServerSession
{
    Q_OBJECT
public:
    ObexQuoteServerSession(QIODevice *device, QObject *parent = 0);

protected slots:
    QObex::ResponseCode connect(const QObexHeader &header);
    QObex::ResponseCode disconnect(const QObexHeader &header);
    QObex::ResponseCode get(const QObexHeader &header);

protected:
    virtual QObex::ResponseCode provideData(const char **data, qint64 *size);

private:
    int m_nextQuoteIndex;
    bool m_sentNextQuote;
};


ObexQuoteServerSession::ObexQuoteServerSession(QIODevice *device, QObject *parent)
    : QObexServerSession(device, parent)
{
    qsrand(QDateTime::currentDateTime().toTime_t());
}

QObex::ResponseCode ObexQuoteServerSession::connect(const QObexHeader &header)
{
    Q_UNUSED(header);
    return QObex::Success;      // accept the CONNECT request
}

QObex::ResponseCode ObexQuoteServerSession::disconnect(const QObexHeader &header)
{
    Q_UNUSED(header);
    return QObex::Success;      // accept the DISCONNECT request
}

QObex::ResponseCode ObexQuoteServerSession::get(const QObexHeader &header)
{
    Q_UNUSED(header);

    m_nextQuoteIndex = qrand() % DILBERT_QUOTES.size();
    m_sentNextQuote = false;

    return QObex::Success;      // accept the PUT request
}

QObex::ResponseCode ObexQuoteServerSession::provideData(const char **data, qint64 *size)
{
    if (m_sentNextQuote) {
        *size = 0;  // indicate there is no more data to be sent
    } else {
        const QByteArray &quote = DILBERT_QUOTES[m_nextQuoteIndex];
        *data = quote.constData();
        *size = quote.size();
        m_sentNextQuote = true;
    }
    return QObex::Success;
}


// ====================================================================

ObexQuoteServer::ObexQuoteServer(QObject *parent)
    : QObject(parent),
      m_tcpServer(new QTcpServer(this))
{
    connect(m_tcpServer, SIGNAL(newConnection()),
            this, SLOT(newConnection()));
}

bool ObexQuoteServer::run()
{
    return m_tcpServer->listen();
}

QHostAddress ObexQuoteServer::serverAddress() const
{
    return m_tcpServer->serverAddress();
}

quint16 ObexQuoteServer::serverPort()
{
    return m_tcpServer->serverPort();
}

void ObexQuoteServer::newConnection()
{
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    ObexQuoteServerSession *obexServer = new ObexQuoteServerSession(socket);
    connect(socket, SIGNAL(disconnected()),
            obexServer, SLOT(deleteLater()));
}

#include "obexquoteserver.moc"
